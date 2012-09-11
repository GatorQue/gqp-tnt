/**
 * Provides the NetworkSystem class for handing network control of all IEntity
 * classes that are registered.
 *
 * @file src/NetworkSystem.cpp
 * @author Ryan Lindeman
 * @date 20120712 - Initial Release
 * @date 20120730 - Improved network synchronization for multiplayer game play
 * @date 20120731 - Add sound effects and player spawn points
 * @date 20120910 - Fix SFML v1.6 issues
 */
#include "NetworkSystem.hpp"
#include <SFML/Network.hpp>
#include <GQE/Entity/classes/Instance.hpp>
#include "TnTApp.hpp"

NetworkSystem::NetworkSystem(TnTApp& theApp):
  ISystem("NetworkSystem", theApp),
  mUpdateStep(ActionWait),
  mGameTick(0),
  mClient(theApp.mClient)
{
}

NetworkSystem::~NetworkSystem()
{
}

void NetworkSystem::AddProperties(GQE::IEntity* theEntity)
{
  theEntity->mProperties.Add<bool>("bNetworkLocal", false);
  theEntity->mProperties.Add<GQE::Uint32>("uNetworkID", 0);
#if (SFML_VERSION_MAJOR < 2)
  theEntity->mProperties.Add<sf::IPAddress>("sNetworkAddr", sf::IPAddress(sf::IPAddress::LocalHost));
#else
  theEntity->mProperties.Add<sf::IpAddress>("sNetworkAddr", sf::IpAddress(sf::IpAddress::LocalHost));
#endif
  theEntity->mProperties.Add<unsigned short>("uNetworkPort", 0);
  theEntity->mProperties.Add<float>("fSpeed", 8.0f);
  theEntity->mProperties.Add<GQE::Uint32>("uKeyState", 0);
  theEntity->mProperties.Add<GQE::Uint32>("uKeyStatePrevious", 0);
  theEntity->mProperties.Add<bool>("bKeyState", false);
  theEntity->mProperties.Add<sf::Vector2f>("vVelocity",sf::Vector2f(0,0));
}

void NetworkSystem::HandleInit(GQE::IEntity* theEntity)
{
}

void NetworkSystem::HandleEvents(sf::Event theEvent)
{
}

