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

#include "Player.hpp"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <memory>
#include <range/v3/all.hpp>
#include <sstream>
#include <utility>


#include "tuningConstants.hpp"
#include "map/Field.hpp"
#include "World.hpp"
#include "Random.hpp"
#include "SchedulerTaskClasses.hpp"
#include "Logger.hpp"
#include "PlayerManager.hpp"
#include "MonitoringClients.hpp"
#include "Config.hpp"
#include "Showcase.hpp"
#include "LongTimeAction.hpp"

#include "data/Data.hpp"
#include "data/ContainerObjectTable.hpp"

#include "netinterface/NetInterface.hpp"
#include "netinterface/BasicClientCommand.hpp"
#include "netinterface/protocol/ClientCommands.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"

#include "db/ConnectionManager.hpp"
#include "db/Connection.hpp"
#include "db/DeleteQuery.hpp"
#include "db/InsertQuery.hpp"
#include "db/SelectQuery.hpp"
#include "db/UpdateQuery.hpp"
#include "db/Result.hpp"
#include "db/SchemaHelper.hpp"

#include "dialog/InputDialog.hpp"
#include "dialog/MessageDialog.hpp"
#include "dialog/MerchantDialog.hpp"
#include "dialog/SelectionDialog.hpp"
#include "dialog/CraftingDialog.hpp"

#include "script/LuaPlayerDeathScript.hpp"
#include "script/LuaDepotScript.hpp"

extern std::shared_ptr<LuaPlayerDeathScript>playerDeathScript;
extern std::shared_ptr<LuaDepotScript>depotScript;

Player::Player(std::shared_ptr<NetInterface> newConnection)
    :  onlinetime(0), Connection(std::move(newConnection)), turtleActive(false),
      clippingActive(true), admin(false), questWriteLock(false), monitoringClient(false), dialogCounter(0) {
    screenwidth = 0;
    screenheight = 0;
    Character::setAlive(true);
    SetMovement(movement_type::walk);

    time(&lastaction);
    time(&lastkeepalive);

    ltAction = std::make_unique<LongTimeAction>(this, _world);

    const auto loginCommand = Connection->getLoginData();
    setName(loginCommand->getLoginName());
    pw = loginCommand->getPassword();

    check_logindata();

    if (status == 7) {
        Logger::error(LogFacility::Player) << to_string() << " did not select a skill package" << Log::end;
        throw LogoutException(NOSKILLS);
    }

    if (!loadGMFlags()) {
        Logger::error(LogFacility::Player) << "Failed to load gm flags for " << to_string() << Log::end;
        throw LogoutException(UNSTABLECONNECTION);
    }

    if (!hasGMRight(gmr_allowlogin) && !World::get()->isLoginAllowed()) {
        throw Player::LogoutException(SERVERSHUTDOWN);
    }

    if (monitoringClient && !hasGMRight(gmr_ban)) {
        throw Player::LogoutException(NOACCOUNT);
    }

    last_ip = Connection->getIPAdress();

#if 0 // TODO move code to BBIWI specific class
    // we don't want to load more if we have a monitoring client
    if (monitoringClient) {
        ServerCommandPointer cmd = std::make_shared<BBLoginSuccessfulTC>();
        Connection->addCommand(cmd);
        Logger::info(LogFacility::Admin) << to_string() << " connects with monitoring client" << Log::end;
        return;
    }
#endif

    // now load inventory...
    if (!load()) {
        throw LogoutException(CORRUPTDATA);
    }
}

void Player::login() {
    position pos = getPosition();
    
    try {
        _world->walkableFieldNear(pos).setPlayer();
    } catch (FieldNotFound &) {
        try {
            pos.x = Config::instance().playerstart_x;
            pos.y = Config::instance().playerstart_y;
            pos.z = Config::instance().playerstart_z;
            _world->walkableFieldNear(pos).setPlayer();
        } catch (FieldNotFound &) {
            throw LogoutException(NOPLACE);
        }
    }

    setPosition(pos);

    sendCompleteQuestProgress();

    sendWeather(_world->weather);

    // send player login data
    ServerCommandPointer cmd = std::make_shared<IdTC>(getId());
    Connection->addCommand(cmd);
    // position
    cmd = std::make_shared<SetCoordinateTC>(pos);
    Connection->addCommand(cmd);

    effects.load();

    //send the basic data to the monitoring client
    cmd = std::make_shared<BBPlayerTC>(getId(), getName(), pos);
    _world->monitoringClientList->sendCommand(cmd);

    // send weather and time before sending the map, to display everything correctly from the start
    _world->sendIGTime(this);
    _world->sendWeather(this);

    // send std::map, items and chars around...
    sendFullMap();
    _world->sendAllVisibleCharactersToPlayer(this , true);

    // sent inventory
    for (unsigned short int i = 0; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS; ++i) {
        sendCharacterItemAtPos(i);
    }

    sendAllSkills();
    // notify other players
    _world->sendCharacterMoveToAllVisiblePlayers(this, NORMALMOVE, 4);
    // additional nop info
    _world->sendSpinToAllVisiblePlayers(this);
    cmd = std::make_shared<PlayerSpinTC>(getFaceTo(), getId());
    Connection->addCommand(cmd);

    // send attributes
    sendAttrib(hitpoints);
    sendAttrib(mana);
    sendAttrib(foodlevel);
    sendAttrib(sex);
    sendAttrib(age);
    sendAttrib(weight);
    sendAttrib(height);
    sendAttrib(attitude);
    sendAttrib(luck);
    sendAttrib(strength);
    sendAttrib(dexterity);
    sendAttrib(constitution);
    sendAttrib(intelligence);
    sendAttrib(perception);
    sendAttrib(willpower);
    sendAttrib(essence);
    sendAttrib(agility);
    //end of changes


    // send magic skills
    sendMagicFlags(getMagicType());

    checkBurden();

    time(&logintime);
    time(&lastkeepalive);
    time(&lastsavetime);

    // start processing client commands
    Connection->activate(this);
}

auto Player::getScreenRange() const -> unsigned short int {
    return (screenwidth > screenheight) ? 2*screenwidth : 2*screenheight;
}

void Player::setAlive(bool alive) {
    bool wasAlive = isAlive();
    Character::setAlive(alive);

    if (wasAlive && !alive) {
        updateAppearanceForAll(true);

        ltAction->abortAction();

        if (playerDeathScript) {
            playerDeathScript->playerDeath(this);
        }
    }
}

void Player::openShowcase(Container *container, const ScriptItem &item, bool carry) {
    using namespace ranges;
    auto containsContainer = [container](const auto &showcase) {return showcase.second->contains(container);};
    auto result = find_if(showcases, containsContainer);

    if (result != showcases.end()) {
        const auto lookAt = item.getLookAt(this);
        ServerCommandPointer cmd = std::make_shared<UpdateShowcaseTC>(result->first, lookAt,
                                                                      container->getSlotCount(),
                                                                      container->getItems());
        Connection->addCommand(cmd);
        return;
    }

    if (showcases.size() < MAXSHOWCASES) {
        uint8_t showcaseId;

        if (container == backPackContents) {
            showcaseId = BACKPACK_SHOWCASE;
        } else if (container->isDepot()) {
            showcaseId = 1;

            while (isShowcaseOpen(showcaseId)) {
                ++showcaseId;
            }
        } else {
            while (isShowcaseOpen(showcaseCounter) || showcaseCounter <= MAX_DEPOT_SHOWCASE) {
                ++showcaseCounter;
            }

            showcaseId = showcaseCounter;
        }

        showcases[showcaseId] = std::make_unique<Showcase>(container, carry);
        const auto lookAt = item.getLookAt(this);
        ServerCommandPointer cmd = std::make_shared<UpdateShowcaseTC>(showcaseId, lookAt, container->getSlotCount(), container->getItems());
        Connection->addCommand(cmd);
    } else {
        inform("ERROR: Unable to open more containers.");
    }
}

void Player::updateShowcase(Container *container) const {
    if (isShowcaseOpen(container)) {
        auto id = getShowcaseId(container);
        ItemLookAt lookAt;
        ServerCommandPointer cmd = std::make_shared<UpdateShowcaseTC>(id, lookAt, container->getSlotCount(), container->getItems());
        Connection->addCommand(cmd);
    }
}

void Player::updateShowcaseSlot(Container *container, TYPE_OF_CONTAINERSLOTS slot) const {
    if (isShowcaseOpen(container) && slot < container->getSlotCount()) {
        auto showcase = getShowcaseId(container);
        const auto &items = container->getItems();
        const auto it = items.find(slot);

        if (it != items.end()) {
            const auto &item = it->second;
            ServerCommandPointer cmd = std::make_shared<UpdateShowcaseSlotTC>(showcase, slot, item);
            Connection->addCommand(cmd);
        } else {
            ServerCommandPointer cmd = std::make_shared<UpdateShowcaseSlotTC>(showcase, slot);
            Connection->addCommand(cmd);
        }
    }
}

auto Player::isShowcaseOpen(uint8_t showcase) const -> bool {
    return showcases.find(showcase) != showcases.cend();
}

auto Player::isShowcaseOpen(Container *container) const -> bool {
    using namespace ranges;
    auto containsContainer = [container](const auto &showcase) {return showcase->contains(container);};
    auto showcaseView = showcases | view::values;
    auto result = find_if(showcaseView, containsContainer);
    return result != showcaseView.end();
}

auto Player::isShowcaseInInventory(uint8_t showcase) const -> bool {
    if (isShowcaseOpen(showcase)) {
        return showcases.at(showcase)->inInventory();
    }

    return false;
}

auto Player::getShowcaseId(Container *container) const -> uint8_t {
    using namespace ranges;
    auto containsContainer = [container](const auto &showcase) {return showcase.second->contains(container);};
    auto result = find_if(showcases, containsContainer);
    
    if (result != showcases.end()) {
        return result->first;
    }

    throw std::logic_error("container has no showcase!");
}

auto Player::getShowcaseContainer(uint8_t showcase) const -> Container * {
    auto it = showcases.find(showcase);

    if (it != showcases.end()) {
        return it->second->getContainer();
    }

    return nullptr;
}

void Player::closeShowcase(uint8_t showcase) {
    if (isShowcaseOpen(showcase)) {
        showcases.erase(showcase);
        ServerCommandPointer cmd = std::make_shared<ClearShowCaseTC>(showcase);
        Connection->addCommand(cmd);
    }
}

void Player::closeShowcase(Container *container) {
    for (auto it = showcases.cbegin(); it != showcases.cend();) {
        if (it->second->contains(container)) {
            ServerCommandPointer cmd = std::make_shared<ClearShowCaseTC>(it->first);
            Connection->addCommand(cmd);
            it = showcases.erase(it);
        } else {
            ++it;
        }
    }
}

void Player::closeOnMove() {
    closeAllShowcasesOfMapContainers();
    closeDialogsOnMove();
}

void Player::closeAllShowcasesOfMapContainers() {
    for (auto it = showcases.cbegin(); it != showcases.cend();) {
        if (!it->second->inInventory()) {
            ServerCommandPointer cmd = std::make_shared<ClearShowCaseTC>(it->first);
            Connection->addCommand(cmd);
            it = showcases.erase(it);
        } else {
            ++it;
        }
    }
}

void Player::closeAllShowcases() {
    for (const auto &showcase : showcases) {
        ServerCommandPointer cmd = std::make_shared<ClearShowCaseTC>(showcase.first);
        Connection->addCommand(cmd);
    }

    showcases.clear();
}

void Player::lookIntoShowcaseContainer(uint8_t showcase, unsigned char pos) {
    if (isShowcaseOpen(showcase)) {
        Container *showcaseContainer = getShowcaseContainer(showcase);
        
        auto isShowcaseContainer = [showcaseContainer](auto container) {return container == showcaseContainer;};
        bool allowedToOpenContainer = ranges::any_of(depotContents | ranges::view::values, isShowcaseContainer);

        if (showcaseContainer && allowedToOpenContainer) {
            Container *tempc;
            ScriptItem tempi;

            if (showcaseContainer->viewItemNr(pos, tempi, tempc)) {
                if (tempc) {
                    openShowcase(tempc, tempi, isShowcaseInInventory(showcase));
                }
            }
        }
    }
}

auto Player::lookIntoBackPack() -> bool {
    if ((items[BACKPACK].getId() != 0) && backPackContents) {
        openShowcase(backPackContents, static_cast<ScriptItem>(items[BACKPACK]), true);
        return true;
    }

    return false;
}

