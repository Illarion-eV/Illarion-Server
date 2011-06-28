//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#ifndef COMMAND_HPP
#define COMMAND_HPP

//! class mapping from a function to a Command functor used in the command map
class Command : public CommandType {

	public:

		Command( bool( World::*fun )( Player* ) ) : first(true), arguments(1), is_void(false), fun1_1(fun) {}

		Command( void( World::*fun )( Player* ) ) : first(true), arguments(1), is_void(true), fun1_1_v(fun) {}

		Command( bool( World::*fun )( const std::string& ) ) : first(false), arguments(1), is_void(false), fun1_2(fun) {}

		Command( void( World::*fun )( const std::string& ) ) : first(false), arguments(1), is_void(true), fun1_2_v(fun) {}

		Command( bool( World::*fun )( Player*, const std::string& ) ) : arguments(2), is_void(false), fun2(fun) {}

		Command( void( World::*fun )( Player*, const std::string& ) ) : arguments(2), is_void(true), fun2_v(fun) {}

		Command( bool( World::*fun )() ) : arguments(0), is_void(false), fun0(fun) {}

		Command( void( World::*fun )() ) : arguments(0), is_void(true), fun0_v(fun) {}

		Command( bool( *fun )( World*, Player*, const std::string& ) ) : arguments(3), is_void(false), fun3(fun) {}

		Command( void( *fun )( World*, Player*, const std::string& ) ) : arguments(3), is_void(true), fun3_v(fun) {}

		bool operator()( World* world, Player* cp, const std::string& text ) {
			switch (arguments) {
				case 0:
					if (is_void) {
						( world->*fun0_v )();
						return true;
					} else
						return ( world->*fun0 )();
					break;
				case 1:
					if ( first )
						if (is_void) {
							( world->*fun1_1_v )( cp );
							return true;
						} else return ( world->*fun1_1 )( cp );
					else
						if (is_void) {
							( world->*fun1_2_v )( text );
							return true;
						} else return ( world->*fun1_2 )( text );
					break;
				case 2:
					if (is_void) {
						( world->*fun2_v )( cp, text );
						return true;
					} else return ( world->*fun2 )( cp, text );
					break;
				case 3:
					if (is_void) {
						fun3_v( world, cp, text );
						return true;
					} else return fun3( world, cp, text );
					break;
			}
			return false;
		}

		virtual ~Command() {}

	private:

		bool first;
		char arguments;
		bool is_void;

		bool( World::*fun0 )();
		void( World::*fun0_v )();
		bool( World::*fun1_1 )( Player* );
		void( World::*fun1_1_v)( Player* );
		bool( World::*fun1_2 )( const std::string& );
		void( World::*fun1_2_v)( const std::string& );
		bool( World::*fun2 )( Player*, const std::string& );
		void( World::*fun2_v )( Player*, const std::string& );
		bool( *fun3 )( World*, Player*, const std::string& );
		void( *fun3_v )( World*, Player*, const std::string& );

};

#endif
