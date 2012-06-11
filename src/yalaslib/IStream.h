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

#ifndef LAS_ISTREAM_H
#define LAS_ISTREAM_H

#include "types.h"

#include <iostream>
#include <inttypes.h>
#include <cassert>

namespace yalas
{

	// A utillity reading a LAS file from a stream.
	class IStream
	{
		public:
			
			enum Status
			{
				Success = 0,
				Invalid,
				
				InvalidHeader,				//!< Header could not be read or not a LAS file
				UnexpectedHeaderAlignment,
				UnsupportedPointDataFormat,
				StreamFailure
			};
			
		protected:
			std::istream&		_istream;
			Status				_status;
			types::Header13		_header;
			
			
			
		private:
			void			read_header();
			
		public:
			//! Intialize this instance with the given input stream.
			//! It is assumed to be open already and ready for reading.
			//! \note the stream will not be closed when this instance 
			//! is destroyed, this is up to the user
			IStream(std::istream& instream);
			
		public:
			
			inline
			Status	status() const {
				return _status;
			}
			
			inline
			const types::Header13&	header() const {
				return _header;
			}
			
			//! Call this to set the instance to begin iterating on point records.
			//! You may only call read_next_point() if this method was called.
			//! \return Status to indicate success or failure
			//! \note you may call this method each time you want to restart iterating all samples.
			Status reset_point_iteration();
			
			//! Read the next point record from the stream
			//! Note that the point type you are using must match the format specified 
			//! in the header.
			template <typename PointType>
			inline
			Status read_next_point(PointType& p)
			{
				char buf[PointType::record_size];	// point format 1
				assert(sizeof(buf) == _header.point_data_record_length);
				assert(PointType::format_id == _header.point_data_format_id);
				
				// stream exceptions are enabled
				_istream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
				
				if (_istream.eof()) {
					return StreamFailure;
				} else if (_istream.fail()) {
					_status = StreamFailure;
				} else {
					p.init_from_raw(buf);
					p.adjust_coordinate(&_header.x_scale,& _header.x_offset);
				}
				return _status;
			}
			
			
	};

}// END namespace yalas
#endif // LAS_ISTREAM_H
