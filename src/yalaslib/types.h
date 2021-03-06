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

#ifndef YALAS_TYPES_H
#define YALAS_TYPES_H

#include <cstdlib>
#include "baselib/inttypes_compat.h"

namespace yalas {
namespace types {

struct Header13Aligned
{
	char		sig[4];
	uint16_t	source_id;
	uint16_t	global_encoding;
	uint32_t	guid_data1;
	uint16_t	guid_data2;
	uint16_t	guid_data3;
	uint8_t		guid_data4[8];
	uint8_t		version_major;
	uint8_t		version_minor;
	char		system_identifier[32];
	char		generating_software[32];
	uint16_t	creation_day_of_year;
	uint16_t	creation_year;
	uint16_t	header_size;
	uint32_t	offset_to_point_data;
	uint32_t	num_variable_length_records;
};

struct Header13 : public Header13Aligned
{
	uint8_t		point_data_format_id;
	uint16_t	point_data_record_length;
	uint32_t	num_point_records;
	uint32_t	num_points_by_return[5];
	double		x_scale;
	double		y_scale;
	double		z_scale;
	double		x_offset;
	double		y_offset;
	double		z_offset;
	double		max_x;
	double		min_x;
	double		max_y;
	double		min_y;
	double		max_z;
	double		min_z;
	uint64_t	start_of_waveform_data_packet_record;
	
	// convert this instance to host order - not required, its little-endian. Should convert it to big-endian on those systems though
	Header13&	to_host_order();
};


//! Default point structure
struct PointDataRecord0
{
	int32_t		x;
	int32_t		y;
	int32_t		z;
	uint16_t	intensity;
	uint8_t		flags;
	uint8_t		classification;
	int8_t		scan_angle_rank;
	uint8_t		user_data;
	uint16_t	point_source_id;
	
	static const size_t	record_size = 4+4+4+2+1+1+1+1+2;
	static const uint8_t format_id = 0;
	
	// ----------------------------------------
	// Interface
	// ----------------------------------------
	//! \name Interface
	//! @{
	
	inline
	uint8_t		return_number() const {
		return flags & 0x07;
	}
	
	inline
	uint8_t		num_returns() const {
		return flags & (0x07 << 3);
	}
	
	inline
	uint8_t		scan_dir() const {
		return flags & (0x01 << 6);
	}
	
	inline
	uint8_t		edge_of_flight() const {
		return flags & (0x01 << 7);
	}
	
	//! Initialize the fields in this instance from the given data.
	//! It will read record_size bytes.
	//! \return new position of the data pointer
	inline
	const void* init_from_raw(const void* data)
	{
		const uint8_t* c = reinterpret_cast<const uint8_t*>(data);
		x = *(int32_t*)c;
		c += sizeof(x);
		y = *(int32_t*)c;
		c += sizeof(x);
		z = *(int32_t*)c;
		c += sizeof(x);
		intensity = *(uint16_t*)c;
		c += sizeof(intensity);
		
		flags = *c;
		c += sizeof(flags);
		classification = *c;
		c += sizeof(classification);
		scan_angle_rank = *c;
		c += sizeof(scan_angle_rank);
		user_data = *c;
		c += sizeof(user_data);
		point_source_id= *(uint16_t*)c;
		c += sizeof(point_source_id);
		
		return c;
	}
	
	//! Call this after init_from_raw with the respective scale and offset information
	//! obtained from the header
	inline
	void adjust_coordinate(const double* scale, const double* offset)
	{
		x = static_cast<uint32_t>((x * scale[0]) + offset[0]);
		y = static_cast<uint32_t>((y * scale[1]) + offset[1]);
		z = static_cast<uint32_t>((z * scale[2]) + offset[2]);
	}
	
	//! @} end Interface
	
};

//! Point structure with additional GPS time
struct PointDataRecord1 : public PointDataRecord0
{
	double		gps_time;
	
	static const size_t record_size = PointDataRecord0::record_size + 8;
	static const uint8_t format_id = 1;
	
	//! Initialize the fields in this instance from the given data.
	//! It must be of size record_size, as this amount of bytes will be read.
	//! \return new position of the data pointer
	inline
	const void* init_from_raw(const void* data)
	{
		const uint8_t* c = reinterpret_cast<const uint8_t*>(PointDataRecord0::init_from_raw(data));
		gps_time = *(double*)c;
		c += sizeof(gps_time);
		
		return c;
	}
};

//! RGB info for point structures
struct RGBInfo
{
	uint16_t		red;
	uint16_t		green;
	uint16_t		blue;
	
	static const size_t record_size = sizeof(uint16_t) * 3;
	
