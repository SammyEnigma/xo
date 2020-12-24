#pragma once

namespace xo
{
	template< typename ValueT, typename TimeT >
	struct cached_var
	{
		cached_var( ValueT init_val, TimeT init_time ) : value_( init_val ), time_( init_time ) {}

		template< typename Fun >
		const ValueT& operator()( const TimeT& t, const Fun& f ) {
			if ( t != time_ ) {
				value_ = f();
				time_ = t;
			}
			return value_;
		}
	private:
		ValueT value_;
		TimeT time_;
	};
}