void NetworkSystem::UpdateFixed()
{
  // Did someone initiate loading a new level?
  bool anLoading = false;

  // How many players have committed keyboard state information?
  unsigned int anCount = 0;
  unsigned int anTotal = 0;

  // Iterator for looping through each IEntity class
  std::map<const GQE::Uint32, std::deque<GQE::IEntity*> >::iterator anIter;

  // Which step are we on right now for UpdateFixed?
  switch(mUpdateStep)
  {
  case ActionWait:
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
          // Send local input for this local player to other remote players
          SendLocalInput(anEntity);
        }
        else
        {
          // See if there is any remote input information to receive
          ReceiveRemoteInput();
        }
        
        // Has this player finished loading their level?
        if(anEntity->mProperties.Get<bool>("bLoading") == false)
        {
          // Increment our committed count number
          anCount++;
        }

        // Increment our total committed members count
        anTotal++;
      } // while(anQueue != anIter->second.end())
      // Increment map iterator
      anIter++;
    } //while(anIter != mEntities.end())

    //ILOG() << "NetworkSystem::ActionWait gt=" << mGameTick
    //  << " count=" << anCount << " total=" << anTotal << std::endl;
    // Have we received all committed members, then act on commitment
    if(anCount == anTotal)
    {
      // Switch to next step
      mUpdateStep = ActionCommit;
    }
    break;
  case ActionCommit: // Gather local keystate information
    // Increment our game tick value
    mGameTick++;

    //ILOG() << "NetworkSystem::ActionCommit gt=" << mGameTick << std::endl;
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

        // Are we a local player who needs to get our current keyboard state?
        if(anEntity->mProperties.Get<bool>("bNetworkLocal"))
        {
          // Save previous KeyState information
          anEntity->mProperties.Set<GQE::Uint32>("uKeyStatePrevious", 
            anEntity->mProperties.Get<GQE::Uint32>("uKeyState"));

          // Save previous Position information
          anEntity->mProperties.Set<sf::Vector2f>("vPositionPrevious", 
            anEntity->mProperties.Get<sf::Vector2f>("vPosition"));

          // Save previous Screen information
          anEntity->mProperties.Set<sf::Vector2u>("wScreenPrevious", 
            anEntity->mProperties.Get<sf::Vector2u>("wScreen"));

          // Save previous Loading information
          anEntity->mProperties.Set<bool>("bLoadingPrevious", 
            anEntity->mProperties.Get<bool>("bLoading"));

          // Gather input for this local player
          UpdateLocalInput(anEntity);
        }
        //ILOG() << "NetworkState::ActionCommit id=" << anEntity->GetID() << " uKeyState=" << 
        //  anEntity->mProperties.Get<GQE::Uint32>("uKeyState") << std::endl;
        //ILOG() << "NetworkState::ActionCommit position(" <<
        //  anEntity->mProperties.Get<sf::Vector2f>("vPosition").x << ", " <<
        //  anEntity->mProperties.Get<sf::Vector2f>("vPosition").y << ")" << std::endl;
        //ILOG() << "NetworkState::ActionCommit screen(" <<
        //  anEntity->mProperties.Get<sf::Vector2u>("wScreen").x << ", " <<
        //  anEntity->mProperties.Get<sf::Vector2u>("wScreen").y << ")" << std::endl;

      } // while(anQueue != anIter->second.end())
      // Increment map iterator
      anIter++;
    } //while(anIter != mEntities.end())

    // Switch to next step
    mUpdateStep = ActionBroadcast;
    break;
  case ActionBroadcast: // Send local and receive remote keystate information
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
          // Send local input for this local player to other remote players
          SendLocalInput(anEntity);
        }
        else
        {
          // See if there is any remote input information to receive
          ReceiveRemoteInput();
        }
        
        // Does this player have a committed keyboard state?
        if(anEntity->mProperties.Get<bool>("bKeyState"))
        {
          // Increment our committed count number
          anCount++;
        }

        // Has this player started loading a new level?
        if(anEntity->mProperties.Get<bool>("bLoading"))
        {
          anLoading = true;
        }

        // Increment our total committed members count
        anTotal++;
      } // while(anQueue != anIter->second.end())
      // Increment map iterator
      anIter++;
    } //while(anIter != mEntities.end())

    //ILOG() << "NetworkSystem::ActionBroadcast count=" << anCount << " total=" << anTotal << std::endl;
    // Have we received all committed members, then act on commitment
    if(anCount == anTotal && anLoading == false)
    {
      // Switch to next step
      mUpdateStep = ActionVelocity;
    }
    else if(anLoading == true)
    {
      // Switch to wait step
      mUpdateStep = ActionWait;
    }
    break;
  case ActionVelocity: // Use keystate information received to generate velocity information
    //ILOG() << "NetworkSystem::ActionVelocity gt=" << mGameTick << std::endl;
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

        // Process the input keystate information for this Entity
        ProcessInput(anEntity);
      } // while(anQueue != anIter->second.end())
      // Increment map iterator
      anIter++;
    } //while(anIter != mEntities.end())

    // Switch to next step
    mUpdateStep = ActionPosition;
    break;
  case ActionPosition: // Use velocity information sanitized by LevelSystem to move positions
    //ILOG() << "NetworkSystem::ActionPosition gt=" << mGameTick << std::endl;
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

        // Process the input keystate information for this Entity
        ProcessVelocity(anEntity);
      } // while(anQueue != anIter->second.end())
      // Increment map iterator
      anIter++;
    } //while(anIter != mEntities.end())

    // Switch to first step
    mUpdateStep = ActionCommit;
    break;
  }
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

void NetworkSystem::ProcessInput(GQE::IEntity* theEntity)
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
  else
  {
    WLOG() << "NetworkSystem::ProcessInput() missing input for id=" <<
      theEntity->mProperties.Get<GQE::Uint32>("uNetworkID") << std::endl;
  }
}

void NetworkSystem::ProcessVelocity(GQE::IEntity* theEntity)
{
  // Get the LevelSystem properties
  sf::Vector2f anPosition = theEntity->mProperties.Get<sf::Vector2f>("vPosition");

  // Get the NetworkSystem properties
  sf::Vector2f anVelocity = theEntity->mProperties.Get<sf::Vector2f>("vVelocity");

  // Now update the current movement properties
  anPosition += anVelocity;

  // Now update the RenderSystem properties of this IEntity class
  theEntity->mProperties.Set<sf::Vector2f>("vPosition", anPosition);
}

