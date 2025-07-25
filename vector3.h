// vector3.h
#ifndef VECTOR3_H
#define VECTOR3_H

class Vector3 {
public:
    float x, y, z;  // 3D坐标

    // 构造函数
    Vector3();
    Vector3(float x, float y, float z);
    
    // 向量运算
    Vector3 operator+(const Vector3& other) const;
    Vector3 operator-(const Vector3& other) const;
    Vector3 operator*(float scalar) const;
    
    // 点积和叉积
    float dot(const Vector3& other) const;
    Vector3 cross(const Vector3& other) const;
    
    // 向量归一化
    Vector3 normalized() const;
    
    // 其他向量操作...
};

#endif // VECTOR3_H