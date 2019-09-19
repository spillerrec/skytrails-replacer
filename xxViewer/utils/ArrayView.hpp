/*	This file is part of VirtualAA2, which is free software and is licensed
 * under the terms of the GNU GPL v3.0. (see http://www.gnu.org/licenses/ ) */
#ifndef ARRAY_VIEW_HPP
#define ARRAY_VIEW_HPP

#include "debug.hpp"

#include <algorithm>
#include <memory>
#include <type_traits>
#include <stdint.h>
#include <string>


/** Access to a sequential area of memory of type T */
template<typename T>
class ArrayView{
	protected:
		T* data;
		size_t lenght;
		
	public:
		ArrayView() : data(nullptr), lenght(0) {} //TODO: remove?
		ArrayView( T* data, size_t lenght ) : data(data), lenght(lenght) { }
		
		using type = typename std::remove_cv<T>::type;
		
		/// @return The length of the view
		auto size() const{ return lenght; }
		
		/** Access an element in the view
		 *  @param index The index of the element
		 *  @return The element at index */
		T& operator[]( int index )       { return data[index]; } //TODO: Check in debug mode
		T  operator[]( int index ) const { return data[index]; } //TODO: Check in debug mode
		
		      T* begin()       { return data         ; }
		const T* begin() const { return data         ; }
		      T* end()         { return data + lenght; }
		const T* end()   const { return data + lenght; }
		
		template<typename T2>
		bool operator==( ArrayView<T2> other ) const{
			if( size() != other.size() )
				return false;
			
			return std::equal( begin(), end(), other.begin() );
		}
		
		template<typename T2>
		bool lexicographical_less( ArrayView<T2> other )
			{ return std::lexicographical_compare( begin(), end(), other.begin(), other.end() ); }
		
		/// @return This view copied to a std::basic_string
		std::basic_string<type> toBasicString() const{
			auto buf = std::make_unique<type[]>( size() );
			std::copy( begin(), end(), buf.get() );
			return std::basic_string<type>( buf.get(), size() );
		}
		
		/// @return true if this view starts with the contents of that
		bool startsWith( ArrayView<T> that ) const {
			if( that.size() > size() )
				return false;
			return std::equal( that.begin(), that.end(), begin() );
		}
		
		/// @return true if this view ends with the contents of that
		bool endsWith( ArrayView<T> that ) const {
			if( that.size() > size() )
				return false;
			return std::equal( that.begin(), that.end(), end()-that.size() );
		}
		
		/// @return An iterator to the first occurrence of needle, or end() if not found
		auto find( const T& needle ) const { return std::find( begin(), end(), needle ); }
		
		/// @return The index of the first occurrence of needle, or size() if not found
		auto findIndex( const T& needle ) const{ return find( needle ) - begin(); }
		
		ArrayView<T> subView( int start, size_t amount ){ return { data+start, amount }; } //TODO: check range in debug mode
		ArrayView<T> subView( int start, size_t amount ) const { return { data+start, amount }; } //TODO: check range in debug mode
		
		/// @return A new view containing 'amount' of elements from the start
		ArrayView<T> left(  size_t amount ){
			require( amount <= size() );
			return subView( 0, amount );
		}
		
		/// @return A new view containing 'amount' of elements from the end
		ArrayView<T> right( size_t amount ){
			require( amount <= size() );
			return subView( size()-amount, amount );
		}
		
		void copyTo( ArrayView<T> out ) const {
			for( size_t i=0; i<std::min(size(), out.size()); i++ )
				out[i] = (*this)[i];
		}
};

using      ByteView = ArrayView<      uint8_t>;
using ConstByteView = ArrayView<const uint8_t>;

inline ConstByteView makeConst( ByteView view )
	{ return ConstByteView( view.begin(), view.size() ); }

struct NotByteView : public ByteView {
	NotByteView() {}
	NotByteView( ByteView view ) : ByteView( view ) { }
	uint8_t  operator[]( int index ) const { return ~(data[index]); } //TODO: Check in debug mode
	
	std::string toString() const;
};



#endif
