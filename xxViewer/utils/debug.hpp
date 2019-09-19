/*	This file is part of VirtualAA2, which is free software and is licensed
 * under the terms of the GNU GPL v3.0. (see http://www.gnu.org/licenses/ ) */
#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <stdexcept>
#include <string>

template<typename T1, typename T2>
void assert_equals( const T1& a, const T2& b, const char* exp, const char* file, int line ){
#ifndef NDEBUG
	if( !(a == b) )
		throw std::logic_error( exp + std::string(" (") + std::to_string(a) + ") != " + std::to_string( b ) + " @ " + file + ":" + std::to_string(line) );
#endif
}

inline void always( bool always_true, const char* msg ){
	if( !always_true )
		throw std::runtime_error( msg );
}

#define assert_eq(a,b) { assert_equals( a, b, #a , __FILE__ , __LINE__ ); }
#define require( exp ) { always( exp , #exp ); }


#endif
