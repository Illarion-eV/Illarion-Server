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


#ifndef _PLAYER_HPP_
#define _PLAYER_HPP_

#include <chrono>
#include <memory>
#include <string>
#include <set>
#include <queue>
#include <unordered_set>
#include <unordered_map>

#include "Character.hpp"

#include "Showcase.hpp"
#include "Item.hpp"
#include "netinterface/BasicServerCommand.hpp"
#include "netinterface/NetInterface.hpp"
#include "dialog/MerchantDialog.hpp"
#include "dialog/SelectionDialog.hpp"
#include "script/LuaScript.hpp"


struct WeatherStruct;
class Dialog;
class Timer;
class LongTimeAction;

enum gm_rights {
    gmr_allowlogin = 1, //GM is allowed to login if nologin is true
    gmr_basiccommands = 2, //Basic Commands like !who !what !? !fi and !inform
    gmr_warp = 4, //GM is allowed to Warp (includes #jump_to)
    gmr_summon = 8, // GM is allowed to Summon
    gmr_undef = 16,
    gmr_settiles = 32, //GM is allowed to change tiles (includes ton and toff command)
    gmr_clipping = 64, //GM is allowed to change his clipping state (Walk trough walls)
    gmr_warpfields = 128, //GM is allowed to manipulate Warpfields
    gmr_import = 256, //GM is allowed to import maps and Warpfields. includes also createArea
    gmr_visible = 512, //GM is allowed to change his visiblity state
    gmr_reload = 1024, //GM is allowed to reload the tabels (With #r or !rd) includes the right to set the spawnstate !setspawn
    gmr_ban = 2048, //GM is allowed to ban players.
    gmr_loginstate = 4096, //GM is allowed to change loginstate
    gmr_save = 8192, //GM is allowed to save players and maps (!playersave and #mapsave)
    gmr_broadcast = 16384, //GM is allowed to broadcast messages
    gmr_forcelogout = 32768, //GM is allowed to force a logout of players
    gmr_getgmcalls = 65536, //Char gets GM messages
    gmr_isnotshownasgm = 131072 //Char is not shown at gm in the list/ has no officially gm state.
};

//! ein Spieler in newmud
class Player : public Character {

public:

    ////////////////////////////////////////
    // possible Exceptions thrown by Player
    ////////////////////////////////////////
    // exception is thrown if logout of player is only possible result
    class LogoutException {
    public:
        LogoutException(const char &reason) : m_reason(reason) {}
        inline const char &getReason() {
            return m_reason;
        }
    private:
        char m_reason;
    };

    enum viewdir {
        upper = 0,
        right = 1,
        lower = 2,
        left = 3
    };

    typedef enum {
        upperleft = 0,
        upperright = 1,
        lowerleft = 2,
        lowerright = 3
    } mapreferences;

    virtual unsigned short getType() const override {
        return player;
    }

    virtual std::string to_string() const override;

    void workoutCommands();
    void checkFightMode();

    //! definiert eine "Zweiseitige Warteschlange" vom Typ unsigned char
    //typedef deque<unsigned char> BYTEDEQUE;

    //! die letzte IP des Spielers als std::string
    std::string last_ip;

    //BYTEDEQUE//
    //! alle Langzeiteffekte die auf den Spieler einwirken
    int longTimeEffects;

    //! Passwort
    std::string pw;

    //! E-Mail
    std::string email;

    bool newPlayer = false;

    //! Zeit (in Sekunden) die der Spieler insgesamt online war
    unsigned long int onlinetime;

    time_t logintime;

    //! Zeitpunkt zu dem die Klasseninstanz erstellt bzw. das letzte mal gespeichert wurde
    time_t lastsavetime;

    //! Zeitpunkt der letzten Lebensmeldung vom Client
    time_t lastkeepalive;

    //! Zeitpunkt der letzten Aktion des Spielers
    time_t lastaction;

    std::chrono::time_point<std::chrono::steady_clock> reachingTargetField = {};

    //! Location
    std::string location;

    //! Realer Name
    std::string realname;

    //! Screen resolution;
    uint8_t screenwidth;
    uint8_t screenheight;

    virtual unsigned short int getScreenRange() const override;

