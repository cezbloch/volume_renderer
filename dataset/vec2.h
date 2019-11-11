#pragma once

#include <ostream>
#include <cmath>


// vec2 class
template<class T>
struct basic_vec2 {
	T x,y;

	basic_vec2() {}
	explicit basic_vec2(T v) { x = y = v; }
	basic_vec2(T x_, T y_) : x(x_), y(y_) {}
	template<class U> explicit basic_vec2(const basic_vec2<U> &v) : x(v.x), y(v.y) {}
};


// vec2 typedefs

typedef basic_vec2<float> vec2;
typedef basic_vec2<int> vec2i;
typedef basic_vec2<bool> vec2b;
typedef basic_vec2<unsigned char> vec2c;

// vec2 operators

template<class T>
std::ostream &operator<<(std::ostream &s, const basic_vec2<T> &v)
	{ return s << v.x << ' ' << v.y; }

#define DEFINE_VEC2_OPERATOR(op) \
	template<class T> \
	inline basic_vec2<T> operator op(const basic_vec2<T> &l, const basic_vec2<T> &r) { \
		basic_vec2<T> t; \
		t.x = l.x op r.x; \
		t.y = l.y op r.y; \
		return t; \
	} \
	template<class T> \
	inline basic_vec2<T> operator op(T l, const basic_vec2<T> &r) { \
		basic_vec2<T> t; \
		t.x = l op r.x; \
		t.y = l op r.y; \
		return t; \
	} \
	template<class T> \
	inline basic_vec2<T> operator op(const basic_vec2<T> &l, T r) { \
		basic_vec2<T> t; \
		t.x = l.x op r; \
		t.y = l.y op r; \
		return t; \
	}

DEFINE_VEC2_OPERATOR(+)
DEFINE_VEC2_OPERATOR(-)
DEFINE_VEC2_OPERATOR(*)
DEFINE_VEC2_OPERATOR(/)

template<class T>
inline basic_vec2<T> operator-(const basic_vec2<T> &v) {
	basic_vec2<T> t;
	t.x = -v.x;
	t.y = -v.y;
	return t;
}

template<class T>
inline bool operator==(const basic_vec2<T> &l, const basic_vec2<T> &r)
	{ return l.x == r.x && l.y == r.y; }
template<class T>
inline bool operator!=(const basic_vec2<T> &l, const basic_vec2<T> &r)
	{ return !(l == r); }


// vec2 operations

template<class T>
inline float dot(const basic_vec2<T> &l, const basic_vec2<T> &r)
	{ return l.x*r.x + l.y*r.y; }

template<class T> inline float length2(const basic_vec2<T> &v) { return dot(v,v); }
template<class T> inline float length(const basic_vec2<T> &v) { return std::sqrt(length2(v)); }
template<class T> inline basic_vec2<T> normalize(const basic_vec2<T> &v) { return v / length(v); }
