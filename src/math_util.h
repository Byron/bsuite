#ifndef MATH_UTIL_H
#define MATH_UTIL_H

#include <maya/MFloatPoint.h>
#include <vector>


//! Structure with 3 floats and most general vector methods
struct Float3
{
	float x;
	float y;
	float z;
	
	Float3(float x=0.f, float y=0.f, float z=0.f)
	    : x(x)
	    , y(y)
	    , z(z)
	{}
	
	Float3(const MFloatPoint& r)
	    : x(r.x)
	    , y(r.y)
	    , z(r.z)
	{}
	
	inline Float3 operator - (const Float3& r) const {
		return Float3(x - r.x, y - r.y, z - r.z);
	}
	inline Float3 operator + (const Float3& r) const {
		return Float3(x + r.x, y + r.y, z + r.z);
	}
	inline Float3& operator += (const Float3& r) {
		x += r.x; 
		y += r.y; 
		z += r.z;
		return *this;
	}
	inline Float3 operator * (float v) const {
		return Float3(x * v, y * v, z * v);
	}
	inline Float3 operator / (float v) const {
		return Float3(x / v, y / v, z / v);
	}
};

//! Simple 4 float structure
struct Float4 : public Float3
{
	float w;
	
	Float4(float x=0.f, float y=0.f, float z=0.f, float w=0.f)
	    : Float3(x,y,z)
	    , w(w)
	{}
};


typedef std::vector<Float3>	Float3Vector;

#endif // MATH_UTIL_H
