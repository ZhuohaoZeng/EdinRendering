#pragma once
#include "vec3.h"
#include <cmath>
class mat3 
{
public:
    double m[3][3];

    // Constructors
    mat3() : m{{0,0,0},{0,0,0},{0,0,0}} {}
    mat3(double m00, double m01, double m02,
         double m10, double m11, double m12,
         double m20, double m21, double m22)
        : m{{m00,m01,m02},{m10,m11,m12},{m20,m21,m22}} {}
    
    static mat3 identity() {
        return mat3(1,0,0, 0,1,0, 0,0,1);
    }

    // Access
    const double* operator[](int i) const { return m[i]; }
    double*       operator[](int i) { return m[i]; }

    mat3 operator+(const mat3& n) const {
        return mat3(
            m[0][0]+n.m[0][0], m[0][1]+n.m[0][1], m[0][2]+n.m[0][2],
            m[1][0]+n.m[1][0], m[1][1]+n.m[1][1], m[1][2]+n.m[1][2],
            m[2][0]+n.m[2][0], m[2][1]+n.m[2][1], m[2][2]+n.m[2][2]
        );
    }

    mat3 operator-(const mat3& n) const {
        return mat3(
            m[0][0]-n.m[0][0], m[0][1]-n.m[0][1], m[0][2]-n.m[0][2],
            m[1][0]-n.m[1][0], m[1][1]-n.m[1][1], m[1][2]-n.m[1][2],
            m[2][0]-n.m[2][0], m[2][1]-n.m[2][1], m[2][2]-n.m[2][2]
        );
    }

    mat3 operator*(double t) const {
        return mat3(
            m[0][0]*t, m[0][1]*t, m[0][2]*t,
            m[1][0]*t, m[1][1]*t, m[1][2]*t,
            m[2][0]*t, m[2][1]*t, m[2][2]*t
        );
    }

    mat3 operator/(double t) const {
        return *this * (1.0/t);
    }

    // Matrix-vector multiplication
    vec3 operator*(const vec3& v) const {
        return vec3(
            m[0][0]*v.x() + m[0][1]*v.y() + m[0][2]*v.z(),
            m[1][0]*v.x() + m[1][1]*v.y() + m[1][2]*v.z(),
            m[2][0]*v.x() + m[2][1]*v.y() + m[2][2]*v.z()
        );
    }

    // Matrix-matrix multiplication
    mat3 operator*(const mat3& n) const {
        mat3 r;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                r.m[i][j] = m[i][0]*n.m[0][j] + m[i][1]*n.m[1][j] + m[i][2]*n.m[2][j];
        return r;
    }
    // Transpose
    mat3 transpose() const {
        return mat3(
            m[0][0], m[1][0], m[2][0],
            m[0][1], m[1][1], m[2][1],
            m[0][2], m[1][2], m[2][2]
        );
    }

    // Determinant
    double determinant() const {
        return
            m[0][0]*(m[1][1]*m[2][2] - m[1][2]*m[2][1]) -
            m[0][1]*(m[1][0]*m[2][2] - m[1][2]*m[2][0]) +
            m[0][2]*(m[1][0]*m[2][1] - m[1][1]*m[2][0]);
    }

    // Inverse
    mat3 inverse() const {
        double det = determinant();
        if (std::fabs(det) < 1e-12)
            return mat3(); // return zero matrix if singular

        double invDet = 1.0 / det;
        mat3 inv;
        inv.m[0][0] =  (m[1][1]*m[2][2] - m[1][2]*m[2][1]) * invDet;
        inv.m[0][1] = -(m[0][1]*m[2][2] - m[0][2]*m[2][1]) * invDet;
        inv.m[0][2] =  (m[0][1]*m[1][2] - m[0][2]*m[1][1]) * invDet;

        inv.m[1][0] = -(m[1][0]*m[2][2] - m[1][2]*m[2][0]) * invDet;
        inv.m[1][1] =  (m[0][0]*m[2][2] - m[0][2]*m[2][0]) * invDet;
        inv.m[1][2] = -(m[0][0]*m[1][2] - m[0][2]*m[1][0]) * invDet;

        inv.m[2][0] =  (m[1][0]*m[2][1] - m[1][1]*m[2][0]) * invDet;
        inv.m[2][1] = -(m[0][0]*m[2][1] - m[0][1]*m[2][0]) * invDet;
        inv.m[2][2] =  (m[0][0]*m[1][1] - m[0][1]*m[1][0]) * invDet;

        return inv;
    }

    // Inverse-transpose (useful for transforming normals)
    mat3 inverse_transpose() const {
        return inverse().transpose();
    }
};

inline mat3 operator*(double t, const mat3& m) { return m * t; }
inline mat3 eulerXYZ_to_mat3(const vec3& euler_xyz_rad)
{
    double cx = std::cos(euler_xyz_rad.x());
    double sx = std::sin(euler_xyz_rad.x());
    double cy = std::cos(euler_xyz_rad.y());
    double sy = std::sin(euler_xyz_rad.y());
    double cz = std::cos(euler_xyz_rad.z());
    double sz = std::sin(euler_xyz_rad.z());

    mat3 R;
    R.m[0][0] = cz*cy;              R.m[1][0] = sz*cy;              R.m[2][0] = -sy;
    // Column 1
    R.m[0][1] = cz*sy*sx - sz*cx;   R.m[1][1] = sz*sy*sx + cz*cx;   R.m[2][1] = cy*sx;
    // Column 2
    R.m[0][2] = cz*sy*cx + sz*sx;   R.m[1][2] = sz*sy*cx - cz*sx;   R.m[2][2] = cy*cx;
    return R;
}