    //! die Verbindung zum Spieler, -- Achtung ! Die Verbindung wird NICHT im Destruktor gel�cht
    // , da sie auch extern erstellt wird und durch das Einfgen in diverse
    // Vektoren oft Destruktoren fr tempor�e Player aufgerufen werden, die noch
    // ben�igte Verbindungen l�chen wrden!
    std::shared_ptr<NetInterface> Connection;

private:
    std::set<uint32_t> visibleChars;
    std::unordered_set<TYPE_OF_CHARACTER_ID> knownPlayers;
    std::unordered_map<TYPE_OF_CHARACTER_ID, std::string> namedPlayers;
    typedef std::queue<ClientCommandPointer> CLIENTCOMMANDLIST;
    CLIENTCOMMANDLIST immediateCommands;
    CLIENTCOMMANDLIST queuedCommands;
    std::mutex commandMutex;

public:
    void receiveCommand(ClientCommandPointer cmd);

    virtual bool isNewPlayer() const override;
    
    const std::string &nls(const std::string &german, const std::string &english) const;

    virtual void setAlive(bool alive) override;

    void checkBurden();

    virtual bool pageGM(const std::string &ticket) override;

    // send a char appearance; always or only if char not yet visible
    void sendCharAppearance(TYPE_OF_CHARACTER_ID id, const ServerCommandPointer &appearance, bool always);

    // removes a Char from sight
    void sendCharRemove(TYPE_OF_CHARACTER_ID id, const ServerCommandPointer &removechar);


    /**
    *a long time needed action for the player
    */
    std::unique_ptr<LongTimeAction> ltAction;

    virtual void startAction(unsigned short int wait, unsigned short int ani=0, unsigned short int redoani=0, unsigned short int sound=0, unsigned short int redosound=0) override;
    virtual void abortAction() override;
    virtual void successAction() override;
    virtual void actionDisturbed(Character *disturber) override;

    /**
    *changes the source of the last action of this Player
    *<b>Lua: [:changeSource]</b>
    *@param cc source is a character the pointer to this character
    */
    virtual void changeSource(Character *cc) override;

    /**
    *changes the source of the last action for this player
    *<b>Lua: [:changeSource]</b>
    *@param sI source is a item the new item
    */
    inline virtual void changeSource(const ScriptItem &sI) override;

    /**
    *changes the Source of the last action for this player.
    *<b>Lua: [:changeSource]</b>
    *@param pos source is a position the new position
    */
    inline virtual void changeSource(const position &pos) override;

    /**
    *changes the Source of the last action to nothing for this player
    *<b>Lua: [:changeSource]</b>
    */
    inline virtual void changeSource() override;

    /**
    *changes the Target of the last action for this player.
    *<b>Lua: [:changeTarget]</b>
    *@param cc target is a character the pointer to this character
    */
    inline virtual void changeTarget(Character *cc) override;

    /**
    *changes the Target of the last action for this player.
    *<b>Lua: [:changeTarget]</b>
    *@param sI target is a item the new item
    */
    inline virtual void changeTarget(const ScriptItem &sI) override;

    /**
    *changes the target of the last action for this player.
    *<b>Lua: [:changeTarget]</b>
    *@param pos Target is a position the new position
    */
    inline virtual void changeTarget(const position &pos) override;

    /**
    *changes the Target of the last action to nothing for this player
    *<b>Lua: [:changeTarget]</b>
    */
    inline virtual void changeTarget() override;

    /**
    * returns the number of seconds the player has been idle, not actively issuing commands
    * <b>Lua: [:idleTime]</b>
    * @return number of seconds the player has been idle
    */
    virtual uint32_t idleTime() const override;

    /**
    * send a book ID to the client
    * <b>Lua: [:sendBook]</b>
    * @param bookID id of the book
    */
    virtual void sendBook(uint16_t bookID) override;

    /**
    * send a character description to the player if the  char is a player
    * <b>Lua: [:sendCharDescription]</b>
    * @param id of the character from which the description is sended
    * @param desc the current descpription
    */
    virtual void sendCharDescription(TYPE_OF_CHARACTER_ID id,const std::string &desc) override;

    //! normal constructor
    Player(std::shared_ptr<NetInterface> newConnection) throw(LogoutException);

    //! check if username/password is ok
    void check_logindata() throw(LogoutException);

    //Checks if a Player has a special GM right
    bool hasGMRight(gm_rights right) const;

    //! save char to db
    bool save() throw();

    //! load data from db
    // \param no_attributes don't load contents of table "player"
    bool load() throw();

    void login() throw(LogoutException);

