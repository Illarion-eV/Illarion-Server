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


#include "World.hpp"
#include "data/ContainerObjectTable.hpp"
#include "data/WeaponObjectTable.hpp"
#include "data/ArmorObjectTable.hpp"
#include "data/CommonObjectTable.hpp"
#include "data/TilesModificatorTable.hpp"
#include "script/LuaItemScript.hpp"
#include "script/LuaTriggerScript.hpp"
#include "script/LuaDepotScript.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/NetInterface.hpp"
#include <map>

extern boost::shared_ptr<LuaDepotScript>depotScript;

//
//   Constraints:
//
//   Immer wenn ein Item/Container in g_item/g_cont verschoben wird, muss die Variable tempWeight auf das Gesamtgewicht
//   der bewegten Items gesetzt werden. Sind g_item/g_cont leer, dann ist auch tempWeight==0.
//
//   Das Gesamtgewicht aller Items eines Character (ohne die in g_item/g_cont) steht immer in sumWeight und muss entsprechend aktualisiert werden.

//  atomare Funktionen  //
bool World::putItemOnInvPos( Character* cc, unsigned char pos ) 
{
        bool gotWeapon = false;
#ifdef World_ItemMove_DEBUG
        std::cout << "putItemOnInvPos(Character.. Start" << std::endl;
#endif
        //item is not finished so we can't move it to a body pos 
        if ( (g_item.quality < 100) && (pos < MAX_BODY_ITEMS) )
        {
            cc->inform("you can't wear unfinished items!");
            return false;
        }
        if ( pos == BACKPACK ) 
        {
               if ( cc->characterItems[ BACKPACK ].id == 0 ) 
               {
                      ContainerStruct cont;
                      if ( ContainerItems->find( g_item.id, cont ) ) 
                      {
                             cc->characterItems[ BACKPACK ] = g_item;
                             cc->characterItems[ BACKPACK ].number = 1;
                             if ( g_cont == NULL ) 
                             {
                                    g_cont = new Container(cont.ContainerVolume);
                             } else {
                                    Player* temp = dynamic_cast<Player*>(cc);
                                    if (temp)
                                           closeShowcaseForOthers(temp, g_cont);
                             }

                             cc->backPackContents = g_cont;

                             g_item.id = 0;
                             g_item.number = 0;
                             g_item.wear = 0;
                             g_item.quality = 0;
                             g_item.data = 0;
                             g_item.data_map.clear();
                             g_cont = NULL;
                             cc->updateAppearanceForAll( true );
                             return true;
                      }
               }
        } else if ( g_cont == NULL ) {
               if ( pos < MAX_BODY_ITEMS ) {
                      if ( cc->characterItems[ pos ].id == 0 || cc->characterItems[pos].id == g_item.id) {
                             if ( ( pos == RIGHT_TOOL ) || ( pos == LEFT_TOOL ) ) 
                             {
                                    if ( WeaponItems->find( g_item.id, tempWeapon ) ) 
                                    {
                                            //We take a weapon
                                           gotWeapon = true;
                                           if ((tempWeapon.WeaponType==4) || (tempWeapon.WeaponType==5) || (tempWeapon.WeaponType==6) || (tempWeapon.WeaponType==13)) 
                                           {
                                                  if (( pos == RIGHT_TOOL ) && (cc->characterItems[ LEFT_TOOL ].id == 0)) 
                                                  {
                                                         if (cc->characterItems[pos].id == 0 && g_item.number == 1) 
                                                         {
                                                                cc->characterItems[ pos ] = g_item;
                                                                cc->characterItems[ LEFT_TOOL ].id = BLOCKEDITEM;
                                                                cc->characterItems[ LEFT_TOOL ].wear = 255;
                                                                cc->characterItems[ LEFT_TOOL ].number = 1;
                                                                g_item.id = 0;
                                                                g_item.number = 0;
                                                                g_item.wear = 0;
                                                                g_item.quality = 0;
                                                                g_item.data = 0;
                                                                g_item.data_map.clear();
                                                                cc->updateAppearanceForAll( true );
                                                                return true;
                                                         }
                                                         // we don't want to add any more two handers...
                                                         else 
                                                         {
                                                                return false;
                                                         }
                                                  } 
                                                  else if (( pos == LEFT_TOOL ) && (cc->characterItems[ RIGHT_TOOL ].id == 0)) 
                                                  {
                                                         if (cc->characterItems[pos].id == 0 && g_item.number == 1) 
                                                         {
                                                                //std::cout << "Zweihaender rechts  platziert" << std::endl;
                                                                cc->characterItems[ pos ] = g_item;
                                                                cc->characterItems[ RIGHT_TOOL ].id = BLOCKEDITEM;
                                                                cc->characterItems[ RIGHT_TOOL ].wear = 255;
                                                                cc->characterItems[ RIGHT_TOOL ].number = 1;
                                                                g_item.id = 0;
                                                                g_item.number = 0;
                                                                g_item.wear = 0;
                                                                g_item.quality = 0;
                                                                g_item.data =0;
                                                                g_item.data_map.clear();
                                                              cc->updateAppearanceForAll( true );
                                                                return true;
                                                         }
                                                         // we don't want to add any more two handers...
                                                         else 
                                                         {
                                                             return false;
                                                         }
                                                  } 
                                                  else 
                                                  {
                                                      return false;
                                                  }
                                           }
                                    }

                                    if (cc->characterItems[pos].id == 0) {
                                           //Funktion fr Stackable einbauen
                                           // if ( isStackable )
                                           if ( !isStackable(g_item) && !ContainerItems->find(g_item.id) ) {
                                                  if ( g_item.number > 1 ) {
                                                         return false;
                                                  }
                                           }
                                           //Ende Stackable
                                           cc->characterItems[ pos ] = g_item;
                                           g_item.id = 0;
                                           g_item.number = 0;
                                           g_item.wear = 0;
                                           g_item.quality = 0;
                                           g_item.data = 0;
                                           g_item.data_map.clear();
                                            
#ifdef World_ItemMove_DEBUG
                                           std::cout << "putItemOnInvPos(Character.. Ende 1" << std::endl;
#endif
                                           cc->updateAppearanceForAll( true );
                                           return true;
                                    } else {
                                           //Funktion fr Stackable einbauen
                                           // if ( isStackable )
                                           if ( !isStackable(g_item) && !ContainerItems->find(g_item.id) ) {
                                                  return false;
                                           }
                       if ( g_item.data != cc->characterItems[pos].data ) return false;
                                           if ( g_item.quality < 100 || cc->characterItems[pos].quality < 100 ) return false;
                                           //Ende Stackable
                                           if (g_item.number + cc->characterItems[pos].number < 250) 
                                           {
                                              // transfer all items
                                              cc->characterItems[pos].number += g_item.number;
                                              if( g_item.quality < cc->characterItems[pos].quality )
                                                  cc->characterItems[pos].quality = g_item.quality;
                                              g_item.id = 0;
                                              g_item.number = 0;
                                              g_item.wear = 0;
                                              g_item.quality = 0;
                                              g_item.data = 0;
                                              g_item.data_map.clear();
                                              cc->updateAppearanceForAll( true );
                                              return true;
                                           } 
                                           else 
                                           {
                                                  return false;
                                           }
                                    }
                             } else {
                                    if (g_item.number == 1 && cc->characterItems[pos].id == 0) {
                                           if ( ArmorItems->find( g_item.id, tempArmor ) ) {
                                                  unsigned char flag;
                                                  switch ( pos ) {
                                                         case HEAD :
                                                                flag = FLAG_HEAD;
                                                                break;
                                                         case NECK :
                                                                flag = FLAG_NECK;
                                                                break;
                                                         case BREAST :
                                                                flag = FLAG_BREAST;
                                                                break;
                                                         case HANDS :
                                                                flag = FLAG_HANDS;
                                                                break;
                                                         case FINGER_LEFT_HAND :
                                                                flag = FLAG_FINGER;
                                                                break;
                                                         case FINGER_RIGHT_HAND :
                                                                flag = FLAG_FINGER;
                                                                break;
                                                         case LEGS :
                                                                flag = FLAG_LEGS;
                                                                break;
                                                         case FEET :
                                                                flag = FLAG_FEET;
                                                                break;
                                                         case OAT :
                                                                flag = FLAG_COAT;
                                                                break;
                                                         default :
                                                                flag = 0xFF;
                                                                break;
                                                  }

                                                  if ( ( tempArmor.BodyParts & flag ) != 0 ) {
                                                         cc->characterItems[ pos ] = g_item;
                                                         g_item.id = 0;
                                                         g_item.number = 0;
                                                         g_item.wear = 0;
                                                         g_item.quality = 0;
                                                         g_item.data = 0;
                                                         g_item.data_map.clear();
#ifdef World_ItemMove_DEBUG
                                                         std::cout << "putItemOnInvPos(Character.. Ende 2" << std::endl;
#endif
                                                         cc->updateAppearanceForAll( true ); 
                                                         return true;
                                                  }
                                           }
                                    }
                             }
                      }
               } else if ( pos < MAX_BODY_ITEMS + MAX_BELT_SLOTS ) {
                      if ( cc->characterItems[ pos ].id == 0 ) {
                             //Funktion fr Stackable einbauen
                             // if ( isStackable )
                             if ( !isStackable(g_item) && !ContainerItems->find(g_item.id) ) {
                                    if ( g_item.number > 1 ) {
                                           return false;
                                    }
                             }
                             //Ende Stackable
                             cc->characterItems[ pos ] = g_item;
                             g_item.id = 0;
                             g_item.number = 0;
                             g_item.wear = 0;
                             g_item.quality = 0;
                             g_item.data = 0;
                             g_item.data_map.clear();
#ifdef World_ItemMove_DEBUG
                             std::cout << "putItemOnInvPos(Character.. Ende 3" << std::endl;
#endif
                             cc->updateAppearanceForAll( true );
                             return true;
                      } else if ( cc->characterItems[ pos ].id == g_item.id ) {
                             //Funktion fr Stackable einbauen
                             // if ( isStackable )
                             if ( !isStackable(g_item) && !ContainerItems->find(g_item.id) ) {
                                    return false;
                             }
                 if ( g_item.data != cc->characterItems[pos].data ) return false;
                             if ( g_item.quality < 100 || cc->characterItems[pos].quality < 100 ) return false;
                             //Ende Stackable
                             int temp =cc->characterItems[ pos ].number + g_item.number;
                             if (temp <= MAXITEMS) {
                                    cc->characterItems[ pos ].number=temp;
                                    if( g_item.quality < cc->characterItems[ pos ].quality )
                                        cc->characterItems[ pos ].quality = g_item.quality;
                                    g_item.id = 0;
                                    g_item.number = 0;
                                    g_item.wear = 0;
                                    g_item.quality = 0;
                                    g_item.data = 0;
                                    g_item.data_map.clear();
                                    cc->updateAppearanceForAll( true );
                                    return true;
                             }
                      }
               }
        }

#ifdef World_ItemMove_DEBUG
        std::cout << "putItemOnInvPos(Character.. Ende 4" << std::endl;
#endif

        return false;

}