	inline
	const void* init_from_raw(const void* data)
	{
		const uint8_t* c = reinterpret_cast<const uint8_t*>(data);
		red = *(uint16_t*)c;
		c += sizeof(red);
		
		green = *(uint16_t*)c;
		c += sizeof(green);
		
		blue = *(uint16_t*)c;
		c += sizeof(blue);
		
		return c;
	}
};


//! Point structure with additional RGB info
struct PointDataRecord2 : public PointDataRecord0, public RGBInfo
{
	static const size_t record_size = PointDataRecord0::record_size + RGBInfo::record_size;
	static const uint8_t format_id = 2;
	
	//! Initialize the fields in this instance from the given data.
	//! It must be of size record_size, as this amount of bytes will be read.
	//! \return new position of the data pointer
	inline
	const void* init_from_raw(const void* data)
	{
		return RGBInfo::init_from_raw(PointDataRecord0::init_from_raw(data));
	}
};



//! Point structure with GPS and RGB info
struct PointDataRecord3 : public PointDataRecord1, public RGBInfo
{
	static const size_t record_size = PointDataRecord1::record_size + RGBInfo::record_size;
	static const uint8_t format_id = 3;
	
	//! Initialize the fields in this instance from the given data.
	//! It must be of size record_size, as this amount of bytes will be read.
	//! \return new position of the data pointer
	inline
	const void* init_from_raw(const void* data)
	{
		return RGBInfo::init_from_raw(PointDataRecord1::init_from_raw(data));
	}
};


// Waveform packet info for point records
struct WaveformInfo 
{
	uint8_t		wave_packet_descriptor_index;			//!< index into packe descriptor user data
	uint64_t	waveform_data_offset;					//!< offset in bytes
	uint32_t	waveform_packet_size;					//!< packet size in bytes
	float		return_point_waveform_location;
	float		extra_x;								//!< parametric line equation value for point extrapolation
	float		extra_y;
	float		extra_z;
	
	static const size_t record_size = 1 + 8 + 4 + sizeof(float)*4;
	
	inline
	const void* init_from_raw(const void* data)
	{
		const uint8_t* c = reinterpret_cast<const uint8_t*>(data);
		wave_packet_descriptor_index = *c;
		c += sizeof(wave_packet_descriptor_index);
		
		waveform_data_offset = *(uint64_t*)c;
		c += sizeof(waveform_data_offset);
		
		waveform_packet_size = *(uint32_t*)c;
		c += sizeof(waveform_packet_size);
		
		return_point_waveform_location = *(float*)c;
		c += sizeof(return_point_waveform_location);
		
		extra_x = *(float*)c;
		c += sizeof(extra_x);
		extra_y = *(float*)c;
		c += sizeof(extra_y);
		extra_z = *(float*)c;
		c += sizeof(extra_z);
		
		return c;
	}
	
};

// Point structure with GPS and Waveform packet info
struct PointDataRecord4 : public PointDataRecord1, public WaveformInfo
{
	static const size_t record_size = PointDataRecord1::record_size + WaveformInfo::record_size;
	static const uint8_t format_id = 4;
	
	//! Initialize the fields in this instance from the given data.
	//! It must be of size record_size, as this amount of bytes will be read.
	//! \return new position of the data pointer
	inline
	const void* init_from_raw(const void* data)
	{
		return WaveformInfo::init_from_raw(PointDataRecord1::init_from_raw(data));
	}
	
};

// Point structure with GPS and color and Waveform packet info
struct PointDataRecord5 : public PointDataRecord3, public WaveformInfo
{
	static const size_t record_size = PointDataRecord3::record_size + WaveformInfo::record_size;
	static const uint8_t format_id = 5;
	
	//! Initialize the fields in this instance from the given data.
	//! It must be of size record_size, as this amount of bytes will be read.
	//! \return new position of the data pointer
	inline
	const void* init_from_raw(const void* data)
	{
		return WaveformInfo::init_from_raw(PointDataRecord3::init_from_raw(data));
	}
	
};

//! A generic point data record which can be specialized to the actual format you need.
template <uint8_t format_id>
struct point_data_record
{
};

template <>
struct point_data_record<0> : public PointDataRecord0
{
};

template <>
struct point_data_record<1> : public PointDataRecord1
{
};

template <>
struct point_data_record<2> : public PointDataRecord2
{
};

template <>
struct point_data_record<3> : public PointDataRecord3
{
};

template <>
struct point_data_record<4> : public PointDataRecord4
{
};

template <>
struct point_data_record<5> : public PointDataRecord5
{
};


}// end LASTypes
}// END yalas
#endif // types
