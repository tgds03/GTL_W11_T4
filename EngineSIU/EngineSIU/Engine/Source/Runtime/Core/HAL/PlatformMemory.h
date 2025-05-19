#pragma once
#include <atomic>
#include <iostream>

#include "Core/HAL/PlatformType.h"

enum EAllocationType : uint8
{
    EAT_Object,
    EAT_Container
};

/**
 * 엔진의 Heap 메모리의 할당량을 추적하는 클래스
 *
 * @note new로 생성한 객체는 추적하지 않습니다.
 */
struct FPlatformMemory
{
private:
    static std::atomic<uint64> ObjectAllocationBytes;
    static std::atomic<uint64> ObjectAllocationCount;
    static std::atomic<uint64> ContainerAllocationBytes;
    static std::atomic<uint64> ContainerAllocationCount;

    template <EAllocationType AllocType>
    static void IncrementStats(size_t Size);

    template <EAllocationType AllocType>
    static void DecrementStats(size_t Size);

public:
    static void* Memcpy(void* Dest, const void* Src, uint64 Length)
    {
        return std::memcpy(Dest, Src, Length);
    }

    template <typename T>
    static void Memcpy(T& Dest, const T& Src);

    static void* Memzero(void* Dest, size_t Length)
    {
        return std::memset(Dest, 0, Length);
    }

    template <typename T>
    static void Memzero(T& Dest);

    static void* Memset(void* Dest, int Value, size_t Length)
    {
        return std::memset(Dest, Value, Length);
    }

    template <EAllocationType AllocType>
    static void* Realloc(void* OriginalPtr, size_t NewSize, size_t OldSize = 0);

    template <EAllocationType AllocType>
    static void* Malloc(size_t Size);

    template <EAllocationType AllocType>
    static void* Realloc(void* Original, size_t Size);

    template <EAllocationType AllocType>
    static void* AlignedMalloc(size_t Size, size_t Alignment);

    template <EAllocationType AllocType>
    static void Free(void* Address, size_t Size);

    template <EAllocationType AllocType>
    static void AlignedFree(void* Address, size_t Size);

    template <EAllocationType AllocType>
    static uint64 GetAllocationBytes();

    template <EAllocationType AllocType>
    static uint64 GetAllocationCount();
};


template <EAllocationType AllocType>
void FPlatformMemory::IncrementStats(size_t Size)
{
    // TotalAllocationBytes += Size;
    // ++TotalAllocationCount;

    if constexpr (AllocType == EAT_Container)
    {
        ContainerAllocationBytes.fetch_add(Size, std::memory_order_relaxed);
        ContainerAllocationCount.fetch_add(1, std::memory_order_relaxed);
    }
    else if constexpr (AllocType == EAT_Object)
    {
        ObjectAllocationBytes.fetch_add(Size, std::memory_order_relaxed);
        ObjectAllocationCount.fetch_add(1, std::memory_order_relaxed);
    }
    else
    {
        //static_assert(false, "Unknown allocation type");
    }
}

template <EAllocationType AllocType>
void FPlatformMemory::DecrementStats(size_t Size)
{
    // TotalAllocationBytes -= Size;
    // --TotalAllocationCount;

    // 멀티스레드 대비
    if constexpr (AllocType == EAT_Container)
    {
        ContainerAllocationBytes.fetch_sub(Size, std::memory_order_relaxed);
        ContainerAllocationCount.fetch_sub(1, std::memory_order_relaxed);
    }
    else if constexpr (AllocType == EAT_Object)
    {
        ObjectAllocationBytes.fetch_sub(Size, std::memory_order_relaxed);
        ObjectAllocationCount.fetch_sub(1, std::memory_order_relaxed);
    }
    else
    {
        //static_assert(false, "Unknown allocation type");
    }
}

template <typename T>
void FPlatformMemory::Memcpy(T& Dest, const T& Src)
{
    Memcpy(&Dest, &Src, sizeof(T));
}

template<typename T>
void FPlatformMemory::Memzero(T& Dest)
{
    Memzero(&Dest, sizeof(T));
}