bool World::putItemOnInvPos( Player* cc, unsigned char pos ) 
{
#ifdef World_ItemMove_DEBUG
        std::cout << "putItemOnInvPos(Player.. Start" << std::endl;
#endif
        //item is not finished so we can't move it to a body pos only left hand, and right hand
        if ( (g_item.quality < 100) && (pos < MAX_BODY_ITEMS) )
        {
            if ( cc->getPlayerLanguage() == 0)cc->inform("Du kannst keine unfertigen Gegenst�nde anlegen!");
            else cc->inform("You can't wear unfinished items!");
std::cout << "UTF-8 TEST: Gegenst�nde" << std::endl;
            return false;
        }
        if ( putItemOnInvPos( ( Character* ) cc, pos ) ) {
               if (pos == LEFT_TOOL) {
                      cc->sendCharacterItemAtPos( pos );
                      cc->sendCharacterItemAtPos( RIGHT_TOOL );
               } else if (pos == RIGHT_TOOL) {
                      cc->sendCharacterItemAtPos( pos );
                      cc->sendCharacterItemAtPos( LEFT_TOOL );
               } else {
                      cc->sendCharacterItemAtPos( pos );
               }
#ifdef World_ItemMove_DEBUG
               std::cout << "putItemOnInvPos(Player.. Ende 1" << std::endl;
#endif
               return true;
        } else {
#ifdef World_ItemMove_DEBUG
               std::cout << "putItemOnInvPos(Player.. Ende 1" << std::endl;
#endif
               return false;
        }
}

bool World::takeItemFromInvPos( Character* cc, unsigned char pos, unsigned char count ) {
#ifdef World_ItemMove_DEBUG
        std::cout << "takeItemFromInvPos(Character.. Start" << std::endl;
#endif
        //check if take the weapon to update the appearance.
        bool takeWeapon = false;
        if ( pos == BACKPACK ) {
               if ( cc->characterItems[ BACKPACK ].id != 0 ) {
                      g_item = cc->characterItems[ BACKPACK ];
                      g_cont = cc->backPackContents;



                      if ( g_cont == NULL ) {
                             ContainerStruct cont;
                             if ( ContainerItems->find(g_item.id, cont) )
                                    g_cont = new Container(cont.ContainerVolume);
                             else
                                    g_cont = new Container(50000);
                      }

                      cc->characterItems[ BACKPACK ].id = 0;
                      cc->characterItems[ BACKPACK ].number = 0;
                      cc->characterItems[ BACKPACK ].wear = 0;
                      cc->characterItems[ BACKPACK ].quality = 0;
                      cc->characterItems[ BACKPACK ].data = 0;
                      cc->characterItems[ BACKPACK ].data_map.clear();
                      cc->backPackContents = NULL;
#ifdef World_ItemMove_DEBUG
                      std::cout << "takeItemFromInvPos(Character.. Ende 1" << std::endl;
#endif
                      cc->updateAppearanceForAll( true );
                      return true;
               }
        } else if ( pos < MAX_BODY_ITEMS + MAX_BELT_SLOTS ) {
               if ( ( cc->characterItems[ pos ].id != 0 ) && ( cc->characterItems[ pos ].id != BLOCKEDITEM )) {
                      if ( ( pos == RIGHT_TOOL ) || ( pos == LEFT_TOOL ) ) 
                      {
                             if ( WeaponItems->find( cc->characterItems[ pos ].id, tempWeapon ) ) 
                             {
                                 takeWeapon = true;
                                    g_item = cc->characterItems[ pos ];
                                    g_cont = NULL;
                                    //Stacking von Items verhindern
                                    if ( !isStackable(g_item) && !ContainerItems->find(g_item.id) ) {
                                           if ( g_item.number > 1 && count > 1 ) {
                                                  g_item.id = 0;
                                                  g_item.number = 0;
                                                  g_item.wear = 0;
                                                g_item.quality = 0;
                                                g_item.data = 0;
                                                  g_cont = NULL;
                                                  return false;
                                           }
                                    }
                                    //Ende Stacking von Items verhindern
                                    if (g_item.number > count) {
                                           cc->characterItems[ pos ].number -= count;
                                           g_item.number=count;
                                    } else {

                                           if ((tempWeapon.WeaponType==4) || (tempWeapon.WeaponType==5) || (tempWeapon.WeaponType==6) || (tempWeapon.WeaponType==13)) {
                                                  cc->characterItems[ LEFT_TOOL ].id=0;
                                                  cc->characterItems[ LEFT_TOOL ].wear=0;
                                                  cc->characterItems[ LEFT_TOOL ].number=0;
                                                  cc->characterItems[ LEFT_TOOL ].quality=0;
                                                  cc->characterItems[ LEFT_TOOL ].data=0;
                                                  cc->characterItems[ LEFT_TOOL ].data_map.clear();
                                                  cc->characterItems[ RIGHT_TOOL ].id=0;
                                                  cc->characterItems[ RIGHT_TOOL ].wear=0;
                                                  cc->characterItems[ RIGHT_TOOL ].number=0;
                                                  cc->characterItems[ RIGHT_TOOL ].quality=0;
                                                  cc->characterItems[ RIGHT_TOOL ].data=0;
                                                  cc->characterItems[ RIGHT_TOOL ].data_map.clear();
                                           } else {
                                                  cc->characterItems[ pos ].id=0;
                                                  cc->characterItems[ pos ].wear=0;
                                                  cc->characterItems[ pos ].number=0;
                                                  cc->characterItems[ pos ].quality=0;
                                                  cc->characterItems[ pos ].data=0;
                                                  cc->characterItems[ pos ].data_map.clear();
                                           }
                                    }
                                    cc->updateAppearanceForAll( true );
                                    return true;
                             }
                      }
                      //std::cout << "takeItemFromInvPos, als normales Item behandelt" << std::endl;
                      g_item = cc->characterItems[ pos ];
                      g_cont = NULL;
                      if ( isStackable(g_item) && count > 1 && !ContainerItems->find(g_item.id) ) {
                             if (g_item.number > count) {
                                    cc->characterItems[ pos ].number -= count;
                                    g_item.number=count;
                             } else {
                                    cc->characterItems[ pos ].id = 0;
                                    cc->characterItems[ pos ].number = 0;
                                    cc->characterItems[ pos ].wear = 0;
                                    cc->characterItems[ pos ].quality = 0;
                                    cc->characterItems[ pos ].data=0;
                                    cc->characterItems[ pos ].data_map.clear();
                             }
                             cc->updateAppearanceForAll( true );
                             return true;
                      } else {
                             if (g_item.number > 1) {
                                    cc->characterItems[ pos ].number -= 1;
                                    g_item.number=1;
                             } else {
                                    cc->characterItems[ pos ].id = 0;
                                    cc->characterItems[ pos ].number = 0;
                                    cc->characterItems[ pos ].wear = 0;
                                    cc->characterItems[ pos ].quality = 0;
                                    cc->characterItems[ pos ].data = 0;
                                    cc->characterItems[ pos ].data_map.clear();
                             }
                             cc->updateAppearanceForAll( true );
                             return true;
                      }
               }
        } // Rucksack oder normal

        g_item.id = 0;
        g_item.number = 0;
        g_item.wear = 0;
        g_item.quality = 0;
        g_item.data = 0;
        g_item.data_map.clear();
        g_cont = NULL;

        return false;
}



bool World::takeItemFromInvPos( Player* cc, unsigned char pos, unsigned char count  ) {
#ifdef World_ItemMove_DEBUG
        std::cout << "takeItemFromInvPos(Player.. Start" << std::endl;
#endif
        if ( pos == BACKPACK ) {
               if ( cc->characterItems[ BACKPACK ].id != 0 ) {
                      if ( cc->backPackContents != NULL ) {
                             for ( int i = 0; i < MAXSHOWCASES; ++i ) {
                                    if ( cc->showcases[ i ].contains( cc->backPackContents ) ) {
                                           cc->showcases[ i ].clear();
                                           boost::shared_ptr<BasicServerCommand>cmd(new ClearShowCaseTC( i ) );
                                           cc->Connection->addCommand( cmd );
                                    }
                             }
                      }
               }
        }

        if ( takeItemFromInvPos( ( Character* ) cc, pos, count ) ) {
               if (pos == LEFT_TOOL) {
                      cc->sendCharacterItemAtPos( pos );
                      cc->sendCharacterItemAtPos( RIGHT_TOOL );
               } else if (pos == RIGHT_TOOL) {
                      cc->sendCharacterItemAtPos( pos );
                      cc->sendCharacterItemAtPos( LEFT_TOOL );
               } else {
                      cc->sendCharacterItemAtPos( pos );
               }
#ifdef World_ItemMove_DEBUG
               std::cout << "takeItemFromInvPos(Player.. Ende 1" << std::endl;
#endif
               return true;
        } else {
#ifdef World_ItemMove_DEBUG
               std::cout << "takeItemFromInvPos(Player.. Ende 2" << std::endl;
#endif
               return false;
        }
}



bool World::takeItemFromShowcase( Player* cc, unsigned char showcase, unsigned char pos, unsigned char count ) {

#ifdef World_ItemMove_DEBUG
        std::cout << "takeItemFromShowcase(Player.. Start" << std::endl;
#endif
        if ( showcase < MAXSHOWCASES ) {
               Container * ps = cc->showcases[ showcase ].top();
               if ( ps != NULL ) {
                      if ( ps->TakeItemNr( pos, g_item, g_cont, count ) ) {
                             if ( g_cont != NULL ) {
                                    sendChangesOfContainerContentsCM( ps, g_cont );
                             } else {
                                    sendChangesOfContainerContentsIM( ps );
                             }
#ifdef World_ItemMove_DEBUG
                             std::cout << "takeItemFromShowcase(Player.. Ende 1" << std::endl;
#endif

                             return true;
                      }
               }
        }

        g_item.id = 0;
        g_item.number = 0;
        g_item.wear = 0;
        g_item.quality = 0;
        g_item.data = 0;
        g_item.data_map.clear();
        g_cont = NULL;
#ifdef World_ItemMove_DEBUG
        std::cout << "takeItemFromShowcase(Player.. Ende 2" << std::endl;
#endif

        return false;

}


bool World::putItemInShowcase( Player* cc, unsigned char showcase, unsigned char pos ) {
#ifdef World_ItemMove_DEBUG
        std::cout << "putItemInShowcase.. Start" << std::endl;
#endif
        //Unstackable von Items
        if ( !isStackable(g_item) && !ContainerItems->find(g_item.id) ) {
               if ( g_item.number > 1 ) {
                      return false;
               }
        }
        //Ende unstackable von Items
#ifdef World_BagOnlyInDepot
        bool isdepot = false;
#endif
        if ( showcase < MAXSHOWCASES ) {
               Container * ps = cc->showcases[ showcase ].top();
               if ( ps != NULL ) {
                      if ( g_cont != NULL ) {
                             //Ein Container in ein Depot einfgen
                             std::map<uint32_t,Container*>::iterator it;
#ifdef World_BagOnlyInDepot
                             //Loop through all depots if ps is a depot if yes is depot set to true
                for ( it = cc->depotContents.begin(); it != cc->depotContents.end(); ++it )
                             {
                                 if ( it->second == ps )
                                 {
                        isdepot = true;
                        break;
                    }                                    
                             }
                if ( isdepot )
#endif                
                {
                                    if ( !cc->showcases[ showcase ].contains( g_cont ) ) {
                                           if ( ps->InsertContainer( g_item, g_cont ) ) {
                                                  sendChangesOfContainerContentsCM( ps, g_cont );
                                                  g_item.id = 0;
                                                  g_item.number = 0;
                                                  g_item.wear = 0;
                                                  g_item.quality = 0;
                                                  g_item.data = 0;
                                                  g_item.data_map.clear();
                                                  g_cont = NULL;
#ifdef World_ItemMove_DEBUG
                                                  std::cout << "putItemInShowcase.. Ende 1" << std::endl;
#endif
                                                  if (cc->showcases[ showcase ].inInventory()) {}
                                                  return true;
                                           }
                                    }
                                    //Falls man eine Tasche in eine Tasche legen will dies unterbinden
                                    else return false;
                             }
                      } else {
                             if ( ps->InsertItem( g_item, pos ) ) {
                                    sendChangesOfContainerContentsIM( ps );
                                    g_item.id = 0;
                                    g_item.number = 0;
                                    g_item.wear = 0;
                                    g_item.quality = 0;
                                    g_item.data = 0;
                                    g_item.data_map.clear();
#ifdef World_ItemMove_DEBUG
                                    std::cout << "putItemInShowcase.. Ende 2" << std::endl;
#endif
                                    return true;
                             }
                      }
               }
        }
#ifdef World_ItemMove_DEBUG
        std::cout << "putItemInShowcase.. Ende 3" << std::endl;
#endif

        return false;

}