auto Player::lookIntoContainerOnField(direction dir) -> bool {
    position containerPosition = getPosition();
    containerPosition.move(dir);

    try {
        map::Field &field = World::get()->fieldAt(containerPosition);
        Item item;

        if (field.viewItemOnStack(item)) {
            if (item.getId() != DEPOTITEM && item.isContainer()) {
                auto it = field.containers.find(item.getNumber());

                if (it != field.containers.end()) {
                    openShowcase(it->second, static_cast<ScriptItem>(item), false);
                    return true;
                }
            } else {
                if (item.getId() == DEPOTITEM) {
                    ScriptItem scriptItem(item);
                    scriptItem.type = ScriptItem::it_field;
                    scriptItem.pos = containerPosition;

                    if (depotScript && depotScript->existsEntrypoint("onOpenDepot")) {
                        if (depotScript->onOpenDepot(this, scriptItem)) {
                            openDepot(scriptItem);
                        }
                    } else {
                        openDepot(scriptItem);
                    }
                } else {

                }
            }
        }
    } catch (FieldNotFound &) {
    }

    return false;
}

void Player::sendCharacters() {
    _world->sendAllVisibleCharactersToPlayer(this, true);
}


void Player::sendCharacterItemAtPos(unsigned char cpos) {
    if (cpos < (MAX_BELT_SLOTS + MAX_BODY_ITEMS)) {
        // gltiger Wert
        ServerCommandPointer cmd = std::make_shared<UpdateInventoryPosTC>(cpos, items[cpos].getId(), items[cpos].getNumber());
        Connection->addCommand(cmd);
    }
}


void Player::sendWeather(WeatherStruct weather) {
    ServerCommandPointer cmd = std::make_shared<UpdateWeatherTC>(weather);
    Connection->addCommand(cmd);
}


void Player::ageInventory() {
    for (unsigned char i = 0; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS; ++i) {
        if (items[ i ].getId() != 0) {
            const auto &itemStruct = Data::Items[items[ i ].getId()];

            if (itemStruct.rotsInInventory) {
                if (!items[ i ].survivesAgeing()) {
                    if (items[ i ].getId() != itemStruct.ObjectAfterRot) {
                        items[ i ].setId(itemStruct.ObjectAfterRot);

                        const auto &afterRotItemStruct = Data::Items[itemStruct.ObjectAfterRot];

                        if (afterRotItemStruct.isValid()) {
                            items[ i ].setWear(afterRotItemStruct.AgeingSpeed);
                        }

                        sendCharacterItemAtPos(i);
                    } else {
                        items[ i ].reset();
                        sendCharacterItemAtPos(i);
                    }

                    // The personal light might have changed!
                    updateAppearanceForAll(true);
                }
            }
        }
    }

    if ((items[ BACKPACK ].getId() != 0) && backPackContents) {
        backPackContents->doAge(true);
        updateBackPackView();
    }

    for (const auto &depotMapEntry : depotContents) {
        const auto &depot = depotMapEntry.second;

        if (depot) {
            depot->doAge(true);
            updateShowcase(depot);
        }
    }

    checkBurden();
}

void Player::learn(TYPE_OF_SKILL_ID skill, uint32_t actionPoints, uint8_t opponent) {

    uint16_t majorSkillValue = getSkill(skill);
    uint16_t minorSkillValue = getMinorSkill(skill);

    Character::learn(skill, actionPoints, opponent);

    uint16_t newMajorSkillValue = getSkill(skill);
    uint16_t newMinorSkillValue = getMinorSkill(skill);

    if (newMinorSkillValue != minorSkillValue || newMajorSkillValue != majorSkillValue) {
        sendSkill(skill, newMajorSkillValue, newMinorSkillValue);
    }
}


auto Player::createItem(Item::id_type id, Item::number_type number, Item::quality_type quality, script_data_exchangemap const *data) -> int {
    int temp = Character::createItem(id, number, quality, data);

    for (unsigned char i = 0; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS; ++i) {
        if (items[ i ].getId() != 0) {
            sendCharacterItemAtPos(i);
        }
    }

    updateBackPackView();
    checkBurden();

    return temp;
}


auto Player::eraseItem(TYPE_OF_ITEM_ID itemid, int count, script_data_exchangemap const *data) -> int {
    int temp = count;

    if ((items[ BACKPACK ].getId() != 0) && backPackContents) {
        temp = backPackContents->eraseItem(itemid, temp, data);
        updateBackPackView();
    }

    if (temp > 0) {
        // BACKPACK als Item erstmal auslassen
        for (unsigned char i = MAX_BELT_SLOTS + MAX_BODY_ITEMS - 1; i > 0; --i) {
            if ((items[ i ].getId() == itemid) && (data == nullptr || items[ i ].hasData(*data)) && (temp > 0)) {
                if (temp >= items[ i ].getNumber()) {
                    temp = temp - items[ i ].getNumber();
                    items[ i ].reset();

                    if (i == LEFT_TOOL || i == RIGHT_TOOL) {
                        unsigned char offhand = (i==LEFT_TOOL)?RIGHT_TOOL:LEFT_TOOL;

                        if (items[ offhand ].getId() == BLOCKEDITEM) {
                            // delete the occupied slot if the item was a two hander...
                            items[ offhand ].reset();
                            sendCharacterItemAtPos(offhand);
                        }
                    }
                } else {
                    items[ i ].setNumber(items[ i ].getNumber() - temp);
                    temp = 0;
                }

                sendCharacterItemAtPos(i);
            }
        }

        if (World::get()->getItemStatsFromId(itemid).Brightness > 0) {
            updateAppearanceForAll(true);
        }
    }

    checkBurden();
    return temp;
}

auto Player::increaseAtPos(unsigned char pos, int count) -> int {
    int temp = count;

    if ((pos > 0) && (pos < MAX_BELT_SLOTS + MAX_BODY_ITEMS)) {
        if (weightOK(items[ pos ].getId(), count, nullptr)) {

            temp = items[ pos ].getNumber() + count;
            auto maxStack = items[pos].getMaxStack();

            if (temp > maxStack) {
                items[ pos ].setNumber(maxStack);
                temp = temp - maxStack;
            } else if (temp <= 0) {
                bool updateBrightness = World::get()->getItemStatsFromId(items[ pos ].getId()).Brightness > 0;
                temp = count + items[ pos ].getNumber();
                items[ pos ].reset();

                if (pos == RIGHT_TOOL && items[LEFT_TOOL].getId() == BLOCKEDITEM) {
                    items[LEFT_TOOL].reset();
                    sendCharacterItemAtPos(LEFT_TOOL);
                }

                if (updateBrightness) {
                    updateAppearanceForAll(true);
                }
            } else {
                items[ pos ].setNumber(temp);
                temp = 0;
            }
        }
    }

    sendCharacterItemAtPos(pos);
    checkBurden();
    return temp;
}

auto Player::createAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, int count) -> int {
    int temp = Character::createAtPos(pos,newid,count);
    sendCharacterItemAtPos(pos);
    checkBurden();
    return temp;

}

auto Player::swapAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid , uint16_t newQuality) -> bool {
    if (Character::swapAtPos(pos, newid, newQuality)) {
        sendCharacterItemAtPos(pos);
        checkBurden();
        return true;
    }

    return false;
}


void Player::updateBackPackView() {
    if (backPackContents) {
        updateShowcase(backPackContents);
    }
}


void Player::sendSkill(TYPE_OF_SKILL_ID skill, unsigned short int major, unsigned short int minor) {
    ServerCommandPointer cmd = std::make_shared<UpdateSkillTC>(skill, major, minor);
    Connection->addCommand(cmd);
    cmd = std::make_shared<BBSendSkillTC>(getId(), skill, major, minor);
    _world->monitoringClientList->sendCommand(cmd);
}


void Player::sendAllSkills() {
    for (const auto &skill : skills) {
        if (skill.second.major>0) {
            sendSkill(skill.first, skill.second.major, skill.second.minor);
        }
    }
}


void Player::sendMagicFlags(int type) {
    if ((type >= 0) && (type < 4) && !monitoringClient) {
        ServerCommandPointer cmd = std::make_shared<UpdateMagicFlagsTC>(type, getMagicFlags(type));
        Connection->addCommand(cmd);
    }
}


auto Player::saveBaseAttributes() -> bool {
    if (getBaseAttributeSum() != getMaxAttributePoints()) {
        Database::SelectQuery query;
        query.addColumn("player", "ply_strength");
        query.addColumn("player", "ply_dexterity");
        query.addColumn("player", "ply_constitution");
        query.addColumn("player", "ply_agility");
        query.addColumn("player", "ply_intelligence");
        query.addColumn("player", "ply_perception");
        query.addColumn("player", "ply_willpower");
        query.addColumn("player", "ply_essence");
        query.addEqualCondition<TYPE_OF_CHARACTER_ID>("player", "ply_playerid", getId());
        query.addServerTable("player");

        auto result = query.execute();

        if (result.empty()) {
            throw LogoutException(NOCHARACTERFOUND);
        }

        auto row = result.front();

        setBaseAttribute(Character::strength, row["ply_strength"].as<uint16_t>());
        setBaseAttribute(Character::dexterity, row["ply_dexterity"].as<uint16_t>());
        setBaseAttribute(Character::constitution, row["ply_constitution"].as<uint16_t>());
        setBaseAttribute(Character::agility, row["ply_agility"].as<uint16_t>());
        setBaseAttribute(Character::intelligence, row["ply_intelligence"].as<uint16_t>());
        setBaseAttribute(Character::perception, row["ply_perception"].as<uint16_t>());
        setBaseAttribute(Character::willpower, row["ply_willpower"].as<uint16_t>());
        setBaseAttribute(Character::essence, row["ply_essence"].as<uint16_t>());

        return false;
    }

    Database::UpdateQuery query;
    query.addAssignColumn<uint16_t>("ply_agility", getBaseAttribute(Character::agility));
    query.addAssignColumn<uint16_t>("ply_constitution", getBaseAttribute(Character::constitution));
    query.addAssignColumn<uint16_t>("ply_dexterity", getBaseAttribute(Character::dexterity));
    query.addAssignColumn<uint16_t>("ply_essence", getBaseAttribute(Character::essence));
    query.addAssignColumn<uint16_t>("ply_intelligence", getBaseAttribute(Character::intelligence));
    query.addAssignColumn<uint16_t>("ply_perception", getBaseAttribute(Character::perception));
    query.addAssignColumn<uint16_t>("ply_strength", getBaseAttribute(Character::strength));
    query.addAssignColumn<uint16_t>("ply_willpower", getBaseAttribute(Character::willpower));
    query.addEqualCondition<TYPE_OF_CHARACTER_ID>("player", "ply_playerid", getId());
    query.addServerTable("player");
    query.execute();

    return true;
}


void Player::sendAttrib(Character::attributeIndex attribute) {
    auto value = getAttribute(attribute);

    if (!monitoringClient) {
        ServerCommandPointer cmd = std::make_shared<UpdateAttribTC>(getId(), attributeStringMap[attribute], value);
        Connection->addCommand(cmd);
    }

    ServerCommandPointer cmd = std::make_shared<BBSendAttribTC>(getId(), attributeStringMap[attribute], value);
    _world->monitoringClientList->sendCommand(cmd);
}


void Player::handleAttributeChange(Character::attributeIndex attribute) {
    Character::handleAttributeChange(attribute);
    sendAttrib(attribute);

    if (attribute == Character::strength) {
        checkBurden();
    }
}


void Player::startMusic(short int title) {
    ServerCommandPointer cmd = std::make_shared<MusicTC>(title);
    Connection->addCommand(cmd);
}


void Player::defaultMusic() {
    ServerCommandPointer cmd = std::make_shared<MusicDefaultTC>();
    Connection->addCommand(cmd);
}


// Setters and Getters //

auto Player::getStatus() const -> unsigned char {
    return status;
}


void Player::setStatus(unsigned char status) {
    this->status = status;
}


// What time does the status get reset?
auto Player::getStatusTime() const -> time_t {
    return statustime;
}


void Player::setStatusTime(time_t time) {
    statustime = time;
}


