/**
 * Provides the Tmx Asset type used by the AssetManager for managing TMX files
 * created in the open source Tiled map editor program.
 *
 * @file src/TmxAsset.hpp
 * @author Ryan Lindeman
 * @date 20120712 - Initial Release
 */
#ifndef TMX_ASSET_HPP_INCLUDED
#define TMX_ASSET_HPP_INCLUDED
#include <GQE/Core/interfaces/TAsset.hpp>
#include <TmxParser/Tmx.h>

/// Provides the Image asset class
class TmxAsset : public GQE::TAsset<Tmx::Map>
{
  public:
    /**
     * TmxAsset default constructor is used when you don't know the asset
     * filename until later.
     */
    TmxAsset();

    /**
     * TmxAsset constructor
     * @param[in] theAssetID to uniquely identify this asset
     * @param[in] theLoadTime (Now, Later) of when to load this asset
     * @param[in] theLoadStyle (File, Mem, Network) to use when loading this asset
     * @param[in] theDropTime at (Zero, Exit) for when to unload this asset
     */
    TmxAsset(const GQE::typeAssetID theAssetID,
        GQE::AssetLoadTime theLoadTime = GQE::AssetLoadLater,
        GQE::AssetLoadStyle theLoadStyle = GQE::AssetLoadFromFile,
        GQE::AssetDropTime theDropTime = GQE::AssetDropAtZero);

    /**
     * TmxAsset deconstructor
     */
    virtual ~TmxAsset();

  protected:

  private:
    // Variables
    ///////////////////////////////////////////////////////////////////////////
}; // class TmxAsset
#endif // TMX_ASSET_HPP_INCLUDED
/**
 * @class TmxHandler
 * @ingroup Examples
 * @section DESCRIPTION
 * The TmxAsset class is responsible for loading a TMX map file created by the
 * Tiled map editor.
 *
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