void NetworkSystem::ReceiveRemoteInput(void)
{
  sf::Socket::Status anResult = sf::Socket::Error;

  // Attempt to receive packets from remote players
  do {
    sf::Packet anData;
#if (SFML_VERSION_MAJOR < 2)
    sf::IPAddress anRemoteAddr;
#else
    sf::IpAddress anRemoteAddr;
#endif
    unsigned short anRemotePort;
  
#if (SFML_VERSION_MAJOR < 2)
    // See if a packet can be received on our client socket
    anResult = mClient.Receive(anData, anRemoteAddr, anRemotePort);
#else
    // See if a packet can be received on our client socket
    anResult = mClient.receive(anData, anRemoteAddr, anRemotePort);
#endif

    // Process packet if one was received
    if(anResult == sf::Socket::Done)
    {
      std::string anTemp;
      unsigned int anCurGameTick;
      GQE::Uint32 anID;
      std::string anAddr;
      unsigned short anPort;
      GQE::Uint32 anCurKeyState;
      sf::Vector2f anCurPosition;
      sf::Vector2u anCurScreen;
      bool anCurLoading;
      unsigned int anPrevGameTick;
      GQE::Uint32 anPrevKeyState;
      sf::Vector2f anPrevPosition;
      sf::Vector2u anPrevScreen;
      bool anPrevLoading;
      
      // First receive our current game tick value
      anData >> anCurGameTick;
      // Next receive the remote client ID value
      anData >> anID;
      // Next receive the remote client IP Address
      anData >> anAddr;
      // Next receive the remote client port
      anData >> anPort;
      // Next receive the remote client keystate information
      anData >> anCurKeyState;
      // Next receive the remote client position information
      anData >> anTemp;
      anCurPosition = GQE::ParseVector2f(anTemp, sf::Vector2f(512.0f,384.0f));
      // Next receive the remote client screen information
      anData >> anTemp;
      anCurScreen = GQE::ParseVector2u(anTemp, sf::Vector2u(0,0));
      // Next receive the remote client loading state
      anData >> anCurLoading;
      // Next receive the remote clients previous game tick value
      anData >> anPrevGameTick;
      // Next receive the remote client previous keystate information
      anData >> anPrevKeyState;
      // Next receive the remote client previous position information
      anData >> anTemp;
      anPrevPosition = GQE::ParseVector2f(anTemp, sf::Vector2f(512.0f,384.0f));
      // Next receive the remote client previous screen information
      anData >> anTemp;
      anPrevScreen = GQE::ParseVector2u(anTemp, sf::Vector2u(0,0));
      // Next receive the remote client previous loading state
      anData >> anPrevLoading;
      
      // Is this the game tick we are looking for?
      if(anCurGameTick == mGameTick)
      {
        // Search through each z-order map to find theEntity this belongs to
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
            if(anEntity->mProperties.Get<GQE::Uint32>("uNetworkID") == anID)
            {
              // Make note of the keystate information
              anEntity->mProperties.Set<GQE::Uint32>("uKeyState", anCurKeyState);
              // Let the system know this player has already provided keystate information
              anEntity->mProperties.Set<bool>("bKeyState", true);
              // Let the system know this players current position information
              anEntity->mProperties.Set<sf::Vector2f>("vPosition", anCurPosition);
              // Let the system know this players current screen information
              anEntity->mProperties.Set<sf::Vector2u>("wScreen", anCurScreen);
              // Let the system know if this player is currently loading still
              anEntity->mProperties.Set<bool>("bLoading", anCurLoading);
            }
          } // while(anQueue != anIter->second.end())

          // Increment map iterator
          anIter++;
        } //while(anIter != mEntities.end())
      } //if(anCurGameTick == mGameTick)
      else if(anPrevGameTick == mGameTick)
      {
        // Search through each z-order map to find theEntity this belongs to
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
            if(anEntity->mProperties.Get<GQE::Uint32>("uNetworkID") == anID)
            {
              // Make note of the keystate information
              anEntity->mProperties.Set<GQE::Uint32>("uKeyState", anPrevKeyState);
              // Let the system know this player has already provided keystate information
              anEntity->mProperties.Set<bool>("bKeyState", true);
              // Let the system know this players previous position information
              anEntity->mProperties.Set<sf::Vector2f>("vPosition", anPrevPosition);
              // Let the system know this players previous screen information
              anEntity->mProperties.Set<sf::Vector2u>("wScreen", anPrevScreen);
              // Let the system know if this player is currently loading still
              anEntity->mProperties.Set<bool>("bLoading", anPrevLoading);
            }
          } // while(anQueue != anIter->second.end())

          // Increment map iterator
          anIter++;
        } //while(anIter != mEntities.end())
      }
    } //if(anResult == sf::Socket::Done)
  } while(anResult == sf::Socket::Done);
}