// Who banned/jailed the player?
auto Player::getStatusGM() const -> std::string {
    using namespace Database;
    SelectQuery query;
    query.addColumn("chars", "chr_playerid");
    query.addEqualCondition<TYPE_OF_CHARACTER_ID>("chars", "chr_playerid", statusgm);
    query.addServerTable("chars");

    Result result = query.execute();

    std::string statusgmstring;

    if (!result.empty()) {
        const ResultField field = result.front()["chr_playerid"];

        if (!field.is_null()) {
            statusgmstring = field.as<std::string>();
        }
    }

    return statusgmstring;
}


void Player::setStatusGM(TYPE_OF_CHARACTER_ID gm) {
    statusgm = gm;
}


// Why where they banned/jailed?
auto Player::getStatusReason() const -> std::string {
    return statusreason;
}


void Player::setStatusReason(const std::string &reason) {
    statusreason = reason;
}

// World Map Turtle Graphics
void Player::setTurtleActive(bool tturtleActive) {
    turtleActive = tturtleActive;
    setClippingActive(!tturtleActive);
}


void Player::setClippingActive(bool tclippingActive) {
    clippingActive = tclippingActive;
}


auto Player::getTurtleActive() const -> bool {
    return turtleActive;
}


auto Player::getClippingActive() const -> bool {
    return clippingActive;
}


void Player::setTurtleTile(unsigned char tturtletile) {
    turtletile = tturtletile;
}


auto Player::getTurtleTile() const -> unsigned char {
    return turtletile;
}


void Player::setAdmin(uint32_t tAdmin) {
    admin = tAdmin;
}


auto Player::isAdmin() const -> bool {
    return (admin>0 && !hasGMRight(gmr_isnotshownasgm));
}


void Player::check_logindata() {
    Database::PConnection connection = Database::ConnectionManager::getInstance().getConnection();

    try {
        Database::SelectQuery charQuery(connection);
        charQuery.addColumn("chars", "chr_playerid");
        charQuery.addColumn("chars", "chr_accid");
        charQuery.addColumn("chars", "chr_status");
        charQuery.addColumn("chars", "chr_statustime");
        charQuery.addColumn("chars", "chr_onlinetime");
        charQuery.addColumn("chars", "chr_lastsavetime");
        charQuery.addColumn("chars", "chr_sex");
        charQuery.addColumn("chars", "chr_race");
        charQuery.addEqualCondition<std::string>("chars", "chr_name", getName());
        charQuery.addServerTable("chars");

        Database::Result charResult = charQuery.execute();

        if (charResult.empty()) {
            throw LogoutException(NOCHARACTERFOUND);
        }

        Database::ResultTuple charRow = charResult.front();

        TYPE_OF_CHARACTER_ID account_id;

        setId(charRow["chr_playerid"].as<TYPE_OF_CHARACTER_ID>());
        account_id = charRow["chr_accid"].as<TYPE_OF_CHARACTER_ID>();
        status = charRow["chr_status"].as<uint16_t>();

        if (!charRow["chr_statustime"].is_null()) {
            statustime = charRow["chr_statustime"].as<time_t>();
        }

        onlinetime = charRow["chr_onlinetime"].as<time_t>();
        lastsavetime = charRow["chr_lastsavetime"].as<time_t>();
        setAttribute(Character::sex, charRow["chr_sex"].as<uint16_t>());
        setRace(charRow["chr_race"].as<TYPE_OF_RACE_ID>());

        // first we check the status since we already retrieved it
        switch (status) {
        case BANNED:
            // player banned forever
            throw LogoutException(BYGAMEMASTER);

        case BANNEDFORTIME:
            // player banned for some time...
            // check if time is up?
            time_t time_now;
            time(&time_now);

            if (time_now > statustime) {
                status=statustime=0;
            } else {
                throw LogoutException(BYGAMEMASTER);
            }

            break;

        case WAITINGVALIDATION:
            // player not approved yet
            throw LogoutException(NOACCOUNT);
        }

        // next we get the infos for the account and check if the account is active

        Database::SelectQuery accQuery(connection);
        accQuery.addColumn("account", "acc_passwd");
        accQuery.addColumn("account", "acc_lang");
        accQuery.addColumn("account", "acc_state");
        accQuery.addEqualCondition("account", "acc_id", account_id);
        accQuery.addAccountTable("account");

        Database::Result accResult = accQuery.execute();

        if (accResult.empty()) {
            throw LogoutException(NOACCOUNT);
        }

        Database::ResultTuple accRow = accResult.front();

        int acc_state;
        std::string real_pwd;

        real_pwd = accRow["acc_passwd"].as<std::string>();
        _player_language = static_cast<Language>(accRow["acc_lang"].as<uint16_t>());
        acc_state = accRow["acc_state"].as<uint16_t>();

        // check if account is active
        if (acc_state < 3) { // TODO how is acc_state defined??
            throw LogoutException(NOACCOUNT);
        }

        if (acc_state > 3) {
            throw LogoutException(BYGAMEMASTER);
        }

        // check password
        if (pw != real_pwd) {
            Logger::alert(LogFacility::Player) << to_string() << " sent wrong password from ip: " << Connection->getIPAdress() << Log::end;
            throw LogoutException(WRONGPWD);
        }

        std::stringstream newPlayerQueryString;
        newPlayerQueryString << "SET search_path TO "<< Database::SchemaHelper::getServerSchema() << "; SELECT is_new_player(" << account_id << ");";
        Database::Query newPlayerQuery(connection, newPlayerQueryString.str());
        auto newPlayerResult = newPlayerQuery.execute();
        
        if (!newPlayerResult.empty()) {
            const auto &row = newPlayerResult.front();
            newPlayer = row["is_new_player"].as<bool>(false);
        }

        Database::SelectQuery playerQuery(connection);
        playerQuery.addColumn("player", "ply_posx");
        playerQuery.addColumn("player", "ply_posy");
        playerQuery.addColumn("player", "ply_posz");
        playerQuery.addColumn("player", "ply_faceto");
        playerQuery.addColumn("player", "ply_age");
        playerQuery.addColumn("player", "ply_weight");
        playerQuery.addColumn("player", "ply_body_height");
        playerQuery.addColumn("player", "ply_hitpoints");
        playerQuery.addColumn("player", "ply_mana");
        playerQuery.addColumn("player", "ply_attitude");
        playerQuery.addColumn("player", "ply_luck");
        playerQuery.addColumn("player", "ply_strength");
        playerQuery.addColumn("player", "ply_dexterity");
        playerQuery.addColumn("player", "ply_constitution");
        playerQuery.addColumn("player", "ply_agility");
        playerQuery.addColumn("player", "ply_intelligence");
        playerQuery.addColumn("player", "ply_perception");
        playerQuery.addColumn("player", "ply_willpower");
        playerQuery.addColumn("player", "ply_essence");
        playerQuery.addColumn("player", "ply_foodlevel");
        playerQuery.addColumn("player", "ply_lifestate");
        playerQuery.addColumn("player", "ply_magictype");
        playerQuery.addColumn("player", "ply_magicflagsmage");
        playerQuery.addColumn("player", "ply_magicflagspriest");
        playerQuery.addColumn("player", "ply_magicflagsbard");
        playerQuery.addColumn("player", "ply_magicflagsdruid");
        playerQuery.addColumn("player", "ply_poison");
        playerQuery.addColumn("player", "ply_mental_capacity");
        playerQuery.addColumn("player", "ply_hair");
        playerQuery.addColumn("player", "ply_beard");
        playerQuery.addColumn("player", "ply_hairred");
        playerQuery.addColumn("player", "ply_hairgreen");
        playerQuery.addColumn("player", "ply_hairblue");
        playerQuery.addColumn("player", "ply_hairalpha");
        playerQuery.addColumn("player", "ply_skinred");
        playerQuery.addColumn("player", "ply_skingreen");
        playerQuery.addColumn("player", "ply_skinblue");
        playerQuery.addColumn("player", "ply_skinalpha");
        playerQuery.addEqualCondition("player", "ply_playerid", getId());
        playerQuery.addServerTable("player");

        Database::Result playerResult = playerQuery.execute();

        if (playerResult.empty()) {
            throw LogoutException(NOACCOUNT);
        }

        Database::ResultTuple playerRow = playerResult.front();

        position pos;
        pos.x = playerRow["ply_posx"].as<int32_t>();
        pos.y = playerRow["ply_posy"].as<int32_t>();
        pos.z = playerRow["ply_posz"].as<int32_t>();
        setPosition(pos);
        setFaceTo((Character::face_to) playerRow["ply_faceto"].as<uint16_t>());

        setAttribute(Character::age, playerRow["ply_age"].as<uint16_t>());
        setAttribute(Character::weight, playerRow["ply_weight"].as<uint16_t>());
        setAttribute(Character::height, playerRow["ply_body_height"].as<uint16_t>());
        setAttribute(Character::hitpoints, playerRow["ply_hitpoints"].as<uint16_t>());
        setAttribute(Character::mana, playerRow["ply_mana"].as<uint16_t>());
        setAttribute(Character::attitude, playerRow["ply_attitude"].as<uint16_t>());
        setAttribute(Character::luck, playerRow["ply_luck"].as<uint16_t>());
        setAttribute(Character::strength, playerRow["ply_strength"].as<uint16_t>());
        setAttribute(Character::dexterity, playerRow["ply_dexterity"].as<uint16_t>());
        setAttribute(Character::constitution, playerRow["ply_constitution"].as<uint16_t>());
        setAttribute(Character::agility, playerRow["ply_agility"].as<uint16_t>());
        setAttribute(Character::intelligence, playerRow["ply_intelligence"].as<uint16_t>());
        setAttribute(Character::perception, playerRow["ply_perception"].as<uint16_t>());
        setAttribute(Character::willpower, playerRow["ply_willpower"].as<uint16_t>());
        setAttribute(Character::essence, playerRow["ply_essence"].as<uint16_t>());
        setAttribute(Character::foodlevel, playerRow["ply_foodlevel"].as<uint32_t>());

        Character::setAlive(playerRow["ply_lifestate"].as<uint16_t>() != 0);
        setMagicType(magic_type(playerRow["ply_magictype"].as<uint16_t>()));
        setMagicFlags(MAGE, playerRow["ply_magicflagsmage"].as<uint64_t>());
        setMagicFlags(PRIEST, playerRow["ply_magicflagspriest"].as<uint64_t>());
        setMagicFlags(BARD, playerRow["ply_magicflagsbard"].as<uint64_t>());
        setMagicFlags(DRUID, playerRow["ply_magicflagsdruid"].as<uint64_t>());
        poisonvalue = playerRow["ply_poison"].as<uint16_t>();
        mental_capacity = playerRow["ply_mental_capacity"].as<uint32_t>();
        _appearance.hairtype = playerRow["ply_hair"].as<uint16_t>();
        _appearance.beardtype = playerRow["ply_beard"].as<uint16_t>();
        _appearance.hair.red = playerRow["ply_hairred"].as<uint16_t>();
        _appearance.hair.green = playerRow["ply_hairgreen"].as<uint16_t>();
        _appearance.hair.blue = playerRow["ply_hairblue"].as<uint16_t>();
        _appearance.hair.alpha = playerRow["ply_hairalpha"].as<uint16_t>();
        _appearance.skin.red = playerRow["ply_skinred"].as<uint16_t>();
        _appearance.skin.green = playerRow["ply_skingreen"].as<uint16_t>();
        _appearance.skin.blue = playerRow["ply_skinblue"].as<uint16_t>();
        _appearance.skin.alpha = playerRow["ply_skinalpha"].as<uint16_t>();
    } catch (std::exception &e) {
        Logger::error(LogFacility::Player) << "Exception on loading player: " << e.what() << Log::end;
        throw LogoutException(NOCHARACTERFOUND);
    }
}

struct container_struct {
    Container *container;
    unsigned int id;
    unsigned int depotid;

    container_struct(Container *cc, unsigned int aboveid, unsigned int depot = 0)
        : container(cc), id(aboveid), depotid(depot) { }
}
;

