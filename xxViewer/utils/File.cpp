/*	This file is part of VirtualAA2, which is free software and is licensed
 * under the terms of the GNU GPL v3.0. (see http://www.gnu.org/licenses/ ) */

#include "File.hpp"

File::File( const char* filepath, const char* flags )
	:	FileOperations( std::fopen( filepath, flags ) ) { }
File::File( const wchar_t* filepath, const wchar_t* flags )
	:	FileOperations( _wfopen( filepath, flags ) ) { }
