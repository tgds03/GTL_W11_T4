#pragma once
#include <algorithm>
#include <utility>
#include <vector>

#include "ContainerAllocator.h"
#include "Serialization/Archive.h"


template <typename T, typename Allocator = FDefaultAllocator<T>>
class TArray
{
public:
    using SizeType = typename Allocator::SizeType;
    using ElementType = T;
    using ArrayType = std::vector<ElementType, Allocator>;

private:
    ArrayType ContainerPrivate;

public:
    // Iterator를 사용하기 위함
    auto begin() noexcept { return ContainerPrivate.begin(); }
    auto end() noexcept { return ContainerPrivate.end(); }
    auto begin() const noexcept { return ContainerPrivate.begin(); }
    auto end() const noexcept { return ContainerPrivate.end(); }
    auto rbegin() noexcept { return ContainerPrivate.rbegin(); }
    auto rend() noexcept { return ContainerPrivate.rend(); }
    auto rbegin() const noexcept { return ContainerPrivate.rbegin(); }
    auto rend() const noexcept { return ContainerPrivate.rend(); }

    T& operator[](SizeType Index);
    const T& operator[](SizeType Index) const;
    void operator+(const TArray& OtherArray);

public:
    ArrayType& GetContainerPrivate() { return ContainerPrivate; }
    const ArrayType& GetContainerPrivate() const { return ContainerPrivate; }

    TArray();
    ~TArray() = default;

    // 이니셜라이저 생성자
    TArray(std::initializer_list<T> InitList);

    // 복사 생성자
    TArray(const TArray& Other);

    // 이동 생성자
    TArray(TArray&& Other) noexcept;

    // 복사 할당 연산자
    TArray& operator=(const TArray& Other);

    // 이동 할당 연산자
    TArray& operator=(TArray&& Other) noexcept;

    /** Element를 Number개 만큼 초기화 합니다. */
    void Init(const T& Element, SizeType Number);
    SizeType Add(const T& Item);
    SizeType Add(T&& Item);
    SizeType AddUnique(const T& Item);

    template <typename... Args>
    SizeType Emplace(Args&&... Item);

    /** Array가 비어있는지 확인합니다. */
    bool IsEmpty() const;

    /** Array를 비웁니다 */
    void Empty();

    /** Item과 일치하는 모든 요소를 제거합니다. */
    SizeType Remove(const T& Item);

    /** 왼쪽부터 Item과 일치하는 요소를 1개 제거합니다. */
    bool RemoveSingle(const T& Item);

    /** 특정 위치에 있는 요소를 제거합니다. */
    void RemoveAt(SizeType Index);

    void RemoveAt(SizeType Index, SizeType Count = 1, bool bAllowShrinking = true);

    /** Predicate에 부합하는 모든 요소를 제거합니다. */
    template <typename Predicate>
        requires std::is_invocable_r_v<bool, Predicate, const T&>
    SizeType RemoveAll(const Predicate& Pred);

    T* GetData();
    const T* GetData() const;

    /**
     * Array에서 Item을 찾습니다.
     * @param Item 찾으려는 Item
     * @return Item의 인덱스, 찾을 수 없다면 -1
     */
    SizeType Find(const T& Item);
    bool Find(const T& Item, SizeType& Index);

    /**
     * Finds an item by predicate.
     *
     * @param Pred The predicate to match.
     * @returns Index to the first matching element, or INDEX_NONE if none is found.
     */
    template <typename Predicate>
    SizeType IndexOfByPredicate(Predicate Pred) const
    {
        /* 기존 언리얼 코드
        const ElementType* Start = GetData();
        for (const ElementType* Data = Start, DataEnd = (Start + Num()); Data != DataEnd; ++Data)
        {
            if (std::invoke(Pred, *Data))
            {
                return static_cast<SizeType>(Data - Start);
            }
        }
        */
        for (int32 i = 0; i < Num(); ++i)
        {
            const ElementType* Data = &ContainerPrivate[i];
            if (std::invoke(Pred, *Data))
            {
                return static_cast<SizeType>(i);
            }
        }
        return INDEX_NONE;
    }

    /** 요소가 존재하는지 확인합니다. */
    bool Contains(const T& Item) const;

    /** Array Size를 가져옵니다. */
    SizeType Num() const;