auto Player::save() noexcept -> bool {
    using namespace Database;

    Logger::info(LogFacility::Player) << "Saving " << to_string() << Log::end;

    PConnection connection = ConnectionManager::getInstance().getConnection();

    try {
        connection->beginTransaction();

        {
            DeleteQuery introductionQuery(connection);
            introductionQuery.addEqualCondition<TYPE_OF_CHARACTER_ID>("introduction", "intro_player", getId());
            introductionQuery.addServerTable("introduction");
            introductionQuery.execute();
        }

        {
            InsertQuery introductionQuery(connection);
            const InsertQuery::columnIndex playerColumn = introductionQuery.addColumn("intro_player");
            const InsertQuery::columnIndex knownPlayerColumn = introductionQuery.addColumn("intro_known_player");
            introductionQuery.addServerTable("introduction");

            for (const auto player : knownPlayers) {
                introductionQuery.addValue<TYPE_OF_CHARACTER_ID>(playerColumn, getId());
                introductionQuery.addValue<TYPE_OF_CHARACTER_ID>(knownPlayerColumn, player);
            }

            introductionQuery.execute();
        }

        {
            DeleteQuery namingQuery(connection);
            namingQuery.addEqualCondition<TYPE_OF_CHARACTER_ID>("naming", "name_player", getId());
            namingQuery.addServerTable("naming");
            namingQuery.execute();
        }

        {
            InsertQuery namingQuery(connection);
            const InsertQuery::columnIndex playerColumn = namingQuery.addColumn("name_player");
            const InsertQuery::columnIndex namedPlayerColumn = namingQuery.addColumn("name_named_player");
            const InsertQuery::columnIndex playerNameColumn = namingQuery.addColumn("name_player_name");
            namingQuery.addServerTable("naming");

            for (const auto &playerAndName : namedPlayers) {
                namingQuery.addValue<TYPE_OF_CHARACTER_ID>(playerColumn, getId());
                namingQuery.addValue<TYPE_OF_CHARACTER_ID>(namedPlayerColumn, playerAndName.first);
                namingQuery.addValue<std::string>(playerNameColumn, playerAndName.second);
            }

            namingQuery.execute();
        }

        time(&lastsavetime);
        {
            UpdateQuery query(connection);
            query.addAssignColumn<uint16_t>("chr_status", (uint16_t) status);
            query.addAssignColumn<std::string>("chr_lastip", last_ip);
            query.addAssignColumn<uint32_t>("chr_onlinetime", onlinetime + lastsavetime - logintime);
            query.addAssignColumn<time_t>("chr_lastsavetime", lastsavetime);

            if (status != 0) {
                query.addAssignColumn<time_t>("chr_statustime", statustime);
                query.addAssignColumn<TYPE_OF_CHARACTER_ID>("chr_statusgm", statusgm);
                query.addAssignColumn<std::string>("chr_statusreason", statusreason);
            } else {
                query.addAssignColumnNull("chr_statustime");
                query.addAssignColumnNull("chr_statusgm");
                query.addAssignColumnNull("chr_statusreason");
            }

            query.addEqualCondition<TYPE_OF_CHARACTER_ID>("chars", "chr_playerid", getId());
            query.setServerTable("chars");

            query.execute();
        }

        {
            UpdateQuery query(connection);
            query.addAssignColumn<int32_t>("ply_posx", getPosition().x);
            query.addAssignColumn<int32_t>("ply_posy", getPosition().y);
            query.addAssignColumn<int32_t>("ply_posz", getPosition().z);
            query.addAssignColumn<uint16_t>("ply_faceto", (uint16_t) getFaceTo());
            query.addAssignColumn<uint16_t>("ply_hitpoints", getAttribute(Character::hitpoints));
            query.addAssignColumn<uint16_t>("ply_mana", getAttribute(Character::mana));
            query.addAssignColumn<uint32_t>("ply_foodlevel", getAttribute(Character::foodlevel));
            query.addAssignColumn<uint32_t>("ply_lifestate", isAlive() ? 1 : 0);
            query.addAssignColumn<uint32_t>("ply_magictype", (uint32_t) getMagicType());
            query.addAssignColumn<uint64_t>("ply_magicflagsmage", getMagicFlags(MAGE));
            query.addAssignColumn<uint64_t>("ply_magicflagspriest", getMagicFlags(PRIEST));
            query.addAssignColumn<uint64_t>("ply_magicflagsbard", getMagicFlags(BARD));
            query.addAssignColumn<uint64_t>("ply_magicflagsdruid", getMagicFlags(DRUID));
            query.addAssignColumn<uint16_t>("ply_poison", poisonvalue);
            query.addAssignColumn<uint32_t>("ply_mental_capacity", mental_capacity);
            query.addAssignColumn<uint16_t>("ply_hair", _appearance.hairtype);
            query.addAssignColumn<uint16_t>("ply_beard", _appearance.beardtype);
            query.addAssignColumn<uint16_t>("ply_hairred", _appearance.hair.red);
            query.addAssignColumn<uint16_t>("ply_hairgreen", _appearance.hair.green);
            query.addAssignColumn<uint16_t>("ply_hairblue", _appearance.hair.blue);
            query.addAssignColumn<uint16_t>("ply_hairalpha", _appearance.hair.alpha);
            query.addAssignColumn<uint16_t>("ply_skinred", _appearance.skin.red);
            query.addAssignColumn<uint16_t>("ply_skingreen", _appearance.skin.green);
            query.addAssignColumn<uint16_t>("ply_skinblue", _appearance.skin.blue);
            query.addAssignColumn<uint16_t>("ply_skinalpha", _appearance.skin.alpha);
            query.addEqualCondition<TYPE_OF_CHARACTER_ID>("player", "ply_playerid", getId());
            query.addServerTable("player");
            query.execute();
        }

        {
            DeleteQuery query(connection);
            query.addEqualCondition<TYPE_OF_CHARACTER_ID>("playerskills", "psk_playerid", getId());
            query.setServerTable("playerskills");
            query.execute();
        }

        if (!skills.empty()) {
            InsertQuery query(connection);
            const InsertQuery::columnIndex playerIdColumn = query.addColumn("psk_playerid");
            const InsertQuery::columnIndex skillIdColumn = query.addColumn("psk_skill_id");
            const InsertQuery::columnIndex valueColumn = query.addColumn("psk_value");
            const InsertQuery::columnIndex minorColumn = query.addColumn("psk_minor");

            // now store the skills
            for (const auto &skill : skills) {
                query.addValue<uint16_t>(skillIdColumn, skill.first);
                query.addValue<uint16_t>(valueColumn, (uint16_t) skill.second.major);
                query.addValue<uint16_t>(minorColumn, (uint16_t) skill.second.minor);
            }

            query.addValues<TYPE_OF_CHARACTER_ID>(playerIdColumn, getId(), InsertQuery::FILL);
            query.addServerTable("playerskills");
            query.execute();
        }

        {
            DeleteQuery query(connection);
            query.addEqualCondition<TYPE_OF_CHARACTER_ID>("playeritems", "pit_playerid", getId());
            query.setServerTable("playeritems");
            query.execute();
        }

        {
            DeleteQuery query(connection);
            query.addEqualCondition<TYPE_OF_CHARACTER_ID>("playeritem_datavalues", "idv_playerid", getId());
            query.setServerTable("playeritem_datavalues");
            query.execute();
        }

        {
            std::list<container_struct> containers;

            // add backpack to containerlist
            if (items[ BACKPACK ].getId() != 0 && backPackContents) {
                containers.emplace_back(backPackContents, BACKPACK+1);
            }

            // add depot to containerlist
            ranges::transform(depotContents, ranges::back_inserter(containers), [](const auto &depot)
                    {return container_struct(depot.second, 0, depot.first);});

            InsertQuery itemsQuery(connection);
            const InsertQuery::columnIndex itemsPlyIdColumn = itemsQuery.addColumn("pit_playerid");
            const InsertQuery::columnIndex itemsLineColumn = itemsQuery.addColumn("pit_linenumber");
            const InsertQuery::columnIndex itemsContainerColumn = itemsQuery.addColumn("pit_in_container");
            const InsertQuery::columnIndex itemsDepotColumn = itemsQuery.addColumn("pit_depot");
            const InsertQuery::columnIndex itemsItmIdColumn = itemsQuery.addColumn("pit_itemid");
            const InsertQuery::columnIndex itemsWearColumn = itemsQuery.addColumn("pit_wear");
            const InsertQuery::columnIndex itemsNumberColumn = itemsQuery.addColumn("pit_number");
            const InsertQuery::columnIndex itemsQualColumn = itemsQuery.addColumn("pit_quality");
            const InsertQuery::columnIndex itemsSlotColumn = itemsQuery.addColumn("pit_containerslot");
            itemsQuery.setServerTable("playeritems");

            InsertQuery dataQuery(connection);
            const InsertQuery::columnIndex dataPlyIdColumn = dataQuery.addColumn("idv_playerid");
            const InsertQuery::columnIndex dataLineColumn = dataQuery.addColumn("idv_linenumber");
            const InsertQuery::columnIndex dataKeyColumn = dataQuery.addColumn("idv_key");
            const InsertQuery::columnIndex dataValueColumn = dataQuery.addColumn("idv_value");
            dataQuery.setServerTable("playeritem_datavalues");

            int linenumber = 0;

            // save all items directly on the body...
            for (int thisItemSlot = 0; thisItemSlot < MAX_BODY_ITEMS + MAX_BELT_SLOTS; ++thisItemSlot) {
                ++linenumber;

                //if there is no item on this place, do not save it
                if (items[ thisItemSlot ].getId() == 0) {
                    continue;
                }

                itemsQuery.addValue<int32_t>(itemsLineColumn, linenumber);
                itemsQuery.addValue<int16_t>(itemsContainerColumn, 0);
                itemsQuery.addValue<int32_t>(itemsDepotColumn, 0);
                itemsQuery.addValue<TYPE_OF_ITEM_ID>(itemsItmIdColumn, items[thisItemSlot].getId());
                itemsQuery.addValue<uint16_t>(itemsWearColumn, items[thisItemSlot].getWear());
                itemsQuery.addValue<uint16_t>(itemsNumberColumn, items[thisItemSlot].getNumber());
                itemsQuery.addValue<uint16_t>(itemsQualColumn, items[thisItemSlot].getQuality());
                itemsQuery.addValue<TYPE_OF_CONTAINERSLOTS>(itemsSlotColumn, 0);

                for (auto it = items[ thisItemSlot ].getDataBegin(); it != items[ thisItemSlot ].getDataEnd(); ++it) {
                    if (it->second.length() > 0) {
                        dataQuery.addValue<int32_t>(dataLineColumn, linenumber);
                        dataQuery.addValue<std::string>(dataKeyColumn, it->first);
                        dataQuery.addValue<std::string>(dataValueColumn, it->second);
                    }
                }
            }

            // add backpack contents...
            while (!containers.empty()) {
                // get container to save...
                container_struct &currentContainerStruct = containers.front();
                Container &currentContainer = *currentContainerStruct.container;
                auto containedItems = currentContainer.getItems();

                for (const auto &slotAndItem : containedItems) {
                    const Item &item = slotAndItem.second;
                    itemsQuery.addValue<int32_t>(itemsLineColumn, ++linenumber);
                    itemsQuery.addValue<int16_t>(itemsContainerColumn, (int16_t) currentContainerStruct.id);
                    itemsQuery.addValue<int32_t>(itemsDepotColumn, (int32_t) currentContainerStruct.depotid);
                    itemsQuery.addValue<TYPE_OF_ITEM_ID>(itemsItmIdColumn, item.getId());
                    itemsQuery.addValue<uint16_t>(itemsWearColumn, item.getWear());
                    itemsQuery.addValue<uint16_t>(itemsNumberColumn, item.getNumber());
                    itemsQuery.addValue<uint16_t>(itemsQualColumn, item.getQuality());
                    itemsQuery.addValue<TYPE_OF_CONTAINERSLOTS>(itemsSlotColumn, slotAndItem.first);

                    for (auto it = item.getDataBegin(); it != item.getDataEnd(); ++it) {
                        dataQuery.addValue<int32_t>(dataLineColumn, linenumber);
                        dataQuery.addValue<std::string>(dataKeyColumn, it->first);
                        dataQuery.addValue<std::string>(dataValueColumn, it->second);
                    }

                    // if it is a container, add it to the list of containers to save...
                    if (item.isContainer()) {
                        auto containedContainers = currentContainer.getContainers();
                        auto iterat = containedContainers.find(slotAndItem.first);

                        if (iterat != containedContainers.end()) {
                            containers.emplace_back(iterat->second, linenumber);
                        }
                    }
                }

                containers.pop_front();
            }

            itemsQuery.addValues(itemsPlyIdColumn, getId(), InsertQuery::FILL);
            dataQuery.addValues(dataPlyIdColumn, getId(), InsertQuery::FILL);

            itemsQuery.execute();
            dataQuery.execute();
        }

        connection->commitTransaction();

        if (!effects.save()) {
            Logger::error(LogFacility::Player) << "error while saving lteffects for " << to_string() << Log::end;
        }

        return true;
    } catch (std::exception &e) {
        Logger::error(LogFacility::Player) << "Playersave caught exception: " << e.what() << Log::end;
        connection->rollbackTransaction();
        return false;
    }
}

