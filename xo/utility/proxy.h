#pragma once

#include "xo/system/assert.h"
#include "xo/container/indexed_set.h"

namespace xo
{
	template< typename T, typename I = uint32_t >
	struct proxy_data {
		static const T& get_value( I index ) { xo_error( "proxy_data not implemented" ); }
		static I get_index( const T& value ) { xo_error( "proxy_data not implemented" ); }
	};

	template< typename T, typename I = uint32_t >
	struct proxy
	{
		using value_t = typename T;
		using index_t = typename I;

		proxy() : index_( typename proxy_data< T, I >::get_index( value_t() ) ) {}
		proxy( const value_t& value ) : index_( typename proxy_data< T, I >::get_index( value ) ) {}

		proxy& set( const value_t& value ) { index_ = typename proxy_data< T, I >::get_index( value ); return *this; }
		proxy& operator=( const value_t& value ) { return set( value ); }

		const value_t& get() const { return typename proxy_data< T, I >::get_value( index_ ); }
		operator const value_t&() const { return get(); }

		friend bool operator==( const proxy a, const proxy b ) { return a.index_ == b.index_; }
		friend bool operator!=( const proxy a, const proxy b ) { return a.index_ != b.index_; }

	private:
		index_t index_;
	};

	template< typename T, typename I > prop_node to_prop_node( const proxy<T, I>& q ) {
		return to_prop_node( q.get() );
	}

	template< typename T, typename I > bool from_prop_node( const prop_node& pn, proxy<T, I>& q ) {
		if ( T value; from_prop_node( pn, value ) )
		{ q.set( value ); return true; }
		else return false;
	}
}