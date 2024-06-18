local PORT = 3012

local illarion_protocol = Proto("Illarion", "Illarion Protocol")

local id = ProtoField.uint8("illarion.message_id", "id", base.HEX)
local not_id = ProtoField.uint8("illarion.message_not_id", "not_id", base.HEX)
local message_length = ProtoField.int16("illarion.message_length", "length", base.DEC)
local message_crc = ProtoField.int16("illarion.message_crc", "crc", base.DEC)

local character_id = ProtoField.int32("illarion.character_id", "character_id", base.DEC)
local position_x = ProtoField.int16("illarion.position.x", "x", base.DEC)
local position_y = ProtoField.int16("illarion.position.y", "y", base.DEC)
local position_z = ProtoField.int16("illarion.position.z", "z", base.DEC)
local move_mode = ProtoField.uint8("illarion.move_mode", "move_mode", base.DEC)
local move_speed = ProtoField.int16("illarion.move_speed", "move_speed", base.DEC)

illarion_protocol.fields = {
                            id,
                            not_id,
                            message_length,
                            message_crc,
                            character_id,
                            position_x,
                            position_y,
                            position_z,
                            move_mode,
                            move_speed
                           }

local server_commands = {}
server_commands[0x00] = "KEEPALIVE"
server_commands[0xCA] = "ID"
server_commands[0xBD] = "SETCOORDINATE"
server_commands[0xA1] = "MAPSTRIPE"
server_commands[0xA2] = "MAPCOMPLETE"
server_commands[0xE0] = "PLAYERSPIN"
server_commands[0xC1] = "UPDATEINVENTORYPOS"
server_commands[0xC4] = "CLEARSHOWCASE"
server_commands[0xB7] = "UPDATEWEATHER"
server_commands[0xD1] = "UPDATESKILL"
server_commands[0xB8] = "UPDATEMAGICFLAGS"
server_commands[0xB9] = "UPDATEATTRIB"
server_commands[0xB0] = "UPDATELOAD"
server_commands[0xC8] = "MUSIC"
server_commands[0xCE] = "MUSICDEFAULT"
server_commands[0xD7] = "SAY"
server_commands[0xD5] = "WHISPER"
server_commands[0xD6] = "SHOUT"
server_commands[0xD8] = "INFORM"
server_commands[0xD4] = "INTRODUCE"
server_commands[0xDF] = "MOVEACK"
server_commands[0xC5] = "UPDATESHOWCASE"
server_commands[0xCF] = "UPDATESHOWCASESLOT"
server_commands[0xC9] = "GRAPHICEFFECT"
server_commands[0xC7] = "SOUND"
server_commands[0xCB] = "ANIMATION"
server_commands[0xCD] = "BOOK"
server_commands[0xC6] = "ADMINVIEWPLAYERS"
server_commands[0xC3] = "ITEMREMOVE"
server_commands[0xC2] = "ITEMPUT"
server_commands[0xC0] = "LOOKATMAPITEM"
server_commands[0xBC] = "LOOKATTILE"
server_commands[0xBF] = "LOOKATSHOWCASEITEM"
server_commands[0xBE] = "LOOKATINVENTORYITEM"
server_commands[0xB5] = "LOOKATDIALOGITEM"
server_commands[0xBB] = "ATTACKACKNOWLEDGED"
server_commands[0xBA] = "TARGETLOST"
server_commands[0xCC] = "LOGOUT"
server_commands[0xB6] = "UPDATETIME"
server_commands[0xE1] = "APPEARANCE"
server_commands[0xE2] = "REMOVECHAR"
server_commands[0x18] = "LOOKATCHARRESULT"
server_commands[0x19] = "ITEMUPDATE"
server_commands[0x50] = "INPUTDIALOG"
server_commands[0x51] = "MESSAGEDIALOG"
server_commands[0x52] = "MERCHANTDIALOG"
server_commands[0x53] = "SELECTIONDIALOG"
server_commands[0x54] = "CRAFTINGDIALOG"
server_commands[0x55] = "CRAFTINGDIALOGUPDATE"
server_commands[0x56] = "AUCTIONDIALOG"
server_commands[0x5F] = "CLOSEDIALOG"
server_commands[0x40] = "QUESTPROGRESS"
server_commands[0x41] = "ABORTQUEST"
server_commands[0x42] = "AVAILABLEQUESTS"

