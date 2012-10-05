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

CommandFactory::CommandFactory() {
    //TODO add the commands
    templateList[ C_MESSAGEDIALOG_TS ] = new MessageDialogTS();
    templateList[ C_INPUTDIALOG_TS ] = new InputDialogTS();
    templateList[ C_MERCHANTDIALOG_TS ] = new MerchantDialogTS();
    templateList[ C_SELECTIONDIALOG_TS ] = new SelectionDialogTS();
    templateList[ C_CRAFTINGDIALOG_TS ] = new CraftingDialogTS();
    templateList[ C_LOGIN_TS ] = new LoginCommandTS();
    templateList[ C_SCREENSIZE_TS ] = new ScreenSizeCommandTS();
    templateList[ C_LOOKATMAPITEM_TS ] = new LookAtMapItemTS();
    templateList[ C_USE_TS ] = new UseTS();
    templateList[ C_CAST_TS ] = new CastTS();
    templateList[ C_ATTACKPLAYER_TS ] = new AttackPlayerTS();
    templateList[ C_INTRODUCE_TS ] = new IntroduceTS();
    templateList[ C_SAY_TS ] = new SayTS();
    templateList[ C_SHOUT_TS ] = new ShoutTS();
    templateList[ C_WHISPER_TS ] = new WhisperTS();
    templateList[ C_REFRESH_TS ] = new RefreshTS();
    templateList[ C_LOGOUT_TS ] = new LogOutTS();
    templateList[ C_LOOKINTOCONTAINERONFIELD_TS ] = new LookIntoContainerOnFieldTS();
    templateList[ C_LOOKINTOINVENTORY_TS ] = new LookIntoInventoryTS();
    templateList[ C_LOOKINTOSHOWCASECONTAINER_TS ] = new LookIntoShowCaseContainerTS();
    templateList[ C_CLOSECONTAINERINSHOWCASE_TS ] = new CloseContainerInShowCaseTS();
    templateList[ C_DROPITEMFROMSHOWCASEONMAP_TS ] = new DropItemFromShowCaseOnMapTS();
    templateList[ C_MOVEITEMBETWEENSHOWCASES_TS ] = new MoveItemBetweenShowCasesTS();
    templateList[ C_MOVEITEMFROMMAPINTOSHOWCASE_TS ] = new MoveItemFromMapIntoShowCaseTS();
    templateList[ C_MOVEITEMFROMMAPTOPLAYER_TS ] = new MoveItemFromMapToPlayerTS();
    templateList[ C_DROPITEMFROMPLAYERONMAP_TS ] = new DropItemFromInventoryOnMapTS();
    templateList[ C_MOVEITEMINSIDEINVENTORY_TS ] = new MoveItemInsideInventoryTS();
    templateList[ C_MOVEITEMFROMSHOWCASETOPLAYER_TS ] = new MoveItemFromShowCaseToPlayerTS();
    templateList[ C_MOVEITEMFROMPLAYERTOSHOWCASE_TS ] = new MoveItemFromPlayerToShowCaseTS();
    templateList[ C_LOOKATSHOWCASEITEM_TS ] = new LookAtShowCaseItemTS();
    templateList[ C_LOOKATINVENTORYITEM_TS ] = new LookAtInventoryItemTS();
    templateList[ C_ATTACKSTOP_TS ] = new AttackStopTS();
    templateList[ C_REQUESTSKILLS_TS ] = new RequestSkillsTS();
    templateList[ C_KEEPALIVE_TS ] = new KeepAliveTS();
    templateList[ BB_KEEPALIVE_TS ] = new BBKeepAliveTS();
    templateList[ BB_BROADCAST_TS ] = new BBBroadCastTS();
    templateList[ BB_DISCONNECT_TS ] = new BBDisconnectTS();
    templateList[ BB_BAN_TS ] = new BBBanTS();
    templateList[ BB_TALKTO_TS ] = new BBTalktoTS();
    templateList[ BB_CHANGEATTRIB_TS ] = new BBChangeAttribTS();
    templateList[ BB_CHANGESKILL_TS ] = new BBChangeSkillTS();
    templateList[ BB_SERVERCOMMAND_TS ] = new BBServerCommandTS();
    templateList[ BB_WARPPLAYER_TS ] = new BBWarpPlayerTS();
    templateList[ BB_SPEAKAS_TS ] = new BBSpeakAsTS();
    templateList[ BB_REQUESTSTATS_TS ] = new BBRequestStatTS();
    templateList[ BB_REQUESTSKILLS_TS ] = new BBRequestSkillsTS();

    for (int i = C_IMOVERSTART_TS; i <= C_IMOVEREND_TS; ++i) {
        templateList[i] = new IMoverActionTS(i - C_IMOVERSTART_TS);
    }

    templateList[ C_CHARMOVE_TS ] = new CharMoveTS();

    for (int i = C_PSPINRSTART_TS; i <= C_PSPINREND_TS; ++i) {
        templateList[i] = new PSpinActionTS(i - C_PSPINRSTART_TS);
    }

    templateList[ C_LOOKATCHARACTER_TS ] = new LookAtCharacterTS();
    templateList[ C_REQUESTAPPEARANCE_TS ] = new RequestAppearanceTS();

}


CommandFactory::~CommandFactory() {
    templateList.clear();
}

boost::shared_ptr<BasicClientCommand>  CommandFactory::getCommand(unsigned char commandId) {
    OMMANDLIST::iterator it;
    it = templateList.find(commandId);

    if (it != templateList.end()) {
        BasicClientCommand *cmd;
        cmd = it->second;
        return cmd->clone();
    }

    return boost::shared_ptr<BasicClientCommand>();
}

