/**
 * Provides the ControlSystem class for handing all IEntity controls in a game.
 *
 * @file src/ControlSystem.hpp
 * @author Ryan Lindeman
 * @date 20120712 - Initial Release
 */
#ifndef CONTROL_SYSTEM_HPP_INCLUDED
#define CONTROL_SYSTEM_HPP_INCLUDED
#include <GQE/Entity/interfaces/ISystem.hpp>
#include <GQE/Entity/classes/Prototype.hpp>

class ControlSystem : public GQE::ISystem
{
  public:
    ControlSystem(GQE::IApp& theApp);

    virtual ~ControlSystem();

    /**
     * RegisterPrototype is responsible for adding the properties to a prototype.
     * @param[in] thePrototype is the prototype to use.
     */
    virtual void AddProperties(GQE::IEntity* theEntity);

    /**
     * HandleEvents is responsible for letting each Instance class have a
     * chance to handle theEvent specified.
     * @param[in] theEvent to handle
     */
    virtual void HandleEvents(sf::Event theEvent);

    /**
     * UpdateFixed is called a specific number of times every game loop and
     * this method will allow each Instance class a chance to have its
     * UpdateFixed method called for each game loop iteration.
     */
    virtual void UpdateFixed(void);

    /**
     * UpdateVariable is called every time the game loop draws a frame and
     * includes the elapsed time between the last UpdateVariable call for
     * use with equations that use time as a variable. (e.g. physics velocity
     * and acceleration equations).
     */
    virtual void UpdateVariable(float theElapsedTime);

    /**
     * Draw is called during the game loop after events and the fixed update
     * loop calls are completed and depends largely on the speed of the
     * computer to determine how frequently it will be called. This gives the
     * EntityManager a chance to call the Draw method for each Instance
     * class.
     */
    virtual void Draw(void);
  protected:
    /**
     * HandleInit is called to allow each derived ISystem to perform any
     * initialization steps when a new IEntity is added.
     */
    virtual void HandleInit(GQE::IEntity* theEntity);

    /**
     * HandleCleanup is called when the IEntity that was added is finally
     * dropped from this ISystem and gives the derived ISystem class a chance
     * to perform any custom work before the IEntity is deleted.
     */
    virtual void HandleCleanup(GQE::IEntity* theEntity);
  private:
    static const unsigned int UP_OFFSET    = 0; // First row in player image is moving up
    static const unsigned int LEFT_OFFSET  = 1; // Second row in player image is moving left
    static const unsigned int DOWN_OFFSET  = 2; // Third row in player image is moving down
    static const unsigned int RIGHT_OFFSET = 3; // Forth row in player image is moving right
    static const unsigned int KEY_UP    = 0x00000001; // Up key is being pressed
    static const unsigned int KEY_LEFT  = 0x00000002; // Left key is being pressed
    static const unsigned int KEY_DOWN  = 0x00000004; // Down key is being pressed
    static const unsigned int KEY_RIGHT = 0x00000008; // Right key is being pressed
    static const unsigned int KEY_SPACE = 0x00000010; // Spacebar key is being pressed
    static const unsigned int KEY_ENTER = 0x00000020; // Enter key is being pressed
};
#endif // CONTROL_SYSTEM_HPP_INCLUDED

/**
 * @class GQE::ControlSystem
 * The ControlSystem class is used to cause all IEntity classes registered to
 * have their velocity information updated according to the keyboard keys that
 * are currently pressed. The properties provided by this ISystem are as follows:
 * - fSpeed: The float which determines the speed of the player
 * - uKeyState: The GQE::Uint32 value that represents the keys being pressed
 * The ControlSystem class makes use of the following properties provided by the
 * RenderSystem class:
 * - bSpriteRect: The sf::IntRect currently being shown
 * The ControlSystem class makes use of the following properties provided by the
 * MovementSystem class:
 * - vVelocity: The sf::vector2f representing the speed of the player
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
