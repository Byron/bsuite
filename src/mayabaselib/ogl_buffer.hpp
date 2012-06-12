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

#ifndef ogl_buffer_H
#define ogl_buffer_H

//********************************************************************
//**	Include
//********************************************************************
#include <mayabaselib/ogl_headers.h>
#include <stdlib.h>

//********************************************************************
//**	Enumerations
//********************************************************************

enum BufferMode
{
	GPUMemory,		//!< Use GPU memory
	SystemMemory	//!< Use System Memory
};

enum BufferType
{
	VertexArray = 1,//!< Id for the vertex array
	ColorArray = 2	//!< Id for the color array
};


//! A generic primitive suitable for drawing
template <typename Type, size_t num_fields>
struct draw_primitive
{
	typedef Type type;
	static const size_t	field_count = num_fields;
	
	type				dat[field_count];
};


//********************************************************************
//**	Template Types
//********************************************************************

//! A buffer object which provides you with storage space to put your cached point clouds.
//! This base provides an interface declaration which can be used among the different versions.
template <BufferMode mode, typename VertexPrimitive,  typename ColorPrimitive>
class ogl_buffer
{
	public:
	// ----------------------------------------
	// Public Types
	// ----------------------------------------
	//! \name Public Types
	//! @{
	typedef VertexPrimitive		vertex_primitive;
	typedef ColorPrimitive		color_primitive;
		
	static const BufferMode		buffer_mode = mode;
	
	typedef ogl_buffer<mode, VertexPrimitive, ColorPrimitive> this_type;
	//! @} end Public Types
	
	
	public:
	// ----------------------------------------
	// Query Interface
	// ----------------------------------------
	//! \name Query Interface
	//! @{
	
	//! Call this to initialize access to the data stored in the buffer !
	//! \return true on success
	//! \note only works if is_valid() returns true
	bool			begin_access();
	
	//! \return the first byte in the buffer of the given type, in the respective format T
	//! Will be 0 if the buffer is not set !
	template <typename T>
	inline
	const T*		buf_begin(BufferType) const;
	
	//! \return one past the last byte in the buffer of the given type, or 0 if the buffer is not set
	template <typename T>
	inline
	const T*		buf_end(BufferType) const;
	
	//! non-const version of the one above
	template <typename T>
	inline
	T*				buf_begin(BufferType);
	
	//! non-const version of the one above
	template <typename T>
	inline
	T*				buf_end(BufferType);
	
	//! Call to end buffer access
	void			end_access();
	
	//! \return true if all the buffers in this instance are set and can be used !
	bool			is_valid() const;
	//! @} end Query Interface
	
	
	public:
	// ----------------------------------------
	// Edit Interface
	// ----------------------------------------
	//! \name Edit Interface
	//! @{
	
	//! Draw the data in this instance as efficiently as possible
	void draw(MGLFunctionTable* glf) const;
	
	//! Resize the buffers in this instance to the given value
	//! Setting this to 0 will effectively clear the buffer
	//! \return true on success, false on failure.
	//! \note as it is possible to store data directly on the graphics-card, a glf pointer needs to be provided
	//! If the resize succeeds, your buffer is_valid(), otherwise it is not.
	bool resize(MGLFunctionTable* glf, const size_t new_size = 0);
	
	//! @} end Edit Interface
	
};

//! The system buffered version uses memory in the system's main memory and draws it using 
//! DrawArrays. This easily speeds up drawing by a factor of two.
template <typename VertexPrimitive, typename ColorPrimitive>
class ogl_system_buffer : public ogl_buffer <SystemMemory, VertexPrimitive, ColorPrimitive>
{
	VertexPrimitive*		_vtx_buf;		//!< Buffer pointer for vertex data
	ColorPrimitive*			_col_buf;		//!< Buffer pointer for color data
	size_t					_len;			//!< Amount of primitives stored in the buffer
	
	public:
	ogl_system_buffer()
		: _vtx_buf(0)
		, _col_buf(0)
		, _len(0)
	{}
	
	~ogl_system_buffer()
	{
		resize(0, 0);
	}
	
	public:
	
	bool			begin_access() {
		return is_valid();
	}
	
	template <typename T>
	inline
	const T*		buf_begin(BufferType type) const {
		switch(type)
		{
		case VertexArray:	return reinterpret_cast<const T*>(_vtx_buf);
		case ColorArray:	return reinterpret_cast<const T*>(_col_buf);
		default: return 0;
		}
	}
	
	template <typename T>
	inline
	const T*		buf_end(BufferType type) const {
		switch(type)
		{
		case VertexArray:	return reinterpret_cast<const T*>(_vtx_buf + _len);
		case ColorArray:	return reinterpret_cast<const T*>(_col_buf + _len);
		default: return 0;
		}
	}
	
	template <typename T>
	inline
	T*				buf_begin(BufferType type) {
		switch(type)
		{
		case VertexArray:	return reinterpret_cast<T*>(_vtx_buf);
		case ColorArray:	return reinterpret_cast<T*>(_col_buf);
		default: return 0;
		}
	}
	
	template <typename T>
	inline
	T*				buf_end(BufferType type) {
		switch(type)
		{
		case VertexArray:	return reinterpret_cast<T*>(_vtx_buf + _len);
		case ColorArray:	return reinterpret_cast<T*>(_col_buf + _len);
		default: return 0;
		}
	}
	
	void			end_access(){}
	
	bool			is_valid() const {
		return _len != 0;
	}
	
	inline
	void draw(MGLFunctionTable* glf) const {
		
	}
	
	//! Resize this instance. Its safe to pass in 0 as glf
	inline
	bool resize(MGLFunctionTable*, const size_t new_size = 0) {
		if (new_size == _len) {
			return true;
		}
		
		if (new_size == 0) {
			_len = 0;
			free(_vtx_buf);
			_vtx_buf = 0;
			free(_col_buf);
			_col_buf = 0;
			return true;
		}
		
		if (_len) {
			_vtx_buf = reinterpret_cast<VertexPrimitive*>(realloc(_vtx_buf, sizeof(*_vtx_buf) * new_size));
			_col_buf = reinterpret_cast<ColorPrimitive*>(realloc(_col_buf, sizeof(*_col_buf) * new_size));
		} else {
			_vtx_buf = reinterpret_cast<VertexPrimitive*>(malloc(sizeof(*_vtx_buf) * new_size));
			_col_buf = reinterpret_cast<ColorPrimitive*>(malloc(sizeof(*_col_buf) * new_size));
		}
		
		// did any of the buffer's not get allocated ?
		if (!_vtx_buf || !_col_buf) {
			resize(0, 0);
			return false;
		}
		
		_len = new_size;
		return is_valid();
	}
};

//! A GPU buffer which stores data directly on the GPU
template <typename VertexPrimitive, typename ColorPrimitive>
class ogl_gpu_buffer : public ogl_buffer<GPUMemory, VertexPrimitive, ColorPrimitive>
{
	
};

#endif // ogl_buffer_H