void NetworkSystem::SendLocalInput(GQE::IEntity* theEntity)
{
  // Packet for sending our local players KeyState information to this network player
  sf::Packet anData;

  // Prepare a packet for this local player to send to all remote players
  // Start with current game tick number
  anData << mGameTick;
  // Add Network ID of the local player
  anData << theEntity->mProperties.Get<GQE::Uint32>("uNetworkID");
#if (SFML_VERSION_MAJOR < 2)
  // Add IP Address of the local player
  anData << theEntity->mProperties.Get<sf::IPAddress>("sNetworkAddr").ToString();
#else
  // Add IP Address of the local player
  anData << theEntity->mProperties.Get<sf::IpAddress>("sNetworkAddr").toString();
#endif
  // Add port number of the local player
  anData << theEntity->mProperties.Get<unsigned short>("uNetworkPort");
  // Add the current uKeyState property
  anData << theEntity->mProperties.Get<GQE::Uint32>("uKeyState");
  // Add the current vPosition property
  anData << GQE::ConvertVector2f(theEntity->mProperties.Get<sf::Vector2f>("vPosition"));
  // Add the current wScreen property
  anData << GQE::ConvertVector2u(theEntity->mProperties.Get<sf::Vector2u>("wScreen"));
  // Add the current bLoading property
  anData << theEntity->mProperties.Get<bool>("bLoading");
  // Add the previous game tick number
  anData << mGameTick - 1;
  // Add the previous uKeyState property
  anData << theEntity->mProperties.Get<GQE::Uint32>("uKeyStatePrevious");
  // Add the current vPosition property
  anData << GQE::ConvertVector2f(theEntity->mProperties.Get<sf::Vector2f>("vPositionPrevious"));
  // Add the current wScreen property
  anData << GQE::ConvertVector2u(theEntity->mProperties.Get<sf::Vector2u>("wScreenPrevious"));
  // Add the previous bLoading property
  anData << theEntity->mProperties.Get<bool>("bLoadingPrevious");

  // The iterator to use for each z-ordered deque of IEntity classes
  std::map<const GQE::Uint32, std::deque<GQE::IEntity*> >::iterator anIter;
  
  // Now loop through and send this to each remote player
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

      // If this is us, just move on
      if(anEntity == theEntity)
        continue;

      // Are we a local player who needs to send our keyboard state?
      if(anEntity->mProperties.Get<bool>("bNetworkLocal") == false)
      {
#if (SFML_VERSION_MAJOR < 2)
        // Now send our local players keystate to this network client
        mClient.Send(anData, anEntity->mProperties.Get<sf::IPAddress>("sNetworkAddr"),
          anEntity->mProperties.Get<unsigned short>("uNetworkPort"));
#else
        // Now send our local players keystate to this network client
        mClient.send(anData, anEntity->mProperties.Get<sf::IpAddress>("sNetworkAddr"),
          anEntity->mProperties.Get<unsigned short>("uNetworkPort"));
#endif
      }
    } // while(anQueue != anIter->second.end())

    // Increment map iterator
    anIter++;
  } //while(anIter != mEntities.end())
}

void NetworkSystem::UpdateLocalInput(GQE::IEntity* theEntity)
{
  // Start with no keys being pressed
  GQE::Uint32 anKeyState = 0;

#if SFML_VERSION_MAJOR<2
  if(mApp.mInput.IsKeyDown(sf::Key::Left))
  {
    anKeyState |= KEY_LEFT;
  }
  else if(mApp.mInput.IsKeyDown(sf::Key::Right))
  {
    anKeyState |= KEY_RIGHT;
  }
  if(mApp.mInput.IsKeyDown(sf::Key::Up))
  {
    anKeyState |= KEY_UP;
  }
  else if(mApp.mInput.IsKeyDown(sf::Key::Down))
  {
    anKeyState |= KEY_DOWN;
  }
#else
  if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
  {
    anKeyState |= KEY_LEFT;
  }
  else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
  {
    anKeyState |= KEY_RIGHT;
  }
  if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
  {
    anKeyState |= KEY_UP;
  }
  else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
  {
    anKeyState |= KEY_DOWN;
  }
#endif

  // Update the control system properties for this local Entity
  theEntity->mProperties.Set<GQE::Uint32>("uKeyState", anKeyState);
  theEntity->mProperties.Set<bool>("bKeyState", true);
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
