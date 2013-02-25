-- Collection for illustrating the usage and possibilities of different functions


-- UseItem 
function UseItem(User, SourceItem, ltstate)
    
	User:inform("You have used the item with the name '"..world:getItemName(SourceItem.id,Player.english).."' and the ID "..tonumber(SourceItem.id)..".")

end


-- MoveToField
function MoveToField(User)
    
	local coords = tonumber(User.pos.x)..", "..tonumber(User.pos.y)..", "..tonumber(User.pos.z)
    User:inform("You have stepped on the field with the coordinations "..coords..".")
				
end

-- Message box
function UseItem(User, SourceItem, ltstate)
    
	local callback = function(dialog) 
	    User:inform("Du hast die Textbox geschlossen.",
		            "You have closed the message dialogue.")
	end
	
	local dialog = MessageDialog("An inform via message dialogue", "This is a message dialogue. It can be used to send long informs to the character.", callback)
	User:requestMessageDialog(dialog)
	
end

-- Selection dialog
function UseItem(User, SourceItem, ltstate)

	local callback = function(dialog)
		success = dialog:getSuccess()
		if success then
		    local selectionList = {15,302,147}
			local selected = dialog:getSelectedIndex()+1
		    User:inform("You have selected the item with the name '"..world:getItemName(SourceItem.id,Player.english).."' and the ID "..tonumber(selectionList[selected])..". It got added to your inventory.")
		    User:createItem(selectionList[selected],1,333,nil)
		else
			User:inform("You have aborted the selection.")
		end
	end
	local dialog = SelectionDialog("Choose an option", "Here, you can choose one of the given options or you can abort the selection.", callback)
	dialog:addOption(15, "Apple")
	dialog:addOption(302, "Cherries")
	dialog:addOption(147, "Blackberry")
	User:requestSelectionDialog(dialog)    
end

-- increaseAttrib
function UseItem(User, SourceItem, ltstate)

    User:increaseAttrib("hitpoints",100)
	User:inform("You have used the item with the name '"..world:getItemName(SourceItem.id,Player.english).."' and the ID "..tonumber(SourceItem.id)..". It refilled your nutrition bar.")

end

-- MoveItemAfterMove
function MoveItemAfterMove(User, SourceItem, TargetItem)

	local coords = tonumber(TargetItem.pos.x)..", "..tonumber(TargetItem.pos.y)..", "..tonumber(TargetItem.pos.z)
    User:inform("You have thrown the item with the name "..world:getItemName(SourceItem.id,Player.english).." and the ID "..tonumber(SourceItem.id).." to the position "..coords..".")

end

-- ltstate
function UseItem(User, SourceItem, ltstate)
    if ( ltstate == Action.abort ) then
	    User:talk(Character.say, "#me stops working.")
	end
	local sandCheck 
	for xCoord = -1,1 do
	    for yCoord = -1,1 do
		    local checkPos = position(User.pos.x+xCoord,User.pos.y+yCoord,User.pos.z)
			local thedField = world:getField(checkPos)
			if theField.tile == 3 or theField.tile == 12 then
			    sandCheck = true
				break
            end    
		end
	end
	if not sandCheck then
	    User:inform("Use this item near sand to dig some.", Character.highPriority)
	end
	if ( ltstate == Action.none ) then
	    User:startAction( 3, 0, 0, 0, 0)
		User:talk(Character.say, "#me starts to dig for sand.")
    end    	
    local notCreated = User:createItem( 316, 1, 333, nil )
	if ( notCreated > 0 ) then 
		world:createItemFromId( 316, notCreated, User.pos, true, 333, nil )
		User:inform("You can't carry any more.", Character.highPriority)
	else 
		User:startAction( 3, 0, 0, 0, 0);
	end
end