/*	This file is part of VirtualAA2, which is free software and is licensed
 * under the terms of the GNU GPL v3.0. (see http://www.gnu.org/licenses/ ) */
#ifndef BUFFER_HPP
#define BUFFER_HPP

#include "ArrayView.hpp"
#include <memory>

/** Fixed size array of uint8_t */
class Buffer{
	private:
		std::unique_ptr<uint8_t[]> buffer;
		size_t length;
		
	public:
		/// @return pointer to first element of the array
		      auto data()      { return buffer.get(); }
		const auto data() const{ return buffer.get(); }
		
		/// @return Length of the array in bytes
		auto size() const{ return length; }
		
		auto begin(){ return buffer.get(); }
		auto end(){ return buffer.get() + length; }
		
		uint8_t& operator[]( int index )       { return buffer[index]; }
		uint8_t& operator[]( int index ) const { return buffer[index]; }
		
		/// @return A mutable ArrayView of this array
		ArrayView<      uint8_t> view()      { return { data(), size() }; }
		
		/// @return An immutable ArrayView of this array
		ArrayView<const uint8_t> constView() const{ return { data(), size() }; }
		
		/// @return An immutable ArrayView of this array
		ArrayView<const uint8_t> view() const{ return constView(); }
		
	public:
		Buffer() = default;
		Buffer( size_t length ) : buffer( std::make_unique<uint8_t[]>( length ) ), length(length) { }
		
		/** Create array by copying the data
		 *  @param data The data to copy from
		 *  @param length The amount of bytes to copy from data */
		explicit Buffer( const uint8_t* data, size_t length ) : Buffer( length )
			{ std::copy( data, data+length, buffer.get() ); }
		
		/** Initialize from ByteView
		 *  @param view ByteView to copy from */
		explicit Buffer(      ByteView view ) : Buffer( view.begin(), view.size() ) { }
		explicit Buffer( ConstByteView view ) : Buffer( view.begin(), view.size() ) { }
		
		Buffer( Buffer&& other ) : buffer(std::move(other.buffer)), length(other.length)
			{ other.length = 0; }
		explicit Buffer( const Buffer& copy ) : Buffer( copy.view() ) { }
		Buffer& operator=( Buffer&& other ){
			if( &other != this ){
				buffer = std::move(other.buffer);
				length = other.length;
				other.length = 0;
			}
			return *this;
		}
};

#endif
