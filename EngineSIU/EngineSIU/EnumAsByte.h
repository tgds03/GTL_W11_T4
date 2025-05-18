#pragma once

#include <cstdint>
#include <type_traits> 

// (선택적) 만약 C++17 미만이고, enum class인지 일반 enum인지 구분하고 싶다면,
// 간단한 타입 특성 템플릿을 여기에 추가할 수 있습니다.
// 예:
// template <typename T>
// struct MyIsEnumClass : std::false_type {}; // 기본은 false
// template <typename T> requires std::is_enum_v<T> // C++20 concepts
// struct MyIsEnumClass<T> : std::bool_constant<!std::is_convertible_v<T, std::underlying_type_t<T>>> {};

template <typename EnumType>
class TEnumAsByte
{
    static_assert(std::is_enum_v<EnumType>, "TEnumAsByte can only be used with enum types.");

public:
    using UnderlyingType = uint8_t;

    TEnumAsByte() = default;

    TEnumAsByte(const TEnumAsByte&) = default;
    TEnumAsByte& operator=(const TEnumAsByte&) = default;

    TEnumAsByte(TEnumAsByte&&) noexcept = default;
    TEnumAsByte& operator=(TEnumAsByte&&) noexcept = default;

    constexpr TEnumAsByte(EnumType InValue) noexcept // constexpr 및 noexcept 추가 가능
        : Value(static_cast<UnderlyingType>(InValue))
    {
        assert(static_cast<UnderlyingType>(InValue) >= 0 && static_cast<UnderlyingType>(InValue) <= 255);
    }

    /**
     * 정수 값으로 초기화하는 명시적 생성자 (주의해서 사용).
     * 주로 직렬화나 낮은 수준의 데이터 처리 시 사용될 수 있습니다.
     */
    explicit constexpr TEnumAsByte(int InValue) noexcept
        : Value(static_cast<UnderlyingType>(InValue))
    {
        assert(InValue >= 0 && InValue <= 255);
    }

    explicit constexpr TEnumAsByte(UnderlyingType InValue) noexcept
        : Value(InValue)
    {}

    /**
     * TEnumAsByte를 원래 열거형 타입으로 변환 (암시적 변환).
     */
    constexpr operator EnumType() const noexcept
    {
        return static_cast<EnumType>(Value);
    }

    /**
     * 저장된 바이트 값을 직접 얻는 함수 (명시적).
     */
    constexpr UnderlyingType GetByteValue() const noexcept
    {
        return Value;
    }

    /**
     * 저장된 값을 원래 열거형 타입으로 얻는 함수 (명시적, operator EnumType()과 유사).
     */
    constexpr EnumType GetEnumValue() const noexcept
    {
        return static_cast<EnumType>(Value);
    }

    // 비교 연산자들
    constexpr bool operator==(TEnumAsByte Other) const noexcept
    {
        return Value == Other.Value;
    }

    constexpr bool operator!=(TEnumAsByte Other) const noexcept
    {
        return Value != Other.Value;
    }

    constexpr bool operator==(EnumType EnumVal) const noexcept
    {
        return Value == static_cast<UnderlyingType>(EnumVal);
    }

    constexpr bool operator!=(EnumType EnumVal) const noexcept
    {
        return Value != static_cast<UnderlyingType>(EnumVal);
    }

    // 다른 비교 연산자들 (<, >, <=, >=)도 필요에 따라 추가 가능
    // 예:
    // constexpr bool operator<(TEnumAsByte Other) const noexcept
    // {
    //     return Value < Other.Value;
    // }

private:
    UnderlyingType Value{}; // C++11 이상, 값 초기화 (0으로)

    // friend 함수로 GetTypeHash (해시 컨테이너 사용 시 필요)
    // (만약 GetTypeHash를 전역 템플릿 함수로 만들고 싶지 않다면,
    //  std::hash<TEnumAsByte<EnumType>> 특수화를 직접 제공하는 방식도 있음)
    template <typename T>
    friend struct std::hash; // std::hash가 private 멤버에 접근할 수 있도록
};

// TEnumAsByte가 POD 타입처럼 취급될 수 있음을 명시 (선택적, 특정 컨테이너 최적화에 영향 줄 수 있음)
// 만약 직접 만든 IsPODType 템플릿이 있다면 그것을 사용.
// 여기서는 표준 타입 특성을 사용하는 것으로 가정.
// TEnumAsByte는 기본적으로 trivially_copyable하고 standard_layout일 가능성이 높음.
// template <typename EnumType>
// struct MyIsPODType<TEnumAsByte<EnumType>> { static constexpr bool Value = true; };


// std::hash에 대한 특수화 (TEnumAsByte를 std::unordered_map 등의 키로 사용하기 위함)
namespace std
{
    template <typename EnumType>
    struct hash<TEnumAsByte<EnumType>>
    {
        size_t operator()(const TEnumAsByte<EnumType>& Key) const noexcept
        {
            // 내부 바이트 값을 해싱
            return std::hash<typename TEnumAsByte<EnumType>::UnderlyingType>()(Key.GetByteValue());
            // 또는 private 멤버 직접 접근 (friend 선언 시):
            // return std::hash<typename TEnumAsByte<EnumType>::UnderlyingType>()(Key.Value);
        }
    };
}
