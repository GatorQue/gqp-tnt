/**
 * Provides the handling of the TmxAsset classes for the AssetManager in the
 * GQE namespace. The AssetManager is responsible for providing the Asset
 * management facilities for the App base class used in the GQE core library.
 *
 * @file src/TmxHandler.cpp
 * @author Ryan Lindeman
 * @date 20120712 - Initial Release
 */
#include "TmxHandler.hpp"
#include <GQE/Core/loggers/Log_macros.hpp>
#include <GQE/Entity/classes/Instance.hpp>
#include <GQE/Entity/classes/Prototype.hpp>

TmxHandler::TmxHandler() :
  GQE::TAssetHandler<Tmx::Map>()
{
  ILOG() << "TmxHandler::ctor()" << std::endl;
}

TmxHandler::~TmxHandler()
{
  ILOG() << "TmxHandler::dtor()" << std::endl;
}

bool TmxHandler::LoadFromFile(const GQE::typeAssetID theAssetID, Tmx::Map& theMap)
{
  // Start with a return result of false
  bool anResult = false;

  // Retrieve the filename for this asset
  std::string anFilename = GetFilename(theAssetID);
  // Was a valid filename found? then attempt to load the asset from anFilename
  if(anFilename.length() > 0)
  {
    theMap.ParseFile(anFilename);
    if(!theMap.HasError())
    {
      anResult=true;
    }
    else
    {
      ILOG() << "Error Loading Tmx File. Error Code: "<<theMap.GetErrorCode()<<std::endl;
    }

  }
  else
  {
    ELOG() << "TmxHandler::LoadFromFile(" << theAssetID
      << ") No filename provided!" << std::endl;
  }

  // Return anResult of true if successful, false otherwise
  return anResult;
}

bool TmxHandler::LoadFromMemory(const GQE::typeAssetID theAssetID,Tmx::Map& theMap)
{
  // Start with a return result of false
  bool anResult = false;

  // TODO: Retrieve the const char* pointer to load data from
  const char* anData = NULL;
  // TODO: Retrieve the size in bytes of the font to load from memory
  size_t anDataSize = 0;

  // Try to obtain the font from the memory location specified
  if(NULL != anData && anDataSize > 0)
  {

  }
  else
  {
    ELOG() << "TmxHandler::LoadFromMemory(" << theAssetID
      << ") Bad memory location or size!" << std::endl;
  }

  // Return anResult of true if successful, false otherwise
  return anResult;
}

bool TmxHandler::LoadFromNetwork(const GQE::typeAssetID theAssetID, Tmx::Map& theMap)
{
  // Start with a return result of false
  bool anResult = false;

  // TODO: Add load from network for this asset

  // Return anResult of true if successful, false otherwise
  return anResult;
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
