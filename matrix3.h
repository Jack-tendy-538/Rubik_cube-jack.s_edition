// matrix3.h
#ifndef MATRIX3_H
#define MATRIX3_H

#include "vector3.h"

class Matrix3 {
public:
    float m[3][3];  // 3x3矩阵数据

    // 构造函数
    Matrix3();
    Matrix3(const Vector3& row0, const Vector3& row1, const Vector3& row2);
    
    // 单位矩阵
    static Matrix3 identity();
    
    // 旋转矩阵 (绕X/Y/Z轴)
    static Matrix3 rotationX(float angle);
    static Matrix3 rotationY(float angle);
    static Matrix3 rotationZ(float angle);
    
    // 矩阵乘法
    Matrix3 operator*(const Matrix3& other) const;
    Vector3 operator*(const Vector3& vec) const;
    
    // 其他矩阵操作...
};

#endif // MATRIX3_H