bool World::takeItemFromMap( Character* cc, short int x, short int y, short int z ) {
#ifdef World_ItemMove_DEBUG
        std::cout << "takeItemFromMap.. Start" << std::endl;
#endif

        Field* tempf;

        // Feld vorhanden
        if ( GetPToCFieldAt( tempf, x, y, z, tmap ) ) {
               // mindesten 1 Item vorhanden auf dem Feld
               if ( tempf->ViewTopItem( g_item ) ) {
                      if (CommonItems->find(g_item.id, tempCommon)) {
                             if ( tempCommon.Weight < 30000 && g_item.wear != 255 ) {
                                    tempf->TakeTopItem( g_item );
                                    //Verhindern des Stapelns von Items
                                    if ( !isStackable(g_item) && !ContainerItems->find(g_item.id) ) {
                                           if ( g_item.number > 1 ) {
                                                  g_item.id = 0;
                                                  g_item.number = 0;
                                                  g_item.wear = 0;
                                                  g_item.quality = 0;
                                                  g_item.data = 0;
                                                  g_item.data_map.clear();
                                                  g_cont = NULL;
                                                  return false;
                                           }

                                    }
                                    //ende verhindern des Stapelns von ITems

                                    //Ausfhren eines Triggerscriptes. Falls eins fr das Feld vorhanden ist.
                                    //Dies prft nicht nach ob ggf das entfernen des Items nicht geklappt hat
                                    TriggerStruct Trigger;
                                    if ( Triggers->find(position(x,y,z),Trigger) ) {
                                           if ( Trigger.script ) {
                                                  ScriptItem sItem = g_item;
                                                  sItem.pos = position(x,y,z);
                                                  sItem.type = ScriptItem::it_field;
                                                  Trigger.script->TakeItemFromField(sItem,cc);
                                           }
                                    }
                                    //=======================Ende des Triggerscriptes

                                    // item ist ein Container
                                    ContainerStruct cont;
                                    if ( ContainerItems->find( g_item.id, cont ) ) {
                                           MAP_POSITION opos;
                                           opos.x = x;
                                           opos.y = y;
                                           Map::ONTAINERHASH::iterator conmapo = tmap->maincontainers.find( opos );
                                           // containermap fr das Feld gefunden
                                           if ( conmapo != tmap->maincontainers.end() ) {
                                                  Container::ONTAINERMAP::iterator iv = ( *conmapo ).second.find( g_item.number );
                                                  // der Inhalt des angegebenen Containers mit der id g_item.number wurde gefunden
                                                  if ( iv != ( *conmapo ).second.end() ) {
                                                         g_cont = ( *iv ).second;

                                                         // Verweis auf den Container in der Containermap fr das Feld loeschen
                                                         ( *conmapo ).second.erase( iv );
                                                         if ( ( *conmapo ).second.empty() ) {
                                                                // kein Container mehr auf dem Feld -> Containermap fr das Feld loeschen
                                                                tmap->maincontainers.erase( conmapo );
                                                         }
                                                         sendRemoveItemFromMapToAllVisibleCharacters( cc->id, x, y, z, tempf );
#ifdef World_ItemMove_DEBUG
                                                         std::cout << "takeItemFromMap.. Ende 1" << std::endl;
#endif
                                                         return true;
                                                  } else {
                                                         g_cont = new Container(cont.ContainerVolume);
                                                         sendRemoveItemFromMapToAllVisibleCharacters( cc->id, x, y, z, tempf );
#ifdef World_ItemMove_DEBUG
                                                         std::cout << "takeItemFromMap.. Ende 2" << std::endl;
#endif
                                                         return true;
                                                  }
                                           } else {
                                                  g_cont = new Container(cont.ContainerVolume);
                                                  sendRemoveItemFromMapToAllVisibleCharacters( cc->id, x, y, z, tempf );
#ifdef World_ItemMove_DEBUG
                                                  std::cout << "takeItemFromMap.. Ende 3" << std::endl;
#endif
                                                  return true;
                                           }
                                    } else {
                                           // normales Item
                                           g_cont = NULL;
                                           sendRemoveItemFromMapToAllVisibleCharacters( cc->id, x, y, z, tempf );
#ifdef World_ItemMove_DEBUG
                                           std::cout << "takeItemFromMap.. Ende 4" << std::endl;
#endif
                                           return true;
                                    }
                             } // tragbar
                      } // item bekannt
               } // Item vorhanden ?
        } // Feld vorhanden ?

        g_item.id = 0;
        g_item.number = 0;
        g_item.wear = 0;
        g_item.quality = 0;
        g_item.data = 0;
        g_item.data_map.clear();
        g_cont = NULL;

#ifdef World_ItemMove_DEBUG
        std::cout << "takeItemFromMap.. Ende 5" << std::endl;
#endif

        return false;

}


bool World::putItemOnMap( Character* cc, short int x, short int y, short int z ) {
#ifdef World_ItemMove_DEBUG
        std::cout << "putItemOnMap.. Start" << std::endl;
#endif
        Field* tempf;

        //stacking von Items verhindern
        if ( !isStackable(g_item) && !ContainerItems->find(g_item.id) ) {
               if ( g_item.number > 1 ) {
                      return false;
               }
        }
        //Ende stacking von Items verhindern.

        if ( GetPToCFieldAt( tempf, x, y, z, tmap ) ) {
               MAP_POSITION npos;
               npos.x = x;
               npos.y = y;

               if ( TilesModItems->nonPassable( g_item.id ) ) { // nicht passierbares Item, zB. eine grosse Kiste
                      if ( ! tempf->moveToPossible() ) { // das Feld ist nicht betretbar
#ifdef World_ItemMove_DEBUG
                             std::cout << "putItemOnMap.. Ende 1" << std::endl;
#endif
                             return false;
                      }
               }
               ContainerStruct cont;
               if ( ContainerItems->find( g_item.id, cont ) ) {
                      // Container
                      if ( g_cont == NULL ) {
                             g_cont = new Container(cont.ContainerVolume);
                      } else
                             // close the showcase for everyone not in range
                             closeShowcaseIfNotInRange(g_cont,x,y,z);

                      if ( tmap->addContainerToPos( g_item, g_cont, npos ) ) {
                             sendPutItemOnMapToAllVisibleCharacters( x, y, z, g_item, tempf );
                             //Ausfhren eines Triggerscriptes. Falls eins fr das Feld vorhanden ist.
                             //Dies prft nicht nach ob ggf das entfernen des Items nicht geklappt hat
                             TriggerStruct Trigger;
                             if ( Triggers->find(position(x,y,z),Trigger) ) {
                                    if ( Trigger.script ) {
                                           ScriptItem sItem = g_item;
                                           sItem.pos = position(x,y,z);
                                           sItem.type = ScriptItem::it_field;
                                           Trigger.script->PutItemOnField(sItem,cc);
                                    }
                             }
                             //=======================Ende des Triggerscriptes
                             checkField( tempf, x, y, z );
                             g_item.id = 0;
                             g_item.number = 0;
                             g_item.wear = 0;
                             g_item.quality = 0;
                             g_item.data = 0;
                             g_item.data_map.clear();
                             g_cont = NULL;
#ifdef World_ItemMove_DEBUG
                             std::cout << "putItemOnMap.. Ende 2" << std::endl;
#endif
                             return true;
                      }
               } else {
                      // normales Item
                      if ( tempf->addTopItem( g_item ) ) {
                             sendPutItemOnMapToAllVisibleCharacters( x, y, z, g_item, tempf );
                             //Ausfhren eines Triggerscriptes. Falls eins fr das Feld vorhanden ist.
                             //Dies prft nicht nach ob ggf das entfernen des Items nicht geklappt hat
                             TriggerStruct Trigger;
                             if ( Triggers->find(position(x,y,z),Trigger) ) {
                                    if ( Trigger.script ) {
                                           ScriptItem sItem = g_item;
                                           sItem.pos = position(x,y,z);
                                           sItem.type = ScriptItem::it_field;
                                           Trigger.script->PutItemOnField(sItem,cc);
                                    }
                             }
                             //=======================Ende des Triggerscriptes
                             checkField( tempf, x, y, z );
                             g_cont = NULL;
                             g_item.id = 0;
                             g_item.number = 0;
                             g_item.wear = 0;
                             g_item.quality = 0;
                             g_item.data = 0;
                             g_item.data_map.clear();
#ifdef World_ItemMove_DEBUG
                             std::cout << "putItemOnMap.. Ende 3" << std::endl;
#endif
                             return true;
                      }
               }
        }
#ifdef World_ItemMove_DEBUG
        std::cout << "putItemOnMap.. Ende 4" << std::endl;
#endif

        return false;

}


bool World::putItemAlwaysOnMap( Character* cc, short int x, short int y, short int z ) {
#ifdef World_ItemMove_DEBUG
        std::cout << "putItemAlwaysOnMap.. Start" << std::endl;
#endif
        Field* tempf;

        if ( GetPToCFieldAt( tempf, x, y, z, tmap ) ) {
               MAP_POSITION npos;
               npos.x = x;
               npos.y = y;
               ContainerStruct cont;
               if ( ContainerItems->find( g_item.id, cont ) ) {
                      // Container
                      if ( g_cont == NULL ) {
                             g_cont = new Container(cont.ContainerVolume);
                      }

                      if ( tmap->addAlwaysContainerToPos( g_item, g_cont, npos ) ) {
                             sendPutItemOnMapToAllVisibleCharacters( x, y, z, g_item, tempf );
                             //Ausfhren eines Triggerscriptes. Falls eins fr das Feld vorhanden ist.
                             //Dies prft nicht nach ob ggf das entfernen des Items nicht geklappt hat
                             TriggerStruct Trigger;
                             if ( cc && Triggers->find(position(x,y,z),Trigger) ) {
                                    if ( Trigger.script ) {
                                           ScriptItem sItem = g_item;
                                           sItem.pos = position(x,y,z);
                                           sItem.type = ScriptItem::it_field;
                                           Trigger.script->PutItemOnField(sItem,cc);
                                    }
                             }
                             //=======================Ende des Triggerscriptes
                             checkField( tempf, x, y, z );
                             g_item.id = 0;
                             g_item.number = 0;
                             g_item.wear = 0;
                             g_item.quality = 0;
                             g_item.data = 0;
                             g_item.data_map.clear();
                             g_cont = NULL;
#ifdef World_ItemMove_DEBUG
                             std::cout << "putItemAlwaysOnMap.. Ende 2" << std::endl;
#endif
                             return true;
                      }
               } else {
                      // normales Item
                      if ( tempf->PutTopItem( g_item ) ) {
                             sendPutItemOnMapToAllVisibleCharacters( x, y, z, g_item, tempf );
                             //Ausfhren eines Triggerscriptes. Falls eins fr das Feld vorhanden ist.
                             //Dies prft nicht nach ob ggf das entfernen des Items nicht geklappt hat
                             TriggerStruct Trigger;
                             if ( cc && Triggers->find(position(x,y,z),Trigger) ) {
                                    if ( Trigger.script ) {
                                           ScriptItem sItem;
                                           sItem = g_item;
                                           sItem.pos = position(x,y,z);
                                           sItem.type = ScriptItem::it_field;
                                           Trigger.script->PutItemOnField(sItem,cc);
                                    }
                             }
                             //=======================Ende des Triggerscriptes
                             checkField( tempf, x, y, z );
                             g_cont = NULL;
                             g_item.id = 0;
                             g_item.number = 0;
                             g_item.wear = 0;
                             g_item.quality = 0;
                             g_item.data = 0;
                             g_item.data_map.clear();
#ifdef World_ItemMove_DEBUG
                             std::cout << "putItemAlwaysOnMap.. Ende 3" << std::endl;
#endif
                             return true;
                      }
               }
        }
#ifdef World_ItemMove_DEBUG
        std::cout << "putItemAlwaysOnMap.. Ende 4" << std::endl;
#endif

        return false;

}



