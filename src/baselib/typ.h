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

#ifndef MISC_TYPES_H
#define MISC_TYPES_H

#include <stdlib.h>
#include <inttypes.h>
#include <cassert>

//! Utility which makes derived types non-copyable
class NonCopyable
{
	NonCopyable(const NonCopyable&);
	NonCopyable& operator = (const NonCopyable&);
	
	public:
	NonCopyable(){}
};

//! Simple utility to keep a mapped file
//! For now we don't use git++'s sliding window memory map
//! \note as we deal with file-handles, we are (currently) not copyable
class ROMappedFile : NonCopyable
{
	void*		_mem;	//!< Mapped memory, aligned to page boundary, read-onl
	size_t		_len;	//!< mount of mapped bytes
	
	public:
		ROMappedFile();
		~ROMappedFile();
		
	public:
	// ----------------------------------------
	// Interface
	// ----------------------------------------
	//! \name Interface
	//! @{
	
	//! Map a file for read-only access. On success, you may query
	//! you will be able to access the memory for reading. is_mapped() will return true.
	//! \param filepath the path to the file to map
	//! \note currently it will attempt to map the whole file.
	ROMappedFile&	map_file(const char* filepath);
	
	//! Unmap the currently mapped file. If it was mapped, is_mapped() will be return false afterwards.
	ROMappedFile&	unmap_file();
	
	//! \return true if this mapped file is actually mapped
	bool			is_mapped() const;
	
	//! \return pointer at the given offset (relative to its starting location
	template <typename T>
	const T*		mem_at_ofs(const size_t ofs = 0) const
	{
		assert(is_mapped());
		return reinterpret_cast<const T*>(reinterpret_cast<const uint8_t*>(_mem) + ofs);
	}
	
	//! \return a pointer to one past the last byte of mapped memory
	template <typename T>
	const T*		mem_end() const
	{
		return mem_at_ofs<T>(_len);
	}
	
	//! @} end Interface
		
		
};

#endif // MISC_TYPES_H
