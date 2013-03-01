-- Player death
-- deadPlayer - The player (character) whose hitpoints have just been set to zero

module("server.playerdeath", package.seeall)

function playerDeath(deadPlayer)
    deadPlayer:increaseAttrib("hitpoints",10000) -- Respawn
end

function showDeathDialog(deadPlayer)
    local callback = function(nothing) end --empty callback
    local dialog = MessageDialog("Death", "You have died.", callback)
    deadPlayer:requestMessageDialog(dialog) --showing the text
end
