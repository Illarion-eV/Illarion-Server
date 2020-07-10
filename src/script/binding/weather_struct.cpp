/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "script/binding/binding.hpp"

#include <World.hpp>

namespace binding {

auto weather_struct() -> Binding<WeatherStruct> {
    return luabind::class_<WeatherStruct>("WeatherStruct")
            .def_readwrite("cloud_density", &WeatherStruct::cloud_density)
            .def_readwrite("fog_density", &WeatherStruct::fog_density)
            .def_readwrite("wind_dir", &WeatherStruct::wind_dir)
            .def_readwrite("gust_strength", &WeatherStruct::gust_strength)
            .def_readwrite("percipitation_strength", &WeatherStruct::percipitation_strength)
            .def_readwrite("percipitation_type", &WeatherStruct::per_type)
            .def_readwrite("thunderstorm", &WeatherStruct::thunderstorm)
            .def_readwrite("temperature", &WeatherStruct::temperature);
}

} // namespace binding
