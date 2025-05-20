#pragma once
//#include "MathUtility.h"
#include "Rotator.h"
#include "Serialization/Archive.h"

struct FVector;
struct FMatrix;

// FQuat.cpp

// 쿼터니언
struct FQuat
{
    float W, X, Y, Z;

    static const FQuat Identity;

    // 기본 생성자
    explicit FQuat()
        : W(1.0f), X(0.0f), Y(0.0f), Z(0.0f)
    {}

    // FQuat 생성자 추가: 회전 축과 각도를 받아서 FQuat 생성
    explicit FQuat(const FVector& Axis, float Angle);

    // W, X, Y, Z 값으로 초기화
    explicit FQuat(float InW, float InX, float InY, float InZ)
        : W(InW), X(InX), Y(InY), Z(InZ)
    {}

    explicit FQuat(const FMatrix& InMatrix);

    // 쿼터니언의 곱셈 연산 (회전 결합)
    FQuat operator*(const FQuat& Other) const;
    //void operator*=(const FQuat& Other);

    FQuat operator*(const float Scalar) const;
    
    // (쿼터니언) 벡터 회전
    FVector RotateVector(const FVector& Vec) const;

    // 단위 쿼터니언 여부 확인
    bool IsNormalized() const;

    // 쿼터니언 정규화 (단위 쿼터니언으로 만듬)
    void Normalize();

    FQuat GetUnsafeNormal() const;
    FQuat GetSafeNormal(float Tolerance = SMALL_NUMBER) const;

    // 회전 각도와 축으로부터 쿼터니언 생성 (axis-angle 방식)
    static FQuat FromAxisAngle(const FVector& Axis, float Angle);

    static FQuat CreateRotation(float roll, float pitch, float yaw);

    // 쿼터니언을 회전 행렬로 변환
    FMatrix ToMatrix() const;

    bool Equals(const FQuat& Q, float Tolerance = KINDA_SMALL_NUMBER) const;

    FRotator Rotator() const;

    static FQuat Slerp(const FQuat& A, const FQuat& B, float Alpha);

    float Dot(const FQuat& Other) const
    {
        return W * Other.W + X * Other.X + Y * Other.Y + Z * Other.Z;
    }

    static FQuat FindBetweenNormals(const FVector& A, const FVector& B);

    // 반대 방향의 쿼터니언 생성을 위한 단항 연산자
    FQuat operator-() const
    {
        return FQuat(-W, -X, -Y, -Z);
    }

};

inline FArchive& operator<<(FArchive& Ar, FQuat& Q)
{
    return Ar << Q.X << Q.Y << Q.Z << Q.W;
}
