-- This script is the central item lookat.

module("server.itemlookat", package.seeall)

--- Item Lookat
-- The task of this function is to generate a default lookat for an item.
-- This function is called by the server.
--
-- @param[in] player 	the player that is supposed to receive the inform
-- @param[in] item      the item the player is looking at
-- @return true in case the script generated the lookat
function lookAtItem(player, item)
    local lookAt = lookAt()
    lookAt.name = world:getItemName(item.id,Player.english)
    world:itemInform(player, item, lookAt)
    return true
end
