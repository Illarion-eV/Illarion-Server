//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#include "CNewClientView.hpp"
#include "CMap.hpp"
#include "CMapVector.hpp"
#include <iostream>

CNewClientView::~CNewClientView() {}

CNewClientView::CNewClientView() : viewPosition( position(0,0,0) ), exists(false), stripedir( dir_right ), maxtiles(0)
{ 

}

void CNewClientView::fillStripe(position pos, stripedirection dir, int length, CMapVector * maps)
{
    clearStripe();
    viewPosition = pos;
    stripedir = dir;
    readFields( length, maps );
}

void CNewClientView::clearStripe()
{
    for ( int i = 0; i < 100/*<= MAP_DIMENSION+MAP_DOWN_EXTRA+6*/; ++i )
    {
        mapStripe[i] = NULL;
    }
    exists = false;
    viewPosition.x = 0;
    viewPosition.y = 0;
    viewPosition.z = 0;
    maxtiles = 0;
}

void CNewClientView::readFields( int length, CMapVector * maps)
{
    int x = viewPosition.x;
    int y = viewPosition.y;
    int x_inc = (stripedir == dir_right) ? 1 : -1;
    int tmp_maxtiles = 1;
    
    std::vector< CMap* > good_maps;
    std::vector< CMap* >::iterator it;
    if( maps->findAllMapsInRangeOf( 0, length-1, (stripedir == dir_right) ? length-1 : 0, (stripedir == dir_right) ? 0 : length-1, viewPosition, good_maps ) )
    {
        CMap * map = NULL;
    
        for ( int i = 0; i < length; ++i )
        {
	    tempCField = NULL;
            if ( !map || !map->GetPToCFieldAt(tempCField,x,y)  )
	    {
	        map = NULL;
                for ( it = good_maps.begin(); it < good_maps.end(); ++it)
	        {
	            if ( (*it)->GetPToCFieldAt(tempCField,x,y) )
		    {
                        map = *it;
		        break;
	    	    }
	        }
	    }
	    
            if( tempCField )
	    if ( ( ( tempCField->getTileId() != TRANSPARENT ) && ( tempCField->getTileId() != TRANSPARENTDISAPPEAR ) ) || !tempCField->items.empty() ) 
            {
	        exists = true;
		mapStripe[i] = tempCField;
                maxtiles = tmp_maxtiles;
            }
            
	    ++tmp_maxtiles;
            //increase x due to perspective
            x += x_inc;
            //increase y due to perspective
            ++y;
        }
    }
}

