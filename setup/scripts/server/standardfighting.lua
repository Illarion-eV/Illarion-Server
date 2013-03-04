-- Fighting System

module("server.standardfighting", package.seeall)

function onAttack(Attacker, Defender)
    if Attacker:getType() == Character.monster then
        if Defender:increaseAttrib("hitpoints", 0) > 1000 then
            Defender:increaseAttrib("hitpoints", -300)
        end
    else
        Defender:increaseAttrib("hitpoints", -1500)
        world:gfx(13, Defender.pos)
    end	
end
