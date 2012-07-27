/**
 * Provides the Tmx Asset type used by the AssetManager for managing TMX files
 * created in the open source Tiled map editor program.
 *
 * @file src/TmxAsset.cpp
 * @author Ryan Lindeman
 * @date 20120712 - Initial Release
 */
#include <assert.h>
#include <stddef.h>
#include "TmxAsset.hpp"
#include <GQE/Core/loggers/Log_macros.hpp>

TmxAsset::TmxAsset(const GQE::typeAssetID theAssetID,
    GQE::AssetLoadTime theLoadTime, GQE::AssetLoadStyle theLoadStyle,
    GQE::AssetDropTime theDropTime) :
  GQE::TAsset<Tmx::Map>(theAssetID, theLoadTime, theLoadStyle, theDropTime)
{
}

TmxAsset::~TmxAsset()
{
}

/**
 * @section LICENSE
 * Traps and Treasures, a multiplayer action adventure game for the LPC contest
 * Copyright (C) 2012  Ryan Lindeman, Jacob Dix, David Cannon
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
