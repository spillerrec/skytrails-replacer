/*	This file is part of VirtualAA2, which is free software and is licensed
 * under the terms of the GNU GPL v3.0. (see http://www.gnu.org/licenses/ ) */

#include "StringView.hpp"


bool compareInsensitive( WStringView a, WStringView b ){
	if( a.size() != b.size() )
		return false;
	return std::equal( a.begin(), a.end(), b.begin()
		,	[]( auto a, auto b ){ return towlower(a) == towlower(b); }
		);
}