void World::checkField( Field* cfstart, short int x, short int y, short int z ) {
#ifdef World_ItemMove_DEBUG
        std::cout << "checkField(Field* .. Start" << std::endl;
#endif
        if ( cfstart != NULL ) {
               if ( cfstart->HasSpecialItem() ) {
                      if ( cfstart->IsPlayerOnField() ) {
                             Player * temp;
                             if ( Players.find( x, y, z, temp ) ) {
                                    checkFieldAfterMove( temp, cfstart );
                             }
                      } else if ( cfstart->IsMonsterOnField() ) {
                             Monster * temp2;
                             if ( Monsters.find( x, y, z, temp2 ) ) {
                                    checkFieldAfterMove( temp2, cfstart );
                             }
                      } else if ( cfstart->IsNPCOnField() ) {
                             NPC * temp3;
                             if ( Npc.find( x, y, z, temp3 ) ) {
                                    checkFieldAfterMove( temp3, cfstart );
                             }
                      }
               }
        }
#ifdef World_ItemMove_DEBUG
        std::cout << "checkField(Field* .. Ende" << std::endl;
#endif
}


///////// zusammengesetzte Funktionen  ///////////////
void World::dropItemFromShowcaseOnMap( Player* cp, unsigned char showcase, unsigned char pos, short int xc, short int yc, short int zc, unsigned char count ) {
#ifdef World_ItemMove_DEBUG
        std::cout << "dropItemFromShowcaseOnMap: Spieler " << cp->name << " wirft ein Item auf die Karte" << std::endl;
#endif
        if (count == 0) return;
        if ( takeItemFromShowcase( cp, showcase, pos, count ) ) {
               std::cout << "dropItemFromShowcaseOnMap: Item genommen" << std::endl;
               //CommonStruct com;
               //Ausfhren eines MoveItemScripts
               ScriptItem s_item = g_item,t_item = g_item; //Item einmal source und einmal target, das erste ist das Item vor dem bewegen das andere nach dem bewegen
               s_item.pos = cp->pos;
               s_item.itempos = pos;
               if ( showcase == 0 )
                      s_item.type = ScriptItem::it_showcase1;
               else
                      s_item.type = ScriptItem::it_showcase2;
               s_item.owner = cp;
               t_item.pos = position(xc, yc, zc);
               t_item.type = ScriptItem::it_field;
               t_item.owner = cp;
               boost::shared_ptr<LuaItemScript> script = CommonItems->findScript(t_item.id);
               
               if ( script ) 
               {
                   if ( !script->MoveItemBeforeMove(cp, s_item, t_item) ) 
                   {
                       //Script verhindert das Item weg gelegt wird. Item wird zurck gegelgt
                       if ( !putItemInShowcase(cp, showcase,0) ) 
                       {
                            std::cerr<<"DropItemFromShowcase wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                            g_cont = NULL;
                            g_item.id = 0;
                            g_item.number = 0;
                            g_item.wear = 0;
                            g_item.data = 0;
                            g_item.quality = 0;
                            g_item.data_map.clear();
                        }
                        return;
                   }
               }
               /*
               if ( CommonItems->find(t_item.id, com) ) 
               {
                      if ( com.script ) {
                             if ( !com.script->MoveItemBeforeMove(cp, s_item, t_item) ) {
                                    //Script verhindert das Item weg gelegt wird. Item wird zurck gegelgt
                                    if ( !putItemInShowcase(cp, showcase,0) ) {
                                           std::cerr<<"DropItemFromShowcase wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                                           g_cont = NULL;
                                           g_item.id = 0;
                                           g_item.number = 0;
                                           g_item.wear = 0;
                        g_item.data = 0;
                        g_item.quality = 0;
                                    }
                                    return;
                             }
                      }
               }*/
               //Ende Ausfhren eines MoveItemScripts
               short int new_x = xc;
               short int new_y = yc;
               short int new_z = zc;
               if ( !putItemOnMap( cp, new_x, new_y, new_z ) ) {
                      std::cout << "Item konnte nicht auf die Karte gelegt werden -> zurcklegen" << std::endl;
                      if ( !putItemInShowcase( cp, showcase,0 ) ) {
                             std::cerr << "dropItemFromShowcaseOnMap: Datenverlust beim Zurcklegen, Spieler " << cp->name << std::endl;
                             g_cont = NULL;
                             g_item.id = 0;
                             g_item.number = 0;
                             g_item.wear = 0;
                             g_item.quality = 0;
                             g_item.data = 0;
                             g_item.data_map.clear();
                      }
               } 
               else 
               {
                   /*
                      if ( com.script ) {
                             com.script->MoveItemAfterMove(cp, s_item, t_item);
                      }
                      */
                      
                   if ( script )
                   {
                      script->MoveItemAfterMove(cp, s_item, t_item);
                   }
               }
        }
#ifdef World_ItemMove_DEBUG
        std::cout << "dropItemFromShowcaseOnMap: beendet" << std::endl;
#endif
}



void World::moveItemFromShowcaseToPlayer( Player* cp, unsigned char showcase, unsigned char pos, unsigned char cpos, unsigned char count ) {
        bool NOK = false;
        //CommonStruct com;
#ifdef World_ItemMove_DEBUG
        std::cout << "moveItemFromShowcaseToPlayer: Spieler " << cp->name << " nimmt ein Item auf\n"
        << "showcase: " << ( short int ) showcase << " from position: " << ( short int ) pos << " to position: " << ( short int ) cpos << std::endl;
#endif
        if (count == 0) {
               return;
        }
        if ( takeItemFromShowcase( cp, showcase, pos, count ) ) {
#ifdef World_ItemMove_DEBUG
               std::cout << "Item genommen" << std::endl;
#endif

               //Ausfhren eines MoveItemScripts
               //Erzeugen von source und TargetItem
               ScriptItem s_item = g_item, t_item = g_item;
               s_item.pos = cp->pos;
               s_item.itempos = pos;
               if ( showcase == 0 )
                      s_item.type = ScriptItem::it_showcase1;
               else
                      s_item.type = ScriptItem::it_showcase2;
               s_item.owner = cp;
               t_item.pos = cp->pos;
               t_item.itempos = cpos;
               if ( cpos < MAX_BODY_ITEMS )
                      t_item.type = ScriptItem::it_inventory;
               else
                      t_item.type = ScriptItem::it_belt;
               t_item.owner = cp;
               //Ende Erzeugen von Source und Target Item
               boost::shared_ptr<LuaItemScript> script = CommonItems->findScript( t_item.id );
               if ( script ) 
               {
                   if (!script->MoveItemBeforeMove(cp, s_item,t_item) ) 
                   {
                       if ( !putItemInShowcase(cp, showcase,0) ) 
                       {
                           std::cerr<<"MoveItemFromShowcaseToPlayer wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                           g_cont = NULL;
                           g_item.id = 0;
                           g_item.number = 0;
                           g_item.wear = 0;
                           g_item.quality = 0;
                           g_item.data = 0;
                           g_item.data_map.clear();
                       }
                    return;
                    }
               }
               
               /*
               if ( CommonItems->find(t_item.id, com) ) {
                      if ( com.script ) {
                             if (!com.script->MoveItemBeforeMove(cp, s_item,t_item) ) {
                                    if ( !putItemInShowcase(cp, showcase,0) ) {
                                           std::cerr<<"MoveItemFromShowcaseToPlayer wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                                           g_cont = NULL;
                                           g_item.id = 0;
                                           g_item.number = 0;
                                           g_item.wear = 0;
                        g_item.quality = 0;
                        g_item.data = 0;
                                    }
                                    return;
                             }
                      }
               } */
               //Ende Ausfhren eines MoveItemScripts
               if ( ! cp->weightOK( g_item.id, g_item.number, g_cont ) ) {
                      message( zuschwer, cp );
                      NOK = true;
               }

               if (!NOK) {
                      if ( !putItemOnInvPos( cp, cpos ) ) {
                             NOK = true;
                      } else {
                             if ( ( cp->LoadWeight() * 100 ) / cp->maxLoadWeight() >= 50 ) {
                                    if ( ! cp->wasEncumberedSent() ) {
                                           cp->setEncumberedSent( true );
                                           std::string tmessage;
                                           switch ( cp->getPlayerLanguage() ) {
                                                  case Language::german:
                                                         tmessage = "Du bist �berladen.";
                                                         break;
                                                  case Language::english:
                                                         tmessage = "You are encumbered.";
                                                         break;
                                                  case Language::french:
                                                         tmessage = "You are encumbered.";
                                                         break;
                                                  default:
                                                         tmessage = "You are encumbered.";
                                                         break;
                                           }
                                           cp->sendMessage( tmessage );
                                    }
                             } else if ( cp->wasEncumberedSent() ) {
                                    cp->setEncumberedSent( false );

                                    std::string tmessage;
                                    switch ( cp->getPlayerLanguage() ) {
                                           case Language::german:
                                                  tmessage = "Du bist nicht mehr �berladen.";
                                                  break;
                                           case Language::english:
                                                  tmessage = "You are no longer encumbered.";
                                                  break;
                                           case Language::french:
                                                  tmessage = "You are no longer encumbered.";
                                                  break;
                                           default:
                                                  tmessage = "You are no longer encumbered.";
                                                  break;
                                    }

                                    cp->sendMessage( tmessage );
                             }
                             //Script nach erfolgreichen Ausfhren des Moves ausfhren
                             /*
                             if ( com.script ) {
                                    com.script->MoveItemAfterMove(cp, s_item, t_item);
                             }*/
                             if ( script )
                             {
                                 script->MoveItemAfterMove(cp, s_item, t_item);
                             }
                             //Ende des SCripte nach erfolgreichen ausfhren des Moves.
                      }
               }

               if (NOK) {
                      if ( !putItemInShowcase( cp, showcase,0 ) ) {
                             std::cerr << "moveItemFromShowcaseToPlayer: Datenverlust beim Zurcklegen, Spieler " << cp->name << std::endl;
                             g_cont = NULL;
                             g_item.id = 0;
                             g_item.number = 0;
                             g_item.wear = 0;
                             g_item.quality = 0;
                             g_item.data = 0;
                             g_item.data_map.clear();
                      }
               }
        }
#ifdef World_ItemMove_DEBUG
        std::cout << "moveItemFromShowcaseToPlayer: Ende" << std::endl;
#endif
}



