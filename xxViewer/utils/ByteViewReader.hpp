/*	This file is part of VirtualAA2, which is free software and is licensed
 * under the terms of the GNU GPL v3.0. (see http://www.gnu.org/licenses/ ) */
#ifndef BUFFER_READER_HPP
#define BUFFER_READER_HPP

#include "ArrayView.hpp"
#include "Buffer.hpp"
#include "debug.hpp"
#include <iostream>
#include <stdexcept>

inline uint32_t convert32unsigned( uint8_t a, uint8_t b, uint8_t c, uint8_t d )
	{ return a + (b<<8) + (c<<16) + (d<<24); }

inline uint64_t convert64unsigned( uint32_t a, uint32_t b )
	{ return a + (uint64_t(b)<<32); }

class ByteViewReader{
	private:
		ByteView buffer;
		size_t position{ 0 };
		
	public:
		ByteViewReader( ByteView view ) : buffer(view) { }
		ByteViewReader( Buffer& buf ) : buffer(buf.view()) { }
		
		auto tell() const{ return position; }
		auto left() const{ return buffer.size() - position; }
		
		bool seek( size_t new_position ){
			position = new_position;
			return left() > 0;
		}
		
		ByteView makeView( unsigned from, unsigned amount ){
			require( from+amount <= buffer.size() );
			return ByteView( buffer.begin() + from, amount );
		}
		
		bool findNextMatch(ConstByteView str){
			while(left() > str.size()){
				if (buffer.subView(position, str.size()) == str)
					return true;
				read8u();
			}
			
			return false; //Nothing found
		}
		
		ByteView read( unsigned amount ){
			if( amount > left() )
				throw std::runtime_error( "ByteViewReader: Ran out of file" );
			position += amount;
			return makeView( position - amount, amount );
		}
		
		uint8_t read8u(){
			require( left() >= 1 );
			return buffer[position++];
		}
		
		uint16_t read16u(){
			auto b = read( 2 );
			return (b[1] << 8) + b[0];
		}
		
		uint32_t read32u(){
			auto b = read( 4 );
			return convert32unsigned( b[0], b[1], b[2], b[3] );
		}
		
		uint64_t read64u(){
			auto a = read32u();
			auto b = read32u();
			return convert64unsigned( a, b );
		}
		
		ByteView readName(){
			auto lenght = read32u();
			return { read( lenght ) };
		}
		
		template<typename T>
		void readData( T* object, unsigned count ){
			auto data = read( sizeof(T) * count );
			std::copy( data.begin(), data.end(), reinterpret_cast<uint8_t*>( object ) );
		}
		
		template<typename T>
		void readStruct( T& object ){
			auto data = read( sizeof(object) );
			std::copy( data.begin(), data.end(), reinterpret_cast<uint8_t*>( &object ) );
		}
		
		template<typename T>
		void readVector( std::vector<T>& vec, int amount ){
			auto data = read( sizeof(T) * amount );
			vec.resize( amount );
			std::copy( data.begin(), data.end(), reinterpret_cast<uint8_t*>( vec.data() ) );
		}
		
		float readFloat(){
			float out;
			auto buffer = read( 4 );
			std::copy( buffer.begin(), buffer.end(), reinterpret_cast<uint8_t*>( &out ) );
			return out;
		}
		
		double readDouble(){
			double out;
			auto buffer = read( 8 );
			std::copy( buffer.begin(), buffer.end(), reinterpret_cast<uint8_t*>( &out ) );
			return out;
		}
		
};

#endif