auto Player::loadGMFlags() noexcept -> bool {
    try {
        using namespace Database;
        SelectQuery query;
        query.addColumn("gms", "gm_rights_server");
        query.addEqualCondition<TYPE_OF_CHARACTER_ID>("gms", "gm_charid", getId());
        query.addServerTable("gms");

        Result results = query.execute();

        if (results.empty()) {
            setAdmin(0);
        } else {
            setAdmin(results.front()["gm_rights_server"].as<uint32_t>());
        }

        return true;
    } catch (std::exception &e) {
        Logger::error(LogFacility::Player) << "exception: " << e.what() << " while loading gm flags!" << Log::end;
        return false;
    }

    return false;
}

auto Player::load() noexcept -> bool {
    std::map<int, Container *> depots, containers;
    std::map<int, Container *>::iterator it;

    bool dataOK=true;

    using namespace Database;
    PConnection connection = ConnectionManager::getInstance().getConnection();

    try {

        {
            SelectQuery query;
            query.addColumn("questprogress", "qpg_questid");
            query.addColumn("questprogress", "qpg_progress");
            query.addColumn("questprogress", "qpg_time");
            query.addEqualCondition<TYPE_OF_CHARACTER_ID>("questprogress", "qpg_userid", getId());
            query.addServerTable("questprogress");

            Result results = query.execute();

            for (const auto &row : results) {
                const auto questId = row["qpg_questid"].as<TYPE_OF_QUEST_ID>();
                const auto questStatus = row["qpg_progress"].as<TYPE_OF_QUESTSTATUS>(0);
                const auto questTime = row["qpg_time"].as<int>();
                quests[questId] = std::make_pair(questStatus, questTime);
            }
        }

        {
            SelectQuery query;
            query.addColumn("introduction", "intro_known_player");
            query.addEqualCondition<TYPE_OF_CHARACTER_ID>("introduction", "intro_player", getId());
            query.addServerTable("introduction");

            Result results = query.execute();

            for (const auto &row : results) {
                knownPlayers.insert(row["intro_known_player"].as<TYPE_OF_CHARACTER_ID>());
            }
        }

        {
            SelectQuery query;
            query.addColumn("naming", "name_named_player");
            query.addColumn("naming", "name_player_name");
            query.addEqualCondition<TYPE_OF_CHARACTER_ID>("naming", "name_player", getId());
            query.addServerTable("naming");

            Result results = query.execute();

            for (const auto &row : results) {
                namedPlayers.emplace(row["name_named_player"].as<TYPE_OF_CHARACTER_ID>(),
                                     row["name_player_name"].as<std::string>());
            }
        }

        {
            SelectQuery query;
            query.addColumn("playerskills", "psk_skill_id");
            query.addColumn("playerskills", "psk_value");
            query.addColumn("playerskills", "psk_minor");
            query.addEqualCondition<TYPE_OF_CHARACTER_ID>("playerskills", "psk_playerid", getId());
            query.addServerTable("playerskills");

            Result results = query.execute();

            if (!results.empty()) {
                for (const auto &row : results) {
                    setSkill(
                        row["psk_skill_id"].as<uint16_t>(),
                        row["psk_value"].as<uint16_t>(),
                        row["psk_minor"].as<uint16_t>()
                    );
                }
            } else {
                Logger::warn(LogFacility::Player) << to_string() << " has no skills" << Log::end;
            }
        }

        // load data values
        std::vector<uint16_t> ditemlinenumber;
        std::vector<std::string> key;
        std::vector<std::string> value;
        {
            SelectQuery query;
            query.addColumn("playeritem_datavalues", "idv_linenumber");
            query.addColumn("playeritem_datavalues", "idv_key");
            query.addColumn("playeritem_datavalues", "idv_value");
            query.addEqualCondition<TYPE_OF_CHARACTER_ID>("playeritem_datavalues", "idv_playerid", getId());
            query.addOrderBy("playeritem_datavalues", "idv_linenumber", SelectQuery::ASC);
            query.addServerTable("playeritem_datavalues");

            Result results = query.execute();

            for (const auto &row : results) {
                ditemlinenumber.push_back(row["idv_linenumber"].as<uint16_t>());
                key.push_back(row["idv_key"].as<std::string>());
                value.push_back(row["idv_value"].as<std::string>());
            }
        }
        size_t dataRows = ditemlinenumber.size();

        // load inventory
        std::vector<uint16_t> itemlinenumber;
        std::vector<uint16_t> itemincontainer;
        std::vector<uint32_t> itemdepot;
        std::vector<Item::id_type> itemid;
        std::vector<Item::wear_type> itemwear;
        std::vector<Item::number_type> itemnumber;
        std::vector<Item::quality_type> itemquality;
        std::vector<TYPE_OF_CONTAINERSLOTS> itemcontainerslot;
        {
            SelectQuery query;
            query.addColumn("playeritems", "pit_linenumber");
            query.addColumn("playeritems", "pit_in_container");
            query.addColumn("playeritems", "pit_depot");
            query.addColumn("playeritems", "pit_itemid");
            query.addColumn("playeritems", "pit_wear");
            query.addColumn("playeritems", "pit_number");
            query.addColumn("playeritems", "pit_quality");
            query.addColumn("playeritems", "pit_containerslot");
            query.addEqualCondition<TYPE_OF_CHARACTER_ID>("playeritems", "pit_playerid", getId());
            query.addOrderBy("playeritems", "pit_linenumber", SelectQuery::ASC);
            query.addServerTable("playeritems");

            Result results = query.execute();

            for (const auto &row : results) {
                itemlinenumber.push_back(row["pit_linenumber"].as<uint16_t>());
                itemincontainer.push_back(row["pit_in_container"].as<uint16_t>());
                itemdepot.push_back(row["pit_depot"].as<uint32_t>());
                itemid.push_back(row["pit_itemid"].as<Item::id_type>());
                itemwear.push_back((Item::wear_type)(row["pit_wear"].as<uint16_t>()));
                itemnumber.push_back(row["pit_number"].as<Item::number_type>());
                itemquality.push_back(row["pit_quality"].as<Item::quality_type>());
                itemcontainerslot.push_back(row["pit_containerslot"].as<TYPE_OF_CONTAINERSLOTS>());
            }
        }
        size_t itemRows = itemlinenumber.size();

        // load depots
        std::vector<uint32_t> depotid;
        {
            SelectQuery query;
            query.setDistinct(true);
            query.addColumn("playeritems", "pit_depot");
            query.addEqualCondition<TYPE_OF_CHARACTER_ID>("playeritems", "pit_playerid", getId());
            query.addServerTable("playeritems");

            Result results = query.execute();

            ranges::for_each(results, [&depotid](const ResultTuple &row) {
                depotid.push_back(row["pit_depot"].as<uint32_t>());
            });
        }

        size_t depotRows = depotid.size();

        for (size_t i = 0; i < depotRows; ++i) {
            if (depotid[i] != 0) {
                depotContents[depotid[i]] = new Container(DEPOTITEM);
                depots[depotid[i]] = depotContents[depotid[i]];
            }
        }

        unsigned int curdatalinenumber = 0;

        for (unsigned int tuple = 0; tuple < itemRows; ++tuple) {
            unsigned int tempincont = itemincontainer[tuple];
            unsigned int tempdepot = itemdepot[tuple];
            unsigned int linenumber = itemlinenumber[tuple];

            Item tempi(itemid[tuple],
                       itemnumber[tuple],
                       itemwear[tuple],
                       itemquality[tuple]
                      );

            while (curdatalinenumber < dataRows && ditemlinenumber[curdatalinenumber] == linenumber) {
                tempi.setData(key[curdatalinenumber], value[curdatalinenumber]);
                curdatalinenumber++;
            }

            // item is in a depot?
            if (tempdepot && (it = depots.find(tempdepot)) == depots.end()) {
                // serious error occured! player data corrupted!
                Logger::error(LogFacility::Player) << to_string() << " has invalid depot contents!" << Log::end;
                throw std::exception();
            }

            // item is in a container?
            if (tempincont && (it = containers.find(tempincont)) == containers.end()) {
                // serious error occured! player data corrupted!
                Logger::error(LogFacility::Player) << to_string() << " has invalid depot contents 2!" << Log::end;
                throw std::exception();
            }

            if (((!tempincont && ! tempdepot) && linenumber > MAX_BODY_ITEMS + MAX_BELT_SLOTS) || (tempincont && tempdepot)) {
                // serious error occured! player data corrupted!
                Logger::error(LogFacility::Player) << to_string() << " has invalid items!" << Log::end;
                throw std::exception();
            }

            if (tempi.isContainer()) {
                auto *tempc = new Container(tempi.getId());

                if (linenumber > MAX_BODY_ITEMS + MAX_BELT_SLOTS) {
                    if (!it->second->InsertContainer(tempi, tempc, itemcontainerslot[tuple])) {
                        Logger::error(LogFacility::Player) << to_string() << " insert Container wasn't sucessful!" << Log::end;
                    } else {

                    }
                } else {
                    items[ linenumber - 1 ] = tempi;
                }

                containers[linenumber] = tempc;
            } else {
                if (linenumber >= MAX_BODY_ITEMS + MAX_BELT_SLOTS + 1) {
                    it->second->InsertItem(tempi, itemcontainerslot[tuple]);
                } else {
                    items[ linenumber - 1 ] = tempi;
                }
            }
        }

        if ((it=containers.find(BACKPACK + 1)) != containers.end()) {
            backPackContents = it->second;
        } else {
            backPackContents = nullptr;
        }

        for (const auto &depot : depots) {
            auto depotIt = depotContents.find(depot.first);

            if (depotIt != depotContents.end()) {
                depotIt->second = depot.second;
            } else {
                depotIt->second = nullptr;
            }
        }
    } catch (std::exception &e) {
        Logger::error(LogFacility::Player) << "Exception on loading player: " << e.what() << Log::end;
        dataOK = false;
    }

    if (!dataOK) {
        std::map<int, Container *>::reverse_iterator rit;
        std::map<uint32_t,Container *>::reverse_iterator rit2;

        // clean up...
        for (rit=containers.rbegin(); rit != containers.rend(); ++rit) {
            delete rit->second;
        }

        for (rit=depots.rbegin(); rit != depots.rend(); ++rit) {
            delete rit->second;
        }

        for (rit2 = depotContents.rbegin(); rit2 != depotContents.rend(); ++rit2) {
            delete rit2->second;
        }

        backPackContents = nullptr;
    }

    //#endif
    return dataOK;
}

void Player::increasePoisonValue(short int value) {
    if ((poisonvalue == 0) && value > 0) {
        static const std::string german = "Du bist vergiftet.";
        static const std::string english = "You are poisoned.";
        inform(german, english, informScriptMediumPriority);
    }

    if ((poisonvalue + value) >= MAXPOISONVALUE) {
        poisonvalue = MAXPOISONVALUE;
    } else if ((poisonvalue + value) <= 0) {
        poisonvalue = 0;
        static const std::string german = "Es geht dir auf einmal besser.";
        static const std::string english = "You suddenly feel better.";
        inform(german, english, informScriptMediumPriority);
    } else {
        poisonvalue += value;
    }
}

auto Player::getMinActionPoints() const -> short int {
    return P_MIN_AP;
}

auto Player::getMaxActionPoints() const -> short int {
    return P_MAX_AP;
}

auto Player::getMinFightPoints() const -> short int {
    return P_MIN_FP;
}

auto Player::getMaxFightPoints() const -> short int {
    return P_MAX_FP;
}

auto Player::setSkill(TYPE_OF_SKILL_ID skill, short int major, short int minor) -> unsigned short int {
    Character::setSkill(skill, major, minor);
    sendSkill(skill, major, minor);
    return major;
}