void World::dropItemFromPlayerOnMap( Player* cp, unsigned char cpos, short int xc, short int yc, short int zc, unsigned char count ) {
#ifdef World_ItemMove_DEBUG
        std::cout << "dropItemFromPlayerOnMap: Spieler " << cp->name << " wirft ein Item auf die Karte" << std::endl;
#endif
        if (count == 0) return;
        //CommonStruct com;
        if ( takeItemFromInvPos( cp, cpos, count ) ) {
               std::cout << "dropItemFromPlayerOnMap Item genommen" << std::endl;
               //Ausfhren eines MoveItemScripts
               ScriptItem s_item = g_item, t_item = g_item;
               s_item.pos = cp->pos;
               if (cpos < MAX_BODY_ITEMS)
                      s_item.type = ScriptItem::it_inventory;
               else
                      s_item.type = ScriptItem::it_belt;
               s_item.itempos = cpos;
               s_item.owner = cp;
               t_item.pos = position( xc, yc, zc );
               t_item.type = ScriptItem::it_field;
               t_item.owner = cp;
               boost::shared_ptr<LuaItemScript> script = CommonItems->findScript( t_item.id );
               if ( script )
               {
                   if ( !script->MoveItemBeforeMove(cp, s_item, t_item) ) 
                   {
                       //std::cout<<"Legen des Items vom Spieler auf Karte vom Script unterbunden, zurck legen"<<std::endl;
                       if ( !putItemOnInvPos(cp, cpos) ) 
                       {
                            std::cerr<<"MoveItemFromPlayerOnMap wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                            g_cont = NULL;
                            g_item.id = 0;
                            g_item.number = 0;
                            g_item.wear = 0;
                            g_item.quality = 0;
                            g_item.data = 0;
                            g_item.data_map.clear();
                        }
                        return;
                    }                   
               }
                   
                   
               //Ende Definieren der Source und TargetItems
               /*
               if ( CommonItems->find(t_item.id, com) ) {
                      if ( com.script ) {
                             if ( !com.script->MoveItemBeforeMove(cp, s_item, t_item) ) {
                                    //std::cout<<"Legen des Items vom Spieler auf Karte vom Script unterbunden, zurck legen"<<std::endl;
                                    if ( !putItemOnInvPos(cp, cpos) ) {
                                           std::cerr<<"MoveItemFromPlayerOnMap wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                                           g_cont = NULL;
                                           g_item.id = 0;
                                           g_item.number = 0;
                                           g_item.wear = 0;
                        g_item.quality = 0;
                        g_item.data = 0;
                                    }
                                    return;
                             }
                      }
               }*/
               //Ende Ausfhren eines MoveItemScripts
               short int new_x = xc;
               short int new_y = yc;
               short int new_z = zc;
               if ( !putItemOnMap( cp, new_x, new_y, new_z ) ) {
                      std::cout << "Item konnte nicht auf die Karte gelegt werden -> zurcklegen" << std::endl;
                      if ( !putItemOnInvPos( cp, cpos ) ) {
                             std::cerr << "dropItemFromPlayerOnMap: Datenverlust beim Zurcklegen, Spieler " << cp->name << std::endl;
                             g_cont = NULL;
                             g_item.id = 0;
                             g_item.number = 0;
                             g_item.wear = 0;
                             g_item.quality = 0;
                             g_item.data = 0;
                             g_item.data_map.clear();
                      }
               } 
               /*
               else if ( com.script ) {
                      com.script->MoveItemAfterMove(cp, s_item, t_item);
               }*/
               else if ( script )
               {
                   script->MoveItemAfterMove( cp, s_item, t_item);
               }

        }
#ifdef World_ItemMove_DEBUG
        std::cout << "dropItemFromPlayerOnMap: Ende" << std::endl;
#endif
}



void World::dropItemFromMonsterOnMap( Monster* cm, unsigned char cpos, char xo, char yo, char zo, unsigned char count ) {
#ifdef World_ItemMove_DEBUG
        std::cout << "dropItemFromMonsterOnMap: Monster " << cm->name << " wirft ein Item auf die Karte" << std::endl;
#endif
        if (count == 0) return;
        if ( takeItemFromInvPos( cm, cpos, count ) ) {      // take item from monster position
               std::cout << "Item genommen" << std::endl;
               short int new_x = cm->pos.x + xo;               // select coordinates for item to drop
               short int new_y = cm->pos.y + yo;
               short int new_z = cm->pos.z + zo;

               if ( !putItemOnMap( cm, new_x, new_y, new_z ) ) {  // put item there
                      std::cout << "Item konnte nicht auf die Karte gelegt werden -> zurcklegen" << std::endl;
                      if ( !putItemOnInvPos( cm, cpos ) ) {
                             std::cerr << "dropItemFromMonsterOnMap: Datenverlust beim Zurcklegen, Monster " << cm->name << std::endl;
                             g_cont = NULL;
                             g_item.id = 0;
                             g_item.number = 0;
                             g_item.wear = 0;
                             g_item.quality = 0;
                             g_item.data = 0;
                             g_item.data_map.clear();
                      }
               }
        }
#ifdef World_ItemMove_DEBUG
        std::cout << "dropItemFromMonsterOnMap: Ende" << std::endl;
#endif
}



void World::moveItemBetweenBodyParts( Player* cp, unsigned char opos, unsigned char npos, unsigned char count ) {
#ifdef World_ItemMove_DEBUG
        std::cout << "moveItemBetweenBodyParts: Spieler " << cp->name << " verschiebt ein Item" << std::endl;
#endif
        //CommonStruct com;
        if (count == 0) return;
        if ( takeItemFromInvPos( cp, opos, count ) ) {
               std::cout << "Item genommen" << std::endl;
               ScriptItem s_item = g_item, t_item = g_item;
               s_item.owner = cp;
               s_item.pos = cp->pos;
               if (opos < MAX_BODY_ITEMS )
                      s_item.type = ScriptItem::it_inventory;
               else
                      s_item.type = ScriptItem::it_belt;
               s_item.itempos = opos;
               t_item.owner = cp;
               t_item.pos = cp->pos;
               if (npos < MAX_BODY_ITEMS)
                      t_item.type = ScriptItem::it_inventory;
               else
                      t_item.type = ScriptItem::it_belt;
               t_item.itempos = npos;
               boost::shared_ptr<LuaItemScript> script = CommonItems->findScript( t_item.id );
               if ( script )
               {
                   if (!script->MoveItemBeforeMove(cp, s_item, t_item) )
                   {
                       if ( !putItemOnInvPos(cp, opos) ) 
                       {
                           std::cerr<<"MoveItemFromPlayerOnMap wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                           g_cont = NULL;
                           g_item.id = 0;
                           g_item.number = 0;
                           g_item.wear = 0;
                           g_item.quality = 0;
                           g_item.data_map.clear();
                       }
                       return;
                   }
               }
                   
               /*
               if ( CommonItems->find(t_item.id, com) ) {
                      if ( com.script ) {
                             if (!com.script->MoveItemBeforeMove(cp, s_item, t_item) ) {
                                    if ( !putItemOnInvPos(cp, opos) ) {
                                           std::cerr<<"MoveItemFromPlayerOnMap wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                                           g_cont = NULL;
                                           g_item.id = 0;
                                           g_item.number = 0;
                                           g_item.wear = 0;
                        g_item.quality = 0;
                                    }
                                    return;
                             }
                      }
               }*/
               //Ende Ausfhren eines MoveItemScripts

               if ( !putItemOnInvPos( cp, npos ) ) {
                      std::cout << "Item konnte nicht verschoben werden -> zurcklegen" << std::endl;
                      if ( !putItemOnInvPos( cp, opos ) ) {
                             std::cerr << "moveItemBetweenBodyParts: Datenverlust beim Zurcklegen, Spieler " << cp->name << std::endl;
                             g_cont = NULL;
                             g_item.id = 0;
                             g_item.number = 0;
                             g_item.wear = 0;
                             g_item.quality = 0;
                             g_item.data = 0;
                             g_item.data_map.clear();
                      }
               }
               else 
               {
                     // if (com.script) com.script->MoveItemAfterMove(cp, s_item, t_item);
                     if ( script ) script->MoveItemAfterMove(cp, s_item, t_item);
               }
        }
#ifdef World_ItemMove_DEBUG
        std::cout << "moveItemBetweenBodyParts: Ende" << std::endl;
#endif
}



void World::moveItemFromPlayerIntoShowcase( Player* cp, unsigned char cpos, unsigned char showcase, unsigned char pos, unsigned char count ) {
#ifdef World_ItemMove_DEBUG
        std::cout << "moveItemFromPlayerIntoShowcase: Spieler " << cp->name << " verschiebt Item von der Karte in ein showcase" << std::endl;
#endif
      //  CommonStruct com;

        if (count == 0) return;
        if ( takeItemFromInvPos( cp, cpos, count ) ) {
               std::cout << "Item genommen" << std::endl;
               //Ausfhren eines Move Item Scriptes
               ScriptItem s_item = g_item, t_item = g_item;
               if (cpos < MAX_BODY_ITEMS)
                      s_item.type = ScriptItem::it_inventory;
               else
                      s_item.type = ScriptItem::it_belt;
               s_item.pos = cp->pos;
               s_item.itempos = cpos;
               s_item.owner = cp;
               if ( showcase == 0 )
                      t_item.type = ScriptItem::it_showcase1;
               else
                      t_item.type = ScriptItem::it_showcase2;
               t_item.pos = cp->pos;
               t_item.owner = cp;
               t_item.itempos = pos;
               boost::shared_ptr<LuaItemScript> script = CommonItems->findScript( t_item.id );
               if ( script )
               {
                    if (!script->MoveItemBeforeMove(cp, s_item, t_item) ) 
                    {
                        if ( !putItemOnInvPos(cp, cpos) ) 
                        {
                            std::cerr<<"MoveItemFromPlayerOnMap wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                            g_cont = NULL;
                            g_item.id = 0;
                            g_item.number = 0;
                            g_item.wear = 0;
                            g_item.quality = 0;
                            g_item.data = 0;
                            g_item.data_map.clear();
                         }
                         return;
                    }
               }
               /*
               if ( CommonItems->find(t_item.id, com) ) {
                      if ( com.script ) {

                             if (!com.script->MoveItemBeforeMove(cp, s_item, t_item) ) {
                                    if ( !putItemOnInvPos(cp, cpos) ) {
                                           std::cerr<<"MoveItemFromPlayerOnMap wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                                           g_cont = NULL;
                                           g_item.id = 0;
                                           g_item.number = 0;
                                           g_item.wear = 0;
                        g_item.quality = 0;
                        g_item.data = 0;
                                    }
                                    return;
                             }
                      }
               }*/
               //Ende Ausfhren eines MoveItemScripts
               if ( !putItemInShowcase( cp, showcase, pos ) ) {
                      std::cout << "Item konnte nicht eingefgt werden -> zurcklegen" << std::endl;
                      if ( !putItemOnInvPos( cp, cpos ) ) {
                             std::cerr << "moveItemFromPlayerIntoShowcase: Datenverlust beim Zurcklegen, Spieler " << cp->name << std::endl;
                             g_cont = NULL;
                             g_item.id = 0;
                             g_item.number = 0;
                             g_item.wear = 0;
                             g_item.quality = 0;
                             g_item.data = 0;
                             g_item.data_map.clear();
                      }
               } 
               else 
               {
                      //if ( com.script ) com.script->MoveItemAfterMove(cp, s_item, t_item);
                      if ( script ) script->MoveItemAfterMove(cp, s_item, t_item);
               }
        }
#ifdef World_ItemMove_DEBUG
        std::cout << "moveItemFromPlayerIntoShowcase: Ende" << std::endl;
#endif
}



