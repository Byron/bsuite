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

#include "LAS_IStream.h"

#include <iostream>
#include <cstring>
#ifdef WIN32
	#include <Winsock2.h 
#else
	#include <arpa/inet.h>
#endif
#include <cassert>

template <typename T>
inline void read_n(std::istream& istream, T& dest) 
{
	// stream exceptions are enabled
	istream.read(reinterpret_cast<char*>(&dest), sizeof(T));
}

void LAS_IStream::read_header()
{
	_istream.exceptions(std::istream::failbit|std::istream::failbit|std::istream::eofbit);
	_istream.read(reinterpret_cast<char*>(&_header), sizeof(LAS_Types::Header13Aligned));
	
	const std::istream::pos_type expected_ofs = 4+2+2+4+2+2+8+1+1+32+32+2+2+2+4+4;
	if (_istream.tellg() != expected_ofs) {
		std::cerr << _istream.tellg() << " != " << expected_ofs << std::endl;
		_status = UnexpectedHeaderAlignment;
		return;
	}
	
	if (strcmp(_header.sig, "LASF") != 0) {
		_status = InvalidHeader;
		return;
	}
	
	read_n(_istream, _header.point_data_format_id);
	read_n(_istream, _header.point_data_record_length);
	read_n(_istream, _header.num_point_records);
	read_n(_istream, _header.num_points_by_return);
	read_n(_istream, _header.x_scale);
	read_n(_istream, _header.y_scale);
	read_n(_istream, _header.z_scale);
	read_n(_istream, _header.x_offset);
	read_n(_istream, _header.y_offset);
	read_n(_istream, _header.z_offset);
	read_n(_istream, _header.max_x);
	read_n(_istream, _header.min_x);
	read_n(_istream, _header.max_y);
	read_n(_istream, _header.min_y);
	read_n(_istream, _header.max_z);
	read_n(_istream, _header.min_z);
	read_n(_istream, _header.start_of_waveform_data_packet_record);
	
	
	if (_header.point_data_format_id != 1) {
		_status = UnsupportedPointDataFormat;
		return;
	}
	_status = Success;
}

LAS_IStream::LAS_IStream(std::istream &instream)
	: _istream(instream)
	, _status(Invalid)
{
	memset(&_header, 0, sizeof(_header));
	std::istream::iostate state = _istream.exceptions();
	try {
		read_header();
		_istream.exceptions(state);
	} catch (const std::exception& exc) {
		std::cerr << exc.what() << std::endl;
		_istream.exceptions(state);
		_status = StreamFailure;
		throw;
	}
}

LAS_IStream::Status LAS_IStream::reset_point_iteration()
{
	if (_status != Success) {
		return _status;
	}
	// required to reset error, otherwise we cannot seek !
	if (_istream.eof()) {
		_istream.clear();
	}
	
	_istream.seekg(_header.offset_to_point_data, std::ios_base::beg);
	
	if (_istream.fail()) {
		_status = StreamFailure;
	}
	return _status;
}

LAS_IStream::Status LAS_IStream::read_next_point(LAS_Types::PointDataRecord1 &p)
{
	char buf[LAS_Types::PointDataRecord1::record_size];	// point format 1
	assert(sizeof(buf) == _header.point_data_record_length);
	
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


LAS_Types::Header13 &LAS_Types::Header13::to_host_order()
{
	source_id = ntohs(source_id);
	global_encoding = ntohs(global_encoding);
	
	guid_data1 = ntohl(guid_data1);
	guid_data2 = ntohs(guid_data2);
	guid_data3 = ntohs(guid_data3);
	
	creation_day_of_year = ntohs(creation_day_of_year);
	creation_year = ntohs(creation_year);
	
	header_size = ntohs(header_size);
	offset_to_point_data = ntohl(offset_to_point_data);
	
	num_variable_length_records = ntohl(num_variable_length_records);
	point_data_record_length = ntohs(point_data_record_length);
	
	num_point_records = ntohl(num_point_records);
	uint32_t*const npbrend = num_points_by_return + sizeof(num_points_by_return) / sizeof(num_points_by_return[0]);
	for (uint32_t* npbr = num_points_by_return; npbr < npbrend; ++npbr) {
		*npbr = ntohl(*npbr);
	}
	
	start_of_waveform_data_packet_record = ((uint64_t)ntohl(start_of_waveform_data_packet_record)) << 32 | ntohl(start_of_waveform_data_packet_record>>32);
	
	return *this;
}
