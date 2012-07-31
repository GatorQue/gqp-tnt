/**
 * Provides the Game State for Traps and Treasures
 *
 * @file src/GameState.hpp
 * @author Ryan Lindeman
 * @date 20120712 - Initial Release
 * @date 20120728 - Game Control fixes needed for multiplayer to work correctly
 * @date 20120730 - Improved network synchronization for multiplayer game play
 */

#ifndef   GAME_STATE_HPP_INCLUDED
#define   GAME_STATE_HPP_INCLUDED
#include <GQE/Core/Core_types.hpp>
#include <GQE/Core/interfaces/IState.hpp>
#include <SFML/Graphics.hpp>
#include <GQE/Entity/systems/AnimationSystem.hpp>
#include <GQE/Entity/systems/MovementSystem.hpp>
#include <GQE/Entity/systems/RenderSystem.hpp>
#include "LevelSystem.hpp"
#include "NetworkSystem.hpp"

// Forward declare the TnTApp class
class TnTApp;

class GameState : public GQE::IState
{
  public:
    /**
     * GameState constructor
     * @param[in] theApp is an address to the App class.
     */
    GameState(TnTApp& theApp);

    /**
     * GameState deconstructor
     */
    virtual ~GameState(void);

    /**
     * DoInit is responsible for initializing this State
     */
    virtual void DoInit(void);

    /**
     * ReInit is responsible for Reseting this state when the
     * StateManager::ResetActiveState() method is called.  This way a Game
     * State can be restarted without unloading and reloading the game assets
     */
    virtual void ReInit(void);

    /**
     * UpdateFixed is responsible for handling all State fixed update needs for
     * this State when it is the active State.
     */
    virtual void UpdateFixed(void);

    /**
     * UpdateVariable is responsible for handling all State variable update
     * needs for this State when it is the active State.
     * @param[in] theElapsedTime since the last Draw was called
     */
    virtual void UpdateVariable(float theElapsedTime);

    /**
     * Draw is responsible for handling all Drawing needs for this State
     * when it is the Active State.
     */
    virtual void Draw(void);

  protected:
    /**
     * HandleCleanup is responsible for performing any cleanup required
     * before this State is removed.
     */
    virtual void HandleCleanup(void);
  private:
    /// The animation system for our players and treasures
    GQE::AnimationSystem mAnimationSystem;
    /// The level system for loading our map level (must come after mRenderSystem)
    LevelSystem          mLevelSystem;
    /// The network system for managing network input/output
    NetworkSystem        mNetworkSystem;
    /// The prototype system for creating players
    GQE::Prototype       mPlayer;
    /// The player ID of the current player
    GQE::Uint32          mPlayerID;
    /// The image to use for the current player
    GQE::ImageAsset*     mPlayerImages;
}; // class GameState

#endif // GAME_STATE_HPP_INCLUDED

/**
 * @class GameState
 * @ingroup Examples
 * The EntityTest GameState class provides the EntityTest example game
 * implementation for the GQE library.
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
