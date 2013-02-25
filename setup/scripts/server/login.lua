-- called after every player login
require("base.common")
require("base.money")
require("base.factions")
require("content.dailymessage")
require("npc.aldania_elthewan")
require("scheduled.factionLeader")

module("server.login", package.seeall);

function onLogin( player )

    world:gfx(31,player.pos); --A nice GFX that announces clearly: A player logged in.

	--General welcome message
    players=world:getPlayersOnline(); --Reading all players online so we can count them
	
	if table.getn(players) > 1 then
	
	    base.common.InformNLS(player,"[Login] Willkommen auf Illarion! Es sind "..table.getn(players).." Spieler online.","[Login] Welcome to Illarion! There are "..table.getn(players).." players online."); --sending a message
    
	else --player is alone
	
	    base.common.InformNLS(player,"[Login] Willkommen auf Illarion! Es ist ein Spieler online.","[Login] Welcome to Illarion! There is one player online."); --sending a message
		
	end
	end