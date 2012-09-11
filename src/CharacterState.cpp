/**
 * Provides the Character Selector State for Traps and Treasures
 *
 * @file src/CharacterState.cpp
 * @author Ryan Lindeman
 * @date 20120712 - Initial Release
 */
#include "CharacterState.hpp"
#include <SFML/Graphics.hpp>
#include <GQE/Core/assets/ImageAsset.hpp>
#include <GQE/Core/interfaces/IApp.hpp>
#include <GQE/Entity/classes/Instance.hpp>

CharacterState::CharacterState(GQE::IApp& theApp) :
  GQE::IState("Game",theApp),
  mAnimationSystem(theApp),
  mRenderSystem(theApp),
  mPlayer("player", 255),
  mCharacter(NULL),
  mCurrentImage(0),
  mMaxCharacterImages(0),
  mBackground("resources/images/character.png", GQE::AssetLoadNow)
{
  // Determine the number of character images available up to MAX_CHARACTERS
  do
  {
    // Create a string stream to create our character filename to load
    std::ostringstream anFilename;

    // Create the filename to the character image
    anFilename << "resources/images/character" << mMaxCharacterImages + 1 << ".png";

    ILOG() << "CharacterState::ctor() checking " << anFilename.str() << std::endl;

    // Attempt to load the image from the filename created above
#if (SFML_VERSION_MAJOR < 2)
    bool anResult = mCharacterImage.LoadFromFile(anFilename.str());
#else
    bool anResult = mCharacterImage.loadFromFile(anFilename.str());
#endif

    // If the image loaded and is a multiple of 64 pixels for both x and y then its ok
    if(anResult == true &&
#if (SFML_VERSION_MAJOR < 2)
      mCharacterImage.GetWidth() % 64 == 0 && mCharacterImage.GetWidth() > 0 &&
      mCharacterImage.GetHeight() % 64 == 0 && mCharacterImage.GetHeight() > 0
#else
      mCharacterImage.getSize().x % 64 == 0 && mCharacterImage.getSize().x > 0 &&
      mCharacterImage.getSize().y % 64 == 0 && mCharacterImage.getSize().y > 0
#endif
      )
    {
      ILOG() << "CharacterState::ctor() resource " << anFilename.str() << " valid!" << std::endl;
      // Increment our max characters to choose from counter
      mMaxCharacterImages++;
    }
    else
    {
      ILOG() << "CharacterState::ctor() resource " << anFilename.str() << " invalid!" << std::endl;
      break;
    }
  } while(mMaxCharacterImages < MAX_CHARACTERS);

  // Load the first character image
  if(mMaxCharacterImages > 0)
  {
    // Load the character image from a file
#if (SFML_VERSION_MAJOR < 2)
    mCharacterImage.LoadFromFile("resources/images/character1.png");
#else
    mCharacterImage.loadFromFile("resources/images/character1.png");
#endif

    // Make note of the current selected character image
    mApp.mProperties.Add<GQE::typeAssetID>("sCharacter", "resources/images/character1.png");
  }
  else
  {
    // Signal the application to exit
    mApp.Quit(GQE::StatusAppMissingAsset);
  }
}

CharacterState::~CharacterState(void)
{
  ILOG() << "CharacterState::dtor()" << std::endl;
}

void CharacterState::DoInit(void)
{
  // First call our base class implementation
  IState::DoInit();
  mApp.mStatManager.SetShow(true);

  // Register all ISystems for the Player prototype
  mPlayer.AddSystem(&mAnimationSystem);
  mPlayer.AddSystem(&mRenderSystem);

  // Create a single player instance and set its various properties
  mCharacter = mPlayer.MakeInstance();

  // Did we get a valid Instance? then set some of its properties now
  if(mCharacter != NULL)
  {
    // Set the player image
    mCharacter->mProperties.Set<sf::Sprite>("Sprite",
      sf::Sprite(mCharacterImage));

    // Get the SpriteRect property from our instance
#if (SFML_VERSION_MAJOR < 2)
    sf::IntRect anSpriteRect(0,64*DOWN_OFFSET,64,64*(DOWN_OFFSET+1));
#else
    sf::IntRect anSpriteRect(0,64*DOWN_OFFSET,64,64);
#endif
    mCharacter->mProperties.Set<sf::IntRect>("rSpriteRect", anSpriteRect);

    // Set our visible property
    mCharacter->mProperties.Set<bool>("bVisible", true);

    // Set our animation properties
    mCharacter->mProperties.Set<float>("fFrameDelay", 0.08f);
    mCharacter->mProperties.Set<sf::Vector2u>("wFrameModifier", sf::Vector2u(1,0));
#if (SFML_VERSION_MAJOR < 2)
    mCharacter->mProperties.Set<sf::IntRect>("rFrameRect",
        sf::IntRect(0,0,mCharacterImage.GetWidth(), mCharacterImage.GetHeight()));
#else
    mCharacter->mProperties.Set<sf::IntRect>("rFrameRect",
        sf::IntRect(0,0,mCharacterImage.getSize().x, mCharacterImage.getSize().y));
#endif

    // Set initial position on the screen to the middle of the screen
#if (SFML_VERSION_MAJOR < 2)
    mCharacter->mProperties.Set<sf::Vector2f>("vPosition",
        sf::Vector2f((float)(mApp.mWindow.GetWidth() - anSpriteRect.GetWidth()) / 2,
          (float)(mApp.mWindow.GetHeight() - anSpriteRect.GetHeight()) / 2));
#else
    mCharacter->mProperties.Set<sf::Vector2f>("vPosition",
        sf::Vector2f((float)(mApp.mWindow.getSize().x - anSpriteRect.width) / 2,
          (float)(mApp.mWindow.getSize().y - anSpriteRect.height) / 2));
#endif
  }
  else
  {
    // Signal the application to exit
    mApp.Quit(GQE::StatusError);
  }
}

