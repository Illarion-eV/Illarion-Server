#ifndef _CBASICCOMMAND_HPP_
#define _CBASICCOMMAND_HPP_

/**
*@ingroup Netinterface
*Basic class for commands which can be sent to a client or received by the server,
*holding a unique byte to identify the command.
*/
class CBasicCommand
{
    private:
        unsigned char definitionByte; /*<Unique command id*/
    public:
        /**
         *Constructor which sets the definition byte
         *\param defByte A unique command id
         */
        CBasicCommand(unsigned char defByte);

        /**
         *Provides read access to the definition byte
         *\return The unique command id
        */
        unsigned char getDefinitionByte(){ return definitionByte; };
};

#endif
