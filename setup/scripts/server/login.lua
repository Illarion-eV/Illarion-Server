-- called after every player login

module("server.login", package.seeall)

function onLogin(player)
    world:gfx(31, player.pos)
    
    local players = world:getPlayersOnline()
    User:inform("[Login] Welcome to Illarion! There are " .. tonumber(#players) .. " players online.")
end