    /** Array의 Capacity를 가져옵니다. */
    SizeType Len() const;

    /** Array의 Size를 Number로 설정합니다. */
    void SetNum(SizeType Number);

    /** Array의 Capacity를 Number로 설정합니다. */
    void Reserve(SizeType Number);

    /** Count만큼 초기화되지 않은 공간을 확장합니다. */
    SizeType AddUninitialized(SizeType Count);

    void Sort();
    template <typename Compare>
        requires std::is_invocable_r_v<bool, Compare, const T&, const T&>
    void Sort(const Compare& CompFn);

    bool IsValidIndex(uint32 ElementIndex) const {
        if (ElementIndex < 0 || ElementIndex >= Len()) return false;

        return true;
    }

    ElementType Pop();

    //void SetNumZeroed(SizeType NewNum, bool bAllowShrinking);

    /**
     * Resizes array to given number of elements, optionally shrinking it.
     * New elements will be zeroed.
     *
     * @param NewNum New size of the array.
     * @param bAllowShrinking Tell if this function can shrink the memory in-use if suitable.
     */
    void SetNumZeroed(SizeType NewNum, bool bAllowShrinking = true)
    {
        if (NewNum > Num())
        {
            AddZeroed(NewNum - Num());
        }
        else if (NewNum < 0)
        {
            OnInvalidNum((USizeType)NewNum);
        }
        else if (NewNum < Num())
        {
            RemoveAt(NewNum, Num() - NewNum, bAllowShrinking);
        }
    }

    /**
     * Adds new items to the end of the array, possibly reallocating the whole
     * array to fit. The new items will be zeroed.
     *
     * Caution, AddZeroed() will create elements without calling the
     * constructor and this is not appropriate for element types that require
     * a constructor to function properly.
     *
     * @param  Count  The number of new items to add.
     * @return Index to the first of the new items.
     * @see Add, AddDefaulted, AddUnique, Append, Insert
     */
    SizeType AddZeroed()
    {
        const SizeType Index = AddUninitialized();
        FPlatformMemory::Memzero((uint8*)AllocatorInstance.GetAllocation() + Index * sizeof(ElementType), sizeof(ElementType));
        return Index;
    }
    SizeType AddZeroed(SizeType Count)
    {
        const SizeType Index = AddUninitialized(Count);
        FPlatformMemory::Memzero((uint8*)AllocatorInstance.GetAllocation() + Index * sizeof(ElementType), Count * sizeof(ElementType));
        return Index;
    }
};


template <typename T, typename Allocator>
T& TArray<T, Allocator>::operator[](SizeType Index)
{
    return ContainerPrivate[Index];
}

template <typename T, typename Allocator>
const T& TArray<T, Allocator>::operator[](SizeType Index) const
{
    return ContainerPrivate[Index];
}

template <typename T, typename Allocator>
void TArray<T, Allocator>::operator+(const TArray& OtherArray)
{
    ContainerPrivate.insert(end(), OtherArray.begin(), OtherArray.end());
}

template <typename T, typename Allocator>
TArray<T, Allocator>::TArray()
    : ContainerPrivate()
{
}

template <typename T, typename Allocator>
TArray<T, Allocator>::TArray(std::initializer_list<T> InitList)
    : ContainerPrivate(InitList)
{
}

template <typename T, typename Allocator>
TArray<T, Allocator>::TArray(const TArray& Other)
    : ContainerPrivate(Other.ContainerPrivate)
{
}

template <typename T, typename Allocator>
TArray<T, Allocator>::TArray(TArray&& Other) noexcept
    : ContainerPrivate(std::move(Other.ContainerPrivate))
{
}

template <typename T, typename Allocator>
TArray<T, Allocator>& TArray<T, Allocator>::operator=(const TArray& Other)
{
    if (this != &Other)
    {
        ContainerPrivate = Other.ContainerPrivate;
    }
    return *this;
}

template <typename T, typename Allocator>
TArray<T, Allocator>& TArray<T, Allocator>::operator=(TArray&& Other) noexcept
{
    if (this != &Other)
    {
        ContainerPrivate = std::move(Other.ContainerPrivate);
    }
    return *this;
}

template <typename T, typename Allocator>
void TArray<T, Allocator>::Init(const T& Element, SizeType Number)
{
    ContainerPrivate.assign(Number, Element);
}