void World::moveItemFromMapIntoShowcase( Player* cp, char direction, unsigned char showcase, unsigned char pos, unsigned char count ) {

        bool NOK = false;
        //CommonStruct com;

#ifdef World_ItemMove_DEBUG
        std::cout << "moveItemFromMapIntoShowcase: Spieler " << cp->name << " verschiebt Item von der Karte in ein showcase" << std::endl;
#endif

        if (count == 0)
               return;

        if ( direction < 11 ) {
               short int old_x = cp->pos.x + moveSteps[ (int)direction ][ 0 ];
               short int old_y = cp->pos.y + moveSteps[ (int)direction ][ 1 ];
               short int old_z = cp->pos.z + moveSteps[ (int)direction ][ 2 ];

               if ( takeItemFromMap( cp, old_x, old_y, old_z) ) {
                      std::cout << "Item genommen" << std::endl;
                      ScriptItem s_item = g_item, t_item = g_item;
                      s_item.pos = position(old_x, old_y, old_z);
                      s_item.type = ScriptItem::it_field;
                      s_item.owner = cp;
                      t_item.pos = cp->pos;
                      if ( showcase == 0 )
                             t_item.type = ScriptItem::it_showcase1;
                      else
                             t_item.type = ScriptItem::it_showcase2;
                      t_item.itempos = pos;
                      t_item.owner = cp;

                      //Ausfhren eines Move Item Scriptes
                      boost::shared_ptr<LuaItemScript> script = CommonItems->findScript( t_item.id );
                      if ( script )
                      {
                          if (!script->MoveItemBeforeMove(cp, s_item, t_item) ) 
                          {
                              if ( !putItemOnMap(cp, old_x, old_y, old_z) ) 
                              {
                                  std::cerr<<"MoveItemFromMapIntoShowcase wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                                  g_cont = NULL;
                                  g_item.id = 0;
                                  g_item.number = 0;
                                  g_item.wear = 0;
                                  g_item.quality = 0;
                                  g_item.data = 0;
                                  g_item.data_map.clear();
                              }
                          return;
                          }
                      }
                      /*
                      if ( CommonItems->find(t_item.id, com) ) {
                             if ( com.script ) {
                                    if (!com.script->MoveItemBeforeMove(cp, s_item, t_item) ) {
                                           if ( !putItemOnMap(cp, old_x, old_y, old_z) ) {
                                                  std::cerr<<"MoveItemFromMapIntoShowcase wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                                                  g_cont = NULL;
                                                  g_item.id = 0;
                                                  g_item.number = 0;
                                                  g_item.wear = 0;
                            g_item.quality = 0;
                            g_item.data = 0;
                                           }
                                           return;
                                    }
                             }
                      }*/
                      //Ende Ausfhren eines MoveItemScripts
                      Item tempitem = g_item;
                      if ( count < g_item.number ) {
                             std::cout << "nicht alles verschieben" << std::endl;
                             g_item.number = count;
                      }

                      if ( ! cp->weightOK( g_item.id, g_item.number, g_cont ) ) {
                             message( zuschwer, cp );
                             NOK = true;
                      }

                      if (!NOK) {
                             if ( !putItemInShowcase( cp, showcase,pos ) ) {
                                    NOK =true;
                             }

                             if ( ( cp->LoadWeight() * 100 ) / cp->maxLoadWeight() >= 50 ) {
                                    if ( ! cp->wasEncumberedSent() ) {
                                           cp->setEncumberedSent( true );
                                           std::string tmessage;
                                           switch ( cp->getPlayerLanguage() ) {
                                                  case Language::german:
                                                         tmessage = "Du bist �berladen.";
                                                         break;
                                                  case Language::english:
                                                         tmessage = "You are encumbered.";
                                                         break;
                                                  case Language::french:
                                                         tmessage = "You are encumbered.";
                                                         break;
                                                  default:
                                                         tmessage = "You are encumbered.";
                                                         break;
                                           }
                                           cp->sendMessage( tmessage );
                                    }
                             } else if ( cp->wasEncumberedSent() ) {
                                    cp->setEncumberedSent( false );
                                    std::string tmessage;
                                    switch ( cp->getPlayerLanguage() ) {
                                           case Language::german:
                                                  tmessage = "Du bist nicht mehr �berladen.";
                                                  break;
                                           case Language::english:
                                                  tmessage = "You are no longer encumbered.";
                                                  break;
                                           case Language::french:
                                                  tmessage = "You are no longer encumbered.";
                                                  break;
                                           default:
                                                  tmessage = "You are no longer encumbered.";
                                                  break;
                                    }
                                           cp->sendMessage( tmessage );
                             }
                             /*
                             if ( com.script ) {
                                    com.script->MoveItemAfterMove(cp, s_item, t_item);
                             }*/
                             if ( script ) script->MoveItemAfterMove(cp, s_item, t_item);


                      }

                      if (NOK) {
                             std::cout << "Item konnte nicht eingefgt werden -> zurcklegen" << std::endl;
                             g_item = tempitem;
                             if ( !putItemOnMap( cp, old_x, old_y, old_z ) ) {
                                    std::cerr << "moveItemFromMapIntoShowcase: Datenverlust beim Zurcklegen, Spieler " << cp->name << std::endl;
                                    g_cont = NULL;
                                    g_item.id = 0;
                                    g_item.number = 0;
                                    g_item.wear = 0;
                                    g_item.quality = 0;
                                    g_item.data = 0;
                                    g_item.data_map.clear();
                             }
                             return;
                      }

                      if (count < tempitem.number && !ContainerItems->find(tempitem.id) ) {
                             g_item = tempitem;
                             g_item.number -= count;
                             if ( !putItemOnMap( cp, old_x, old_y, old_z ) ) {
                                    std::cerr << "moveItemFromMapIntoShowcase : Datenverlust beim teilweisen Verschieben, Spieler " << cp->name << std::endl;
                                    g_cont = NULL;
                                    g_item.id = 0;
                                    g_item.number = 0;
                                    g_item.wear = 0;
                                    g_item.quality = 0;
                                    g_item.data = 0;
                                    g_item.data_map.clear();
                                    return;
                             }
                      }
                      return;
               }
        }
#ifdef World_ItemMove_DEBUG
        std::cout << "moveItemFromMapIntoShowcase: Ende" << std::endl;
#endif

        return;

}



void World::moveItemFromMapToPlayer( Player* cp, char direction, unsigned char cpos, unsigned char count ) {
        bool NOK = false;
        //CommonStruct com;

#ifdef World_ItemMove_DEBUG
        std::cout << "moveItemFromMapToPlayer: Spieler " << cp->name << " verschiebt Item von der Karte an den Koerper" << std::endl;
#endif

        if (count == 0)
               return;

        if ( direction < 11 ) {
               short int old_x = cp->pos.x + moveSteps[ (int)direction ][ 0 ];
               short int old_y = cp->pos.y + moveSteps[ (int)direction ][ 1 ];
               short int old_z = cp->pos.z + moveSteps[ (int)direction ][ 2 ];

               if ( takeItemFromMap( cp, old_x, old_y, old_z) ) {
#ifdef World_ItemMove_DEBUG
                      std::cout << "Item genommen" << std::endl;
#endif
                      ScriptItem s_item = g_item, t_item = g_item;
                      //Ausfhren eines Move Item Scriptes
                      s_item.pos = position(old_x, old_y, old_z);
                      s_item.type = ScriptItem::it_field;
                      s_item.owner = cp;
                      t_item.pos = cp->pos;
                      if (cpos < MAX_BODY_ITEMS)
                             t_item.type = ScriptItem::it_inventory;
                      else
                             t_item.type = ScriptItem::it_belt;
                      t_item.owner = cp;
                      t_item.itempos = cpos;
                      boost::shared_ptr<LuaItemScript> script = CommonItems->findScript( t_item.id );
                      if ( script )
                      {
                          if (!script->MoveItemBeforeMove(cp, s_item, t_item) ) 
                          {
                              if ( !putItemOnMap(cp, old_x, old_y, old_z) ) 
                              {
                                  std::cerr<<"MoveItemFromMapToPlayer wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                                  g_cont = NULL;
                                  g_item.id = 0;
                                  g_item.number = 0;
                                  g_item.wear = 0;
                                  g_item.quality = 0;
                                  g_item.data = 0;
                                  g_item.data_map.clear();
                              }
                              return;
                          }
                      }
                      /*
                      if ( CommonItems->find(t_item.id, com) ) {
                             if ( com.script ) {
                                    if (!com.script->MoveItemBeforeMove(cp, s_item, t_item) ) {
                                           if ( !putItemOnMap(cp, old_x, old_y, old_z) ) {
                                                  std::cerr<<"MoveItemFromMapToPlayer wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                                                  g_cont = NULL;
                                                  g_item.id = 0;
                                                  g_item.number = 0;
                                                  g_item.wear = 0;
                            g_item.quality = 0;
                            g_item.data = 0;
                                           }
                                           return;
                                    }
                             }
                      }
                      */
                      //Ende Ausfhren eines MoveItemScripts
                      Item tempitem = g_item;
                      if ( count < g_item.number ) {
#ifdef World_ItemMove_DEBUG
                             std::cout << "nicht alles verschieben" << std::endl;
#endif
                             g_item.number = count;
                      }

                      if ( ! cp->weightOK( g_item.id, g_item.number, g_cont ) ) {
                             message( zuschwer, cp );
                             NOK = true;
                      }

                      if (!NOK) {
                             if ( !putItemOnInvPos( cp, cpos ) ) {
                                    NOK = true;
                             }

                             if ( ( cp->LoadWeight() * 100 ) / cp->maxLoadWeight() >= 50 ) {
                                    if ( ! cp->wasEncumberedSent() ) {
                                           cp->setEncumberedSent( true );
                                           std::string tmessage;
                                           switch ( cp->getPlayerLanguage() ) {
                                                  case Language::german:
                                                         tmessage = "Du bist �berladen.";
                                                         break;
                                                  case Language::english:
                                                         tmessage = "You are encumbered.";
                                                         break;
                                                  case Language::french:
                                                         tmessage = "You are encumbered.";
                                                         break;
                                                  default:
                                                         tmessage = "You are encumbered.";
                                                         break;
                                           }
                                           cp->sendMessage( tmessage );
                                    }
                             } else if ( cp->wasEncumberedSent() ) {
                                    cp->setEncumberedSent( false );
                                    std::string tmessage;
                                    switch ( cp->getPlayerLanguage() ) {
                                           case Language::german:
                                                  tmessage = "Du bist nicht mehr �berladen.";
                                                  break;
                                           case Language::english:
                                                  tmessage = "You are no longer encumbered.";
                                                  break;
                                           case Language::french:
                                                  tmessage = "You are no longer encumbered.";
                                                  break;
                                           default:
                                                  tmessage = "You are no longer encumbered.";
                                                  break;
                                    }
                                           cp->sendMessage( tmessage );
                             }
                             /*
                             if ( com.script ) {
                                    com.script->MoveItemAfterMove(cp, s_item, t_item);
                             }*/
                             if ( script ) script->MoveItemAfterMove(cp, s_item, t_item);
                      }

                      if (NOK) {
#ifdef World_ItemMove_DEBUG
                             std::cout << "Item konnte nicht eingefgt werden -> zurcklegen" << std::endl;
#endif
                             g_item = tempitem;
                             if ( ! putItemOnMap( cp, old_x, old_y, old_z ) ) {
                                    std::cerr << "moveItemFromMapToPlayer: Datenverlust beim Zurcklegen, Spieler " << cp->name << std::endl;
                                    g_cont = NULL;
                                    g_item.id = 0;
                                    g_item.number = 0;
                                    g_item.wear = 0;
                                    g_item.quality = 0;
                                    g_item.data = 0;
                                    g_item.data_map.clear();
                             }
                             return;
                      }

                      if (count < tempitem.number && !ContainerItems->find(tempitem.id)) {
                             g_item = tempitem;
                             g_item.number -= count;
                             if ( !putItemOnMap( cp, old_x, old_y, old_z ) ) {
                                    std::cerr << "moveItemFromMapToPlayer : Datenverlust beim teilweisen Verschieben, Spieler " << cp->name << std::endl;
                                    g_cont = NULL;
                                    g_item.id = 0;
                                    g_item.number = 0;
                                    g_item.wear = 0;
                                    g_item.quality = 0;
                                    g_item.data = 0;
                                    g_item.data_map.clear();
                                    return;
                             }
                      }

                      return;
               }
        }
#ifdef World_ItemMove_DEBUG
        std::cout << "moveItemFromMapToPlayer: Ende" << std::endl;
#endif
}


