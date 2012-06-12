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
	bool			begin_access();
	
	//! \return the first byte in the buffer of the given primitive type
	//! Will be 0 if the buffer is not set !
	template <typename Primitive>
	inline
	const Primitive*		buf_begin() const;
	
	//! \return one past the last byte in the buffer of the primitive given type, or 0 if the buffer is not set
	template <typename Primitive>
	inline
	const Primitive*		buf_end() const;
	
	//! non-const version of the one above
	template <typename Primitive>
	inline
	Primitive*				buf_begin();
	
	//! non-const version of the one above
	template <typename Primitive>
	inline
	Primitive*				buf_end();
	
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
	
	template <typename T>
	inline
	const T*		buf_begin() const {
		switch(T::buffer_type)
		{
		case VertexArray:	return reinterpret_cast<const T*>(_vtx_buf);
		case ColorArray:	return reinterpret_cast<const T*>(_col_buf);
		default: return 0;
		}
	}
	
	template <typename T>
	inline
	const T*		buf_end() const {
		switch(T::buffer_type)
		{
		case VertexArray:	return reinterpret_cast<const T*>(_vtx_buf + _len);
		case ColorArray:	return reinterpret_cast<const T*>(_col_buf + _len);
		default: return 0;
		}
	}
	
	template <typename T>
	inline
	T*				buf_begin() {
		switch(T::buffer_type)
		{
		case VertexArray:	return reinterpret_cast<T*>(_vtx_buf);
		case ColorArray:	return reinterpret_cast<T*>(_col_buf);
		default: return 0;
		}
	}
	
	template <typename T>
	inline
	T*				buf_end() {
		switch(T::buffer_type)
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
	bool draw(MGLFunctionTable* glf) const {
		if (glf == 0 || !is_valid()) {
			return false;
		}
		
		glf->glPushClientAttrib(MGL_CLIENT_VERTEX_ARRAY_BIT);
		glf->glPushAttrib(MGL_ALL_ATTRIB_BITS);
		{
			setup_primitive_array<VertexPrimitive>(glf, _vtx_buf);
			if (_col_buf) {
				setup_primitive_array<ColorPrimitive>(glf, _col_buf);
			}
			
			glf->glDrawArrays(MGL_POINTS, 0, _len);
		}
		glf->glPopAttrib();
		glf->glPopClientAttrib();
		
		return glf->glGetError() == 0;
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
	static const size_t			num_buffers = 2;
		
	private:
	
	enum BufferIndex
	{
		Vtx = 0,
		Col = 1
	};
	
	static const MGLuint	invalid_buf = -1;	//!< indicates the buffer is invalid
	
	MGLuint				_gl_buf[num_buffers];	//!< buffer with vtx primitives
	size_t				_len;					//!< amount of primitives per buffer
	MGLFunctionTable*	_glf;					//!< gl function table - without it, we cannot work
	
	private:
	inline bool			is_valid_buf(MGLuint buf) const {
		return buf != invalid_buf;
	}
	
	public:
		ogl_gpu_buffer()
			: _len(0)
			, _glf(0)
		{
			_gl_buf[Vtx] = _gl_buf[Col] = invalid_buf;
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
		
		
		//! \note will automatically revive formerly deleted buffers !
		inline
		bool resize(const size_t new_size = 0) {
			if (_len == new_size) {
				return true;
			}
			
			if (_glf == 0) {
				return false;
			}
			
			if (new_size == 0) {
				_glf->glDeleteBuffersARB(num_buffers, _gl_buf);
				_gl_buf[Vtx] = _gl_buf[Col] = invalid_buf;
				
				// for good measure, make the user reset the pointer after he set us 0
				_glf = 0;
			} else {
				if (!is_valid_buf(_gl_buf[Vtx])) {
					assert(!is_valid_buf(_gl_buf[Col]));	//!< no vtx means no color too
					_glf->glGenBuffersARB(2, &_gl_buf[Vtx]);
				} else if (!is_valid_buf(_gl_buf[Col])) {
					assert(is_valid_buf(_gl_buf[Vtx]));		//!< can only have missing color buffer
					_glf->glGenBuffersARB(1, &_gl_buf[Col]);
				}// handle initialization
				
				// assure both buffers have data. Existing data will automatically be deleted.
				_glf->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, _gl_buf[Vtx]);
				_glf->glBufferDataARB(MGL_ARRAY_BUFFER_ARB, sizeof(VertexPrimitive) * new_size, 0, MGL_STREAM_DRAW_ARB);
				
				_glf->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, _gl_buf[Col]);
				_glf->glBufferDataARB(MGL_ARRAY_BUFFER_ARB, sizeof(ColorPrimitive) * new_size, 0, MGL_STREAM_DRAW_ARB);
				
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
			return is_valid_buf(_gl_buf[0]);
		}
		
};

#endif // ogl_buffer_H