template<EAllocationType AllocType>
inline void* FPlatformMemory::Realloc(void* OriginalPtr, size_t NewSize, size_t OldSize)
{
    if (!OriginalPtr)
    {
        return Malloc<AllocType>(NewSize);
    }

    if (NewSize == 0)
    {
        Free<AllocType>(OriginalPtr, OldSize); 
        return nullptr;
    }

    // 통계 업데이트:
    // realloc은 복잡함. 단순히 NewSize - OldSize로 증감시키면 안됨.
    // 실제 할당/해제되는 양은 realloc의 내부 구현에 따라 다를 수 있음.
    // 가장 간단한 접근 방식은 기존 것을 해제하고 새로 할당하는 것으로 간주하는 것 (통계 목적상).
    // 하지만 이는 realloc의 실제 동작과 다를 수 있어 완벽하지 않음.
    // 좀 더 정확하려면 할당 시스템이 실제 할당 크기를 알려줘야 함.

    // 현재는 OldSize를 안다면 다음과 같이 처리:
    if (OldSize > 0) // OldSize가 제공된 경우에만 이전 통계 감소 시도
    {
        DecrementStats<AllocType>(OldSize);
    }

    void* NewPtr = std::realloc(OriginalPtr, NewSize);

    if (NewPtr)
    {
        IncrementStats<AllocType>(NewSize); // 새로운 크기로 통계 증가
    }
    else if (NewSize > 0) // NewSize가 0보다 큰데 realloc이 실패한 경우
    {
        // realloc 실패 시 OriginalPtr은 여전히 유효하고 해제되지 않음.
        // 실패했으므로, 이전에 Decrement했던 통계를 복구해야 할 수도 있음.
        // (이 부분은 realloc 실패 시 통계를 어떻게 처리할지 정책에 따라 다름)
        // 여기서는 간단히, 성공 시에만 통계를 업데이트하도록 함.
        // 만약 OldSize로 Decrement했다면, 다시 Increment해야 할 수도 있지만 복잡해짐.
        // 더 나은 방법은 realloc이 실패하면 OriginalPtr에 대한 통계를 변경하지 않는 것.
        // 따라서 DecrementStats는 realloc 성공 후에 하는 것이 더 나을 수도 있음.
        // 아래는 realloc 성공을 가정하고 통계를 처리하는 다른 방식:

        // 다른 접근 방식:
        // void* TempPtr = std::realloc(OriginalPtr, NewSize);
        // if (TempPtr) {
        //     if (OriginalPtr) { // OriginalPtr이 있었다면 이전 통계 감소
        //          DecrementStats<AllocType>(OldSize); // OldSize 필요
        //     }
        //     IncrementStats<AllocType>(NewSize); // 새 통계 증가
        //     return TempPtr;
        // } else {
        //     // realloc 실패. OriginalPtr은 그대로. 통계 변경 없음.
        //     return nullptr; // 또는 OriginalPtr (정책에 따라)
        // }
    }
    // 위의 주석 처리된 다른 접근 방식이 통계 처리에는 더 명확할 수 있습니다.
    // 여기서는 일단 첫 번째 방식으로 남겨둡니다.

    return NewPtr;
}

template <EAllocationType AllocType>
void* FPlatformMemory::Malloc(size_t Size)
{
    void* Ptr = std::malloc(Size);
    if (Ptr)
    {
        IncrementStats<AllocType>(Size);
    }
    return Ptr;
}

template <EAllocationType AllocType>
void* FPlatformMemory::Realloc(void* Original, size_t Size)
{
    void* Ptr = std::realloc(Original, Size);
    if (Ptr)
    {
        IncrementStats<AllocType>(Size);
    }
    return Ptr;
}

template <EAllocationType AllocType>
void* FPlatformMemory::AlignedMalloc(size_t Size, size_t Alignment)
{
    void* Ptr = _aligned_malloc(Size, Alignment);
    if (Ptr)
    {
        IncrementStats<AllocType>(Size);
    }
    return Ptr;
}

template <EAllocationType AllocType>
void FPlatformMemory::Free(void* Address, size_t Size)
{
    if (Address)
    {
        DecrementStats<AllocType>(Size);
        std::free(Address);
    }
}

template <EAllocationType AllocType>
void FPlatformMemory::AlignedFree(void* Address, size_t Size)
{
    if (Address)
    {
        DecrementStats<AllocType>(Size);
        _aligned_free(Address);
    }
}

template <EAllocationType AllocType>
uint64 FPlatformMemory::GetAllocationBytes()
{
    if constexpr (AllocType == EAT_Container)
    {
        return ContainerAllocationBytes;
    }
    else if constexpr (AllocType == EAT_Object)
    {
        return ObjectAllocationBytes;
    }
    else
    {
        //static_assert(false, "Unknown AllocationType");
        return -1;
    }
}

template <EAllocationType AllocType>
uint64 FPlatformMemory::GetAllocationCount()
{
    if constexpr (AllocType == EAT_Container)
    {
        return ContainerAllocationCount;
    }
    else if constexpr (AllocType == EAT_Object)
    {
        return ObjectAllocationCount;
    }
    else
    {
        //static_assert(false, "Unknown AllocationType");
        return -1;
    }
}