template <typename T, typename Allocator>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::Add(const T& Item)
{
    return Emplace(Item);
}

template <typename T, typename Allocator>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::Add(T&& Item)
{
    return Emplace(std::move(Item));
}

template <typename T, typename Allocator>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::AddUnique(const T& Item)
{
    if (SizeType Index; Find(Item, Index))
    {
        return Index;
    }
    return Add(Item);
}

template <typename T, typename Allocator>
template <typename... Args>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::Emplace(Args&&... Item)
{
    ContainerPrivate.emplace_back(std::forward<Args>(Item)...);
    return Num()-1;
}

template <typename T, typename Allocator>
bool TArray<T, Allocator>::IsEmpty() const
{
    return ContainerPrivate.empty();
}

template <typename T, typename Allocator>
void TArray<T, Allocator>::Empty()
{
    ContainerPrivate.clear();
}

template <typename T, typename Allocator>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::Remove(const T& Item)
{
    auto oldSize = ContainerPrivate.size();
    ContainerPrivate.erase(std::remove(ContainerPrivate.begin(), ContainerPrivate.end(), Item), ContainerPrivate.end());
    return static_cast<SizeType>(oldSize - ContainerPrivate.size());
}

template <typename T, typename Allocator>
bool TArray<T, Allocator>::RemoveSingle(const T& Item)
{
    auto it = std::find(ContainerPrivate.begin(), ContainerPrivate.end(), Item);
    if (it != ContainerPrivate.end())
    {
        ContainerPrivate.erase(it);
        return true;
    }
    return false;
}

template <typename T, typename Allocator>
void TArray<T, Allocator>::RemoveAt(SizeType Index)
{
    if (Index >= 0 && static_cast<SizeType>(Index) < ContainerPrivate.size())
    {
        ContainerPrivate.erase(ContainerPrivate.begin() + Index);
    }
}

// 지정된 범위의 요소 제거
template<typename T, typename Allocator>
inline void TArray<T, Allocator>::RemoveAt(SizeType Index, SizeType Count, bool bAllowShrinking)
{
    if (Count <= 0)
    {
        return;
    }

    SizeType CurrentSize = ContainerPrivate.size();
    if (Index >= 0 && static_cast<SizeType>(Index) < CurrentSize && (static_cast<SizeType>(Index) + Count) <= CurrentSize)
    {
        ContainerPrivate.erase(ContainerPrivate.begin() + Index, ContainerPrivate.begin() + Index + Count);

        if (bAllowShrinking)
        {
            ContainerPrivate.shrink_to_fit();
        }
    }
}

template <typename T, typename Allocator>
template <typename Predicate>
    requires std::is_invocable_r_v<bool, Predicate, const T&>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::RemoveAll(const Predicate& Pred)
{
    auto oldSize = ContainerPrivate.size();
    ContainerPrivate.erase(std::remove_if(ContainerPrivate.begin(), ContainerPrivate.end(), Pred), ContainerPrivate.end());
    return static_cast<SizeType>(oldSize - ContainerPrivate.size());
}

template <typename T, typename Allocator>
T* TArray<T, Allocator>::GetData()
{
    return ContainerPrivate.data();
}

template <typename T, typename Allocator>
const T* TArray<T, Allocator>::GetData() const
{
    return ContainerPrivate.data();
}

template <typename T, typename Allocator>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::Find(const T& Item)
{
    const auto it = std::find(ContainerPrivate.begin(), ContainerPrivate.end(), Item);
    return it != ContainerPrivate.end() ? std::distance(ContainerPrivate.begin(), it) : INDEX_NONE;
}

template <typename T, typename Allocator>
bool TArray<T, Allocator>::Find(const T& Item, SizeType& Index)
{
    Index = Find(Item);
    return (Index != INDEX_NONE);
}

template <typename T, typename Allocator>
bool TArray<T, Allocator>::Contains(const T& Item) const
{
    for (const T* Data = GetData(), *DataEnd = Data + Num(); Data != DataEnd; ++Data)
    {
        if (*Data == Item)
        {
            return true;
        }
    }
    return false;
}

template <typename T, typename Allocator>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::Num() const
{
    return ContainerPrivate.size();
}

template <typename T, typename Allocator>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::Len() const
{
    return ContainerPrivate.capacity();
}