    //Loads the GM Flag of the character
    bool loadGMFlags() throw();

    /**
    * sends one area relative to the current z coordinate to the player
    * @param zoffs the offset of the z param of the area which should be sended
    */
    void sendRelativeArea(int8_t zoffs);

    /**
         * sends all areas
         */
    void sendFullMap();

    /**
    * sends one complete mapstripe ( z-2, z-1, z, z+1, z+2) to the client
    * @param direction the direction from which the whole stripe has to be sent
    * @param extraStripeForDiagonalMove send an additional stripe for diagonal moves
    */
    void sendDirStripe(viewdir direction, bool extraStripeForDiagonalMove);

    void sendStepStripes(direction dir);

    /**
    * sends one single mapstripe to the client
    * @param direction of the single stripe
    * @param zoffs the offset to the z direction which should be sended
    */
    void sendSingleStripe(viewdir direction, int8_t zoffs);

    virtual bool actionRunning() const override;

    virtual void increasePoisonValue(short int value) override;

    virtual short int getMinActionPoints() const override;
    virtual short int getMaxActionPoints() const override;
    virtual short int getMinFightPoints() const override;
    virtual short int getMaxFightPoints() const override;

    void openShowcase(Container *container, const ScriptItem &item, bool carry);
    void updateShowcase(Container *container) const;
    void updateShowcaseSlot(Container *container, TYPE_OF_CONTAINERSLOTS slot) const;
    bool isShowcaseOpen(uint8_t showcase) const;
    bool isShowcaseOpen(Container *container) const;
    bool isShowcaseInInventory(uint8_t showcase) const;
    uint8_t getShowcaseId(Container *container) const;
    Container *getShowcaseContainer(uint8_t showcase) const;
    void closeShowcase(uint8_t showcase);
    void closeShowcase(Container *container);
    void closeOnMove();
    void closeAllShowcasesOfMapContainers();
    void closeAllShowcases();
    void lookIntoShowcaseContainer(uint8_t showcase, unsigned char pos);
    bool lookIntoBackPack();
    bool lookIntoContainerOnField(direction dir);

    //�dert die Qualit� eines Items an einer bestimmten Stelle
    void changeQualityAt(unsigned char pos, short int amount);

    inline bool isMonitoringClient() {
        return monitoringClient;
    }

    //! L�st den Player Magie lernen. Fr standard Charactere keine Funktion in Player berladen
    //\param type Magierichtung die gelernt werden soll (0 Magier, 1 Priester, 2 Barde, 3 Druide)
    //\param flag Magieflags die gelernt werden sollen
    virtual void teachMagic(unsigned char type, unsigned char flag) override;

    //! Sets the active magic type of a player (druid/mage/...)
    //\param newMagType Magierichtung, die jetzt aktiv ist (0 Magier, 1 Priester, 2 Barde, 3 Druide)
    virtual void setMagicType(magic_type newMagType) override {
        Character::setMagicType(newMagType);
        sendMagicFlags(newMagType);
    }

    virtual std::string getSkillName(TYPE_OF_SKILL_ID s) const override;
    //! Returns the language which the player specified when creating the Character (german/english)
    virtual Language getPlayerLanguage() const override;

    void sendCharacters();

    //! schickt an den Spieler die Daten des Items an
    // einer Positon des K�pers
    // \param cpos die Position des Item am K�per
    void sendCharacterItemAtPos(unsigned char cpos);

    /**
    *Sends the weather to the client
    *@weather the weather struct of the weather which is sended
    *@interior ist the player inside outside?
    */
    void sendWeather(WeatherStruct weather);

    void ageInventory();

    virtual int createItem(Item::id_type id, Item::number_type number, Item::quality_type quality, script_data_exchangemap const *data) override;

    virtual void learn(TYPE_OF_SKILL_ID skill, uint32_t actionPoints, uint8_t opponent) override;

    virtual unsigned short int increaseSkill(TYPE_OF_SKILL_ID skill, short int amount) override;

    virtual void deleteAllSkills() override;

    //! löscht count Items mit der ID itemid aus dem Inventory des Player
    // und schickt ein Update an den Spieler
    // \param itemid die Id der zu löschenden Items
    // \param count die Anzahl der zu löschenden Items
    // \oaram data Datawert der zu löschenden Items
    // \return Anzahl der Items, die nicht gelöscht werden konnten
    virtual int eraseItem(TYPE_OF_ITEM_ID itemid, int count, script_data_exchangemap const *data = nullptr) override;

