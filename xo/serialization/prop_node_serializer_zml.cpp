#include "prop_node_serializer_zml.h"

#include <fstream>

namespace xo
{
	string get_zml_token( char_stream& str )
	{
		while ( true )
		{
			string t = str.get_token( "={}[];", "\"'" );
			if ( t == ";" || t == "#" )
			{
				// comment: skip rest of line
				str.get_line();
				continue;
			}
			else return trim_str( t );
		}
	}

	void read_zml_layer( char_stream& str, prop_node& parent, const string& close, error_code* ec, const path& folder )
	{
		prop_node merge_pn;
		for ( string t = get_zml_token( str ); t != close; t = get_zml_token( str ) )
		{
			if ( t.empty() ) // check if the stream has ended while expecting a close tag
				return set_error_or_throw( ec, "Error parsing ZML: unexpected end of stream" );

			// check directive statement
			if ( t[ 0 ] == '#' )
			{
				if ( t == "#include" )
					parent.append( load_zml( folder / path( get_zml_token( str ) ), ec ) );
				else if ( t == "#merge" )
					merge_pn.merge( load_zml( folder / path( get_zml_token( str ) ), ec ) );
				else return set_error_or_throw( ec, "Unknown directive: " + t );
			}
			else
			{
				// check if we're reading an array or if this is a label
				if ( close != "]" )
				{
					// read label
					if ( !isalpha( t[ 0 ] ) )
						return set_error_or_throw( ec, "Error parsing ZML: invalid label " + t );
					parent.push_back( t );

					// read =
					t = get_zml_token( str );
					if ( t == "=" )
						t = get_zml_token( str );
					else if ( t != "{" && t != "[" )
						return set_error_or_throw( ec, "Error parsing ZML: expected '=', '{' or '['" );
				}
				else parent.push_back( "" ); // add array child

											 // read value
				if ( t == "{" ) // new group
					read_zml_layer( str, parent.back().second, "}", ec, folder );
				else if ( t == "[" ) // new array
					read_zml_layer( str, parent.back().second, "]", ec, folder );
				else // just a value
					parent.back().second.set_value( std::move( t ) );
			}
		}

		// add children from #merge directive
		parent.merge( merge_pn, false );
	}

	prop_node parse_zml( char_stream& str, error_code* ec, const path& folder )
	{
		prop_node root;
		read_zml_layer( str, root, "", ec, folder );
		return root;
	}

	prop_node parse_zml( const char* str, error_code* ec )
	{
		return parse_zml( char_stream( str, "\n\r\t\v " ), ec, path() );
	}

	void write_zml_node( std::ostream& str, const string& label, const prop_node& pn, int level )
	{
		auto indent = string( level, '\t' );

		// = and value
		if ( !label.empty() )
		{
			str << indent << try_quoted( label, ";{}[]#" );
			str << " = " << ( pn.empty() ? "\"\"" : try_quoted( pn.get_value(), ";{}[]#" ) );
		}

		// check if this is an array
		bool is_array = find_if( pn, [&]( const prop_node::pair_t& n ) { return !n.first.empty(); } ) == pn.end();
		auto depth = pn.count_layers();
		string separate = depth == 1 ? " " : "\n" + indent;

		if ( depth == 0 )
		{
			str << std::endl;
		}
		else if ( is_array && depth > 0 )
		{
			str << "[" << separate;
			for ( auto& node : pn )
				str << try_quoted( node.second.get_value(), ";{}[]" ) << separate;
			str << "]" << std::endl;
		}
		else if ( depth == 1 && pn.size() <= 4 )
		{
			str << "{" << separate;
			for ( auto& node : pn )
				str << node.first << " = " << try_quoted( node.second.get_value(), ";{}[]" ) << separate;
			str << "}" << std::endl;
		}
		else
		{
			// multi-line children
			str << "{" << std::endl;
			for ( auto& node : pn )
				write_zml_node( str, node.first, node.second, level + 1 );
			str << indent << "}" << std::endl;
		}
	}

	xo::prop_node prop_node_serializer_zml::read_stream( std::istream& str, error_code* ec, path parent_folder )
	{
		// TODO: more efficient. parser should be able to take any stream type.
		std::string file_contents( std::istreambuf_iterator<char>( str ), {} );
		return parse_zml( char_stream( file_contents.c_str(), "\n\r\t\v " ), ec, parent_folder );
	}

	std::ostream& prop_node_serializer_zml::write_stream( std::ostream& str, const prop_node& pn, error_code* ec )
	{
		for ( auto& node : pn )
			write_zml_node( str, node.first, node.second, 0 );
		return str;
	}

	XO_API prop_node load_zml( const path& filename, error_code* ec, path parent_folder )
	{
		std::ifstream str( filename.c_str() );
		if ( str )
			return prop_node_serializer_zml().read_stream( str, ec, filename.parent_path() );
		else return set_error_or_throw( ec, "Could not open file: " + filename.string() ), prop_node();
	}
}