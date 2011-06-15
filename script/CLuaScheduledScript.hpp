#ifndef C_LUA_SCHEDULED_SCRIPT
#define C_LUA_SCHEDULED_SCRIPT

#include "CLuaScript.hpp"

/** class definition so pointers to the class can be used **/
class CWorld;

class CLuaScheduledScript : public CLuaScript
{
    public:
        
        /**
        *standard constructor which loads the script and adds basic functions
        *@see CLuaScript
        *@param filename the name of the script which should be loaded
        *@param world a pointer to the current gameworld
        */
        CLuaScheduledScript(std::string filename) throw(ScriptException);
        
        /**
        *virtual destructor
        */
        virtual ~CLuaScheduledScript() throw();
        
        /**
        *a function which calls a special function  inside the script
        *@param name the name of the function which should be called (the current cycle is given to that lua function)
        *@param currentCycle the current cycle 
        *@param lastCycle the last Cycle when the script was called
        *@param nextCycle the next Cycle when the script is called
        */
        void callFunction(std::string name, uint32_t currentCycle, uint32_t lastCycle, uint32_t nextCycle);
        
    private:
        
        //bool m_dataOk; /**< if true all loading was successfully otherwise false */
        CLuaScheduledScript(const CLuaScheduledScript&);
	CLuaScheduledScript& operator=(CLuaScheduledScript&);
};
#endif        