    //! ver�dert die Anzahl des Item an der Position pos um count
    // und schickt ein Update an den Spieler
    // \param pos die Stelle im Inventory die ge�dert werden soll
    // \param count die �derungsanzahl
    // \return der Rest von count, der nicht ge�dert werden konnnte
    int increaseAtPos(unsigned char pos, int count);

    int createAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, int count);

    //! setzt die Id des Item an der Position pos auf newid
    // und schickt ein Update an den Spieler
    // \param pos die Stelle im Inventory die ge�dert werden soll
    // \param newid die neue Id des Item
    // \return true falls erfolgreich, false sonst
    bool swapAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, uint16_t newQuality = 0);

    //! schickt ein Update der Ansicht des Rucksackinhalts an den Client
    void updateBackPackView();

    //! sendet alle Namen der Skills des Player mit den entsprechenden Typen/Werten an den Client
    void sendAllSkills();

    //! sendet Magie-Flags an den Client
    // \param der Magietyp fr den die Flags an den Client gechickt werden sollen
    void sendMagicFlags(int type);

    void sendSkill(TYPE_OF_SKILL_ID skill, unsigned short int major, unsigned short int minor);

    unsigned short int setSkill(TYPE_OF_SKILL_ID skill, short int major, short int minor);

    virtual bool saveBaseAttributes() override;

    //! sendet ein Attributupdate an den Client
    // \param name der Name des Attributs
    // \param value der Wert des Attributs
    void sendAttrib(Character::attributeIndex attribute);

    virtual void handleAttributeChange(Character::attributeIndex attribute) override;


    void startMusic(short int which);
    void defaultMusic();

    bool sendTextInFile(const std::string &filename);

    // Setters and Getters //
    unsigned char GetStatus() const;
    void SetStatus(unsigned char thisStatus);

    // What time does the status get reset?
    time_t GetStatusTime() const;
    void SetStatusTime(time_t thisStatustime);

    // Who banned/jailed the player?
    std::string GetStatusGM() const;
    void SetStatusGM(TYPE_OF_CHARACTER_ID thisStatusGM);

    // Why where they banned/jailed?
    std::string GetStatusReason() const;
    void SetStatusReason(const std::string &thisStatusreason);

    // World Map Turtle Graphics
    void setTurtleActive(bool tturtleActive);
    bool getTurtleActive() const;
    void setTurtleTile(unsigned char tturtletile);
    unsigned char getTurtleTile() const;

    // Clipping on/off (default to on)
    void setClippingActive(bool tclippingActive);
    bool getClippingActive() const;

    //Set for Admin state, uin32_t bit flag
    void setAdmin(uint32_t tAdmin);
    virtual bool isAdmin() const override;

    // player gets informed about something
    virtual void inform(const std::string &text, informType type = informServer) const override;
    virtual void inform(const std::string &german, const std::string &english, informType type = informServer) const override;

    virtual void turn(direction dir) override;
    virtual void turn(const position &posi) override;

    // player heard something
    virtual void receiveText(talk_type tt, const std::string &message, Character *cc) override;

    bool knows(Player *player) const;
    void getToKnow(Player *player);
    virtual void introducePlayer(Player *player) override;
    void namePlayer(TYPE_OF_CHARACTER_ID playerId, const std::string &name);
    std::string getCustomNameOf(Player *player) const;

    virtual bool moveToPossible(const Field &field) const override;
    // Move the Player
    using Character::move;
    bool move(direction dir, uint8_t mode);

    bool isOvertaxed();

    virtual bool Warp(const position &newPos) override;
    virtual bool forceWarp(const position &newPos) override;

    void openDepot(const ScriptItem &item);

    virtual void setQuestProgress(TYPE_OF_QUEST_ID questid, TYPE_OF_QUESTSTATUS progress) override;
    void sendAvailableQuests();
    void sendQuestProgress(TYPE_OF_QUEST_ID questId, TYPE_OF_QUESTSTATUS progress);
    void sendCompleteQuestProgress();
    virtual TYPE_OF_QUESTSTATUS getQuestProgress(TYPE_OF_QUEST_ID questid, int &time) const override;

