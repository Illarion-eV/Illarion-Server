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


#ifndef CCOMMAND_HPP
#define CCOMMAND_HPP

//! class mapping from a function to a Command functor used in the command map
class CCommand : public CCommandType {

	public:

		CCommand( bool( CWorld::*fun )( CPlayer* ) ) : first(true), arguments(1), is_void(false), fun1_1(fun) {}

		CCommand( void( CWorld::*fun )( CPlayer* ) ) : first(true), arguments(1), is_void(true), fun1_1_v(fun) {}

		CCommand( bool( CWorld::*fun )( const std::string& ) ) : first(false), arguments(1), is_void(false), fun1_2(fun) {}

		CCommand( void( CWorld::*fun )( const std::string& ) ) : first(false), arguments(1), is_void(true), fun1_2_v(fun) {}

		CCommand( bool( CWorld::*fun )( CPlayer*, const std::string& ) ) : arguments(2), is_void(false), fun2(fun) {}

		CCommand( void( CWorld::*fun )( CPlayer*, const std::string& ) ) : arguments(2), is_void(true), fun2_v(fun) {}

		CCommand( bool( CWorld::*fun )() ) : arguments(0), is_void(false), fun0(fun) {}

		CCommand( void( CWorld::*fun )() ) : arguments(0), is_void(true), fun0_v(fun) {}

		CCommand( bool( *fun )( CWorld*, CPlayer*, const std::string& ) ) : arguments(3), is_void(false), fun3(fun) {}

		CCommand( void( *fun )( CWorld*, CPlayer*, const std::string& ) ) : arguments(3), is_void(true), fun3_v(fun) {}

		bool operator()( CWorld* world, CPlayer* cp, const std::string& text ) {
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

	private:

		bool first;
		char arguments;
		bool is_void;

		bool( CWorld::*fun0 )();
		void( CWorld::*fun0_v )();
		bool( CWorld::*fun1_1 )( CPlayer* );
		void( CWorld::*fun1_1_v)( CPlayer* );
		bool( CWorld::*fun1_2 )( const std::string& );
		void( CWorld::*fun1_2_v)( const std::string& );
		bool( CWorld::*fun2 )( CPlayer*, const std::string& );
		void( CWorld::*fun2_v )( CPlayer*, const std::string& );
		bool( *fun3 )( CWorld*, CPlayer*, const std::string& );
		void( *fun3_v )( CWorld*, CPlayer*, const std::string& );

};

#endif