auto Player::increaseSkill(TYPE_OF_SKILL_ID skill, short int amount) -> unsigned short int {
    Character::increaseSkill(skill, amount);
    int major = getSkill(skill);
    int minor = getMinorSkill(skill);
    sendSkill(skill, major, minor);
    return major;
}

void Player::turn(direction dir) {
    Character::turn(dir);
    increaseActionPoints(-P_SPIN_COST);
}

void Player::turn(const position &pos) {
    Character::turn(pos);
    increaseActionPoints(-P_SPIN_COST);
}

void Player::receiveText(talk_type tt, const std::string &message, Character *cc) {
    ServerCommandPointer cmd = std::make_shared<SayTC>(cc->getPosition(), message);

    switch (tt) {
    case tt_say:
        Connection->addCommand(cmd);
        break;

    case tt_whisper:
        cmd = std::make_shared<WhisperTC>(cc->getPosition(), message);
        Connection->addCommand(cmd);
        break;

    case tt_yell:
        cmd = std::make_shared<ShoutTC>(cc->getPosition(), message);
        Connection->addCommand(cmd);
        break;
    }
}

auto Player::knows(Player *player) const -> bool {
    return this == player || knownPlayers.find(player->getId()) != knownPlayers.cend();
}

void Player::getToKnow(Player *player) {
    if (!knows(player)) {
        knownPlayers.insert(player->getId());
    }
}

void Player::introducePlayer(Player *player) {
    getToKnow(player);

    ServerCommandPointer cmd = std::make_shared<IntroduceTC>(player->getId(), player->getName());
    Connection->addCommand(cmd);
}

void Player::namePlayer(TYPE_OF_CHARACTER_ID playerId, const std::string &name) {
    const Character *character = World::get()->findCharacter(playerId);

    if (character->getType() == player && this->getId() != playerId) {
        if (name.length() > 0) {
            namedPlayers[playerId] = name;
        } else {
            namedPlayers.erase(playerId);
        }
    }
}

auto Player::getCustomNameOf(Player *player) const -> std::string {
    const auto it = namedPlayers.find(player->getId());

    if (it != namedPlayers.cend()) {
        return it->second;
    } else {
        return {};
    }
}

void Player::deleteAllSkills() {
    Character::deleteAllSkills();
    sendAllSkills();

}

void Player::teachMagic(unsigned char type,unsigned char flag) {
/*
    if (type < 4) {
        unsigned long int tflags=0;

        for (int i = 0; i < 4; ++i) {
            tflags|=magic.flags[ i ];
        }

        if (tflags == 0) {
            switch (type) {
            case 0:
                magic.type = MAGE;
                break;

            case 1:
                magic.type = PRIEST;
                break;

            case 2:
                magic.type = BARD;
                break;

            case 3:
                magic.type = DRUID;
                break;

            default:
                magic.type = MAGE;
                break;
            }
        }

        unsigned long int temp = 1;
        temp <<= flag;
        magic.flags[ type ] |= temp;
        sendMagicFlags(magic.type);
    }
*/
}

void Player::inform(const std::string &message, informType type) const {
    ServerCommandPointer cmd = std::make_shared<InformTC>(type, message);
    Connection->addCommand(cmd);
}

void Player::inform(const std::string &german, const std::string &english, informType type) const {
    inform(nls(german, english), type);
}

void Player::checkBurden() {
    loadLevel = loadFactor();
    auto cmd = std::make_shared<UpdateLoadTC>(LoadWeight(), maxLoadWeight());
    Connection->addCommand(cmd);
}

auto Player::isOvertaxed() -> bool {
    checkBurden();
    auto level = loadFactor();
    return level == LoadLevel::overtaxed;
}

auto Player::moveToPossible(const map::Field &field) const -> bool {
    return Character::moveToPossible(field) || (!getClippingActive() && (isAdmin() || hasGMRight(gmr_isnotshownasgm)));
}

auto Player::move(direction dir, uint8_t mode) -> bool {
    using std::chrono::steady_clock;
    using std::chrono::milliseconds;
    auto now = steady_clock::now();

    if (now + milliseconds(800) < reachingTargetField) {
        auto cmd =
            std::make_shared<MoveAckTC>(getId(), getPosition(), STILLMOVING, 0);
        Connection->addCommand(cmd);
        return false;
    } else if (now > reachingTargetField) {
        reachingTargetField = now;
    }

    _world->TriggerFieldMove(this, false);
    closeOnMove();

    // if we actively move we look into that direction...
    if (mode != PUSH) {
        turn(dir);
    }

    if (mode == RUNNING && loadFactor() != LoadLevel::unburdened) {
        mode = NORMALMOVE;
    }

    size_t steps;
    size_t j = 0;
    bool cont = true;

    if (mode != RUNNING) {
        steps = 1;
    } else {
        steps = 2;
    }

    position newpos, oldpos;
    TYPE_OF_WALKINGCOST walkcost = 0;

    while (j < steps && cont) {

        // check if we can move to our target field
        try {
            oldpos = getPosition();
            newpos = getPosition();
            newpos.move(dir);

            map::Field &oldField = _world->fieldAt(oldpos);
            map::Field &newField = _world->fieldAtOrBelow(newpos);

            bool diagonalMove = oldpos.x != newpos.x && oldpos.y != newpos.y;
            walkcost += getMoveTime(newField, diagonalMove, mode == RUNNING);

            if (isOvertaxed()) {
                ServerCommandPointer cmd = std::make_shared<MoveAckTC>(getId(), getPosition(), NOMOVE, 0);
                Connection->addCommand(cmd);
                return false;
            } else {
                if (mode != RUNNING || j == 1) {
                    increaseActionPoints(walkcost/100);
                }
            }

            if (moveToPossible(newField)) {
                oldField.removeChar();
                newField.setChar();

                if (newpos.z != oldpos.z) {
                    setPosition(newpos);
                    ServerCommandPointer cmd = std::make_shared<MoveAckTC>(getId(), getPosition(), NOMOVE, 0);
                    Connection->addCommand(cmd);
                    cmd = std::make_shared<SetCoordinateTC>(getPosition());
                    Connection->addCommand(cmd);
                    sendFullMap();
                    cont = false;
                } else {
                    if (mode != RUNNING || j == 1) {
                        reachingTargetField += milliseconds(walkcost);
                        ServerCommandPointer cmd = std::make_shared<MoveAckTC>(getId(), newpos, mode, walkcost);
                        Connection->addCommand(cmd);
                    }

                    if (j == 1) {
                        sendStepStripes(dir);
                    }

                    setPosition(newpos);

                    if (mode != RUNNING || j == 1) {
                        sendStepStripes(dir);
                    }
                }

                if (mode != RUNNING || j == 1 || !cont) {
                    _world->sendCharacterMoveToAllVisiblePlayers(this, mode, walkcost);
                    _world->sendAllVisibleCharactersToPlayer(this, true);
                }

                if (newField.isWarp()) {
                    position warpPos;
                    newField.getWarp(warpPos);
                    Warp(warpPos);
                    cont = false;
                }

                _world->checkFieldAfterMove(this, newField);

                _world->TriggerFieldMove(this, true);
                ServerCommandPointer cmd = std::make_shared<BBPlayerMoveTC>(getId(), getPosition());
                _world->monitoringClientList->sendCommand(cmd);

                if (mode != RUNNING || j == 1) {
                    return true;
                }
            } else {
                throw FieldNotFound();
            }
        } catch (FieldNotFound &) {
            if (j == 1) {
                reachingTargetField += milliseconds(walkcost);
                ServerCommandPointer cmd = std::make_shared<MoveAckTC>(getId(), getPosition(), NORMALMOVE, walkcost);
                Connection->addCommand(cmd);
                sendStepStripes(dir);
                _world->sendCharacterMoveToAllVisiblePlayers(this, mode, walkcost);
                _world->sendAllVisibleCharactersToPlayer(this, true);
                return true;
            } else if (j == 0) {
                ServerCommandPointer cmd = std::make_shared<MoveAckTC>(getId(), getPosition(), NOMOVE, 0);
                Connection->addCommand(cmd);
                return false;
            }
        }

        ++j;
    }

    ServerCommandPointer cmd = std::make_shared<MoveAckTC>(getId(), getPosition(), NOMOVE, 0);
    Connection->addCommand(cmd);
    return false;
}


auto Player::Warp(const position &newPos) -> bool {
    bool warped = Character::Warp(newPos);

    if (warped) {
        handleWarp();
        return true;
    }

    return false;
}

auto Player::forceWarp(const position &newPos) -> bool {
    bool warped = Character::forceWarp(newPos);

    if (warped) {
        handleWarp();
        return true;
    }

    return false;
}

void Player::handleWarp() {
    closeOnMove();
    ServerCommandPointer cmd = std::make_shared<SetCoordinateTC>(getPosition());
    Connection->addCommand(cmd);
    sendFullMap();
    visibleChars.clear();
    _world->sendAllVisibleCharactersToPlayer(this, true);
    cmd = std::make_shared<BBPlayerMoveTC>(getId(), getPosition());
    _world->monitoringClientList->sendCommand(cmd);
}

void Player::openDepot(const ScriptItem &item) {
    const auto depotid = item.getDepot();

    if (depotContents.find(depotid) != depotContents.end()) {
        if (depotContents[depotid]) {
            openShowcase(depotContents[depotid], item, false);
        }
    } else {
        depotContents[depotid] = new Container(DEPOTITEM);
        openShowcase(depotContents[depotid], item, false);
    }
}

void Player::changeQualityAt(unsigned char pos, short int amount) {
    Character::changeQualityAt(pos, amount);
    sendCharacterItemAtPos(pos);
    sendCharacterItemAtPos(LEFT_TOOL);
    sendCharacterItemAtPos(RIGHT_TOOL);
}

auto Player::hasGMRight(gm_rights right) const -> bool {
    return ((right & admin) == static_cast<uint32_t>(right));
}

void Player::setQuestProgress(TYPE_OF_QUEST_ID questid, TYPE_OF_QUESTSTATUS progress) {
    if (questWriteLock) {
        LuaScript::writeDebugMsg("Setting quest progress is not allowed in quest entrypoint!");
        return;
    }

    questWriteLock = true;
    using namespace Database;
    PConnection connection = ConnectionManager::getInstance().getConnection();
    int timeNow = int(time(nullptr));

    try {
        connection->beginTransaction();

        SelectQuery query(connection);
        query.addColumn("questprogress", "qpg_progress");
        query.addEqualCondition<TYPE_OF_CHARACTER_ID>("questprogress", "qpg_userid", getId());
        query.addEqualCondition<TYPE_OF_QUEST_ID>("questprogress", "qpg_questid", questid);
        query.addServerTable("questprogress");

        Result results = query.execute();

        // TODO: Save player from dedicated thread only, see PlayerManager
        //save();

        if (results.empty()) {
            InsertQuery insQuery;
            const InsertQuery::columnIndex userColumn = insQuery.addColumn("qpg_userid");
            const InsertQuery::columnIndex questColumn = insQuery.addColumn("qpg_questid");
            const InsertQuery::columnIndex progressColumn = insQuery.addColumn("qpg_progress");
            const InsertQuery::columnIndex timeColumn = insQuery.addColumn("qpg_time");
            insQuery.addServerTable("questprogress");

            insQuery.addValue<TYPE_OF_CHARACTER_ID>(userColumn, getId());
            insQuery.addValue<TYPE_OF_QUEST_ID>(questColumn, questid);
            insQuery.addValue<TYPE_OF_QUESTSTATUS>(progressColumn, progress);
            insQuery.addValue<int>(timeColumn, timeNow);

            insQuery.execute();
        } else {
            UpdateQuery updQuery;
            updQuery.addAssignColumn<TYPE_OF_QUESTSTATUS>("qpg_progress", progress);
            updQuery.addAssignColumn<int>("qpg_time", timeNow);
            updQuery.addEqualCondition<TYPE_OF_CHARACTER_ID>("questprogress", "qpg_userid", getId());
            updQuery.addEqualCondition<TYPE_OF_QUEST_ID>("questprogress", "qpg_questid", questid);
            updQuery.setServerTable("questprogress");

            updQuery.execute();
        }

        connection->commitTransaction();
    } catch (std::exception &e) {
        Logger::error(LogFacility::Script) << "Setting quest progress failed for " << to_string() << ": " << e.what() << Log::end;
        connection->rollbackTransaction();
        questWriteLock = false;
        return;
    }

    quests[questid] = std::make_pair(progress, timeNow);
    sendQuestProgress(questid, progress);
    questWriteLock = false;
}

