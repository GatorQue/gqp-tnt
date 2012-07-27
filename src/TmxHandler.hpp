/**
 * Provides the handling of the TmxAsset classes for the AssetManager in the
 * GQE namespace. The AssetManager is responsible for providing the Asset
 * management facilities for the App base class used in the GQE core library.
 *
 * @file src/TmxHandler.hpp
 * @author Ryan Lindeman
 * @date 20120712 - Initial Release
 */
#ifndef   TMX_HANDLER_HPP_INCLUDED
#define   TMX_HANDLER_HPP_INCLUDED

#include <SFML/Graphics.hpp>
#include <GQE/Core/Core_types.hpp>
#include <GQE/Core/interfaces/TAssetHandler.hpp>
#include "TmxAsset.hpp"

/// Provides the TmxHandler class.
class TmxHandler : public GQE::TAssetHandler<Tmx::Map>
{
  public:
    /**
     * TmxHandler constructor
     */
    TmxHandler();

    /**
     * TmxHandler deconstructor
     */
    virtual ~TmxHandler();

  protected:
    /**
     * LoadFromFile is responsible for loading theAsset from a file and must
     * be defined by the derived class since the interface for TYPE is
     * unknown at this stage.
     * @param[in] theAssetID of the asset to be loaded
     * @param[in] theAsset pointer to load
     * @return true if the asset was successfully loaded, false otherwise
     */

    virtual bool LoadFromFile(const GQE::typeAssetID theAssetID,Tmx::Map& theMap);

    /**
     * LoadFromMemory is responsible for loading theAsset from memory and
     * must be defined by the derived class since the interface for TYPE is
     * unknown at this stage.
     * @param[in] theAssetID of the asset to be loaded
     * @param[in] theAsset pointer to load
     * @return true if the asset was successfully loaded, false otherwise
     */
    virtual bool LoadFromMemory(const GQE::typeAssetID theAssetID, Tmx::Map& theMap);

    /**
     * LoadFromNetwork is responsible for loading theAsset from network and
     * must be defined by the derived class since the interface for TYPE is
     * unknown at this stage.
     * @param[in] theAssetID of the asset to be loaded
     * @param[in] theAsset pointer to load
     * @return true if the asset was successfully loaded, false otherwise
     */
    virtual bool LoadFromNetwork(const GQE::typeAssetID theAssetID,Tmx::Map& theMap);

  private:

}; // class TmxHandler
#endif // TMX_HANDLER_HPP_INCLUDED
/**
 * @class TmxHandler
 * @ingroup Examples
 * @section DESCRIPTION
 * The TmxHandler class is responsible for managing all TmxAsset classes.
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
