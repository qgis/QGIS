/*
===============================================================================

  FILE:  excepts.hpp
  
  CONTENTS:
    Exception types

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
    uday.karan@gmail.com - Hobu, Inc.
  
  COPYRIGHT:
  
    (c) 2007-2014, martin isenburg, rapidlasso - tools to catch reality
    (c) 2014, Uday Verma, Hobu, Inc.

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
===============================================================================
*/

#ifndef __excepts_hpp__
#define __excepts_hpp__

#include <stdexcept>

namespace laszip {
#define __make_exception_class(name,msg) \
	struct name : public std::runtime_error {  name() : std::runtime_error(msg) {} }

	__make_exception_class(file_not_found, "The specified file was not found");
	__make_exception_class(invalid_magic, "File magic is not valid");
	__make_exception_class(old_style_compression, "The file seems to have old style compression which is not supported");
	__make_exception_class(not_compressed, "The file doesn't seem to be compressed");

	__make_exception_class(invalid_header_request, "Cannot request for headers while the file state in invalid");

	__make_exception_class(no_laszip_vlr, "No LASzip VLR was found in the VLRs section");
	__make_exception_class(laszip_format_unsupported, "Only LASzip POINTWISE CHUNKED decompressor is supported");

	__make_exception_class(chunk_table_read_error, "There was a problem reading the chunk table");
	__make_exception_class(unknown_chunk_table_format, "The chunk table version number is unknown");

	__make_exception_class(unknown_schema_type, "The LAZ schema is not recognized");
	__make_exception_class(unknown_record_item_type, "The record item type is not supported");

	__make_exception_class(write_open_failed, "Could not open file for writing");

	__make_exception_class(end_of_file, "Reached End of file");


	struct not_supported : public std::runtime_error {
		not_supported(const char *msg) : std::runtime_error(msg) { }
	};
}

#endif // __excepts_hpp__
