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


#include "netinterface/CommandFactory.hpp"
#include "netinterface/protocol/ClientCommands.hpp"
#include "netinterface/protocol/BBIWIClientCommands.hpp"

CCommandFactory::CCommandFactory()
{
    //TODO add the commands
    templateList[ CC_LOGIN_TS ] = new CLoginCommandTS();
    templateList[ CC_SCREENSIZE_TS ] = new CScreenSizeCommandTS();
    templateList[ CC_LOOKATMAPITEM_TS ] = new CLookAtMapItemTS();
    templateList[ CC_USE_TS ] = new CUseTS();
    templateList[ CC_CAST_TS ] = new CCastTS();
    templateList[ CC_ATTACKPLAYER_TS ] = new CAttackPlayerTS();
    templateList[ CC_INTRODUCE_TS ] = new CIntroduceTS();
    templateList[ CC_SAY_TS ] = new CSayTS();
    templateList[ CC_SHOUT_TS ] = new CShoutTS();
    templateList[ CC_WHISPER_TS ] = new CWhisperTS();
    templateList[ CC_REFRESH_TS ] = new CRefreshTS();
    templateList[ CC_LOGOUT_TS ] = new CLogOutTS();
    templateList[ CC_LOOKINTOCONTAINERONFIELD_TS ] = new CLookIntoContainerOnFieldTS();
    templateList[ CC_LOOKINTOINVENTORY_TS ] = new CLookIntoInventoryTS();
    templateList[ CC_LOOKINTOSHOWCASECONTAINER_TS ] = new CLookIntoShowCaseContainerTS();
    templateList[ CC_CLOSECONTAINERINSHOWCASE_TS ] = new CCloseContainerInShowCaseTS();
    templateList[ CC_DROPITEMFROMSHOWCASEONMAP_TS ] = new CDropItemFromShowCaseOnMapTS();
    templateList[ CC_MOVEITEMBETWEENSHOWCASES_TS ] = new CMoveItemBetweenShowCasesTS();
    templateList[ CC_MOVEITEMFROMMAPINTOSHOWCASE_TS ] = new CMoveItemFromMapIntoShowCaseTS();
    templateList[ CC_MOVEITEMFROMMAPTOPLAYER_TS ] = new CMoveItemFromMapToPlayerTS();
    templateList[ CC_DROPITEMFROMPLAYERONMAP_TS ] = new CDropItemFromInventoryOnMapTS();
    templateList[ CC_MOVEITEMINSIDEINVENTORY_TS ] = new CMoveItemInsideInventoryTS();
    templateList[ CC_MOVEITEMFROMSHOWCASETOPLAYER_TS ] = new CMoveItemFromShowCaseToPlayerTS();
    templateList[ CC_MOVEITEMFROMPLAYERTOSHOWCASE_TS ] = new CMoveItemFromPlayerToShowCaseTS();
    templateList[ CC_LOOKATSHOWCASEITEM_TS ] = new CLookAtShowCaseItemTS();
    templateList[ CC_LOOKATINVENTORYITEM_TS ] = new CLookAtInventoryItemTS();
    templateList[ CC_ATTACKSTOP_TS ] = new CAttackStopTS();
    templateList[ CC_REQUESTSKILLS_TS ] = new CRequestSkillsTS();
    templateList[ CC_LOOKATMENUITEM_TS ] = new CLookAtMenuItemTS();
    templateList[ CC_KEEPALIVE_TS ] = new CKeepAliveTS();
    templateList[ BB_KEEPALIVE_TS ] = new CBBKeepAliveTS();
    templateList[ BB_BROADCAST_TS ] = new CBBBroadCastTS();
    templateList[ BB_DISCONNECT_TS ] = new CBBDisconnectTS();
    templateList[ BB_BAN_TS ] = new CBBBanTS();
    templateList[ BB_TALKTO_TS ] = new CBBTalktoTS();
    templateList[ BB_CHANGEATTRIB_TS ] = new CBBChangeAttribTS();
    templateList[ BB_CHANGESKILL_TS ] = new CBBChangeSkillTS();
    templateList[ BB_SERVERCOMMAND_TS ] = new CBBServerCommandTS();
    templateList[ BB_WARPPLAYER_TS ] = new CBBWarpPlayerTS();
    templateList[ BB_SPEAKAS_TS ] = new CBBSpeakAsTS();
    templateList[ BB_REQUESTSTATS_TS ] = new CBBRequestStatTS();
    templateList[ BB_REQUESTSKILLS_TS ] = new CBBRequestSkillsTS();
    for ( int i = CC_IMOVERSTART_TS; i <= CC_IMOVEREND_TS; ++i)templateList[i] = new CIMoverActionTS( i - CC_IMOVERSTART_TS );
    templateList[ CC_CHARMOVE_TS ] = new CCharMoveTS();
    for ( int i = CC_PSPINRSTART_TS; i <= CC_PSPINREND_TS; ++i)templateList[i] = new CPSpinActionTS( i - CC_PSPINRSTART_TS );
    templateList[ CC_LOOKATCHARACTER_TS ] = new CLookAtCharacterTS();
    templateList[ CC_REQUESTAPPEARANCE_TS ] = new CRequestAppearanceTS();

}


CCommandFactory::~CCommandFactory()
{
    templateList.clear();
}

boost::shared_ptr<CBasicClientCommand>  CCommandFactory::getCommand(unsigned char commandId )
{
    COMMANDLIST::iterator it;
    it = templateList.find( commandId );
    if ( it != templateList.end() )
    {
        CBasicClientCommand * cmd;
        cmd = it->second;
        return cmd->clone();
    }
    return boost::shared_ptr<CBasicClientCommand>();
}
    
