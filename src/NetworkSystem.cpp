/**
 * Provides the NetworkSystem class for handing network control of all IEntity
 * classes that are registered.
 *
 * @file src/NetworkSystem.cpp
 * @author Ryan Lindeman
 * @date 20120712 - Initial Release
 */
#include "NetworkSystem.hpp"
#include <SFML/Network.hpp>
#include <GQE/Entity/classes/Instance.hpp>

NetworkSystem::NetworkSystem(GQE::IApp& theApp):
  ISystem("NetworkSystem",theApp)
{
}

NetworkSystem::~NetworkSystem()
{
}

void NetworkSystem::AddProperties(GQE::IEntity* theEntity)
{
  theEntity->mProperties.Add<bool>("bNetworkLocal", false);
  theEntity->mProperties.Add<GQE::Uint32>("uNetworkID", 0);
  theEntity->mProperties.Add<sf::IpAddress>("sNetworkAddr", sf::IpAddress(sf::IpAddress::LocalHost));
  theEntity->mProperties.Add<unsigned short>("uNetworkPort", 0);
  theEntity->mProperties.Add<GQE::Uint32>("uKeyState", 0);
  theEntity->mProperties.Add<bool>("bKeyState", false);
}

void NetworkSystem::HandleInit(GQE::IEntity* theEntity)
{
}

void NetworkSystem::HandleEvents(sf::Event theEvent)
{
}

void NetworkSystem::UpdateFixed()
{
  GQE::Uint32 anClientID = 0;
  std::string anClientAddr;
  unsigned short anClientPort = 0;
  GQE::Uint32 anKeyState = 0;

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

      // Are we a local player who needs to send our keyboard state?
      if(anEntity->mProperties.Get<bool>("bNetworkLocal"))
      {
        // Process this entity as a local entity (send its uKeyState property)
        ProcessNetworkLocal(anEntity);
        // Get our local ID information to send to the other players
        anClientID = anEntity->mProperties.Get<GQE::Uint32>("uNetworkID");
        // Get our local IP Address to send to other players
        anClientAddr = anEntity->mProperties.Get<sf::IpAddress>("sNetworkAddr").toString();
        // Get our local port to send to other players
        anClientPort = anEntity->mProperties.Get<unsigned short>("uNetworkPort");
        // Get our keystate information to send to the other players
        anKeyState = anEntity->mProperties.Get<GQE::Uint32>("uKeyState");
      }
      else
      {
        // Packet for sending our local players KeyState information to this network player
        sf::Packet anData;

        // Process this entity as a network entity
        ProcessNetworkInput(anEntity);

        // Prepare the packet using the information contained in this IEntity
        anData << anClientID;
        anData << anClientAddr;
        anData << anClientPort;
        anData << anKeyState;

        // Now send our local players keystate to this network client
        mClient.send(anData, anEntity->mProperties.Get<sf::IpAddress>("sNetworkAddr"),
          anEntity->mProperties.Get<unsigned short>("uNetworkPort"));
      }
    } // while(anQueue != anIter->second.end())

    // Increment map iterator
    anIter++;
  } //while(anIter != mEntities.end())
}

void NetworkSystem::UpdateVariable(float theElaspedTime)
{
}

void NetworkSystem::Draw()
{
}

void NetworkSystem::HandleCleanup(GQE::IEntity* theEntity)
{
}

void NetworkSystem::ProcessNetworkLocal(GQE::IEntity* theEntity)
{
  GQE::Uint32 anKeyState = theEntity->mProperties.Get<GQE::Uint32>("uKeyState");

  // See if our client socket has been opened yet, if not bind the port
  sf::Socket::Status anResult = mClient.bind(theEntity->mProperties.Get<unsigned short>("uNetworkPort"));
  if(anResult == sf::Socket::Done)
  {
    mClient.setBlocking(false);
  }

  // Attempt to receive packets first for other players
  do {
    sf::Packet anData;
    sf::IpAddress anRemoteAddr;
    unsigned short anRemotePort;
    anResult = mClient.receive(anData, anRemoteAddr, anRemotePort);
    // Process packet if one was received
    if(anResult == sf::Socket::Done)
    {
      GQE::Uint32 anClientID;
      std::string anClientAddr;
      unsigned short anClientPort;
      GQE::Uint32 anClientKeyState;
      anData >> anClientID;
      anData >> anClientAddr;
      anData >> anClientPort;
      anData >> anClientKeyState;
      
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

          // Is this the player we just received a packet for? then update its keystate information
          if(anEntity->mProperties.Get<GQE::Uint32>("uNetworkID") == anClientID)
          {
            anEntity->mProperties.Set<GQE::Uint32>("uKeyState", anClientKeyState);
            anEntity->mProperties.Set<bool>("bKeyState", true);
          }
        } // while(anQueue != anIter->second.end())

        // Increment map iterator
        anIter++;
      } //while(anIter != mEntities.end())
    }
  } while(anResult == sf::Socket::Done);
}

