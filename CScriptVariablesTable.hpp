#ifndef _CSCRIPTVARIABLESTABLE_HPP_
#define _CSCRIPTVARIABLESTABLE_HPP_

#include <map>
#include <string>
#include <luabind/object.hpp>

class CScriptVariablesTable
{
    public:
    
        CScriptVariablesTable();
        ~CScriptVariablesTable();
        
        /**
        * looks for a value in the scriptvariables table
        * @param id of the value which should be found
        * @param ret byref returnes the value which was stored to id
        * @return true if there is a value with the id otherwise false
        */
        bool find(std::string id, std::string &ret );        
        
        /**
        * adds a new value to the scriptvariables table or changes it if it exists
        * @param id of the value which should be changed or added
        * @param value the value of the new value which should be added
        */
        void set(std::string id, luabind::object value);
        
        /**
        * deletes a value from the scriptvariables table
        * @param id of the value which should be deleted
        * @return true if there was such a id to delete otherwise false
        */
        bool remove(std::string id );
        
        /**
        * saves the current table
        */
        void save();
        
        bool isDataOk(){ return m_dataOK; }
        
    protected:
    private:
    
        /**
        * an equalation funktion for the Stringtable
        */
		struct ltstr 
        {
            /**
            * overloading of the () operator to see if two strings have the same structure
            */
			bool operator()( const char* s1, const char* s2 ) const 
            {
				return strcmp( s1, s2 ) < 0;
			}
		};
        
        typedef std::map <const char*,std::string, ltstr> STRINGTABLE;
        STRINGTABLE values_table;
        
        /**
        * deletes all data in the table
        */
        void clearOldTable();
        
        /**
        * reloads the data
        */
        void reload();
        
        bool m_dataOK;
};

#endif
