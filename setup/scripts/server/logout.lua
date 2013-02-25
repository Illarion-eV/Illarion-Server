require("base.keys")
require("base.common")
require("scheduled.factionLeader")

module("server.logout", package.seeall)

function onLogout( theChar )
	
	world:gfx(31,theChar.pos); --A nice GFX that announces clearly: A player logged out.

end