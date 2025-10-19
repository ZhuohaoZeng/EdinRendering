#pragma once
#include <ostream>

//General vectors
template<int n> struct vec {
    double data[n]{0};
    double& operator[](const int i)       {assert(i >= 0 && i <= n); return data[i];}
    double  operator[](const int i) const {assert(i >= 0 && i <= n); return data[i];}
};

template<int n> double norm(const vec<n>& v) 
{
    return std::sqrt(v * v);
};

template<int n> vec<n> normalize(const vec<n>& v)
{
    return v / norm(v);
};

template<int n> double operator*(const vec<n>& lhs, const vec<n>& rhs) 
{
    double ret{};
    for (int i = 0; i < n; ++i) ret += lhs[i] * rhs[i];
    return ret;

};

template<int n> vec<n> operator+(const vec<n>& lhs, const vec<n>& rhs)
{
    vec<n> ret{lhs};
    for (int i = 0; i < n; ++i) ret[i] += rhs[i];
    return ret;
};

template<int n> vec<n> operator-(const vec<n>& lhs, const vec<n>& rhs)
{
    vec<n> ret{lhs};
    for (int i = 0; i < n; ++i) ret[i] -= rhs[i];
    return ret;
};

template<int n> vec<n> operator*(const vec<n>& lhs, const double& rhs) {
    vec<n> ret = lhs;
    for (int i = 0; i < n; ++i) ret[i]*=rhs;
    return ret;
};

template<int n> vec<n> operator*(const double& lhs, const vec<n>& rhs) {
    return rhs * lhs;
};

template<int n> vec<n> operator/(const vec<n>& lhs, const double& rhs) {
    vec<n> ret = lhs;
    for (int i = 0; i < n; ++i) ret[i]/=rhs;
    return ret;
};
 

template<int n> std::ostream& operator<<(std::ostream& out, const vec<n>& v) {
    for (int i = 0; i < n; ++i) out << v[i];
    return out;
};
