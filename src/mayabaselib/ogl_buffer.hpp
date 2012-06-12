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
#include "mayabaselib/detail/ogl_buffer_fun.hpp"
#include "baselib/typ.h"

#include <cassert>


//********************************************************************
//**	Template Types
//********************************************************************

//! A generic primitive suitable for drawing
//! It consists of a field type and an amount of fields
//! Its buffer type indicates in which buffer it will be used
template <typename Type, size_t num_fields, BufferType type>
struct draw_primitive
{
	typedef Type			field_type;
	static const size_t		field_count = num_fields;
	
	field_type				field[field_count];
	
	static const BufferType buffer_type = type;
};


//! A buffer object which provides you with storage space to put your cached point clouds.
//! This base provides an interface declaration which can be used among the different versions.
//! \note  for now, this base class has no other effect but to document the interface.
//! Maybe one day we make some methods virtual for more runtime-flexibility
template <typename VertexPrimitive,  typename ColorPrimitive>
class ogl_buffer : NonCopyable
{
	public:
	// ----------------------------------------
	// Public Types
	// ----------------------------------------
	//! \name Public Types
	//! @{
	typedef VertexPrimitive		vertex_primitive;
	typedef ColorPrimitive		color_primitive;
		
	typedef ogl_buffer<VertexPrimitive, ColorPrimitive> this_type;
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
	bool					begin_access();
	
	//! \return the first byte in the buffer of the given primitive type
	//! Will be 0 if the buffer is not set !
	inline
	void*					begin(const BufferType);
	
	//! \return one past the last byte in the buffer of the primitive given type, or 0 if the buffer is not set
	inline
	void*					end(const BufferType);
	
	//! Call to end buffer access
	void					end_access();
	
	//! \return true if all the buffers in this instance are set and can be used !
	bool					is_valid() const;
	
	//! @} end Query Interface
	
	
	public:
	// ----------------------------------------
	// Edit Interface
	// ----------------------------------------
	//! \name Edit Interface
	//! @{
	
	//! Draw the data in this instance as efficiently as possible
	//! \return true if drawing succeeded
	bool draw(MGLFunctionTable* glf) const;
	
	//! Resize the buffers in this instance to the given value
	//! Setting this to 0 will effectively clear the buffer
	//! \return true on success, false on failure.
	bool resize(const size_t new_size = 0);
	
	//! Delete the buffer of the given type, so it will not be drawn, or consume memory
	//! \return true on success and false if the buffer could not be deleted.
	//! \note deleted buffers will be revived after a call to resize assuming that
	//! it the size is new.
	bool delete_array(const BufferType);
	
	//! Revive a previously deleted array
	//! \return true if the new array now is alive, or false if this instance is not initialized
	bool revive_array(const BufferType);
	
	//! @} end Edit Interface
	
};

//! The system buffered version uses memory in the system's main memory and draws it using 
//! DrawArrays. This easily speeds up drawing by a factor of two.
template <typename VertexPrimitive, typename ColorPrimitive>
class ogl_system_buffer : public ogl_buffer <VertexPrimitive, ColorPrimitive>
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
		resize(0);
	}
	
	public:
	
	static const BufferMode		buffer_mode = SystemMemory;
	
	bool			begin_access() {
		return is_valid();
	}
	
	inline
	void*				begin(const BufferType type) {
		switch(type)
		{
		case VertexArray:	return _vtx_buf;
		case ColorArray:	return _col_buf;
		default: return 0;
		}
	}
	
	inline
	void*				end(const BufferType type) {
		switch(type)
		{
		case VertexArray:	return _vtx_buf + _len;
		case ColorArray:	return _col_buf + _len;
		default: return 0;
		}
	}
	
	void			end_access(){}
	
	bool			is_valid() const {
		return _len != 0;
	}
	
	inline
	bool draw(MGLFunctionTable* glf) const {
		if (!is_valid()) {
			return false;
		}
		
		return draw_arrays<VertexPrimitive, ColorPrimitive>(glf, _vtx_buf, _col_buf, _len);
	}
	
	inline
	bool resize(const size_t new_size = 0) {
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
		
		// As we allocate large blocks, we just use realloc for convenience, which does the same for a 0 pointer
		// as malloc !
		_vtx_buf = reinterpret_cast<VertexPrimitive*>(realloc(_vtx_buf, sizeof(*_vtx_buf) * new_size));
		_col_buf = reinterpret_cast<ColorPrimitive*>(realloc(_col_buf, sizeof(*_col_buf) * new_size));
		
		// did any of the buffer's not get allocated ?
		if (!_vtx_buf || !_col_buf) {
			resize(0);
			return false;
		}
		
		_len = new_size;
		return is_valid();
	}
	
	bool delete_array(const BufferType type)
	{
		if (type != ColorArray) {
			return false;
		}
		
		free(_col_buf);
		_col_buf = 0;
		return true;
	}
	
	bool revive_array(const BufferType type) {
		if (!is_valid() || type != ColorArray) {
			return false;
		}
		
		if (_col_buf == 0) {
			_col_buf = reinterpret_cast<ColorPrimitive*>(malloc(sizeof(*_col_buf) * _len));
		}
		
		return _col_buf != 0;
	}
};

