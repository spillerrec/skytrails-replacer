/*	This file is part of VirtualAA2, which is free software and is licensed
 * under the terms of the GNU GPL v3.0. (see http://www.gnu.org/licenses/ ) */
#ifndef STRING_VIEW_HPP
#define STRING_VIEW_HPP

#include "ArrayView.hpp"

#include <algorithm>
#include <vector>

using  StringView = ArrayView<const char   >;
using WStringView = ArrayView<const wchar_t>;

inline StringView makeView( const char* null_terminated )
	{ return { null_terminated, std::char_traits<char>::length( null_terminated ) }; }
inline WStringView makeView( const wchar_t* null_terminated )
	{ return { null_terminated, std::char_traits<wchar_t>::length( null_terminated ) }; }
inline WStringView makeView( const std::wstring& str ) { return makeView( str.c_str() ); }

bool compareInsensitive( WStringView a, WStringView b );
	
template<typename T>
std::vector<ArrayView<T>> split( ArrayView<T> view, T split_on ){
	std::vector<ArrayView<T>> out;
	
	auto pos = view.begin();
	while( pos < view.end() ){
		auto new_pos = std::find( pos, view.end(), split_on );
		out.emplace_back( pos, new_pos - pos );
		pos = new_pos + 1;
	}
	
	return out;
}

inline bool shiftJisOnlySingleBytes( ConstByteView jis ){
	auto isSingleByte = [](auto c){ return !(c & ~0x7F); };
	return std::all_of( jis.begin(), jis.end(), isSingleByte );
}

#endif
