#include "stopwatch.h"

#include "xo/string/string_tools.h"
#include "xo/container/prop_node.h"

namespace xo
{
	prop_node stopwatch::get_report( int decimals )
	{
		auto old_precision = set_to_str_precision( decimals );
		prop_node pn;
		for ( index_t idx = 0; idx < split_count(); ++idx )
			pn.add_key_value( split_names_[ idx ], split_times_[ idx ] );
		set_to_str_precision( old_precision );

		return pn;
	}
}
