/**
 * Provides the TnTApp class which implements the Traps and Treasures example
 * game for the GQP/GQE projects.
 *
 * @file src/TnTApp.cpp
 * @author Ryan Lindeman
 * @date 20110704 - Initial Release
 */
#include "TntApp.hpp"
#include "CharacterState.hpp"
#include "GameState.hpp"
#include "NetworkState.hpp"
#include "TmxHandler.hpp"

TnTApp::TnTApp(const std::string theTitle) :
  GQE::IApp(theTitle)
{
}

TnTApp::~TnTApp()
{
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