void NetworkSystem::ProcessNetworkInput(GQE::IEntity* theEntity)
{
  if(theEntity->mProperties.Get<bool>("bKeyState"))
  {
    // Get the current KeyState information and process it now
    GQE::Uint32 anKeyState = theEntity->mProperties.Get<GQE::Uint32>("uKeyState");

    // Get the RenderSystem properties we use in ControlSystem
    sf::IntRect anSpriteRect = theEntity->mProperties.Get<sf::IntRect>("rSpriteRect");

    // Get the current control system properties from this IEntity
    float anSpeed = theEntity->mProperties.Get<float>("fSpeed");

    // Create a velocity vector to fill as we process keyboard input
    sf::Vector2f anVelocity(0.0f, 0.0f);

    // Check the network keyboard information to compute the new velocity value
    if((anKeyState & KEY_LEFT) == KEY_LEFT)
    {
      anVelocity.x = -anSpeed;

#if SFML_VERSION_MAJOR<2
      // GetHeight changes as we change top and bottom, so cache its value first
      int anHeight = anSpriteRect.GetHeight();

      // Compute a new height
      anSpriteRect.Top = anHeight*LEFT_OFFSET;
      anSpriteRect.Bottom = anHeight*(LEFT_OFFSET+1);
#else
      anSpriteRect.top = anSpriteRect.height*LEFT_OFFSET;
#endif
    }
    else if((anKeyState & KEY_RIGHT) == KEY_RIGHT)
    {
      anVelocity.x = anSpeed;

#if SFML_VERSION_MAJOR<2
      // GetHeight changes as we change top and bottom, so cache its value first
      int anHeight = anSpriteRect.GetHeight();

      // Compute a new height
      anSpriteRect.Top = anHeight*RIGHT_OFFSET;
      anSpriteRect.Bottom = anHeight*(RIGHT_OFFSET+1);
#else
      anSpriteRect.top = anSpriteRect.height*RIGHT_OFFSET;
#endif
    }
    if((anKeyState & KEY_UP) == KEY_UP)
    {
      anVelocity.y = -anSpeed;

#if SFML_VERSION_MAJOR<2
      // GetHeight changes as we change top and bottom, so cache its value first
      int anHeight = anSpriteRect.GetHeight();

      // Compute a new height
      anSpriteRect.Top = anHeight*UP_OFFSET;
      anSpriteRect.Bottom = anHeight*(UP_OFFSET+1);
#else
      anSpriteRect.top = anSpriteRect.height*UP_OFFSET;
#endif
    }
    else if((anKeyState & KEY_DOWN) == KEY_DOWN)
    {
      anVelocity.y = anSpeed;

#if SFML_VERSION_MAJOR<2
      // GetHeight changes as we change top and bottom, so cache its value first
      int anHeight = anSpriteRect.GetHeight();

      // Compute a new height
      anSpriteRect.Top = anHeight*DOWN_OFFSET;
      anSpriteRect.Bottom = anHeight*(DOWN_OFFSET+1);
#else
      anSpriteRect.top = anSpriteRect.height*DOWN_OFFSET;
#endif
    }

    // Now update the control system properties for this IEntity
    theEntity->mProperties.Set<sf::Vector2f>("vVelocity", anVelocity);
    theEntity->mProperties.Set<sf::IntRect>("rSpriteRect", anSpriteRect);
    // Keystate was processed and is no longer valid
    theEntity->mProperties.Set<bool>("bKeyState", false);
  }
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
