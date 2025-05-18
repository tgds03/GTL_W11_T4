// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Math/MathUtility.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "CoreUObject/UObject/NameTypes.h"
#include "Windows/WindowsPlatformTime.h"

/**
 * Implements a thread-safe SRand based RNG.
 *
 * Very bad quality in the lower bits. Don't use the modulus (%) operator.
 */
struct FRandomStream
{
    friend struct Z_Construct_UScriptStruct_FRandomStream_Statics;

public:

    /**
     * Default constructor.
     *
     * The seed should be set prior to use.
     */
    FRandomStream()
        : InitialSeed(0)
        , Seed(0)
    {
    }

    /**
     * Creates and initializes a new random stream from the specified seed value.
     *
     * @param InSeed The seed value.
     */
    FRandomStream(int32 InSeed)
    {
        Initialize(InSeed);
    }

    /**
     * Creates and initializes a new random stream from the specified name.
     *
     * @note If NAME_None is provided, the stream will be seeded using the current time.
     * @param InName The name value from which the stream will be initialized.
     */
    FRandomStream(FName InName)
    {
        Initialize(InName);
    }

public:

    /**
     * Initializes this random stream with the specified seed value.
     *
     * @param InSeed The seed value.
     */
    void Initialize(int32 InSeed)
    {
        InitialSeed = InSeed;
        Seed = uint32(InSeed);
    }

    /**
     * Initializes this random stream using the specified name.
     *
     * @note If NAME_None is provided, the stream will be seeded using the current time.
     * @param InName The name value from which the stream will be initialized.
     */
    void Initialize(FName InName)
    {
        if (InName != NAME_None)
        {
            std::hash<FName> FNameHasher;
            size_t HashCode = FNameHasher(InName);
            InitialSeed = static_cast<int32>(HashCode);
        }
        else
        {
            InitialSeed = FPlatformTime::Cycles64();
        }

        Seed = uint32(InitialSeed);
    }

    /**
     * Resets this random stream to the initial seed value.
     */
    void Reset() const
    {
        Seed = uint32(InitialSeed);
    }

    int32 GetInitialSeed() const
    {
        return InitialSeed;
    }

    /**
     * Generates a new random seed.
     */
    void GenerateNewSeed()
    {
        Initialize(rand());
    }

    /**
     * Returns a random float number in the range [0, 1).
     *
     * @return Random number.
     */
    float GetFraction() const
    {
        MutateSeed();

        float Result;

        *(uint32*)&Result = 0x3F800000U | (Seed >> 9);

        return Result - 1.0f;
    }

    /**
     * Returns a random number between 0 and MAXUINT.
     *
     * @return Random number.
     */
    uint32 GetUnsignedInt() const
    {
        MutateSeed();

        return Seed;
    }

    /**
     * Gets the current seed.
     *
     * @return Current seed.
     */
    int32 GetCurrentSeed() const
    {
        return int32(Seed);
    }

    /**
     * Mirrors the random number API in FMath
     *
     * @return Random number.
     */
    FORCEINLINE float FRand() const
    {
        return GetFraction();
    }

    /**
     * Helper function for rand implementations.
     *
     * @return A random number >= Min and <= Max
     */
    FORCEINLINE int32 RandRange(int32 Min, int32 Max) const
    {
        const int32 Range = (Max - Min) + 1;

        return Min + FMath::RandHelper(Range);
    }

    /**
     * Get a textual representation of the RandomStream.
     *
     * @return Text describing the RandomStream.
     */
    FString ToString() const
    {
        return FString::Printf(TEXT("FRandomStream(InitialSeed=%i, Seed=%u)"), InitialSeed, Seed);
    }

protected:

    /**
     * Mutates the current seed into the next seed.
     */
    void MutateSeed() const
    {
        Seed = (Seed * 196314165U) + 907633515U;
    }

private:

    // Holds the initial seed.
    int32 InitialSeed;

    // Holds the current seed. This should be an uint32 so that any shift to obtain top bits
    // is a logical shift, rather than an arithmetic shift (which smears down the negative bit).
    mutable uint32 Seed;
};