template <typename T, typename Allocator>
void TArray<T, Allocator>::SetNum(SizeType Number)
{
    ContainerPrivate.resize(Number);
}

template <typename T, typename Allocator>
void TArray<T, Allocator>::Reserve(SizeType Number)
{
    ContainerPrivate.reserve(Number);
}

template <typename T, typename Allocator>
typename TArray<T, Allocator>::SizeType TArray<T, Allocator>::AddUninitialized(SizeType Count)
{
    if (Count <= 0)
    {
        return ContainerPrivate.size();
    }

    // 기존 크기 저장
    SizeType OldSize = ContainerPrivate.size();

    // 메모리를 확장 (초기화하지 않음)
    ContainerPrivate.resize(OldSize + Count);

    // 새 크기를 반환
    return OldSize;
}

template <typename T, typename Allocator>
void TArray<T, Allocator>::Sort()
{
    std::sort(ContainerPrivate.begin(), ContainerPrivate.end());
}

template <typename T, typename Allocator>
template <typename Compare>
    requires std::is_invocable_r_v<bool, Compare, const T&, const T&>
void TArray<T, Allocator>::Sort(const Compare& CompFn)
{
    std::sort(ContainerPrivate.begin(), ContainerPrivate.end(), CompFn);
}

template <typename T, typename Allocator>
typename TArray<T, Allocator>::ElementType TArray<T, Allocator>::Pop()
{
    ElementType Element = ContainerPrivate.back();
    ContainerPrivate.pop_back();
    return Element;
}

template <typename ElementType, typename Allocator>
FArchive& operator<<(FArchive& Ar, TArray<ElementType, Allocator>& Array)
{
    using SizeType = typename TArray<ElementType, Allocator>::SizeType;

    // 배열 크기 직렬화
    SizeType ArraySize = Array.Num();
    Ar << ArraySize;

    if (Ar.IsLoading())
    {
        // 로드 시 배열 크기 설정
        Array.SetNum(ArraySize);
    }

    // 배열 요소 직렬화
    for (SizeType Index = 0; Index < ArraySize; ++Index)
    {
        Ar << Array[Index];
    }

    return Ar;
}

//template <typename T, typename Allocator>
//void TArray<T, Allocator>::SetNumZeroed(SizeType NewNum, bool bAllowShrinking)
//{
//    if (NewNum < 0)
//    {
//        assert(false && "Invalid size for SetNumZeroed");
//        return;
//    }
//
//    SizeType CurrentNum = Num();
//
//    if (NewNum > CurrentNum)
//    {
//        // 배열 확장
//        // std::vector::resize는 새로 추가된 요소를 값 초기화합니다.
//        // 기본 타입(int, float)은 0으로, 클래스는 기본 생성자 호출.
//        // 만약 클래스 T가 기본 생성자에서 멤버를 0으로 초기화하지 않는다면,
//        // 수동으로 0으로 채워주는 과정이 필요할 수 있습니다.
//        ContainerPrivate.resize(NewNum);
//
//        // 만약 T가 기본 타입이 아니고, 기본 생성자가 0으로 초기화하지 않는다면,
//        // 새로 추가된 부분만 명시적으로 0으로 초기화하는 로직이 필요합니다.
//        // 예를 들어, T가 복잡한 구조체이고 모든 멤버를 0으로 만들고 싶다면:
//        if constexpr (!std::is_fundamental_v<T> && !std::is_pointer_v<T>)
//        {
//            for (SizeType i = CurrentNum; i < NewNum; ++i)
//            {
//                // ContainerPrivate[i] = {}; // 값 초기화 (C++11 이상)
//                // 또는 memset(&ContainerPrivate[i], 0, sizeof(T)); // POD 타입에만 주의해서 사용
//                // 또는 T 타입에 적절한 Clear() 또는 Zero() 멤버 함수가 있다면 호출
//            }
//        }
//    }
//    else if (NewNum < CurrentNum)
//    {
//        // 배열 축소
//        ContainerPrivate.resize(NewNum); // 요소 제거
//        if (bAllowShrinking)
//        {
//            // std::vector는 resize만으로는 capacity를 줄이지 않을 수 있음.
//            // 명시적으로 shrink_to_fit() 호출 (C++11 이상)
//            ContainerPrivate.shrink_to_fit();
//        }
//    }
//    else 
//        NewNum == CurrentNum;
//}
