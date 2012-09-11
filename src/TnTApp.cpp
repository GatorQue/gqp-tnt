/**
 * Provides the TnTApp class which implements the Traps and Treasures example
 * game for the GQP/GQE projects.
 *
 * @file src/TnTApp.cpp
 * @author Ryan Lindeman
 * @date 20110704 - Initial Release
 * @date 20120730 - Improved network synchronization for multiplayer game play
 * @date 20120910 - Fix SFML v1.6 issues
 */
#include "TnTApp.hpp"
#include "CharacterState.hpp"
#include "GameState.hpp"
#include "NetworkState.hpp"
#include "TmxHandler.hpp"

TnTApp::TnTApp(const std::string theTitle) :
  GQE::IApp(theTitle),
  mClientID(0)
{
#if (SFML_VERSION_MAJOR < 2)
  // Bind our game client socket to random port provided
  bool anStatus = mClient.Bind(0);
#else
  // Bind our game client socket to random port provided
  sf::Socket::Status anStatus = mClient.bind(sf::Socket::AnyPort);
#endif

  // Make sure we succeeded to bind our client socket
#if (SFML_VERSION_MAJOR < 2)
  if(anStatus == false)
#else
  if(anStatus == sf::Socket::Error)
#endif
  {
    // Signal the application to exit
    Quit(GQE::StatusError);
  }
  else
  {
#if (SFML_VERSION_MAJOR < 2)
    // Set our client as non-blocking
    mClient.SetBlocking(false);
#else
    // Set our client as non-blocking
    mClient.setBlocking(false);
#endif
  }

  // Use time to seed our randomizer
  srand((unsigned int)time(NULL));

  // Use the first random number as our client ID
  mClientID = rand();
}

TnTApp::~TnTApp()
{
#if (SFML_VERSION_MAJOR < 2)
  // Unbind our local client socket
  mClient.Unbind();
#else
  // Unbind our local client socket
  mClient.unbind();
#endif
}

void TnTApp::InitAssetHandlers()
{
  mAssetManager.RegisterHandler(new(std::nothrow) TmxHandler());
}

void TnTApp::InitScreenFactory()
{
  mStateManager.AddInactiveState(new(std::nothrow) NetworkState(*this));
  mStateManager.AddInactiveState(new(std::nothrow) GameState(*this));
  mStateManager.AddActiveState(new(std::nothrow) CharacterState(*this));
}

void TnTApp::HandleCleanup()
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
