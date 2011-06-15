#ifndef __playersave_hh
#define __playersave_hh

#include "tvector.hpp"
#include <map>
#include <string>

class CPlayer;

extern tvector<CPlayer*> players_to_save;
extern std::map<std::string, int> players_tosave_map;

bool start_savethread();
void stop_savethread();

#endif
