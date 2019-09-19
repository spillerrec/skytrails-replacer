/*	This file is part of VirtualAA2, which is free software and is licensed
 * under the terms of the GNU GPL v3.0. (see http://www.gnu.org/licenses/ ) */

#include "ArrayView.hpp"

#include "Buffer.hpp"
#include "debug.hpp"

std::string NotByteView::toString() const{
	Buffer buf( size() );
	for( unsigned i=0; i<size(); i++ )
		buf[i] = (*this)[i];
	assert_eq( buf[size()-1], 0 );
	return std::string( (char*)buf.data(), buf.size() );
}