local client_commands = {}
client_commands[0x0D] = "LOGIN"
client_commands[0xA0] = "SCREENSIZE"
client_commands[0x10] = "CHARMOVE"
client_commands[0x11] = "PLAYERSPIN"
client_commands[0xFF] = "LOOKATMAPITEM"
client_commands[0x18] = "LOOKATCHARACTER"
client_commands[0xFE] = "USE"
client_commands[0xFD] = "CAST"
client_commands[0xFA] = "ATTACKPLAYER"
client_commands[0xF7] = "CUSTOMNAME"
client_commands[0xF6] = "INTRODUCE"
client_commands[0xF5] = "SAY"
client_commands[0xF4] = "SHOUT"
client_commands[0xF3] = "WHISPER"
client_commands[0xF2] = "REFRESH"
client_commands[0xF1] = "LOGOUT"
client_commands[0xEF] = "MOVEITEMFROMMAPTOMAP"
client_commands[0xEE] = "PICKUPALLITEMS"
client_commands[0xED] = "PICKUPITEM"
client_commands[0xEC] = "LOOKINTOCONTAINERONFIELD"
client_commands[0xEB] = "LOOKINTOINVENTORY"
client_commands[0xEA] = "LOOKINTOSHOWCASECONTAINER"
client_commands[0xE9] = "CLOSECONTAINERINSHOWCASE"
client_commands[0xE8] = "DROPITEMFROMSHOWCASEONMAP"
client_commands[0xE7] = "MOVEITEMBETWEENSHOWCASES"
client_commands[0xE6] = "MOVEITEMFROMMAPINTOSHOWCASE"
client_commands[0xE5] = "MOVEITEMFROMMAPTOPLAYER"
client_commands[0xE4] = "DROPITEMFROMPLAYERONMAP"
client_commands[0xE3] = "MOVEITEMINSIDEINVENTORY"
client_commands[0xE2] = "MOVEITEMFROMSHOWCASETOPLAYER"
client_commands[0xE1] = "MOVEITEMFROMPLAYERTOSHOWCASE"
client_commands[0xE0] = "LOOKATSHOWCASEITEM"
client_commands[0xDF] = "LOOKATINVENTORYITEM"
client_commands[0xDE] = "ATTACKSTOP"
client_commands[0xDD] = "REQUESTSKILLS"
client_commands[0xD8] = "KEEPALIVE"
client_commands[0x0E] = "REQUESTAPPEARANCE"
client_commands[0x50] = "INPUTDIALOG"
client_commands[0x51] = "MESSAGEDIALOG"
client_commands[0x52] = "MERCHANTDIALOG"
client_commands[0x53] = "SELECTIONDIALOG"
client_commands[0x54] = "CRAFTINGDIALOG"
client_commands[0x56] = "AUCTIONDIALOG"

local function server_command_name(id)
    return server_commands[id] or "unknown"
end

local function client_command_name(id)
    return client_commands[id] or "unknown"
end

local function server_sends(pinfo)
    return pinfo.src_port == PORT
end

local function add_position(buffer, tree, index)
    local x = buffer(index, 2):int()
    local y = buffer(index + 2, 2):int()
    local z = buffer(index + 4, 2):int()

    position_tree = tree:add(illarion_protocol, buffer(index, 6), "position [" .. x .. ", " .. y .. ", " .. z .."]")
    position_tree:add(position_x, buffer(index, 2))
    position_tree:add(position_y, buffer(index + 2, 2))
    position_tree:add(position_z, buffer(index + 4, 2))

    return index + 6
end

function illarion_protocol.dissector(buffer, pinfo, tree)
    local length = buffer:len()
    if length == 0 then return end

    pinfo.cols.protocol = illarion_protocol.name

    local subtree
    
    if server_sends(pinfo) then
        subtree = tree:add(illarion_protocol, buffer(), "Illarion Server to Client Data")
        local command_id = buffer(0,1):uint()
        local command_name = server_command_name(command_id)
        subtree:add(id, buffer(0,1)):append_text(" (" .. command_name .. ")")
        subtree:add(not_id, buffer(1,1))
        subtree:add(message_length, buffer(2,2))
        subtree:add(message_crc, buffer(4,2))

        if command_name == "MOVEACK" then
            subtree:add(character_id, buffer(6, 4))
            add_position(buffer, subtree, 10)
            subtree:add(move_mode, buffer(16, 1))
            subtree:add(move_speed, buffer(17, 2))
        end
    else
        subtree = tree:add(illarion_protocol, buffer(), "Illarion Client to Server Data")
        local command_id = buffer(0,1):uint()
        subtree:add(id, buffer(0,1)):append_text(" (" .. client_command_name(command_id) .. ")")
        subtree:add(not_id, buffer(1,1))
        subtree:add(message_length, buffer(2,2))
        subtree:add(message_crc, buffer(4,2))
    end
end

local tcp_port = DissectorTable.get("tcp.port")
tcp_port:add(PORT, illarion_protocol)