//! A GPU buffer which stores data directly on the GPU
//! It needs a glfunction table to work, you must set before making any buffer operations
//! Otherwise we work on a cached version of the pointer, which is assumed to remain valid until
//! you set it back to 0
template <typename VertexPrimitive, typename ColorPrimitive>
class ogl_gpu_buffer : public ogl_buffer<VertexPrimitive, ColorPrimitive>
{
	public:
	static const BufferMode		buffer_mode = GPUMemory;
		
	private:
	
	static const MGLuint invalid_buf = 0;		//!< indicates the buffer is invalid
	
	MGLuint				_gl_buf;				//!< buffer id for one buffer, containing all data
	void*				_map;					//!< mapped data pointers, only valid between begin and end access
	size_t				_len;					//!< amount of primitives per buffer
	MGLFunctionTable*	_glf;					//!< gl function table - without it, we cannot work
	bool				_use_col;				//!< if true, we use the color part of the buffer - default true
	
	private:
	static size_t			col_buf_ofs_bytes(const size_t num_primitives) {
		return sizeof(VertexPrimitive) * num_primitives;
	}
	
	static size_t			total_buf_size_bytes(const size_t num_primitives) {
		return (sizeof(VertexPrimitive) + sizeof(ColorPrimitive)) * num_primitives;
	}
	
	static void				unbind_gl_buffer(MGLFunctionTable* glf) {
		glf->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, 0);
	}
	
	public:
		ogl_gpu_buffer()
			: _gl_buf(invalid_buf)
			, _map(0)
			, _len(0)
			, _glf(0)
			, _use_col(true)
		{
		}
		
		~ogl_gpu_buffer()
		{
			resize(0);
		}
		
		
	public:
		// ----------------------------------------
		// Interface
		// ----------------------------------------
		//! \name Interface
		//! @{
		
		//! Set our function table to the given value
		//! If it changes to another one, we will release the caches in the previous context.
		//! Thus you might end up with an empty buffer. This is preferred over just initializing
		//! a buffer of the same size as you could not tell if the values are actually the ones you expect.
		//! Hence you should check if your buffer is still valid after this call and possibly reinitialize it.
		ogl_gpu_buffer&	set_glf(MGLFunctionTable* glf) {
			if (_glf && glf != _glf) {
				resize(0);
			}
			
			_glf = glf;
			return *this;
		}
		
		//! @} end Interface
		
		bool begin_access() {
			if (!is_valid() || !_glf) {
				return false;
			}
			
			assert(_map == 0);	// multiple calls to begin access ?
			_glf->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, _gl_buf);
			_map = _glf->glMapBufferARB(MGL_ARRAY_BUFFER_ARB, MGL_WRITE_ONLY_ARB);
			
			return _glf->glGetError() == 0;
		}
		
		inline
		void* begin(const BufferType type) {
			switch(type)
			{
			case VertexArray: return _map;
			case ColorArray: return static_cast< uint8_t*>(_map) + col_buf_ofs_bytes(_len);
			default: return 0;
			}
		}
		
		inline
		void* end(const BufferType type) {
			switch(type)
			{
			case VertexArray: return begin(ColorArray);
			case ColorArray: return static_cast<uint8_t*>(_map) + total_buf_size_bytes(_len);
			default: return 0;
			}
		}
		
		void end_access() {
			if (!_glf || _map == 0) {
				return;
			}
			
			// assume our buffer is still bound !
			_glf->glUnmapBufferARB(MGL_ARRAY_BUFFER_ARB);
			unbind_gl_buffer(_glf);
			_map = 0;
		}
		
		
		inline
		bool draw(MGLFunctionTable* glf) const {
			if (glf != _glf || !is_valid()) {
				return false;
			}
			
			glf->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, _gl_buf);
			const bool res = draw_arrays<VertexPrimitive, ColorPrimitive>(  glf, 0, 
																   _use_col ? (const uint8_t*)0 + col_buf_ofs_bytes(_len) : 0, 
																   _len);
			unbind_gl_buffer(glf);
			return res;
		}
		
		//! \note will automatically revive formerly deleted buffers !
		inline
		bool resize(const size_t new_size = 0) {
			if (_len == new_size) {
				return true;
			}
			
			if (_glf == 0 || _map != 0) {
				return false;
			}
			
			if (new_size == 0) {
				_glf->glDeleteBuffersARB(1, &_gl_buf);
				_gl_buf = invalid_buf;
				_map = 0;
				
				// for good measure, make the user reset the pointer after he set us 0
				_glf = 0;
			} else {
				if (_gl_buf == invalid_buf) {
					_glf->glGenBuffersARB(1, &_gl_buf);
				}
				// revive color, its how we behave (currently)
				_use_col = true;
				
				_glf->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, _gl_buf);
				_glf->glBufferDataARB(MGL_ARRAY_BUFFER_ARB, total_buf_size_bytes(new_size), 0, MGL_STREAM_DRAW_ARB);
				unbind_gl_buffer(_glf);
				if (_glf->glGetError() != 0) {
					resize(0);
					return false;
				}
			}// end handle size
			
			_len = new_size;
			return is_valid();
		}
		
		inline
		bool is_valid() const {
			return _gl_buf != invalid_buf;
		}
		
		
		bool delete_array(const BufferType type)
		{
			if (type != ColorArray) {
				return false;
			}
			
			_use_col = false;
			return true;
		}
		
		bool revive_array(const BufferType type) {
			if (!is_valid() || type != ColorArray) {
				return false;
			}
			
			_use_col = true;
			return true;
		}
		
};

#endif // ogl_buffer_H
