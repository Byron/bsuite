#ifndef LASFILE_H
#define LASFILE_H

#include <iosfwd>
#include <inttypes.h>


class LAS_IStream
{
	public:
		
		enum Status
		{
			Success = 0,
			Invalid,
			
			InvalidHeader,				//!< Header could not be read or not a LAS file
			UnsupportedPointDataFormat,
			HeaderAlignmentFailure,
			StreamFailure
		};
		
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
		
		struct PointDataRecord0
		{
			int32_t		x;
			int32_t		y;
			int32_t		z;
			uint16_t	intensity;
			uint8_t		flags;
			/*uint8_t		return_number : 3;
			uint8_t		num_returns : 3;
			uint8_t		scan_dir : 1;
			uint8_t		edge_of_flight : 1;*/
			uint8_t		classification;
			int8_t		scan_angle_rank;
			uint8_t		user_data;
			uint16_t	point_source_id;
		};
		
		struct PointDataRecord1 : public PointDataRecord0
		{
			double		gps_time;
		};

	protected:
		std::istream&	_istream;
		Status			_status;
		Header13		_header;
		
		
		
	private:
		void			read_header();
		void			read_next_point(PointDataRecord1& p);
		
	public:
		LAS_IStream(std::istream& instream);
		
	public:
		
		Status	status() const {
			return _status;
		}
		
		const Header13&	header() const {
			return _header;
		}
};

#endif // LASFILE_H
