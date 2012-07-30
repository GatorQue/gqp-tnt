/**
 * Provides the Game State for Traps and Treasures
 *
 * @file src/GameState.cpp
 * @author Ryan Lindeman
 * @date 20120712 - Initial Release
 * @date 20120728 - Game Control fixes needed for multiplayer to work correctly
 */
#include "GameState.hpp"
#include <SFML/Network.hpp>
#include <GQE/Core/assets/FontAsset.hpp>
#include <GQE/Core/assets/ImageAsset.hpp>
#include <GQE/Core/interfaces/IApp.hpp>
#include <GQE/Entity/classes/Prototype.hpp>
#include <GQE/Entity/classes/Instance.hpp>
#include <GQE/Entity/classes/PrototypeManager.hpp>

GameState::GameState(GQE::IApp& theApp) :
  GQE::IState("Game",theApp),
  mAnimationSystem(theApp),
  mControlSystem(theApp),
  mMovementSystem(theApp),
  mLevelSystem(theApp, &mAnimationSystem,
    "resources/Level0.tmx",
    "resources/images/loading.png",
    "resources/arial.ttf",
    32,  // each screen is 32 tiles across
    24), // each screen is 24 tiles down
  mNetworkSystem(theApp),
  mPlayer("player",100),
  mPlayerID(0),
  mPlayerImages(NULL)
{
}

GameState::~GameState(void)
{
  ILOG() << "GameState::dtor()" << std::endl;
}

