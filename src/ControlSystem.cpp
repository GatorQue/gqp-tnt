/**
 * Provides the ControlSystem class for handing all IEntity controls in a game.
 *
 * @file src/ControlSystem.cpp
 * @author Ryan Lindeman
 * @date 20120712 - Initial Release
 */
#include "ControlSystem.hpp"
#include <GQE/Entity/classes/Instance.hpp>

ControlSystem::ControlSystem(GQE::IApp& theApp):
  ISystem("ControlSystem",theApp)
{
}

ControlSystem::~ControlSystem()
{
}

void ControlSystem::AddProperties(GQE::IEntity* theEntity)
{
  theEntity->mProperties.Add<float>("fSpeed", 4.0f);
  theEntity->mProperties.Add<GQE::Uint32>("uKeyState", 0);
}

void ControlSystem::HandleInit(GQE::IEntity* theEntity)
{
}

void ControlSystem::HandleEvents(sf::Event theEvent)
{
}

void ControlSystem::UpdateFixed()
{
  // Search through each z-order map to find theEntityID provided
  std::map<const GQE::Uint32, std::deque<GQE::IEntity*> >::iterator anIter;
  anIter = mEntities.begin();
  while(anIter != mEntities.end())
  {
    std::deque<GQE::IEntity*>::iterator anQueue = anIter->second.begin();
    while(anQueue != anIter->second.end())
    {
      // Get the IEntity address first
      GQE::IEntity* anEntity = *anQueue;

      // Increment the IEntity iterator second
      anQueue++;

      // Get the RenderSystem properties we use in ControlSystem
      sf::IntRect anSpriteRect = anEntity->mProperties.Get<sf::IntRect>("rSpriteRect");

      // Get the current control system properties from this IEntity
      float anSpeed = anEntity->mProperties.Get<float>("fSpeed");

      // Create a velocity vector to fill as we process keyboard input
      sf::Vector2f anVelocity(0.0f, 0.0f);

      // Start with no keys being pressed
      GQE::Uint32 anKeyState = 0;

      // Check the keyboard to compute the new velocity value
#if SFML_VERSION_MAJOR<2
      if(mApp.mInput.IsKeyDown(sf::Key::Left))
      {
        anKeyState |= KEY_LEFT;
        anVelocity.x = -anSpeed;

        // GetHeight changes as we change top and bottom, so cache its value first
        int anHeight = anSpriteRect.GetHeight();

        // Compute a new height
        anSpriteRect.Top = anHeight*LEFT_OFFSET;
        anSpriteRect.Bottom = anHeight*(LEFT_OFFSET+1);
      }
      else if(mApp.mInput.IsKeyDown(sf::Key::Right))
      {
        anKeyState |= KEY_RIGHT;
        anVelocity.x = anSpeed;
        // GetHeight changes as we change top and bottom, so cache its value first
        int anHeight = anSpriteRect.GetHeight();
        anSpriteRect.Top = anHeight*RIGHT_OFFSET;
        anSpriteRect.Bottom = anHeight*(RIGHT_OFFSET+1);
      }
      if(mApp.mInput.IsKeyDown(sf::Key::Up))
      {
        anKeyState |= KEY_UP;
        anVelocity.y = -anSpeed;
        // GetHeight changes as we change top and bottom, so cache its value first
        int anHeight = anSpriteRect.GetHeight();
        anSpriteRect.Top = anHeight*UP_OFFSET;
        anSpriteRect.Bottom = anHeight*(UP_OFFSET+1);
      }
      else if(mApp.mInput.IsKeyDown(sf::Key::Down))
      {
        anKeyState |= KEY_DOWN;
        anVelocity.y = anSpeed;
        // GetHeight changes as we change top and bottom, so cache its value first
        int anHeight = anSpriteRect.GetHeight();
        anSpriteRect.Top = anHeight*DOWN_OFFSET;
        anSpriteRect.Bottom = anHeight*(DOWN_OFFSET+1);
      }
#else
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
      {
        anKeyState |= KEY_LEFT;
        anVelocity.x = -anSpeed;
        anSpriteRect.top = anSpriteRect.height*LEFT_OFFSET;
      }
      else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
      {
        anKeyState |= KEY_RIGHT;
        anVelocity.x = anSpeed;
        anSpriteRect.top = anSpriteRect.height*RIGHT_OFFSET;
      }
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
      {
        anKeyState |= KEY_UP;
        anVelocity.y = -anSpeed;
        anSpriteRect.top = anSpriteRect.height*UP_OFFSET;
      }
      else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
      {
        anKeyState |= KEY_DOWN;
        anVelocity.y = anSpeed;
        anSpriteRect.top = anSpriteRect.height*DOWN_OFFSET;
      }
#endif

      // Now update the control system properties for this IEntity
      anEntity->mProperties.Set<sf::Vector2f>("vVelocity", anVelocity);
      anEntity->mProperties.Set<sf::IntRect>("rSpriteRect", anSpriteRect);
      anEntity->mProperties.Set<GQE::Uint32>("uKeyState", anKeyState);
    } // while(anQueue != anIter->second.end())

    // Increment map iterator
    anIter++;
  } //while(anIter != mEntities.end())
}

void ControlSystem::UpdateVariable(float theElaspedTime)
{
}

void ControlSystem::Draw()
{
}

void ControlSystem::HandleCleanup(GQE::IEntity* theEntity)
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