void World::moveItemBetweenShowcases( Player* cp, unsigned char source, unsigned char pos, unsigned char dest, unsigned char pos2, unsigned char count ) {
        bool NOK = false;
        //CommonStruct com;
#ifdef World_ItemMove_DEBUG
        std::cout << "moveItemBetweenShowcases: Spieler " << cp->name << " verschiebt Item zwischen showcases" << std::endl;
#endif
        if (count == 0) {
               return;
        }

        if ( takeItemFromShowcase( cp, source, pos, count ) ) {
#ifdef World_ItemMove_DEBUG
               std::cout << "Item genommen" << std::endl;
#endif
               ScriptItem s_item = g_item, t_item = g_item;
               s_item.pos = cp->pos;
               if ( source == 0 )
                      s_item.type = ScriptItem::it_showcase1;
               else
                      s_item.type = ScriptItem::it_showcase2;
               s_item.itempos = pos;
               s_item.owner = cp;
               t_item.pos = cp->pos;
               if ( dest == 0 )
                      t_item.type = ScriptItem::it_showcase1;
               else
                      t_item.type = ScriptItem::it_showcase2;
               t_item.itempos = pos2;
               t_item.owner = cp;
               //Ausfhren eines Move Item Scriptes
               boost::shared_ptr<LuaItemScript> script = CommonItems->findScript( t_item.id );
               if ( script )
               {
                   if (!script->MoveItemBeforeMove(cp, s_item, t_item) ) 
                   {
                       if ( !putItemInShowcase(cp, dest, pos2 ) ) 
                       {
                           std::cerr<<"MoveItemBetweenShowcases wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                           g_cont = NULL;
                           g_item.id = 0;
                           g_item.number = 0;
                           g_item.wear = 0;
                           g_item.quality = 0;
                           g_item.data = 0;
                           g_item.data_map.clear();
                       }
                       return;
                   }
               }
               /*
               if ( CommonItems->find(t_item.id, com) ) {
                      if ( com.script ) {
                             if (!com.script->MoveItemBeforeMove(cp, s_item, t_item) ) {
                                    if ( !putItemInShowcase(cp, dest, pos2 ) ) {
                                           std::cerr<<"MoveItemBetweenShowcases wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                                           g_cont = NULL;
                                           g_item.id = 0;
                                           g_item.number = 0;
                                           g_item.wear = 0;
                        g_item.quality = 0;
                        g_item.data = 0;
                                    }
                                    return;
                             }
                      }
               }*/
               //Ende Ausfhren eines MoveItemScripts

               if ( dest < MAXSHOWCASES ) {
                      if (cp->showcases[ dest ].inInventory()) {
                             if ( ! cp->weightOK( g_item.id, g_item.number, g_cont ) ) {
                                    message( zuschwer, cp );
                                    NOK=true;
                             }
                      }
               }

               if (!NOK) {
                      if ( !putItemInShowcase( cp, dest,pos2 ) ) {
                             NOK=true;
                      }

                      if ( ( cp->LoadWeight() * 100 ) / cp->maxLoadWeight() >= 50 ) {
                             if ( ! cp->wasEncumberedSent() ) {
                                    cp->setEncumberedSent( true );
                                    std::string tmessage;
                                    switch ( cp->getPlayerLanguage() ) {
                                           case Language::german:
                                                  tmessage = "Du bist �berladen.";
                                                  break;
                                           case Language::english:
                                                  tmessage = "You are encumbered.";
                                                  break;
                                           case Language::french:
                                                  tmessage = "You are encumbered.";
                                                  break;
                                           default:
                                                  tmessage = "You are encumbered.";
                                                  break;
                                    }
                                           cp->sendMessage( tmessage );
                             }
                      } else if ( cp->wasEncumberedSent() ) {
                             cp->setEncumberedSent( false );
                             std::string tmessage;
                             switch ( cp->getPlayerLanguage() ) {
                                    case Language::german:
                                           tmessage = "Du bist nicht mehr �berladen.";
                                           break;
                                    case Language::english:
                                           tmessage = "You are no longer encumbered.";
                                           break;
                                    case Language::french:
                                           tmessage = "You are no longer encumbered.";
                                           break;
                                    default:
                                           tmessage = "You are no longer encumbered.";
                                           break;
                             }
                                           cp->sendMessage( tmessage );
                      }
                      /*
                      if ( com.script ) {
                             com.script->MoveItemAfterMove(cp, s_item, t_item);
                      }*/
                      if ( script ) script->MoveItemAfterMove(cp, s_item, t_item);

               }

               if (NOK) {
                      std::cout << "Item konnte nicht eingefgt werden -> zurcklegen" << std::endl;
                      if ( ! putItemInShowcase( cp, source,0 ) ) {
                             std::cerr << "moveItemBetweenShowcases: Datenverlust beim Zurcklegen, Spieler " << cp->name << std::endl;
                             g_cont = NULL;
                             g_item.id = 0;
                             g_item.number = 0;
                             g_item.wear = 0;
                             g_item.quality = 0;
                             g_item.data = 0;
                             g_item.data_map.clear();
                      }
               }
        }
#ifdef World_ItemMove_DEBUG
        std::cout << "moveItemBetweenShowcases: Ende" << std::endl;
#endif
}


bool World::moveItem( Character* cc, unsigned char d, short int xc, short int yc, short int zc, unsigned char count ) {
#ifdef World_ItemMove_DEBUG
        std::cout << "moveItem: Character " << cc->name << " bewegt Item" << std::endl;
#endif
        //CommonStruct com;
        if (count == 0)
               return false;

        if ( ( d < 11 ) && ( cc != NULL ) ) {
               short int old_x = cc->pos.x + moveSteps[ d ][ 0 ];
               short int old_y = cc->pos.y + moveSteps[ d ][ 1 ];
               short int old_z = cc->pos.z + moveSteps[ d ][ 2 ];

               short int new_x = xc;
               short int new_y = yc;
               short int new_z = zc;

               if ( takeItemFromMap( cc, old_x, old_y, old_z ) ) {
                      ScriptItem s_item = g_item, t_item = g_item;
                      s_item.pos = position(old_x, old_y, old_z);
                      s_item.type = ScriptItem::it_field;
                      s_item.owner = cc;
                      t_item.pos = position(new_x, new_y, new_z);
                      t_item.type = ScriptItem::it_field;
                      t_item.owner = cc;

                      //Ausfhren eines Move Item Scriptes
                      boost::shared_ptr<LuaItemScript> script = CommonItems->findScript( t_item.id );
                      if ( script )
                      {
                          if (!script->MoveItemBeforeMove(cc, s_item, t_item ) ) 
                          {
                              if ( !putItemOnMap(cc, old_x, old_y, old_z ) ) 
                              {
                                  std::cerr<<"MoveItemOnMap wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cc->name<<std::endl;
                                  g_cont = NULL;
                                  g_item.id = 0;
                                  g_item.number = 0;
                                  g_item.wear = 0;
                                  g_item.quality = 0;
                                  g_item.data = 0;
                                  g_item.data_map.clear();
                              }
                              return false;
                          }
                      }
                      /*
                      if ( CommonItems->find(t_item.id, com) ) {
                             if ( com.script ) {
                                    if (!com.script->MoveItemBeforeMove(cc, s_item, t_item ) ) {
                                           if ( !putItemOnMap(cc, old_x, old_y, old_z ) ) {
                                                  std::cerr<<"MoveItemOnMap wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cc->name<<std::endl;
                                                  g_cont = NULL;
                                                  g_item.id = 0;
                                                  g_item.number = 0;
                                                  g_item.wear = 0;
                            g_item.quality = 0;
                            g_item.data = 0;
                                           }
                                           return false;
                                    }
                             }
                      }
                      */
                      //Ende Ausfhren eines MoveItemScripts

                      Item tempitem=g_item;
                      if ( count < g_item.number ) {
                             g_item.number=count;
                      }

                      if ( !putItemOnMap( cc, new_x, new_y, new_z ) ) {
                             g_item = tempitem;
                             if ( !putItemOnMap( cc, old_x, old_y, old_z ) ) {
                                    std::cerr << "moveitem (Map-Map): Datenverlust beim Zurcklegen, Character " << cc->name << std::endl;
                                    g_cont = NULL;
                                    g_item.id = 0;
                                    g_item.number = 0;
                                    g_item.wear = 0;
                                    g_item.quality = 0;
                                    g_item.data = 0;
                                    g_item.data_map.clear();
                             }
                             return false;
                      }
                      /*
                      else if ( com.script ) 
                      {
                             com.script->MoveItemAfterMove(cc, s_item, t_item );
                      }*/
                      else if ( script ) script->MoveItemAfterMove(cc, s_item, t_item );
                      //Wenn anzahl < als die anzahl des Tempitem und das Tempitem kein Container dann fehlerbehandlung
                      if (count < tempitem.number && !ContainerItems->find(tempitem.id) ) {
                             g_item = tempitem;
                             g_item.number -= count;
                             if ( !putItemOnMap( cc, old_x, old_y, old_z ) ) {
                                    std::cerr << "moveitem (Map-Map): Datenverlust beim teilweisen Verschieben, Character " << cc->name << std::endl;
                                    g_cont = NULL;
                                    g_item.id = 0;
                                    g_item.number = 0;
                                    g_item.wear = 0;
                                    g_item.quality = 0;
                                    g_item.data = 0;
                                    g_item.data_map.clear();
                                    return false;
                             }
                      } 
                      /*
                      else if ( com.script ) 
                      {
                             com.script->MoveItemAfterMove(cc, s_item, t_item );
                      }*/
                      else if ( script ) script->MoveItemAfterMove(cc, s_item, t_item );
                      return true;

               }
        }

#ifdef World_ItemMove_DEBUG
        std::cout << "moveItem: Ende" << std::endl;
#endif

        return false;

}



void World::lookIntoShowcaseContainer( Player* cp, unsigned char showcase, unsigned char pos ) {
#ifdef World_ItemMove_DEBUG
        std::cout << "lookIntoShowcaseContainer: Spieler " << cp->name << " oeffnet einen Container im showcase" << std::endl;
#endif

        if ( ( showcase < MAXSHOWCASES ) && ( cp != NULL ) ) {
               Container * top = cp->showcases[ showcase ].top();
               bool allowedToOpenContainer = false;
               
//Loop through all depots if ps is a depot if yes is depot set to true
        std::map<uint32_t,Container*>::iterator it;
        for ( it = cp->depotContents.begin(); it != cp->depotContents.end(); ++it )
               {
                   if ( it->second == top )
                   {
                 allowedToOpenContainer = true;
                 break;
            }                                    
               }
//end of loop through all the depots

               
               if ( top != NULL && allowedToOpenContainer) {
                      Container * tempc;
                      ScriptItem tempi;
                      if ( top->viewItemNr( pos, tempi, tempc ) ) {
#ifdef World_ItemMove_DEBUG
                             std::cout << "pos gefunden" << std::endl;
#endif
                             if ( tempc != NULL ) {
#ifdef World_ItemMove_DEBUG
                                    std::cout << "Container gefunden" << std::endl;
#endif
                                    // updaten der showcases des Spielers
                                    cp->showcases[ showcase ].openContainer( tempc );
                                    // Aenderungen an den Client schicken
                                    boost::shared_ptr<BasicServerCommand>cmd( new UpdateShowCaseTC( showcase, tempc->items));
                                    cp->Connection->addCommand( cmd );
                             }
                      }
               }
        }
#ifdef World_ItemMove_DEBUG
        std::cout << "lookIntoShowcaseContainer: Ende" << std::endl;
#endif
}



bool World::lookIntoBackPack( Player* cp, unsigned char showcase ) {
#ifdef World_ItemMove_DEBUG
        std::cout << "lookIntoBackPack: Spieler " << cp->name << " schaut in seinen Rucksack" << std::endl;
#endif
        if ( ( showcase < MAXSHOWCASES ) && ( cp != NULL ) ) {
               if ( ( cp->characterItems[ BACKPACK ].id != 0 ) && ( cp->backPackContents != NULL ) ) {
#ifdef World_ItemMove_DEBUG
                      std::cout << "Rucksackinhalt vorhanden" << std::endl;
#endif
                      // bisher geoeffnete Container im showcase schliessen
                      cp->showcases[ showcase ].clear();
                      // updaten des showcases des Spielers
                      cp->showcases[ showcase ].startContainer( cp->backPackContents, true );
                      // Aenderungen an den Client schicken
                      boost::shared_ptr<BasicServerCommand>cmd( new UpdateShowCaseTC( showcase, cp->backPackContents->items));
                      cp->Connection->addCommand(cmd);
#ifdef World_ItemMove_DEBUG
                      std::cout << "lookIntoBackPack: Ende" << std::endl;
#endif
                      return true;
               }
        }
#ifdef World_ItemMove_DEBUG
        std::cout << "lookIntoBackPack: Ende" << std::endl;
#endif
        return false;
}