void GameState::DoInit(void)
{
  // First call our base class implementation
  IState::DoInit();
  //srand(time((time_t*)NULL));
  mApp.mStatManager.SetShow(true);

  // Register all ISystems for the Player prototype
  mPlayer.AddSystem(&mAnimationSystem);
  mPlayer.AddSystem(&mLevelSystem);
  mPlayer.AddSystem(&mMovementSystem);
  mPlayer.AddSystem(&mNetworkSystem);

  // Get the number of players for the current game
  GQE::Uint32 anPlayerCount = mApp.mProperties.Get<GQE::Uint32>("uPlayerCount");
  
  // Create an array to hold each players image
  mPlayerImages = new(std::nothrow) GQE::ImageAsset[anPlayerCount];

  // Make sure our array was created successfully
  if(mPlayerImages != NULL)
  {
    // Loop through each player and create an instance of each player
    for(unsigned int iloop = 0; iloop < anPlayerCount; iloop++)
    {
      // Create a single player instance and set its various properties
      GQE::Instance* anInstance = mPlayer.MakeInstance();

      // Did we get a valid Instance? then set some of its properties now
      if(anInstance != NULL)
      {
        // Create a string stream to create our player id tag
        std::string anPlayerID("sPlayerID");
        std::string anPlayerAddr("sPlayerAddr");
        std::string anPlayerPort("uPlayerPort");
        std::string anPlayerAsset("sPlayerAssetID");
        std::ostringstream anPlayerNumber;

        // Create a string version of our player number to append to the above strings
        anPlayerNumber << iloop + 1;

        // Append player number to each player
        anPlayerID.append(anPlayerNumber.str());
        anPlayerAddr.append(anPlayerNumber.str());
        anPlayerPort.append(anPlayerNumber.str());
        anPlayerAsset.append(anPlayerNumber.str());

        // Set the other properties as NetworkSystem properties
        anInstance->mProperties.Set<GQE::Uint32>("uNetworkID",
          mApp.mProperties.Get<GQE::Uint32>(anPlayerID));
        anInstance->mProperties.Set<sf::IpAddress>("sNetworkAddr",
          sf::IpAddress(mApp.mProperties.Get<std::string>(anPlayerAddr)));
        anInstance->mProperties.Set<unsigned short>("uNetworkPort",
          mApp.mProperties.Get<unsigned short>(anPlayerPort));

        // Retrieve the Image AssetID to use for this player
        GQE::typeAssetID anFilename = mApp.mProperties.Get<GQE::typeAssetID>(anPlayerAsset);

        // Assign this filename as the AssetID for this players image file
        mPlayerImages[iloop].SetID(anFilename);

        // Set the player image loaded and assigned above
        anInstance->mProperties.Set<sf::Sprite>("Sprite",
          sf::Sprite(mPlayerImages[iloop].GetAsset()));

        // Get the SpriteRect property from our instance
    #if (SFML_VERSION_MAJOR < 2)
        sf::IntRect anSpriteRect(0,64*2,64,64*(2+1));
    #else
        sf::IntRect anSpriteRect(0,64*2,64,64);
    #endif
        anInstance->mProperties.Set<sf::IntRect>("rSpriteRect", anSpriteRect);

        // Set our animation properties
        anInstance->mProperties.Set<float>("fFrameDelay", 0.08f);
        anInstance->mProperties.Set<sf::Vector2u>("wFrameModifier", sf::Vector2u(1,0));
    #if (SFML_VERSION_MAJOR < 2)
        anInstance->mProperties.Set<sf::IntRect>("rFrameRect",
            sf::IntRect(0,0,mPlayerImages[iloop].GetAsset().GetWidth(),
            mPlayerImage.GetAsset().GetHeight());
    #else
        anInstance->mProperties.Set<sf::IntRect>("rFrameRect",
            sf::IntRect(0,0,mPlayerImages[iloop].GetAsset().getSize().x,
            mPlayerImages[iloop].GetAsset().getSize().y));
    #endif

        // Determine the initial position on the screen for our player in the game
    #if (SFML_VERSION_MAJOR < 2)
        anInstance->mProperties.Set<sf::Vector2f>("vPosition",
            sf::Vector2f((float)(mApp.mWindow.GetWidth() - anSpriteRect.GetWidth()) / 2,
              (float)(mApp.mWindow.GetHeight() - anSpriteRect.GetHeight()) / 2));
    #else
        anInstance->mProperties.Set<sf::Vector2f>("vPosition",
            sf::Vector2f((float)(mApp.mWindow.getSize().x - anSpriteRect.width) / 2,
              (float)(mApp.mWindow.getSize().y - anSpriteRect.height) / 2));
    #endif

        // Only the first player is a local player, all others are network players to us
        if(iloop == 0)
        {
          // Keep track of our PlayerID
          mPlayerID = anInstance->GetID();

          // We are a local player
          anInstance->mProperties.Set<bool>("bNetworkLocal", true);

          // Add this instance to our ControlSystem
          mControlSystem.AddEntity(anInstance);
        }
        else
        {
          // Add fSpeed for network players since they aren't registered with
          // ControlSystem
          anInstance->mProperties.Add<float>("fSpeed", 4.0f);
        }
      }
      else
      {
        // Signal the application to exit
        mApp.Quit(GQE::StatusError);
      }
    } // for(unsigned int iloop = 0; iloop < anPlayerCount; iloop++)
  }
  else
  {
    // Signal the application to exit
    mApp.Quit(GQE::StatusError);
  }
}

void GameState::ReInit()
{
}

void GameState::UpdateFixed(void)
{
  // ControlSystem should always come first
  mControlSystem.UpdateFixed();

  // NetworkSystem should be called after ControlSystem but before LevelSystem
  mNetworkSystem.UpdateFixed();

  // AnimationSystem works best after ControlSystem
  mAnimationSystem.UpdateFixed();

  // LevelSystem should always come before MovementSystem and after ControlSystem
  mLevelSystem.UpdateFixed();

  // MovementSystem should always come after LevelSystem to allow for Wall collision detection
  mMovementSystem.UpdateFixed();
}

void GameState::UpdateVariable(float theElapsedTime)
{
}

void GameState::Draw(void)
{
  mLevelSystem.Draw();
}

void GameState::HandleCleanup(void)
{
  if(mPlayerImages)
  {
    delete[] mPlayerImages;
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
