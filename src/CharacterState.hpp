/**
 * Provides the Character Selector State for Traps and Treasures
 *
 * @file src/CharacterState.hpp
 * @author Ryan Lindeman
 * @date 20120712 - Initial Release
 */

#ifndef   CHARACTER_STATE_HPP_INCLUDED
#define   CHARACTER_STATE_HPP_INCLUDED

#include <SFML/Graphics.hpp>
#include <GQE/Core/Core_types.hpp>
#include <GQE/Core/interfaces/IState.hpp>
#include <GQE/Entity/systems/AnimationSystem.hpp>
#include <GQE/Entity/systems/RenderSystem.hpp>
#include <GQE/Entity/classes/Prototype.hpp>

class CharacterState : public GQE::IState
{
  public:
    /**
     * CharacterState constructor
     * @param[in] theApp is an address to the App class.
     * @param[in] theCharacter is the IEntity for the current player
     */
    CharacterState(GQE::IApp& theApp);

    /**
     * CharacterState deconstructor
     */
    virtual ~CharacterState(void);

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
     * HandleEvents is responsible for handling input events for this
     * State when it is the active State.
     * @param[in] theEvent to process from the App class Loop method
     */
    virtual void HandleEvents(sf::Event theEvent);

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
    static const unsigned int UP_OFFSET    = 0; // First row in player image is moving up
    static const unsigned int LEFT_OFFSET  = 1; // Second row in player image is moving left
    static const unsigned int DOWN_OFFSET  = 2; // Third row in player image is moving down
    static const unsigned int RIGHT_OFFSET = 3; // Forth row in player image is moving right

    /// Constant representing the number of images to load
    static const unsigned int MAX_CHARACTERS = 50;

    /// The animation system for our players and treasures
    GQE::AnimationSystem mAnimationSystem;
    /// The render system for handling rendering of tiles, players, etc
    GQE::RenderSystem    mRenderSystem;
    /// The prototype system for creating players
    GQE::Prototype       mPlayer;
    /// The instance created using the prototype above
    GQE::Instance*       mCharacter;
#if (SFML_VERSION_MAJOR < 2)
    sf::Image            mCharacterImage;
#else
    sf::Texture          mCharacterImage;
#endif
    /// Keep track of which character image is being tested now
    unsigned int         mCurrentImage;
    /// This is the number of characters found to choose from
    unsigned int         mMaxCharacterImages;
    /// The background image giving instructions on selecting a character
    GQE::ImageAsset      mBackground;  
}; // class CharacterState

#endif // CHARACTER_STATE_HPP_INCLUDED

/**
 * @class CharacterState
 * @ingroup Examples
 * The CharacterState class provides the player a chance to choose which
 * character image they wish to use during the game.
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
