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


#include "typ.h"

#ifndef WIN32
	#include <sys/mman.h>
	#include <stdio.h>
	
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <cstring>

	#include <limits>
#endif

ROMappedFile::ROMappedFile()
	: _mem(0)
	, _len(0)
{
}

ROMappedFile::~ROMappedFile()
{
	unmap_file();
}

ROMappedFile &ROMappedFile::map_file(const char *filepath)
{
	unmap_file();
	assert(!is_mapped());
	
#ifndef WIN32
	const int fid = open(filepath, O_RDONLY);
	if (fid < 0) {
		return *this;
	}
	
	struct stat fid_info;
	memset(&fid_info, 0, sizeof(fid_info));
	
	// check for 32 bit limitations - off_t will be 32 bit on a 32 bit system, so we will be limited 
	// to mapping 2gig there
	if (fstat(fid, &fid_info) < 0 || fid_info.st_size > std::numeric_limits<off_t>::max()) {
		close(fid);
		return *this;
	}
	
	_mem = mmap(0, fid_info.st_size, PROT_READ, MAP_PRIVATE, fid, 0);
	// mmap keeps a reference to the handle, keeping the file open effectively
	close(fid);
	
	if (_mem) {
		_len = fid_info.st_size;
	}
	
	assert (is_mapped());
#endif
	return *this;
}

ROMappedFile &ROMappedFile::unmap_file()
{
#ifndef WIN32
	if (is_mapped()) {
		if (munmap(_mem, _len) == 0) {
			_mem = 0;
			_len = 0;
		} else {
			assert(false);
		}
	}
#endif
	return *this;
}

bool ROMappedFile::is_mapped() const
{
	return _mem != 0;
}
