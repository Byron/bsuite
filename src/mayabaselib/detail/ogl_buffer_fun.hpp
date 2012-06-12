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


#ifndef OGL_BUFFER_FUN_HPP
#define OGL_BUFFER_FUN_HPP

//********************************************************************
//**	Includes
//********************************************************************
#include "mayabaselib/detail/ogl_buffer_types.hpp"

//********************************************************************
//**	Static Mapping: Type to TypeID (i.e. MGLint -> MGL_INT)
//********************************************************************

template <typename DataType>
inline int data_type_to_ogl_constant()
{
	// default
	return 0;
}

template <>
inline int data_type_to_ogl_constant<MGLint>()
{
	return MGL_INT;
}

template <>
inline int data_type_to_ogl_constant<MGLuint>()
{
	return MGL_UNSIGNED_INT;
}

template <>
inline int data_type_to_ogl_constant<MGLshort>()
{
	return MGL_SHORT;
}

template <>
inline int data_type_to_ogl_constant<MGLushort>()
{
	return MGL_UNSIGNED_SHORT;
}

template <>
inline int data_type_to_ogl_constant<MGLfloat>()
{
	return MGL_FLOAT;
}

template <>
inline int data_type_to_ogl_constant<MGLdouble>()
{
	return MGL_DOUBLE;
}


//********************************************************************
//**	Static Mapping: BufferType to pointer function (i.e. VertexArray -> MGL_INT)
//********************************************************************
typedef void (MGLFunctionTable::*GLArrayMemFun)(MGLint size, MGLenum type, MGLsizei stride, const MGLvoid *pointer);

template <BufferType type>
inline GLArrayMemFun buffer_mode_to_array_memfun()
{
	return 0;
}

template <>
inline GLArrayMemFun buffer_mode_to_array_memfun<VertexArray>()
{
	return &MGLFunctionTable::glVertexPointer;
}

template <>
inline GLArrayMemFun buffer_mode_to_array_memfun<ColorArray>()
{
	return &MGLFunctionTable::glColorPointer;
}


//********************************************************************
//**	Utilities
//********************************************************************

//! setup the gl state to draw the given primitive array pointer
//! We assume the draw state has been saved in some way as it will be modified
template <typename Primitive, BufferType type>
void setup_primitive_array(MGLFunctionTable* glf, const Primitive* pris)
{
	// static switch
	glf->glEnableClientState(type);
	(*glf.*(buffer_mode_to_array_memfun<type>()))(Primitive::field_count, data_type_to_ogl_constant<typename Primitive::field_type>(), 0, pris);
}


#endif // OGL_BUFFER_FUN_HPP
