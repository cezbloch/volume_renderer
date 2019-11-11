#pragma once

#include <ostream>
#include <cmath>


// vec3 class
template<class T>
struct basic_vec3 {
	T x,y,z;

	basic_vec3() {}
	explicit basic_vec3(T v) { x = y = z = v; }
	basic_vec3(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {}
	template<class U> explicit basic_vec3(const basic_vec3<U> &v) : x(v.x), y(v.y), z(v.z) {}
};


// vec3 typedefs

typedef basic_vec3<float> vec3;
typedef basic_vec3<int> vec3i;
typedef basic_vec3<bool> vec3b;
typedef basic_vec3<char> vec3c;
typedef basic_vec3<short int> vec3si;
typedef basic_vec3<unsigned short int> vec3usi;


// vec3 operators

template<class T>
std::ostream &operator<<(std::ostream &s, const basic_vec3<T> &v)
	{ return s << v.x << ' ' << v.y << ' ' << v.z; }

#define DEFINE_VEC3_OPERATOR(op) \
	template<class T> \
	inline basic_vec3<T> operator op(const basic_vec3<T> &l, const basic_vec3<T> &r) { \
		basic_vec3<T> t; \
		t.x = l.x op r.x; \
		t.y = l.y op r.y; \
		t.z = l.z op r.z; \
		return t; \
	} \
	template<class T> \
	inline basic_vec3<T> operator op(T l, const basic_vec3<T> &r) { \
		basic_vec3<T> t; \
		t.x = l op r.x; \
		t.y = l op r.y; \
		t.z = l op r.z; \
		return t; \
	} \
	template<class T> \
	inline basic_vec3<T> operator op(const basic_vec3<T> &l, T r) { \
		basic_vec3<T> t; \
		t.x = l.x op r; \
		t.y = l.y op r; \
		t.z = l.z op r; \
		return t; \
	}

DEFINE_VEC3_OPERATOR(+)
DEFINE_VEC3_OPERATOR(-)
DEFINE_VEC3_OPERATOR(*)
DEFINE_VEC3_OPERATOR(/)

template<class T>
inline basic_vec3<T> operator-(const basic_vec3<T> &v) {
	basic_vec3<T> t;
	t.x = -v.x;
	t.y = -v.y;
	t.z = -v.z;
	return t;
}

template<class T>
inline bool operator==(const basic_vec3<T> &l, const basic_vec3<T> &r)
	{ return l.x == r.x && l.y == r.y && l.z == r.z; }
template<class T>
inline bool operator!=(const basic_vec3<T> &l, const basic_vec3<T> &r)
	{ return !(l == r); }


// vec3 operations

template<class T>
inline float dot(const basic_vec3<T> &l, const basic_vec3<T> &r)
	{ return l.x*r.x + l.y*r.y + l.z*r.z; }

template<class T>
inline basic_vec3<T> cross(const basic_vec3<T> &l, const basic_vec3<T> &r) {
	basic_vec3<T> t;
	t.x = l.y*r.z - l.z*r.y;
	t.y = l.z*r.x - l.x*r.z;
	t.z = l.x*r.y - l.y*r.x;
	return t;
}

template<class T> inline float length2(const basic_vec3<T> &v) { return dot(v,v); }
template<class T> inline float length(const basic_vec3<T> &v) { return std::sqrt(length2(v)); }
template<class T> inline basic_vec3<T> normalize(const basic_vec3<T> &v) { return v / length(v); }