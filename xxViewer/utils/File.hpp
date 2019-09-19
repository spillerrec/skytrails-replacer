/*	This file is part of VirtualAA2, which is free software and is licensed
 * under the terms of the GNU GPL v3.0. (see http://www.gnu.org/licenses/ ) */
#ifndef FILE_HPP
#define FILE_HPP

#include "ArrayView.hpp"
#include "Buffer.hpp"

#include <cstdio>

/** Non-owning File reading/writing abstraction */
class FileOperations{
	protected:
		FILE* handle;
	
	public:
		FileOperations( FILE* handle ) : handle(handle) { } //TODO: throw on (handle == nullptr)
		
		template<typename T>
		auto read( ArrayView<T> view )
			{ return fread( view.begin(), sizeof(T), view.size(), handle ); }
		
		auto read( Buffer& buf ){ return read( buf.view() ); }
		
		auto seek( long int offset, int origin )
			{ return fseek( handle, offset, origin ); }
		
		auto tell(){ return ftell( handle ); }
		
		Buffer read( size_t bytes ){
			Buffer buf( bytes );
			read( buf );
			return buf;
		}
		
		Buffer readAll() {
			auto current = tell();
			seek( 0, SEEK_END );
			auto end = tell();
			seek( current, 0 );
			return read( end - current );
		}
		
		template<typename T>
		auto write( ArrayView<T> view )
			{ return fwrite( view.begin(), sizeof(T), view.size(), handle ); }
		
		auto write( const Buffer& buffer ){ return write( buffer.view() ); }
		auto write8u(  uint8_t  value ){ return write( ArrayView<uint8_t >( &value, 1 ) ); }
		auto write32u( uint32_t value ){ return write( ArrayView<uint32_t>( &value, 1 ) ); }
		auto write64u( uint64_t value ){ return write( ArrayView<uint64_t>( &value, 1 ) ); }
};

/** Owning File reading/writing */
class File : public FileOperations{
	public:
		File( const char* filepath, const char* flags="rb" );
		File( const wchar_t* filepath, const wchar_t* flags=L"rb" );
		~File(){ std::fclose( handle ); }
};

#endif
