module("server.logout", package.seeall)

function onLogout(theChar)
    world:gfx(31, theChar.pos) --A nice GFX that announces clearly: a player logged out.
end
