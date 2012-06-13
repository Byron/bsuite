/* Copyright (c) 2012, Sebastian Thiel
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *   Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
 *   in the documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, 
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MATH_UTIL_H
#define MATH_UTIL_H

#include <maya/MFloatPoint.h>
#include <vector>

#include <cmath>


//********************************************************************
//**	Basic Types
//********************************************************************

//! Structure with 3 floats and most general vector methods
struct Float3
{
	float x;
	float y;
	float z;
	
	inline
	Float3(float x=0.f, float y=0.f, float z=0.f)
	    : x(x)
	    , y(y)
	    , z(z)
	{}
	
	inline
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
	
	inline
	Float4(float x=0.f, float y=0.f, float z=0.f, float w=0.f)
	    : Float3(x,y,z)
	    , w(w)
	{}
};


typedef std::vector<Float3>	Float3Vector;




//********************************************************************
//**	Utiltiies
//********************************************************************


//! \return dot product of two vectors
template <typename T>
inline float dot(const T& l, const T& r) {
	return l.x*r.x + l.y*r.y + l.z*r.z;
}

//! \return length of the vector
template <typename T>
inline float len(const T& v) {
	return std::sqrt(dot(v));
}



#endif // MATH_UTIL_H