void Player::sendAvailableQuests() {
    const auto quests = Data::Quests.getQuestsInRange(getPosition(), getScreenRange());
    int someTime;
    std::vector<position> questsAvailableNow;
    std::vector<position> questsAvailableSoon;

    for (const auto &quest : quests) {
        const TYPE_OF_QUEST_ID questId = quest.first;
        const TYPE_OF_QUESTSTATUS questStatus = getQuestProgress(questId, someTime);
        const position &start = quest.second;
        const QuestAvailability availability = Data::Quests.script(questId)->available(this, questStatus);

        if ((availability == questDefaultAvailable && questStatus == 0) || availability == questAvailable) {
            questsAvailableNow.push_back(start);
        } else if (availability == questWillBeAvailable) {
            questsAvailableSoon.push_back(start);
        }
    }

    if (questsAvailableNow.size() > 0 || questsAvailableSoon.size() > 0) {
        ServerCommandPointer cmd = std::make_shared<AvailableQuestsTC>(questsAvailableNow, questsAvailableSoon);
        Connection->addCommand(cmd);
    }
}

void Player::sendQuestProgress(TYPE_OF_QUEST_ID questId, TYPE_OF_QUESTSTATUS progress) {
    if (progress == 0) {
        ServerCommandPointer cmd = std::make_shared<AbortQuestTC>(questId);
        Connection->addCommand(cmd);
        return;
    }

    if (Data::Quests.exists(questId)) {
        const auto &script = Data::Quests.script(questId);

        if (script) {
            std::string title = script->title(this);

            if (title.length() > 0) {
                std::string description = script->description(this, progress);
                TYPE_OF_QUESTSTATUS finalStatus = script->finalStatus();
                std::vector<position> targets;
                script->targets(this, progress, targets);

                ServerCommandPointer cmd = std::make_shared<QuestProgressTC>(questId, title, description, targets, progress == finalStatus);
                Connection->addCommand(cmd);
            }
        }
    }
}

void Player::sendCompleteQuestProgress() {
    for (const auto &quest : quests) {
        TYPE_OF_QUEST_ID questId = quest.first;
        TYPE_OF_QUESTSTATUS progress = quest.second.first;
        sendQuestProgress(questId, progress);
    }
}

auto Player::getQuestProgress(TYPE_OF_QUEST_ID questid, int &time) const -> TYPE_OF_QUESTSTATUS {
    const auto it = quests.find(questid);

    if (it != quests.end()) {
        time = it->second.second;
        return it->second.first;
    } else {
        time = 0;
        return TYPE_OF_QUESTSTATUS(0);
    }
}

void Player::startAction(unsigned short int wait, unsigned short int ani, unsigned short int redoani, unsigned short int sound, unsigned short int redosound) {
    ltAction->startLongTimeAction(wait, ani, redoani, sound, redosound);
}

void Player::abortAction() {
    ltAction->abortAction();
}

void Player::successAction() {
    ltAction->successAction();
}

void Player::actionDisturbed(Character *disturber) {
    ltAction->actionDisturbed(disturber);
}

void Player::changeSource(Character *cc) {
    ltAction->changeSource(cc);
}

void Player::changeSource(const ScriptItem &sI) {
    ltAction->changeSource(sI);
}

void Player::changeSource(const position &pos) {
    ltAction->changeSource(pos);
}

void Player::changeSource() {
    ltAction->changeSource();
}

void Player::changeTarget(Character *cc) {
    ltAction->changeTarget(cc);
}

void Player::changeTarget(const ScriptItem &sI) {
    ltAction->changeTarget(sI);
}

void Player::changeTarget(const position &pos) {
    ltAction->changeTarget(pos);
}

void Player::changeTarget() {
    ltAction->changeTarget();
}

auto Player::getSkillName(TYPE_OF_SKILL_ID s) const -> std::string  {
    if (Data::Skills.exists(s)) {
        const auto &skill = Data::Skills[s];
        return nls(skill.germanName, skill.englishName);
    } else {
        std::string german("unbekannter Skill");
        std::string english("unknown skill");
        return nls(german, english);
    }
}

auto Player::getPlayerLanguage() const -> Language {
    return _player_language;
}

void Player::sendRelativeArea(int8_t zoffs) {
    if ((screenwidth == 0) && (screenheight == 0)) {
        // static view
        int x = getPosition().x;
        int y = getPosition().y - MAP_DIMENSION;
        int z = getPosition().z + zoffs;
        int e = zoffs * 3;

        if (zoffs < 0) {
            x -= e;
            y += e;
            e = 0;
        }

        //schleife von 0ben nach unten durch alle tiles
        World *world = World::get();

        for (int i=0; i <= (MAP_DIMENSION + MAP_DOWN_EXTRA + e) * 2; ++i) {
            world->clientview.fillStripe(position(x,y,z), NewClientView::dir_right, MAP_DIMENSION+1-(i%2));

            if (world->clientview.getExists()) {
                Connection->addCommand(std::make_shared<MapStripeTC>(position(x,y,z), NewClientView::dir_right));
            }

            if (i % 2 == 0) {
                y += 1;
            } else {
                x -= 1;
            }
        }
    } else {
        // dynamic view
        int x = getPosition().x - screenwidth + screenheight;
        int y = getPosition().y - screenwidth - screenheight;
        int z = getPosition().z + zoffs;
        int e = zoffs * 3;

        if (zoffs < 0) {
            x -= e;
            y += e;
            e = 0;
        }

        //schleife von 0ben nach unten durch alle tiles
        World *world = World::get();

        for (int i=0; i <= (2*screenheight + MAP_DOWN_EXTRA + e) * 2; ++i) {
            world->clientview.fillStripe(position(x,y,z), NewClientView::dir_right, 2*screenwidth+1-(i%2));

            if (world->clientview.getExists()) {
                Connection->addCommand(std::make_shared<MapStripeTC>(position(x,y,z), NewClientView::dir_right));
            }

            if (i % 2 == 0) {
                y += 1;
            } else {
                x -= 1;
            }
        }
    }
}

void Player::sendFullMap() {
    for (int8_t i = -2; i <= 2; ++i) {
        sendRelativeArea(i);
    }

    Connection->addCommand(std::make_shared<MapCompleteTC>());
}

void Player::sendDirStripe(viewdir direction, bool extraStripeForDiagonalMove) {
    const auto &pos = getPosition();

    if ((screenwidth == 0) && (screenheight == 0)) {
        // static view
        int x = {}, y = {};
        NewClientView::stripedirection dir = {};
        int length = MAP_DIMENSION + 1;

        switch (direction) {
        case upper:
            x = pos.x;
            y = pos.y - MAP_DIMENSION;
            dir = NewClientView::dir_right;

            if (extraStripeForDiagonalMove) {
                --x;
            }

            break;

        case left:
            x = pos.x;
            y = pos.y - MAP_DIMENSION;
            dir = NewClientView::dir_down;
            length += MAP_DOWN_EXTRA;

            if (extraStripeForDiagonalMove) {
                ++x;
            }

            break;

        case right:
            x = pos.x + MAP_DIMENSION;
            y = pos.y;
            dir = NewClientView::dir_down;
            length += MAP_DOWN_EXTRA;

            if (extraStripeForDiagonalMove) {
                --y;
            }

            break;

        case lower:
            x = pos.x - MAP_DIMENSION - MAP_DOWN_EXTRA;
            y = pos.y + MAP_DOWN_EXTRA;
            dir = NewClientView::dir_right;

            if (extraStripeForDiagonalMove) {
                --y;
            }

            break;
        }

        NewClientView *view = &(World::get()->clientview);

        for (int z = - 2; z <= 2; ++z) {
            int e = (direction != lower && z > 0) ? z*3 : 0; // left, right and upper stripes moved up if z>0 to provide the client with info for detecting roofs
            int l = (dir == NewClientView::dir_down && z > 0) ? e : 0; // right and left stripes have to become longer then

            if (extraStripeForDiagonalMove) {
                ++l;
            }

            view->fillStripe(position(x-z*3+e,y+z*3-e,pos.z+z), dir, length+l);

            if (view->getExists()) {
                Connection->addCommand(std::make_shared<MapStripeTC>(position(x-z*3+e,y+z*3-e,pos.z+z), dir));
            }
        }
    } else {
        // dynamic view
        int x = {}, y = {};
        NewClientView::stripedirection dir = {};
        int length = {};

        switch (direction) {
        case upper:
            x = pos.x - screenwidth + screenheight;
            y = pos.y - screenwidth - screenheight;
            dir = NewClientView::dir_right;
            length = 2*screenwidth + 1;

            if (extraStripeForDiagonalMove) {
                --x;
            }

            break;

        case left:
            x = pos.x - screenwidth + screenheight;
            y = pos.y - screenwidth - screenheight;
            dir = NewClientView::dir_down;
            length = 2*screenheight + 1 + MAP_DOWN_EXTRA;

            if (extraStripeForDiagonalMove) {
                ++x;
            }

            break;

        case right:
            x = pos.x + screenwidth + screenheight;
            y = pos.y + screenwidth - screenheight;
            dir = NewClientView::dir_down;
            length = 2*screenheight + 1 + MAP_DOWN_EXTRA;

            if (extraStripeForDiagonalMove) {
                --y;
            }

            break;

        case lower:
            x = pos.x - screenwidth - screenheight - MAP_DOWN_EXTRA;
            y = pos.y - screenwidth + screenheight + MAP_DOWN_EXTRA;
            dir = NewClientView::dir_right;
            length = 2*screenwidth + 1;

            if (extraStripeForDiagonalMove) {
                --y;
            }

            break;
        }

        NewClientView *view = &(World::get()->clientview);

        for (int z = - 2; z <= 2; ++z) {
            int e = (direction != lower && z > 0) ? z*3 : 0; // left, right and upper stripes moved up if z>0 to provide the client with info for detecting roofs
            int l = (dir == NewClientView::dir_down && z > 0) ? e : 0; // right and left stripes have to become longer then

            if (extraStripeForDiagonalMove) {
                ++l;
            }

            view->fillStripe(position(x-z*3+e,y+z*3-e,pos.z+z), dir, length+l);

            if (view->getExists()) {
                Connection->addCommand(std::make_shared<MapStripeTC>(position(x-z*3+e,y+z*3-e,pos.z+z), dir));
            }
        }
    }
}

void Player::sendStepStripes(direction dir) {
    switch (dir) {
    case (dir_north):
        //bewegung nach norden (Mapstripe links und oben)
        sendDirStripe(upper, false);
        sendDirStripe(left, false);
        break;

    case (dir_northeast):
        //bewegung nach nordosten (Mapstripe oben)
        sendDirStripe(upper, true);
        sendDirStripe(upper, false);
        break;

    case (dir_east) :
        //bewegung nach osten (Mapstripe oben und rechts)
        sendDirStripe(upper, false);
        sendDirStripe(right, false);
        break;

    case (dir_southeast):
        //bewegung suedosten (Mapstripe  rechts)
        sendDirStripe(right, true);
        sendDirStripe(right, false);
        break;

    case (dir_south):
        //bewegung sueden (Mapstripe rechts und unten)
        sendDirStripe(right, false);
        sendDirStripe(lower, false);
        break;

    case (dir_southwest):
        //bewegung suedwesten ( Mapstripe unten )
        sendDirStripe(lower, true);
        sendDirStripe(lower, false);
        break;

    case (dir_west):
        //bewegung westen ( Mapstripe unten und links)
        sendDirStripe(lower, false);
        sendDirStripe(left, false);
        break;

    case (dir_northwest):
        //bewegung nordwesten ( Mapstripe links )
        sendDirStripe(left, true);
        sendDirStripe(left, false);
        break;

    default:
        break;
    }
}

void Player::sendField(const position &pos) {
    auto &view = World::get()->clientview;
    view.fillStripe(pos, NewClientView::dir_right, 1);

    if (view.getExists()) {
        Connection->addCommand(std::make_shared<MapStripeTC>(pos, NewClientView::dir_right));
    }
}

auto Player::idleTime() const -> uint32_t {
    time_t now;
    time(&now);
    return now - lastaction;
}

void Player::sendBook(uint16_t bookID) {
    ServerCommandPointer cmd = std::make_shared<BookTC>(bookID);
    Connection->addCommand(cmd);
}