private:
    void handleWarp();

    template<class DialogType, class DialogCommandType>
    void requestDialog(DialogType *dialog) {
        if (dialog == 0) {
            LuaScript::triggerScriptError("Dialog must not be nil!");
        }

        if (dialogs.size() < 100) {
            unsigned int dialogId = dialogCounter;

            while (dialogs.find(dialogId) != dialogs.end()) {
                ++dialogId;
            }

            dialogs[dialogId] = std::make_shared<DialogType>(*dialog);
            ServerCommandPointer cmd = std::make_shared<DialogCommandType>(*dialog, dialogId);
            Connection->addCommand(cmd);
        } else {
            inform("ERROR: Unable to open more than 100 dialogs.");
        }
    }

    template<class DialogType>
    std::shared_ptr<DialogType> getDialog(unsigned int dialogId) const {
        try {
            return std::dynamic_pointer_cast<DialogType>(dialogs.at(dialogId));
        } catch (std::out_of_range &e) {
            return std::shared_ptr<DialogType>();
        }
    }

public:
    virtual void requestInputDialog(InputDialog *inputDialog) override;
    void executeInputDialog(unsigned int dialogId, bool success, const std::string &input);

    virtual void requestMessageDialog(MessageDialog *messageDialog) override;
    void executeMessageDialog(unsigned int dialogId);

    virtual void requestMerchantDialog(MerchantDialog *merchantDialog) override;
    void executeMerchantDialogAbort(unsigned int dialogId);
    void executeMerchantDialogBuy(unsigned int dialogId, MerchantDialog::index_type index, Item::number_type amount);
    void executeMerchantDialogSell(unsigned int dialogId, uint8_t location, TYPE_OF_CONTAINERSLOTS slot, Item::number_type amount);

    virtual void requestSelectionDialog(SelectionDialog *selectionDialog) override;
    void executeSelectionDialog(unsigned int dialogId, bool success, SelectionDialog::index_type index);

    virtual void requestCraftingDialog(CraftingDialog *craftingDialog) override;
    void executeCraftingDialogAbort(unsigned int dialogId);
    void executeCraftingDialogCraft(unsigned int dialogId, uint8_t craftIndex, uint8_t craftAmount);
    void executeCraftingDialogCraftingComplete(unsigned int dialogId);
    void executeCraftingDialogCraftingAborted(unsigned int dialogId);
    void executeCraftingDialogLookAtCraftable(unsigned int dialogId, uint8_t craftIndex);
    void executeCraftingDialogLookAtIngredient(unsigned int dialogId, uint8_t craftIndex, uint8_t craftIngredient);
    virtual void requestCraftingLookAt(unsigned int dialogId, ItemLookAt &lookAt) override;
    virtual void requestCraftingLookAtIngredient(unsigned int dialogId, ItemLookAt &lookAt) override;

    void invalidateDialogs();
    void closeDialogsOnMove();

    virtual void logAdmin(const std::string &message) override;
private:
    void startCrafting(uint8_t stillToCraft, uint16_t craftingTime, uint16_t sfx, uint16_t sfxDuration, uint32_t dialogId);

private:

    Language _player_language;

    // Status of the player, Okay, waiting authroization, jailed, banned, etc..
    unsigned char status;

    // What time does the status get reset?
    time_t statustime;

    // Who banned/jailed the player?
    TYPE_OF_CHARACTER_ID statusgm;

    // Why where they banned/jailed?
    std::string statusreason;

    // World map turtle graphics
    bool turtleActive;
    unsigned char turtletile;

    // Clipping on/off (default to on)
    bool clippingActive;

    //! gibt an, ob der Spieler erweiterte Rechte hat - are they an admin?
    uint32_t admin;

    bool questWriteLock;

    LoadLevel loadLevel = LoadLevel::unburdened;

    bool monitoringClient;

    const uint8_t BACKPACK_SHOWCASE = 0;
    uint8_t showcaseCounter;
    typedef std::unordered_map<uint8_t, std::unique_ptr<Showcase>> ShowcaseMap;
    ShowcaseMap showcases;

    unsigned int dialogCounter;
    typedef std::unordered_map<unsigned int, std::shared_ptr<Dialog>> DialogMap;
    DialogMap dialogs;

    typedef std::pair<TYPE_OF_QUESTSTATUS, int> QuestStatusTimePair;
    typedef std::unordered_map<TYPE_OF_QUEST_ID, QuestStatusTimePair> QuestMap;
    QuestMap quests;
};

#endif