void CharacterState::ReInit()
{
}

void CharacterState::HandleEvents(sf::Event theEvent)
{
  // Call our Base class implementation
  IState::HandleEvents(theEvent);

  // Get the SpriteRect property for our character
  sf::IntRect anSpriteRect = mCharacter->mProperties.Get<sf::IntRect>("rSpriteRect");

  // Switch to character facing up
  if(
#if (SFML_VERSION_MAJOR < 2)
    (theEvent.Type == sf::Event::KeyReleased) && (theEvent.Key.Code == sf::Key::Up)
#else
    (theEvent.type == sf::Event::KeyReleased) && (theEvent.key.code == sf::Keyboard::Up)
#endif
    )
  {
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

  // Switch to character facing left
  if(
#if (SFML_VERSION_MAJOR < 2)
    (theEvent.Type == sf::Event::KeyReleased) && (theEvent.Key.Code == sf::Key::Left)
#else
    (theEvent.type == sf::Event::KeyReleased) && (theEvent.key.code == sf::Keyboard::Left)
#endif
    )
  {
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

  // Switch to character facing up
  if(
#if (SFML_VERSION_MAJOR < 2)
    (theEvent.Type == sf::Event::KeyReleased) && (theEvent.Key.Code == sf::Key::Down)
#else
    (theEvent.type == sf::Event::KeyReleased) && (theEvent.key.code == sf::Keyboard::Down)
#endif
    )
  {
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

  // Switch to character facing left
  if(
#if (SFML_VERSION_MAJOR < 2)
    (theEvent.Type == sf::Event::KeyReleased) && (theEvent.Key.Code == sf::Key::Right)
#else
    (theEvent.type == sf::Event::KeyReleased) && (theEvent.key.code == sf::Keyboard::Right)
#endif
    )
  {
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

  // Switch to the next character when they press the Space key
  if(
#if (SFML_VERSION_MAJOR < 2)
    (theEvent.Type == sf::Event::KeyReleased) && (theEvent.Key.Code == sf::Key::Space)
#else
    (theEvent.type == sf::Event::KeyReleased) && (theEvent.key.code == sf::Keyboard::Space)
#endif
    )
  {
    // Create a string stream to create our character filename to load
    std::ostringstream anFilename;

    // Increment the current character image being shown
    mCurrentImage = (mCurrentImage + 1) % mMaxCharacterImages;

    // Create the filename to the character image
    anFilename << "resources/images/character" << mCurrentImage + 1 << ".png";

    // Load the character image from a file
#if (SFML_VERSION_MAJOR < 2)
    mCharacterImage.LoadFromFile(anFilename.str());
#else
    mCharacterImage.loadFromFile(anFilename.str());
#endif

    // Make note of the current selected character image
    mApp.mProperties.Set<GQE::typeAssetID>("sCharacter", anFilename.str());
  }

  // Exit out once they pick a character and press the Enter/Return key
  if(
#if (SFML_VERSION_MAJOR < 2)
    (theEvent.Type == sf::Event::KeyReleased) && (theEvent.Key.Code == sf::Key::Return)
#else
    (theEvent.type == sf::Event::KeyReleased) && (theEvent.key.code == sf::Keyboard::Return)
#endif
    )
  {
    // Drop this active state
    mApp.mStateManager.DropActiveState();
  }

  // Update our character properties
  mCharacter->mProperties.Set<sf::IntRect>("rSpriteRect", anSpriteRect);

  // Set the character image
  mCharacter->mProperties.Set<sf::Sprite>("Sprite", sf::Sprite(mCharacterImage));

  // Set the animation frame rect according to the size of the image
#if (SFML_VERSION_MAJOR < 2)
  mCharacter->mProperties.Set<sf::IntRect>("rFrameRect",
      sf::IntRect(0,0,mCharacterImage.GetWidth(), mCharacterImage.GetHeight()));
#else
  mCharacter->mProperties.Set<sf::IntRect>("rFrameRect",
      sf::IntRect(0,0,mCharacterImage.getSize().x, mCharacterImage.getSize().y));
#endif
}

void CharacterState::UpdateFixed(void)
{
  // AnimationSystem works best after ControlSystem
  mAnimationSystem.UpdateFixed();
}

void CharacterState::UpdateVariable(float theElapsedTime)
{
}

void CharacterState::Draw(void)
{
  // Get our Background screen which provides instructions for choosing a character
  sf::Sprite anBackground(mBackground.GetAsset());

#if (SFML_VERSION_MAJOR < 2)
  // Draw our Background screen
  mApp.mWindow.Draw(anBackground);
#else
  // Draw our Background screen
  mApp.mWindow.draw(anBackground);
#endif

  // Allow our RenderSystem to draw our character
  mRenderSystem.Draw();
}

void CharacterState::HandleCleanup(void)
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