void Player::stopAttack() {
    Character::stopAttack();

    ServerCommandPointer cmd = std::make_shared<TargetLostTC>();
    Connection->addCommand(cmd);
}

auto Player::isNewPlayer() const -> bool {
    return newPlayer;
}

auto Player::nls(const std::string &german, const std::string &english) const -> const std::string & {
    switch (getPlayerLanguage()) {
    case Language::german:
        return german;

    default:
        return english;
    }
}

auto Player::pageGM(const std::string &ticket) -> bool {
    try {
        _world->logGMTicket(this, "[Auto] " + ticket, true);
        return true;
    } catch (...) {
    }

    return false;
}


void Player::sendCharDescription(TYPE_OF_CHARACTER_ID id, const std::string &desc) {
    ServerCommandPointer cmd = std::make_shared<CharDescription>(id, desc);
    Connection->addCommand(cmd);
}

void Player::sendCharAppearance(TYPE_OF_CHARACTER_ID id, const ServerCommandPointer &appearance, bool always) {
    //send appearance always or only if the char in question just appeared
    if (always || visibleChars.insert(id).second) {
        Connection->addCommand(appearance);
    }
}

void Player::sendCharRemove(TYPE_OF_CHARACTER_ID id, const ServerCommandPointer &removechar) {
    if (this->getId() != id) {
        visibleChars.erase(id);
        Connection->addCommand(removechar);
    }
}

void Player::requestInputDialog(InputDialog *inputDialog) {
    requestDialog<InputDialog, InputDialogTC>(inputDialog);
}

void Player::executeInputDialog(unsigned int dialogId, bool success, const std::string &input) {
    auto inputDialog = getDialog<InputDialog>(dialogId);

    if (inputDialog) {
        inputDialog->setSuccess(success);

        if (success) {
            inputDialog->setInput(input);
        }

        LuaScript::executeDialogCallback(*inputDialog);
    }

    dialogs.erase(dialogId);
}

void Player::requestMessageDialog(MessageDialog *messageDialog) {
    requestDialog<MessageDialog, MessageDialogTC>(messageDialog);
}

void Player::executeMessageDialog(unsigned int dialogId) {
    auto messageDialog = getDialog<MessageDialog>(dialogId);

    if (messageDialog) {
        LuaScript::executeDialogCallback(*messageDialog);
    }

    dialogs.erase(dialogId);
}

void Player::requestMerchantDialog(MerchantDialog *merchantDialog) {
    requestDialog<MerchantDialog, MerchantDialogTC>(merchantDialog);
}

void Player::executeMerchantDialogAbort(unsigned int dialogId) {
    auto merchantDialog = getDialog<MerchantDialog>(dialogId);

    if (merchantDialog) {
        merchantDialog->setResult(MerchantDialog::playerAborts);
        merchantDialog->setPurchaseIndex(0);
        merchantDialog->setPurchaseAmount(0);
        ScriptItem item;
        merchantDialog->setSaleItem(item);
        LuaScript::executeDialogCallback(*merchantDialog);
    }

    dialogs.erase(dialogId);
}

void Player::executeMerchantDialogBuy(unsigned int dialogId, MerchantDialog::index_type index, Item::number_type amount) const {
    auto merchantDialog = getDialog<MerchantDialog>(dialogId);

    if (merchantDialog) {
        merchantDialog->setResult(MerchantDialog::playerBuys);
        merchantDialog->setPurchaseIndex(index);
        merchantDialog->setPurchaseAmount(amount);
        ScriptItem item;
        merchantDialog->setSaleItem(item);
        LuaScript::executeDialogCallback(*merchantDialog);
    }
}

void Player::executeMerchantDialogSell(unsigned int dialogId, uint8_t location, TYPE_OF_CONTAINERSLOTS slot, Item::number_type amount) {
    auto merchantDialog = getDialog<MerchantDialog>(dialogId);

    if (merchantDialog) {
        merchantDialog->setResult(MerchantDialog::playerSells);
        merchantDialog->setPurchaseIndex(0);
        merchantDialog->setPurchaseAmount(0);
        ScriptItem item;

        if (location == 0) {
            item = GetItemAt(slot);
        } else {
            if (isShowcaseOpen(location-1)) {
                Container *container = nullptr;
                getShowcaseContainer(location-1)->viewItemNr(slot, item, container);
            }
        }

        if (amount < item.getNumber()) {
            item.setNumber(amount);
        }

        merchantDialog->setSaleItem(item);
        LuaScript::executeDialogCallback(*merchantDialog);
    }
}

void Player::executeMerchantDialogLookAt(unsigned int dialogId, uint8_t list, uint8_t slot) {
    auto merchantDialog = getDialog<MerchantDialog>(dialogId);

    if (merchantDialog) {
        merchantDialog->setResult(MerchantDialog::playerLooksAt);
        merchantDialog->setLookAtList(static_cast<MerchantDialog::ListType>(list));
        merchantDialog->setPurchaseIndex(slot);
        auto lookAt = LuaScript::executeDialogCallback<ItemLookAt>(*merchantDialog);

        ServerCommandPointer cmd = std::make_shared<LookAtDialogGroupItemTC>(dialogId, list, slot, lookAt);
        Connection->addCommand(cmd);        
    }
}

void Player::requestSelectionDialog(SelectionDialog *selectionDialog) {
    requestDialog<SelectionDialog, SelectionDialogTC>(selectionDialog);
}

void Player::executeSelectionDialog(unsigned int dialogId, bool success, SelectionDialog::index_type index) {
    auto selectionDialog = getDialog<SelectionDialog>(dialogId);

    if (selectionDialog) {
        selectionDialog->setSuccess(success);

        if (success) {
            selectionDialog->setSelectedIndex(index);
        }

        LuaScript::executeDialogCallback(*selectionDialog);
    }

    dialogs.erase(dialogId);
}

void Player::requestCraftingDialog(CraftingDialog *craftingDialog) {
    requestDialog<CraftingDialog, CraftingDialogTC>(craftingDialog);
}

void Player::executeCraftingDialogAbort(unsigned int dialogId) {
    auto craftingDialog = getDialog<CraftingDialog>(dialogId);

    if (craftingDialog) {
        craftingDialog->setResult(CraftingDialog::playerAborts);
        LuaScript::executeDialogCallback(*craftingDialog);
    }

    dialogs.erase(dialogId);
}

void Player::executeCraftingDialogCraft(unsigned int dialogId, uint8_t craftIndex, uint8_t craftAmount) {
    auto craftingDialog = getDialog<CraftingDialog>(dialogId);

    if (craftingDialog) {
        craftingDialog->setResult(CraftingDialog::playerCrafts);
        craftingDialog->setCraftableId(craftIndex);
        craftingDialog->setCraftableAmount(craftAmount);
        bool craftingPossible = LuaScript::executeDialogCallback<bool>(*craftingDialog);

        if (craftingPossible) {
            abortAction();

            auto stillToCraft = craftingDialog->getCraftableAmount();
            auto craftingTime = craftingDialog->getCraftableTime();
            auto sfx = craftingDialog->getSfx();
            auto sfxDuration = craftingDialog->getSfxDuration();
            startCrafting(stillToCraft, craftingTime, sfx, sfxDuration, dialogId);
        }
    }
}

void Player::executeCraftingDialogCraftingComplete(unsigned int dialogId) {
    auto craftingDialog = getDialog<CraftingDialog>(dialogId);

    if (craftingDialog) {
        craftingDialog->setResult(CraftingDialog::playerCraftingComplete);
        bool renewProductList = LuaScript::executeDialogCallback<bool>(*craftingDialog);

        ServerCommandPointer cmd = std::make_shared<CraftingDialogCraftingCompleteTC>(dialogId);
        Connection->addCommand(cmd);

        if (renewProductList) {
            cmd = std::make_shared<CraftingDialogTC>(*craftingDialog, dialogId);
            Connection->addCommand(cmd);
        }

        auto stillToCraft = craftingDialog->getCraftableAmount() - 1;

        if (stillToCraft > 0) {
            auto craftIndex = craftingDialog->getCraftableId();
            executeCraftingDialogCraft(dialogId, craftIndex, stillToCraft);
        }
    }
}

void Player::executeCraftingDialogCraftingAborted(unsigned int dialogId) {
    auto craftingDialog = getDialog<CraftingDialog>(dialogId);

    if (craftingDialog) {
        craftingDialog->setResult(CraftingDialog::playerCraftingAborted);
        LuaScript::executeDialogCallback(*craftingDialog);

        ServerCommandPointer cmd = std::make_shared<CraftingDialogCraftingAbortedTC>(dialogId);
        Connection->addCommand(cmd);
    }
}

void Player::executeCraftingDialogLookAtCraftable(unsigned int dialogId, uint8_t craftIndex) {
    auto craftingDialog = getDialog<CraftingDialog>(dialogId);

    if (craftingDialog) {
        craftingDialog->setResult(CraftingDialog::playerLooksAtCraftable);
        craftingDialog->setCraftableId(craftIndex);
        auto lookAt = LuaScript::executeDialogCallback<ItemLookAt>(*craftingDialog);
        requestCraftingLookAt(dialogId, lookAt);
    }
}

void Player::executeCraftingDialogLookAtIngredient(unsigned int dialogId, uint8_t craftIndex, uint8_t craftIngredient) {
    auto craftingDialog = getDialog<CraftingDialog>(dialogId);

    if (craftingDialog) {
        craftingDialog->setResult(CraftingDialog::playerLooksAtIngredient);
        craftingDialog->setCraftableId(craftIndex);
        craftingDialog->setIngredientIndex(craftIngredient);
        auto lookAt = LuaScript::executeDialogCallback<ItemLookAt>(*craftingDialog);
        requestCraftingLookAtIngredient(dialogId, lookAt);
    }
}

void Player::requestCraftingLookAt(unsigned int dialogId, ItemLookAt &lookAt) {
    auto craftingDialog = getDialog<CraftingDialog>(dialogId);

    if (craftingDialog) {
        ServerCommandPointer cmd = std::make_shared<LookAtDialogItemTC>(dialogId, craftingDialog->getCraftableId(), lookAt);
        Connection->addCommand(cmd);
    }
}

void Player::requestCraftingLookAtIngredient(unsigned int dialogId, ItemLookAt &lookAt) {
    auto craftingDialog = getDialog<CraftingDialog>(dialogId);

    if (craftingDialog) {
        ServerCommandPointer cmd = std::make_shared<LookAtDialogGroupItemTC>(dialogId, craftingDialog->getCraftableId(), craftingDialog->getIngredientIndex(), lookAt);
        Connection->addCommand(cmd);
    }
}

void Player::startCrafting(uint8_t stillToCraft, uint16_t craftingTime, uint16_t sfx, uint16_t sfxDuration, uint32_t dialogId) {
    SouTar source;
    source.Type = LUA_DIALOG;
    source.dialog = dialogId;
    SouTar target;
    ltAction->setLastAction(std::shared_ptr<LuaScript>(), source, target, LongTimeAction::ACTION_CRAFT);
    startAction(craftingTime, 0, 0, sfx, sfxDuration);

    ServerCommandPointer cmd = std::make_shared<CraftingDialogCraftTC>(stillToCraft, craftingTime, dialogId);
    Connection->addCommand(cmd);
}

void Player::invalidateDialogs() {
    for (const auto &dialog : dialogs) {
        if (dialog.second) {
            ServerCommandPointer cmd = std::make_shared<CloseDialogTC>(dialog.first);
            Connection->addCommand(cmd);
        }
    }

    dialogs.clear();
}

void Player::closeDialogsOnMove() {
    for (auto it = dialogs.begin(); it != dialogs.end();) {

        if (it->second && it->second->closeOnMove()) {
            ServerCommandPointer cmd = std::make_shared<CloseDialogTC>(it->first);
            Connection->addCommand(cmd);
            it = dialogs.erase(it);
        } else if (!it->second) {
            it = dialogs.erase(it);
        } else {
            ++it;
        }
    }
}

void Player::logAdmin(const std::string &message) {
    Logger::info(LogFacility::Admin) << to_string() << " uses admin tool: " << message << Log::end;
}

auto Player::to_string() const -> std::string {
    return (isAdmin()?"Admin ":"Player ") + getName() + "(" + std::to_string(getId()) + ")";
}

auto Player::actionRunning() const -> bool {
	return ltAction->actionRunning();
}
