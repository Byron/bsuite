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
#ifndef YALAS_ITER_H
#define YALAS_ITER_H

#include <baselib/typ.h>

namespace yalas
{

//! A utility iterator which can be utilized in while loops to iterate over 
//! raw uncompressed memory to read points
//! \tparam PointType type of point to use. It must be one of the point types defined in types.h
class MemoryIterator
{
	const uint8_t*	_cur;		//!< Current memory pointer
	const uint8_t*	_end;		//!< 
	const double*	_ofs;
	const double*	_scale;
	
	
	public:
	inline
	MemoryIterator(const uint8_t* beg, const uint8_t* end, const double* ofs, const double* scale)
		: _cur(beg)
		, _end(end)
		, _ofs(ofs)
		, _scale(scale)
	{}
	
	
	template <typename PointType>
	inline
	bool read_next_point(PointType& p) {
		if (_cur < _end) {
			p.init_from_raw(_cur);
			p.adjust_coordinate(_scale, _ofs);
			_cur += PointType::record_size;
			return true;
		}
		return false;
	}
};

}// end namespace yalas

#endif // YALAS_ITER_H
