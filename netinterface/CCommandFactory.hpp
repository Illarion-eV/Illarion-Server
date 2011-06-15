#ifndef _CCOMMANDFACTORY_HPP_
#define _CCOMMANDFACTORY_HPP_

#if __GNUC__ < 3
#include <hash_map>
#else
#include <ext/hash_map>

#if (__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
using __gnu_cxx::hash_map;
#endif

#if (__GNUC__ == 3 && __GNUC_MINOR__ < 1)
using std::hash_map;
#endif

#endif

#include <boost/shared_ptr.hpp>

class CBasicClientCommand;

/**
*factory class which holds templates of BasicServerCommand classes 
*an returns an empty command given by an id
*/
class CCommandFactory
{
    public:

    CCommandFactory();
    ~CCommandFactory();
    
    /**
    *returns a pointer to an emtpy Server Command
    *@param commandId the id of the command which we want to use
    *@return a pointer to an empty command with the given commandId
    */
    boost::shared_ptr<CBasicClientCommand> getCommand( unsigned char commandId );
    
    private:
    
    typedef hash_map< unsigned char, CBasicClientCommand*> COMMANDLIST;
    COMMANDLIST templateList; /*<the list which holds the templates for the concrete classes*/

};

#endif