bool World::lookIntoContainerOnField( Player* cp, char direction, unsigned char showcase ) {

        
#ifdef World_ItemMove_DEBUG
        std::cout << "lookIntoContainerOnField: Spieler " << cp->name << " schaut in einen Container" << std::endl;
#endif

        if ( ( showcase < MAXSHOWCASES ) && ( direction < 11 ) && ( cp != NULL ) ) {
               // Position des Item
               short int old_x = moveSteps[ (int)direction ][ 0 ] + cp->pos.x;
               short int old_y = moveSteps[ (int)direction ][ 1 ] + cp->pos.y;
               short int old_z = moveSteps[ (int)direction ][ 2 ] + cp->pos.z;

               Field* cfold;

               if ( GetPToCFieldAt( cfold, old_x, old_y, old_z, tmap ) ) {
#ifdef World_ItemMove_DEBUG
                      std::cout << "Feld vorhanden" << std::endl;
#endif
                      Item titem;
                      if ( cfold->ViewTopItem( titem ) ) {
#ifdef World_ItemMove_DEBUG
                             std::cout << "mindesten 1 Item vorhanden" << std::endl;
#endif
                             if ( titem.id != 321 && ContainerItems->find( titem.id, tempContainer ) ) {
#ifdef World_ItemMove_DEBUG
                                    std::cout << "item ist ein Container" << std::endl;
#endif
                                    MAP_POSITION opos;
                                    opos.x = old_x;
                                    opos.y = old_y;
                                    Map::ONTAINERHASH::iterator conmapo = tmap->maincontainers.find( opos );
                                    if ( conmapo != tmap->maincontainers.end() ) {
#ifdef World_ItemMove_DEBUG
                                           std::cout << "containermap fr das Feld gefunden" << std::endl;
#endif
                                           Container::ONTAINERMAP::iterator iv = ( *conmapo ).second.find( titem.number );
                                           if ( iv != ( *conmapo ).second.end() ) {
#ifdef World_ItemMove_DEBUG
                                                  std::cout << "der Inhalt des angegebenen Items mit der id titem.number wurde gefunden" << std::endl;
#endif
                                                  // updaten der showcases des Spielers
                                                  cp->showcases[ showcase ].startContainer( ( *iv ).second, false );
                                                  cp->mapshowcaseopen = true;
                                                  // Aenderungen an den Client schicken
                                                  boost::shared_ptr<BasicServerCommand>cmd(new UpdateShowCaseTC( showcase, (*iv).second->items));
                                                  cp->Connection->addCommand( cmd );

#ifdef World_ItemMove_DEBUG
                                                  std::cout << "lookIntoContainerOnField: Ende 1" << std::endl;
#endif
                                                  return true;
                                           } else {
#ifdef World_ItemMove_DEBUG
                                                  std::cout << "lookIntoContainerOnField: kein Containerinhalt vorhanden" << std::endl;
#endif

                                           }
                                    } else {
#ifdef World_ItemMove_DEBUG
                                           std::cout << "lookIntoContainerOnField: kein Container vorhanden" << std::endl;
#endif

                                    }
                             } else {
                                    // check if we got a depot there...
                                    if (titem.id == 321 ) 
                                    {
                                            //titem.data + 1 so no 0 depot is used.
                                            if ( depotScript )
                                            {
                                                if ( depotScript->onOpenDepot( cp, titem ) ) cp->openDepot(titem.data+1);
                                            }
                                            else
                                                cp->openDepot(titem.data+1);
                                                
                          //lookIntoDepot(cp, showcase);
                                    } 
                                    else 
                                    {
#ifdef World_ItemMove_DEBUG
                                           std::cout << "lookIntoContainerOnField: das oberste Item ist kein Container" << std::endl;
#endif

                                    }

                             }
                      }
               }
        }
#ifdef World_ItemMove_DEBUG
        std::cout << "lookIntoContainerOnField: Ende 2" << std::endl;
#endif
        return false;
}



void World::closeContainerInShowcase( Player* cp, unsigned char showcase ) {
#ifdef World_ItemMove_DEBUG
        std::cout << "closeContainerInShowcase: Spieler " << cp->name << " schliesst einen Container" << std::endl;
#endif

        if ( ( showcase < MAXSHOWCASES ) && ( cp != NULL ) ) {
               if ( !cp->showcases[ showcase ].closeContainer() ) 
               {
                      // Container war der letzte geoeffnete -> den showcase loeschen
                      boost::shared_ptr<BasicServerCommand>cmd(new ClearShowCaseTC( showcase ) );
                      cp->Connection->addCommand( cmd );
               } else {
                      // Aenderungen an den Client schicken
                      Container* temp = cp->showcases[ showcase ].top();
                      boost::shared_ptr<BasicServerCommand>cmd(new UpdateShowCaseTC( showcase, temp->items));
                      cp->Connection->addCommand( cmd );
               }
        }
#ifdef World_ItemMove_DEBUG
        std::cout << "closeContainerInShowcase: Ende" << std::endl;
#endif
}


void World::sendRemoveItemFromMapToAllVisibleCharacters( TYPE_OF_ITEM_ID id, short int xo, short int yo, short int zo, Field* cfp ) {
        if ( cfp != NULL ) {
               std::vector < Player* > temp = Players.findAllCharactersInRangeOf(xo, yo, zo, MAXVIEW );

               std::vector < Player* > ::iterator titerator;

               for ( titerator = temp.begin(); titerator < temp.end(); ++titerator ) 
               {
                      // cfp->SetLevel( zoffs ); // DEAD_CODE ???
                      boost::shared_ptr<BasicServerCommand>cmd(new ItemRemoveTC( xo, yo, zo ));
                      ( *titerator )->Connection->addCommand( cmd );
               }
        }
}


void World::sendSwapItemOnMapToAllVisibleCharacter( TYPE_OF_ITEM_ID id, short int xn, short int yn, short int zn, Item &it, Field* cfp)
{
        if ( cfp != NULL ) 
        {
               std::vector < Player* > temp = Players.findAllCharactersInRangeOf(xn, yn, zn, MAXVIEW );

               std::vector < Player* > ::iterator titerator;
               for ( titerator = temp.begin(); titerator < temp.end(); ++titerator ) 
               {
                   std::cout<<"adding swap to "<<(*titerator)->name<<"("<<(*titerator)->id<<")"<<std::endl;
                   boost::shared_ptr<BasicServerCommand>cmd(new ItemSwapTC( xn, yn, zn, id, it) );
                   ( *titerator )->Connection->addCommand( cmd );
                   std::cout<<"adding swap to "<<(*titerator)->name<<"("<<(*titerator)->id<<") ended."<<std::endl;
                      
               }
        }    
}

void World::sendPutItemOnMapToAllVisibleCharacters( short int xn, short int yn, short int zn, Item &it, Field* cfp ) {
        if ( cfp != NULL ) {
               std::vector < Player* > temp = Players.findAllCharactersInRangeOf(xn, yn, zn, MAXVIEW );

               std::vector < Player* > ::iterator titerator;

               for ( titerator = temp.begin(); titerator < temp.end(); ++titerator ) 
               {
                      // cfp->SetLevel( zoffs ); // DEAD_CODE ???
                      boost::shared_ptr<BasicServerCommand>cmd( new ItemPutTC( xn, yn, zn, it ) );
                      ( *titerator )->Connection->addCommand( cmd );
               }
        }
}


void World::sendChangesOfContainerContentsCM( Container* cc, Container* moved )
{
        if ( ( cc != NULL ) && ( moved != NULL ) )
        {
               PLAYERVECTOR::iterator titerator;

               Container* ps;

               for ( titerator = Players.begin(); titerator < Players.end(); ++titerator )
               {
                      for ( MAXCOUNTTYPE i = 0; i < MAXSHOWCASES; ++i ) 
                      {
                             ps = ( *titerator )->showcases[ i ].top();
                             if ( ( ps == cc ) && ( ps != NULL ) ) 
                             {
                                 boost::shared_ptr<BasicServerCommand>cmd(new UpdateShowCaseTC( i , ps->items) );
                                 (*titerator)->Connection->addCommand( cmd );
                             } 
                             else if ( ( *titerator )->showcases[ i ].contains( moved ) ) 
                             {
                                    ( *titerator )->showcases[ i ].clear();
                                    boost::shared_ptr<BasicServerCommand>cmd(new ClearShowCaseTC( i ) );
                                    ( *titerator )->Connection->addCommand( cmd );
                             }
                      }
               }
        }
}



void World::sendChangesOfContainerContentsIM( Container* cc ) {
        if ( cc != NULL ) {
               PLAYERVECTOR::iterator titerator;

               Container* ps;

               for ( titerator = Players.begin(); titerator < Players.end(); ++titerator ) {
                      for ( int i = 0; i < MAXSHOWCASES; ++i ) {
                             ps = ( *titerator )->showcases[ i ].top();
                             if ( ps == cc   ) 
                             {
                                 boost::shared_ptr<BasicServerCommand>cmd(new UpdateShowCaseTC( i , ps->items) );
                                 (*titerator)->Connection->addCommand( cmd );
                             }
                      }
               }
        }
}

//! close the showcase for everyone except the person who took it...
void World::closeShowcaseForOthers( Player* target, Container* moved ) {
        if ( moved != NULL ) {
               PLAYERVECTOR::iterator titerator;

               for ( titerator = Players.begin(); titerator < Players.end(); ++titerator ) {
                      if (target == *titerator)
                             continue;

                      for ( MAXCOUNTTYPE i = 0; i < MAXSHOWCASES; ++i )
                      {
                             if ( ( *titerator )->showcases[ i ].contains( moved ) ) 
                             {
                                    ( *titerator )->showcases[ i ].clear();
                                    boost::shared_ptr<BasicServerCommand>cmd(new ClearShowCaseTC( i ) );
                                    ( *titerator )->Connection->addCommand( cmd );
                             }
                      }
               }
        }
}

void World::closeShowcaseIfNotInRange( Container* moved, short int x, short int y, short int z ) {
        if ( moved != NULL ) {
               PLAYERVECTOR::iterator titerator;

               for ( titerator = Players.begin(); titerator < Players.end(); ++titerator ) {
                      if (abs(x-(*titerator)->pos.x) <= 1 && abs(y-(*titerator)->pos.y) <= 1 && z == (*titerator)->pos.z)
                             continue;
                      for ( MAXCOUNTTYPE i = 0; i < MAXSHOWCASES; ++i ) {
                             if ( ( *titerator )->showcases[ i ].contains( moved ) ) {
                                    ( *titerator )->showcases[ i ].clear();
                                     boost::shared_ptr<BasicServerCommand>cmd(new ClearShowCaseTC( i ) );
                                    ( *titerator )->Connection->addCommand( cmd );
                             }
                      }
               }
        }
}

bool World::isStackable(Item item) 
{
    CommonStruct com;
    //return false for not finished items
    if ( item.quality < 100 ) return false;
    if ( CommonItems->find(item.id, com) ) 
    {
        return com.isStackable;
    } 
    else 
    {
        std::cerr<<"Item mit folgender Id konnte bei isStackable(Item item) nicht gefunden werden: "<<item.id<<"!"<<std::endl;
        return false;
    }
}
