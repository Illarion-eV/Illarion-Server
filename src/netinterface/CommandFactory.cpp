//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#include "make_unique.hpp"
#include "netinterface/CommandFactory.hpp"
#include "netinterface/protocol/ClientCommands.hpp"
#include "netinterface/protocol/BBIWIClientCommands.hpp"

CommandFactory::CommandFactory() {
    templateList[C_MESSAGEDIALOG_TS ] = std::make_unique<MessageDialogTS>();
    templateList[C_INPUTDIALOG_TS ] = std::make_unique<InputDialogTS>();
    templateList[C_MERCHANTDIALOG_TS ] = std::make_unique<MerchantDialogTS>();
    templateList[C_SELECTIONDIALOG_TS ] = std::make_unique<SelectionDialogTS>();
    templateList[C_CRAFTINGDIALOG_TS ] = std::make_unique<CraftingDialogTS>();
    templateList[C_LOGIN_TS ] = std::make_unique<LoginCommandTS>();
    templateList[C_SCREENSIZE_TS ] = std::make_unique<ScreenSizeCommandTS>();
    templateList[C_LOOKATMAPITEM_TS ] = std::make_unique<LookAtMapItemTS>();
    templateList[C_USE_TS ] = std::make_unique<UseTS>();
    templateList[C_CAST_TS ] = std::make_unique<CastTS>();
    templateList[C_ATTACKPLAYER_TS ] = std::make_unique<AttackPlayerTS>();
    templateList[C_INTRODUCE_TS ] = std::make_unique<IntroduceTS>();
    templateList[C_SAY_TS ] = std::make_unique<SayTS>();
    templateList[C_SHOUT_TS ] = std::make_unique<ShoutTS>();
    templateList[C_WHISPER_TS ] = std::make_unique<WhisperTS>();
    templateList[C_REFRESH_TS ] = std::make_unique<RefreshTS>();
    templateList[C_LOGOUT_TS ] = std::make_unique<LogOutTS>();
    templateList[C_PICKUPITEM_TS ] = std::make_unique<PickUpItemTS>();
    templateList[C_PICKUPALLITEMS_TS ] = std::make_unique<PickUpAllItemsTS>();
    templateList[C_LOOKINTOCONTAINERONFIELD_TS ] = std::make_unique<LookIntoContainerOnFieldTS>();
    templateList[C_LOOKINTOINVENTORY_TS ] = std::make_unique<LookIntoInventoryTS>();
    templateList[C_LOOKINTOSHOWCASECONTAINER_TS ] = std::make_unique<LookIntoShowCaseContainerTS>();
    templateList[C_CLOSECONTAINERINSHOWCASE_TS ] = std::make_unique<CloseContainerInShowCaseTS>();
    templateList[C_DROPITEMFROMSHOWCASEONMAP_TS ] = std::make_unique<DropItemFromShowCaseOnMapTS>();
    templateList[C_MOVEITEMBETWEENSHOWCASES_TS ] = std::make_unique<MoveItemBetweenShowCasesTS>();
    templateList[C_MOVEITEMFROMMAPINTOSHOWCASE_TS ] = std::make_unique<MoveItemFromMapIntoShowCaseTS>();
    templateList[C_MOVEITEMFROMMAPTOPLAYER_TS ] = std::make_unique<MoveItemFromMapToPlayerTS>();
    templateList[C_MOVEITEMFROMMAPTOMAP_TS ] = std::make_unique<MoveItemFromMapToMapTS>();
    templateList[C_DROPITEMFROMPLAYERONMAP_TS ] = std::make_unique<DropItemFromInventoryOnMapTS>();
    templateList[C_MOVEITEMINSIDEINVENTORY_TS ] = std::make_unique<MoveItemInsideInventoryTS>();
    templateList[C_MOVEITEMFROMSHOWCASETOPLAYER_TS ] = std::make_unique<MoveItemFromShowCaseToPlayerTS>();
    templateList[C_MOVEITEMFROMPLAYERTOSHOWCASE_TS ] = std::make_unique<MoveItemFromPlayerToShowCaseTS>();
    templateList[C_LOOKATSHOWCASEITEM_TS ] = std::make_unique<LookAtShowCaseItemTS>();
    templateList[C_LOOKATINVENTORYITEM_TS ] = std::make_unique<LookAtInventoryItemTS>();
    templateList[C_ATTACKSTOP_TS ] = std::make_unique<AttackStopTS>();
    templateList[C_REQUESTSKILLS_TS ] = std::make_unique<RequestSkillsTS>();
    templateList[C_KEEPALIVE_TS ] = std::make_unique<KeepAliveTS>();
    templateList[BB_KEEPALIVE_TS ] = std::make_unique<BBKeepAliveTS>();
    templateList[BB_BROADCAST_TS ] = std::make_unique<BBBroadCastTS>();
    templateList[BB_DISCONNECT_TS ] = std::make_unique<BBDisconnectTS>();
    templateList[BB_BAN_TS ] = std::make_unique<BBBanTS>();
    templateList[BB_TALKTO_TS ] = std::make_unique<BBTalktoTS>();
    templateList[BB_CHANGEATTRIB_TS ] = std::make_unique<BBChangeAttribTS>();
    templateList[BB_CHANGESKILL_TS ] = std::make_unique<BBChangeSkillTS>();
    templateList[BB_SERVERCOMMAND_TS ] = std::make_unique<BBServerCommandTS>();
    templateList[BB_WARPPLAYER_TS ] = std::make_unique<BBWarpPlayerTS>();
    templateList[BB_SPEAKAS_TS ] = std::make_unique<BBSpeakAsTS>();
    templateList[C_CHARMOVE_TS ] = std::make_unique<CharMoveTS>();
    templateList[C_PLAYERSPIN_TS ] = std::make_unique<PlayerSpinTS>();
    templateList[C_LOOKATCHARACTER_TS ] = std::make_unique<LookAtCharacterTS>();
    templateList[C_REQUESTAPPEARANCE_TS ] = std::make_unique<RequestAppearanceTS>();
}


CommandFactory::~CommandFactory() {
    templateList.clear();
}

ClientCommandPointer  CommandFactory::getCommand(unsigned char commandId) {
    COMMANDLIST::iterator it;
    it = templateList.find(commandId);

    if (it != templateList.end()) {
        return it->second->clone();
    }

    return ClientCommandPointer();
}

