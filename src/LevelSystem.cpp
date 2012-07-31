/**
 * Provides the LevelSystem class for handing level loading and interaction in
 * a game.
 *
 * @file src/LevelSystem.cpp
 * @author Ryan Lindeman
 * @date 20120712 - Initial Release
 * @date 20120728 - Game Control fixes needed for multiplayer to work correctly
 * @date 20120730 - Improved network synchronization for multiplayer game play
 */
#include "LevelSystem.hpp"
#include <SFML/Graphics.hpp>
#include <GQE/Entity/systems/RenderSystem.hpp>
#include <GQE/Entity/classes/Instance.hpp>
#include <GQE/Core.hpp>

LevelSystem::LevelSystem(GQE::IApp& theApp,
    GQE::ISystem* theAnimationSystem,
    const GQE::typeAssetID theMapFilename,
    const GQE::typeAssetID theLoadingFilename,
    const GQE::typeAssetID theFontFilename,
    GQE::Uint32 theScreenTileWidth,
    GQE::Uint32 theScreenTileHeight,
    GQE::Uint32 theLoaderCount):
  ISystem("LevelSystem",theApp),
  mAnimationSystem(theAnimationSystem),
  mTile("map_tile"),
  mObject("map_object"),
  mTilesets(NULL),
  mScreenTileWidth(theScreenTileWidth),
  mScreenTileHeight(theScreenTileHeight),
  mScreenWidth(0),
  mScreenHeight(0),
  mTileWidth(32),
  mTileHeight(32),
  mTileScale(1.0f,1.0f),
  mMapFilename(theMapFilename),
  mLoadingFilename(theLoadingFilename),
  mScreen(0,0),
  mLoader(NULL),
  mLoaderCount(theLoaderCount)
{
#if (SFML_VERSION_MAJOR < 2)
  // First load our Arial font
  mFont.LoadFromFile(theFontFilename);
#else
  // First load our Arial font
  mFont.loadFromFile(theFontFilename);
#endif

  // Determine which scale to use for our tiles
  switch(mApp.mGraphicRange)
  {
  case GQE::LowRange: // Scale (0.5, 0.5)
    mTileScale.x = 0.5;
    mTileScale.y = 0.5;
    break;
  case GQE::HighRange: // Scale (2.0, 2.0)
    mTileScale.x = 2.0;
    mTileScale.y = 2.0;
    break;
  default:
  case GQE::MidRange: // Scale (1.0, 1.0) already set above
    break;
  }

  // Add our pseudo RenderSystem properties for our Tile prototype
  mTile.mProperties.Add<sf::Sprite>("Sprite", sf::Sprite());
  mTile.mProperties.Add<bool>("bVisible", true);
  mTile.mProperties.Add<sf::IntRect>("rBoundingBox",
    sf::IntRect(0,0,mTileWidth,mTileHeight));
  mTile.mProperties.Add<sf::IntRect>("rSpriteRect",sf::IntRect(0,0,0,0));
  mTile.mProperties.Add<sf::Vector2f>("vPosition",sf::Vector2f(0,0));
  mTile.mProperties.Add<sf::Vector2f>("vScale", mTileScale);

  // Add our pseudo RenderSystem properties for our Object prototype
  mObject.mProperties.Add<sf::Sprite>("Sprite", sf::Sprite());
  mObject.mProperties.Add<bool>("bVisible", true);
  mObject.mProperties.Add<sf::IntRect>("rSpriteRect",sf::IntRect(0,0,0,0));
  mObject.mProperties.Add<sf::IntRect>("rBoundingBox",
    sf::IntRect(0,0,mTileWidth,mTileHeight));
  mObject.mProperties.Add<sf::Vector2f>("vPosition",sf::Vector2f(0,0));
  mObject.mProperties.Add<sf::Vector2f>("vScale", mTileScale);

  // Did they specify theMapFilename? then load it now
  if(mMapFilename.length() > 0)
  {
    // Load theFilename now
    LoadMap(mMapFilename, mLoadingFilename);
  }
}

LevelSystem::~LevelSystem()
{
  // This will cause all screens to be dropped, essentially unloading the map
  //DropAllScreens();

  // Delete the current tilesets
  delete[] mTilesets;

  // Don't keep tileset pointers around
  mTilesets = NULL;
}

void LevelSystem::AddProperties(GQE::IEntity* theEntity)
{
  theEntity->mProperties.Add<GQE::typeAssetID>("sMapFilename", mMapFilename);
  theEntity->mProperties.Add<GQE::typeAssetID>("sLoadingFilename", mLoadingFilename);
  theEntity->mProperties.Add<sf::Vector2u>("wMap", sf::Vector2u(0,0));
  theEntity->mProperties.Add<sf::Vector2u>("wMapU", sf::Vector2u(0,0));
  theEntity->mProperties.Add<sf::Vector2u>("wMapL", sf::Vector2u(0,0));
  theEntity->mProperties.Add<sf::Vector2u>("wMapD", sf::Vector2u(0,0));
  theEntity->mProperties.Add<sf::Vector2u>("wMapR", sf::Vector2u(0,0));
  theEntity->mProperties.Add<sf::Vector2u>("wScreen", sf::Vector2u(0,0));
  theEntity->mProperties.Add<sf::Sprite>("Sprite", sf::Sprite());
  theEntity->mProperties.Add<bool>("bVisible", false);
  theEntity->mProperties.Add<bool>("bLoading", true);
  theEntity->mProperties.Add<bool>("bLoadingPrevious", false);
#if SFML_VERSION_MAJOR<2
  theEntity->mProperties.Add<sf::IntRect>("rBoundingBox",
    sf::IntRect(16*mTileScale.x,32*mTileScale.y,48*mTileScale.x,32*mTileScale.y));
#else
  theEntity->mProperties.Add<sf::IntRect>("rBoundingBox",
    sf::IntRect(16*mTileScale.x,32*mTileScale.y,32*mTileScale.x,32*mTileScale.y));
#endif
  theEntity->mProperties.Add<sf::IntRect>("rSpriteRect",sf::IntRect(0,0,0,0));
  theEntity->mProperties.Add<sf::Vector2f>("vPosition",sf::Vector2f(0,0));
  theEntity->mProperties.Add<sf::Vector2f>("vScale", mTileScale);
  theEntity->mProperties.Add<GQE::Uint32>("uScore", 0);
}

void LevelSystem::HandleEvents(sf::Event theEvent)
{
}

void LevelSystem::UpdateFixed()
{
  // Are we not loading a map now? then see if we need to load one now
  if(mLoader == NULL)
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

        // Calculate new MapX and MapY values for this Entity
        UpdateCoordinates(anEntity);

        // Check for treasures in our current location first
        CheckTreasure(anEntity);

        // Check screen edges before we check for walls
        CheckScreenEdges(anEntity);

        // Check for walls against this IEntity class
        CheckWalls(anEntity);

        if(anEntity->mProperties.Get<bool>("bNetworkLocal"))
        {
          // Retrieve the LevelSystem properties from this IEntity
          GQE::typeAssetID anMapFilename = anEntity->mProperties.Get<GQE::typeAssetID>("sMapFilename");
          GQE::typeAssetID anLoadingFilename = anEntity->mProperties.Get<GQE::typeAssetID>("sLoadingFilename");

          // Does the Filename not match the LevelFilename value, then transition to new map
          if(anMapFilename != mMapFilename)
          {
            // Load the new map
            LoadMap(anMapFilename, anLoadingFilename);
          }
        }
        else
        {
          // Network players should disappear if they are not on the same screen as local players
          sf::Vector2u anScreen = anEntity->mProperties.Get<sf::Vector2u>("wScreen");
          anEntity->mProperties.Set<bool>("bVisible", (anScreen == mScreen));
        }
      } // while(anQueue != anIter->second.end())

      // Increment map iterator
      anIter++;
    } //while(anIter != mEntities.end())
  }
}

void LevelSystem::UpdateVariable(float theElaspedTime)
{
}

void LevelSystem::Draw()
{
  // Are we suppose to be loading a map now? Then call the correct stage
  if(mLoader != NULL)
  {
    // Draw the loading please wait screen and percent complete bar
    DrawBar();

    // One call to each stage is too slow, give each stage several runs
    for(unsigned int i=0; mLoader && i < mLoaderCount; i++)
    {
      switch(mLoader->stage)
      {
        case TilesetStage:
          LoadStage1();
          break;
        case TileStage:
          LoadStage2();
          break;
        case ObjectStage:
          LoadStage3();
          break;
        case WaitingStage:
          LoadStage4();
          break;
        case UnknownStage:
        default:
          // Error, no such stage!?
        case CleanupStage:
          LoadStage5();
          break;
      }
    }
  }
  else
  {
    // Draw the current screen full of tiles
    DrawTiles();

    // Draw our players
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

        // If this Entity is visible, draw the player and his/her score above him
        if(anEntity->mProperties.Get<bool>("bVisible"))
        {
          // Get the other pseudo RenderSystem properties now
          sf::Vector2f anPosition = anEntity->mProperties.Get<sf::Vector2f>("vPosition");
          sf::IntRect anBoundingBox = anEntity->mProperties.Get<sf::IntRect>("rBoundingBox");
          sf::Sprite anSprite=anEntity->mProperties.Get<sf::Sprite>("Sprite");
#if SFML_VERSION_MAJOR<2
          anSprite.SetPosition(anPosition);
          anSprite.SetSubRect(anEntity->mProperties.Get<sf::IntRect>("rSpriteRect"));
          mApp.mWindow.Draw(anSprite);
#else
          anSprite.setPosition(anPosition);
          anSprite.setTextureRect(anEntity->mProperties.Get<sf::IntRect>("rSpriteRect"));
          mApp.mWindow.draw(anSprite);
#endif

#if (SFML_VERSION_MAJOR < 2)
          sf::String anScore("", mFont, 16.0f);
          // Position and color for the current players score
          anScore.SetColor(sf::Color(255,255,255,255));
          anScore.SetPosition(anPosition.x + anBoundingBox.left + 6, anPosition.y - mTileHeight/3);
#else
          sf::Text anScore("", mFont, 16);
          // Position and color for the current players score
          anScore.setColor(sf::Color(255,255,255,255));
          anScore.setPosition(anPosition.x + anBoundingBox.left + 6, anPosition.y - mTileHeight/3);
#endif

          // Create a string stream to create our percent complete string to display
          std::ostringstream anScoreString;

          // Convert score into a string so we can display it
          anScoreString << anEntity->mProperties.Get<GQE::Uint32>("uScore");

#if (SFML_VERSION_MAJOR < 2)
          // Assign our Percent Complete string value created above
          anScore.SetText(anScoreString.str());
          // Draw our score value above our player
          mApp.mWindow.Draw(anScore);
#else
          // Assign our Percent Complete string value created above
          anScore.setString(anScoreString.str());
          // Draw our score value above our player
          mApp.mWindow.draw(anScore);
#endif

          /*
          // START DEBUG: Draw our players bounding box
          anPosition.x += anBoundingBox.left;
          anPosition.y += anBoundingBox.top;

          sf::RectangleShape anBar(sf::Vector2f(anBoundingBox.width, anBoundingBox.height));
          anBar.setPosition(anPosition);
          anBar.setFillColor(sf::Color::Transparent);
          anBar.setOutlineColor(sf::Color::Green);
          anBar.setOutlineThickness(1.0);

          mApp.mWindow.draw(anBar);
          // END DEBUG: Draw our players bounding box
          */
        }
      } // while(anQueue != anIter->second.end())

      // Increment map iterator
      anIter++;
    } //while(anIter != mEntities.end())
  }
}

void LevelSystem::SwitchScreen(sf::Vector2u theScreen)
{
  // First validate theScreen value provided
  if(theScreen.x < mScreenWidth && theScreen.y < mScreenHeight)
  {
    // Make sure we are not currently loading a level
    if(mLoader == NULL)
    {
      // First unload the current screen
      UnloadScreen(mScreen);

      // Next load the new screen
      LoadScreen(theScreen);
    }
    else
    {
      WLOG() << "LevelSystem::SwitchScreen(" << theScreen.x << ", " <<
        theScreen.y << ") Level load in progress, can't switch screens!" << std::endl;
    }
  }
  else
  {
    ELOG() << "LevelSystem::SwitchScreen(" << theScreen.x << ", " <<
      theScreen.y << ") Invalid screen number provided!" << std::endl;
  }
}

bool LevelSystem::LoadMap(const GQE::typeAssetID theMapFilename,
    const GQE::typeAssetID theLoadingFilename)
{
  // Assume load will not be successful
  bool anResult = false;

  // Are we starting a new map load right now? (only one at a time!)
  if(mLoader == NULL)
  {
    // Create our Loader context which will load the map asset
    mLoader = new(std::nothrow) LoadContext(theMapFilename,
        theLoadingFilename);

    // Make sure the initial loading and parsing of the map succeeded
    if(mLoader != NULL &&
        mLoader->map.HasError() == false &&
        mLoader->map.GetNumTilesets() > 0 &&
        mLoader->map.GetWidth() > 0 &&
        mLoader->map.GetHeight() > 0)
    {
      // Compute some total for calculating percent complete
      mLoader->total = mLoader->map.GetNumTilesets() + 
        mLoader->map.GetNumLayers() * mLoader->map.GetWidth() * mLoader->map.GetHeight() +
        mLoader->map.GetNumObjectGroups() * mLoader->map.GetWidth() * mLoader->map.GetHeight() + 1;

      // Allocate ImageAssets to store each tileset images
      mLoader->tilesets = new (std::nothrow) GQE::ImageAsset[mLoader->map.GetNumTilesets()];

      // Set our filenames values
      mMapFilename = theMapFilename;
      mLoadingFilename = theLoadingFilename;

      // Search through each z-order map to loop through each registered IEntity class
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

          // Set our bLoading property to true
          anEntity->mProperties.Set<bool>("bLoading", true);

          // Load the map properties into each registered IEntity class
          LoadProperties(mLoader->map.GetProperties().GetList(), anEntity);
        } // while(anQueue != anIter->second.end())

        // Increment map iterator
        anIter++;
      } //while(anIter != mEntities.end())

      // Drop all our existing screens before loading new ones below
      //DropAllScreens();

      // Calculate the number of screens
      mScreenWidth = mLoader->map.GetWidth() / mScreenTileWidth;
      mScreenHeight = mLoader->map.GetHeight() / mScreenTileHeight;
      mTileWidth = mLoader->map.GetTileWidth() * mTileScale.x;
      mTileHeight = mLoader->map.GetTileHeight() * mTileScale.y;

      // Move on to the first stage
      mLoader->stage = TilesetStage;

      // Now proceed with TileStage during Draw method
      anResult = true; // Load in progress
    }
    else
    {
      ELOG() << "LevelSystem(" << theMapFilename << ", " << theLoadingFilename
        << ") Error in parsing TmxAsset map file!"
        << std::endl;
    }
  }
  else
  {
    WLOG() << "LevelSystem(" << theMapFilename << ", " << theLoadingFilename
      << ") Load already in progress!" << std::endl;
  }

  // Return anResult of true if load was successful, false otherwise
  return anResult;
}

void LevelSystem::UpdateCoordinates(GQE::IEntity* theEntity)
{
  sf::Vector2f anPosition = theEntity->mProperties.Get<sf::Vector2f>("vPosition");
  sf::Vector2f anVelocity = theEntity->mProperties.Get<sf::Vector2f>("vVelocity");
  sf::IntRect anBoundingBox = theEntity->mProperties.Get<sf::IntRect>("rBoundingBox");
  sf::Vector2u anScreen = theEntity->mProperties.Get<sf::Vector2u>("wScreen");

  // Compute the center tile that we are currently on based on our current position
  GQE::Uint32 anTileCenterX = (GQE::Uint32)((anPosition.x + anBoundingBox.left +
    anBoundingBox.width / 2) / mTileWidth) % mScreenTileWidth;
  GQE::Uint32 anTileCenterY = (GQE::Uint32)((anPosition.y + anBoundingBox.top +
    anBoundingBox.height / 2) / mTileHeight) % mScreenTileHeight;

  // Compute the tile if moving left, right, up, or down
  GQE::Uint32 anTileLeft = (GQE::Uint32)((anPosition.x + anVelocity.x + anBoundingBox.left) / mTileWidth) % mScreenTileWidth;
  GQE::Uint32 anTileRight = (GQE::Uint32)((anPosition.x + anVelocity.x + anBoundingBox.left + anBoundingBox.width) / mTileWidth) % mScreenTileWidth;
  GQE::Uint32 anTileUp = (GQE::Uint32)((anPosition.y + anVelocity.y + anBoundingBox.top) / mTileHeight) % mScreenTileHeight;
  GQE::Uint32 anTileDown = (GQE::Uint32)((anPosition.y + anVelocity.y + anBoundingBox.top + anBoundingBox.height) / mTileHeight) % mScreenTileHeight;

  // Compute the map coordinates for no movement
  theEntity->mProperties.Set<sf::Vector2u>("wMap", sf::Vector2u(
    anTileCenterX + anScreen.x * mScreenTileWidth,
    anTileCenterY + anScreen.y * mScreenTileHeight));

  // Compute the map coordinates for up movement
  theEntity->mProperties.Set<sf::Vector2u>("wMapU", sf::Vector2u(
    anTileCenterX + anScreen.x * mScreenTileWidth,
    anTileUp + anScreen.y * mScreenTileHeight));

  // Compute the map coordinates for left movement
  theEntity->mProperties.Set<sf::Vector2u>("wMapL", sf::Vector2u(
    anTileLeft + anScreen.x * mScreenTileWidth,
    anTileCenterY + anScreen.y * mScreenTileHeight));

  // Compute the map coordinates for down movement
  theEntity->mProperties.Set<sf::Vector2u>("wMapD", sf::Vector2u(
    anTileCenterX + anScreen.x * mScreenTileWidth,
    anTileDown + anScreen.y * mScreenTileHeight));

  // Compute the map coordinates for right movement
  theEntity->mProperties.Set<sf::Vector2u>("wMapR", sf::Vector2u(
    anTileRight + anScreen.x * mScreenTileWidth,
    anTileCenterY + anScreen.y * mScreenTileHeight));
}

void LevelSystem::CheckTreasure(GQE::IEntity* theEntity)
{
  sf::Vector2u anMapCC = theEntity->mProperties.Get<sf::Vector2u>("wMap");
  sf::Vector2u anScreen = theEntity->mProperties.Get<sf::Vector2u>("wScreen");
  
  // Search through each z-order map to find theEntityID provided
  std::deque<GQE::IEntity*>::iterator anIter =
    mScreens[anScreen.x + anScreen.y*mScreenWidth].treasures.begin();
  while(anIter != mScreens[anScreen.x + anScreen.y*mScreenWidth].treasures.end())
  {
    // Get the IEntity address first
    GQE::IEntity* anEntity = *anIter;

    // Increment coin iterator
    anIter++;

    // Is this tile visible and matches our current position?
    if(anEntity->mProperties.Get<bool>("bVisible") &&
      anMapCC == anEntity->mProperties.Get<sf::Vector2u>("wMap"))
    {
      // Make the coin disappear
      anEntity->mProperties.Set<bool>("bVisible", false);

      // Add to our players total points according to the value of the treasure
      theEntity->mProperties.Set<GQE::Uint32>("uScore",
        theEntity->mProperties.Get<GQE::Uint32>("uScore") + 
        anEntity->mProperties.Get<GQE::Uint32>("uValue"));
    }
  } //while(anIter != mScreens[anScreen.x + anScreen.y*mScreenWidth].treasures.end())
}

void LevelSystem::CheckWalls(GQE::IEntity* theEntity)
{
  // Get the Velocity of the current player
  sf::Vector2f anVelocity = theEntity->mProperties.Get<sf::Vector2f>("vVelocity");

  // Should we skip this check because we aren't moving?
  if(anVelocity.x > 0.1f || anVelocity.x < -0.1f || anVelocity.y > 0.1f || anVelocity.y < -0.1f)
  {
    // Get the Position of the current player
    sf::Vector2f anPosition = theEntity->mProperties.Get<sf::Vector2f>("vPosition");
    sf::Vector2u anScreen = theEntity->mProperties.Get<sf::Vector2u>("wScreen");
    sf::IntRect anBoundingBox = theEntity->mProperties.Get<sf::IntRect>("rBoundingBox");

    // The map position according to player movement
    sf::Vector2u anMapU = theEntity->mProperties.Get<sf::Vector2u>("wMapU");
    sf::Vector2u anMapL = theEntity->mProperties.Get<sf::Vector2u>("wMapL");
    sf::Vector2u anMapD = theEntity->mProperties.Get<sf::Vector2u>("wMapD");
    sf::Vector2u anMapR = theEntity->mProperties.Get<sf::Vector2u>("wMapR");

    // Search through each z-order map to find theEntityID provided
    std::deque<GQE::IEntity*>::iterator anIter =
      mScreens[anScreen.x + anScreen.y*mScreenWidth].walls.begin();
    while(anIter != mScreens[anScreen.x + anScreen.y*mScreenWidth].walls.end())
    {
      // Get the IEntity address first
      GQE::IEntity* anEntity = *anIter;

      // Increment wall iterator
      anIter++;

      // Get the wMap property for this wall IEntity
      sf::Vector2u anMap = anEntity->mProperties.Get<sf::Vector2u>("wMap");

      // Is this tile visible? then check it for wall collisions
      if(anEntity->mProperties.Get<bool>("bVisible"))
      {
        // Are we moving left and hit a wall?
        if(anVelocity.x < 0.0f && anMapL == anMap)
        {
          // Update position to exactly next to the tile
          anPosition.x = (anMapL.x % mScreenTileWidth) * mTileWidth -
#if (SFML_VERSION_MAJOR < 2)
            anBoundingBox.left + anBoundingBox.GetWidth();
#else
            anBoundingBox.left + anBoundingBox.width;
#endif

          // Cancel velocity in left direction
          anVelocity.x = 0.0f;
        }
        // Are we moving right and hit a wall?
        else if(anVelocity.x > 0.0f && anMapR == anMap)
        {
          // Update position to exactly next to the tile
          anPosition.x = (anMapR.x % mScreenTileWidth) * mTileWidth -
#if (SFML_VERSION_MAJOR < 2)
            anBoundingBox.left - anBoundingBox.GetWidth();
#else
            anBoundingBox.left - anBoundingBox.width;
#endif

          // Cancel velocity in right direction
          anVelocity.x = 0.0f;
        }
        else
        {
          // Do nothing
        }

        // Are we moving up and hit a wall?
        if(anVelocity.y < 0.0f && anMapU == anMap)
        {
          // Update position to exactly next to the tile
          anPosition.y = (anMapU.y % mScreenTileHeight) * mTileHeight -
#if (SFML_VERSION_MAJOR < 2)
            anBoundingBox.top + anBoundingBox.GetHeight();
#else
            anBoundingBox.top + anBoundingBox.height;
#endif

          // Cancel velocity in up direction
          anVelocity.y = 0.0f;
        }
        // Are we moving down and hit a wall?
        else if(anVelocity.y > 0.0f && anMapD == anMap)
        {
          // Update position to exactly next to the tile
          anPosition.y = (anMapD.y % mScreenTileHeight) * mTileHeight -
#if (SFML_VERSION_MAJOR < 2)
            anBoundingBox.top - anBoundingBox.GetHeight();
#else
            anBoundingBox.top - anBoundingBox.height;
#endif

          // Cancel velocity in down direction
          anVelocity.y = 0.0f;
        }
        else
        {
          // Do nothing
        }
      }
      // Special quick exit check if both velocities have been cancelled exit out
      if(anVelocity.x < 0.1f && anVelocity.x > -0.1f && anVelocity.y < 0.1f && anVelocity.y > -0.1f)
      {
        // Exit our while loop, no need to keep checking since we cancelled all movement
        break;
      }
    } //while(anIter != mScreens[anScreen.x + anScreen.y*mScreenWidth].walls.end())

    // Update our velocity value
    theEntity->mProperties.Set<sf::Vector2f>("vVelocity", anVelocity);

    // Update our position value
    theEntity->mProperties.Set<sf::Vector2f>("vPosition", anPosition);
  }
}

void LevelSystem::CheckScreenEdges(GQE::IEntity* theEntity)
{
  // Get the vVelocity property of the current player
  sf::Vector2f anVelocity = theEntity->mProperties.Get<sf::Vector2f>("vVelocity");
  // Get the vPosition property of the current player
  sf::Vector2f anPosition = theEntity->mProperties.Get<sf::Vector2f>("vPosition");
  // Get the wScreen property of the current player
  sf::Vector2u anScreen = theEntity->mProperties.Get<sf::Vector2u>("wScreen");
  // Get the rBoundingBox property of the current player
  sf::IntRect anBoundingBox = theEntity->mProperties.Get<sf::IntRect>("rBoundingBox");

  // The map position according to player movement
  sf::Vector2u anMapU = theEntity->mProperties.Get<sf::Vector2u>("wMapU");
  sf::Vector2u anMapL = theEntity->mProperties.Get<sf::Vector2u>("wMapL");
  sf::Vector2u anMapD = theEntity->mProperties.Get<sf::Vector2u>("wMapD");
  sf::Vector2u anMapR = theEntity->mProperties.Get<sf::Vector2u>("wMapR");

  // Are we moving left and hit a screen edge?
  if(anVelocity.x < 0.0f && 0 == (anMapR.x % mScreenTileWidth) && anScreen.x > 0)
  {
    // Update position to exactly next to the tile
    anPosition.x = (mScreenTileWidth - 1) * mTileWidth -
#if (SFML_VERSION_MAJOR < 2)
      anBoundingBox.left;
#else
      anBoundingBox.left;
#endif

    // Update our screen value
    anScreen.x--;
  }
  // Are we moving right and hit a screen edge?
  else if(anVelocity.x > 0.0f && (mScreenTileWidth - 1) == (anMapL.x % mScreenTileWidth) &&
    anScreen.x < mScreenWidth)
  {
#if (SFML_VERSION_MAJOR < 2)
    anPosition.x = -anBoundingBox.left;
#else
    anPosition.x = -anBoundingBox.left;
#endif

    // Update our screen value
    anScreen.x++;
  }
  else
  {
    // Do nothing
  }

  // Are we moving up and hit a screen edge?
  if(anVelocity.y < 0.0f && 0 == (anMapD.y % mScreenTileHeight) && anScreen.y > 0)
  {
    anPosition.y = (mScreenTileHeight - 1) * mTileHeight -
#if (SFML_VERSION_MAJOR < 2)
      anBoundingBox.top;
#else
      anBoundingBox.top;
#endif

    // Update our screen value
    anScreen.y--;
  }
  // Are we moving down and hit a screen edge?
  else if(anVelocity.y > 0.0f && (mScreenTileHeight-1) == (anMapU.y % mScreenTileHeight) &&
    anScreen.y < mScreenHeight)
  {
#if (SFML_VERSION_MAJOR < 2)
    anPosition.y = -anBoundingBox.top;
#else
    anPosition.y = -anBoundingBox.top;
#endif

    // Update our screen value
    anScreen.y++;
  }
  else
  {
    // Do nothing
  }

  // Update our Position value with any changes made above
  theEntity->mProperties.Set<sf::Vector2f>("vPosition", anPosition);

  // Update our Screen value with any changes made above
  theEntity->mProperties.Set<sf::Vector2u>("wScreen", anScreen);

  // If local player, update our animations to use the new screen
  if(theEntity->mProperties.Get<bool>("bNetworkLocal"))
  {
    SwitchScreen(anScreen);
  }
}

void LevelSystem::DrawBar(void)
{
  if(mLoader != NULL)
  {
    // Get our Loading screen texture
    sf::Sprite anSprite(mLoader->loading.GetAsset());

    // Define our percent complete string value
#if (SFML_VERSION_MAJOR < 2)
    sf::RectangleShape anBar(sf::Vector2f(
          float(mApp.mWindow.getSize().x-60)*mLoader->percent, 35));
    sf::String  anPercent("", mFont, 30.0f);
    // Position and color for the tile being loaded/progress bar
    anPercent.SetColor(sf::Color(0,255,0,128));
    anPercent.SetPosition((mApp.mWindow.GetWidth() / 2)-50,
        (mApp.mWindow.GetHeight() / 2) + 30);
    anBar.SetPosition(30, (mApp.mWindow.GetHeight() / 2) + 30);
    anBar.SetFillColor(sf::Color(0,0,128,255));
#else
    sf::RectangleShape anBar(sf::Vector2f(
          float(mApp.mWindow.getSize().x-60)*mLoader->percent, 35));
    sf::Text    anPercent("", mFont, 30);
    // Position and color for the tile being loaded/progress bar
    anPercent.setColor(sf::Color(0,255,0,128));
    anPercent.setPosition((float)(mApp.mWindow.getSize().x / 2)-50,
        (float)(mApp.mWindow.getSize().y / 2) + 30);
    anBar.setPosition(30.0f, (float)(mApp.mWindow.getSize().y / 2) + 30);
    anBar.setFillColor(sf::Color(0,0,128,255));
#endif

    // Create a string stream to create our percent complete string to display
    std::ostringstream anPercentString;

    // anValue is our percent complete to be displayed
    anPercentString.precision(1);
    anPercentString.width(7);
    anPercentString << std::fixed << mLoader->percent * 100.0f << "%";

#if (SFML_VERSION_MAJOR < 2)
    // Assign our Percent Complete string value created above
    anPercent.SetText(anPercentString.str());
    // Draw our Loading please wait screen first
    mApp.mWindow.Draw(anSprite);
    // Draw our Loading bar below our percent complete number
    mApp.mWindow.Draw(anBar);
    // Draw our percent complete value
    mApp.mWindow.Draw(anPercent);
#else
    // Assign our Percent Complete string value created above
    anPercent.setString(anPercentString.str());
    // Draw our Loading please wait screen first
    mApp.mWindow.draw(anSprite);
    // Draw our Loading bar below our percent complete number
    mApp.mWindow.draw(anBar);
    // Draw our percent complete value
    mApp.mWindow.draw(anPercent);
#endif
  }
}

void LevelSystem::DrawTiles(void)
{
  std::map<const GQE::Uint32, std::deque<GQE::IEntity*> >& anScreen =
    mScreens[mScreen.x + mScreen.y*mScreenWidth].tiles;

  // Search through each z-order map to find theEntityID provided
  std::map<const GQE::Uint32, std::deque<GQE::IEntity*> >::iterator anIter;
  anIter = anScreen.begin();
  while(anIter != anScreen.end())
  {
    std::deque<GQE::IEntity*>::iterator anQueue = anIter->second.begin();
    while(anQueue != anIter->second.end())
    {
      // Get the IEntity address first
      GQE::IEntity* anEntity = *anQueue;

      // Increment the IEntity iterator second
      anQueue++;

      // See if this IEntity is visible, if so draw it now
      if(anEntity->mProperties.Get<bool>("bVisible"))
      {
        // Get the other pseudo RenderSystem properties now
        sf::Sprite anSprite=anEntity->mProperties.Get<sf::Sprite>("Sprite");
#if SFML_VERSION_MAJOR<2
        anSprite.SetPosition(anEntity->mProperties.Get<sf::Vector2f>("vPosition"));
        anSprite.SetSubRect(anEntity->mProperties.Get<sf::IntRect>("rSpriteRect"));
        mApp.mWindow.Draw(anSprite);
#else
        anSprite.setPosition(anEntity->mProperties.Get<sf::Vector2f>("vPosition"));
        anSprite.setTextureRect(anEntity->mProperties.Get<sf::IntRect>("rSpriteRect"));
        mApp.mWindow.draw(anSprite);
#endif
      } // if(anEntity->mProperties.Get<bool>("bVisible"))
    } // while(anQueue != anIter->second.end())

    // Increment map iterator
    anIter++;
  } //while(anIter != mEntities.end())
}

void LevelSystem::HandleInit(GQE::IEntity* theEntity)
{
}

void LevelSystem::HandleCleanup(GQE::IEntity* theEntity)
{
}

/*
void LevelSystem::ResetProperties(bool theVisible)
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

      ////-
      if(anEntity->mProperties.Get<bool>("bNetworkLocal"))
      {
        // Set our LevelSystem properties
        anEntity->mProperties.Set<std::string>("sLevelMap", mMapFilename);
        anEntity->mProperties.Set<std::string>("sLevelLoading", mLoadingFilename);
        //anEntity->mProperties.Set<GQE::Uint32>("uLevelScreen", mScreen);
        anEntity->mProperties.Set<sf::Vector2f>("vPosition", mPosition);
        anEntity->mProperties.Set<bool>("bVisible", theVisible);
      }
      else
      {
        // Network players should disappear if they are not on the same screen as local players
        //GQE::Uint32 anScreen = anEntity->mProperties.Get<GQE::Uint32>("uLevelScreen");
        anEntity->mProperties.Set<bool>("bVisible", (anScreen == mScreen));
      }
      -////
    } // while(anQueue != anIter->second.end())

    // Increment map iterator
    anIter++;
  } //while(anIter != mEntities.end())
}
*/

void LevelSystem::DropAllScreens(void)
{
  /*
  std::map<const GQE::Uint32, ScreenInfo>::iterator anIter = mScreens.begin();
  while(anIter != mScreens.end())
  {
    // Unload each screen in our map of all screens
    UnloadScreen(anIter->first);

    // Clear the deque before moving on to the next one
    anIter->second.clear();

    // Increment our iterator to the next screen
    anIter++;
  }

  // Now clear our map of all screens
  mScreens.clear();

  // Reset our map variables
  mScreenWidth = 0;
  mScreenHeight = 0;

  // Finally reset all the Properties of each registered IEntity, set visible to false
  //ResetProperties(false);
  */
}

void LevelSystem::LoadScreen(sf::Vector2u theScreen)
{
  // Update our cached mScreen value to theScreen
  mScreen = theScreen;

  // Get address to the new current screen
  std::map<const GQE::Uint32, std::deque<GQE::IEntity*> >& anScreen =
    mScreens[theScreen.x + theScreen.y*mScreenWidth].tiles;

  // Search through each z-order map to find theEntityID provided
  std::map<const GQE::Uint32, std::deque<GQE::IEntity*> >::iterator anIter;
  anIter = anScreen.begin();
  while(anIter != anScreen.end())
  {
    std::deque<GQE::IEntity*>::iterator anQueue = anIter->second.begin();
    while(anQueue != anIter->second.end())
    {
      // Get the IEntity address first
      GQE::IEntity* anEntity = *anQueue;

      // Increment the IEntity iterator second
      anQueue++;

      // Is this an animated tile, then drop it from our AnimationSystem
      if(anEntity->mProperties.Get<bool>("bAnimation"))
      {
        mAnimationSystem->AddEntity(anEntity);
      }
    } // while(anQueue != anIter->second.end())

    // Increment map iterator
    anIter++;
  } //while(anIter != mEntities.end())
}

void LevelSystem::UnloadScreen(sf::Vector2u theScreen)
{
  std::map<const GQE::Uint32, std::deque<GQE::IEntity*> >& anScreen =
    mScreens[theScreen.x + theScreen.y*mScreenWidth].tiles;

  // Search through each z-order map to find theEntityID provided
  std::map<const GQE::Uint32, std::deque<GQE::IEntity*> >::iterator anIter;
  anIter = anScreen.begin();
  while(anIter != anScreen.end())
  {
    std::deque<GQE::IEntity*>::iterator anQueue = anIter->second.begin();
    while(anQueue != anIter->second.end())
    {
      // Get the IEntity address first
      GQE::IEntity* anEntity = *anQueue;

      // Increment the IEntity iterator second
      anQueue++;

      // Is this an animated tile, then drop it from our AnimationSystem
      if(anEntity->mProperties.Get<bool>("bAnimation"))
      {
        mAnimationSystem->DropEntity(anEntity->GetID());
      }
    } // while(anQueue != anIter->second.end())

    // Increment map iterator
    anIter++;
  } //while(anIter != mEntities.end())
}

void LevelSystem::LoadStage1(void)
{
  // Sanity check our mLoader value
  if(mLoader != NULL)
  {
    // Sanity check our boundaries
    if(mLoader->map.GetNumTilesets() > 0)
    {
      // Update our loader percent complete value which ranges from 0.0 to 1.0
      mLoader->percent = (float)mLoader->tileset / mLoader->total;

      // Tmx::Tileset to use for this map
      const Tmx::Tileset* anTileset = mLoader->map.GetTileset(mLoader->tileset);

      // Tmx::Image for this Tileset
      const Tmx::Image* anImage = anTileset->GetImage();

      // Create filename for this Tmx::Image
      std::string anFilename("resources/");

      // Append the source information provided to our filename
      anFilename.append(anImage->GetSource());

      // Add a new ImageAsset for each Tmx::Image using anFilename created above
      mLoader->tilesets[mLoader->tileset].SetID(anFilename);

      // Increment our counters for the next call to LoadStage1
      if(++mLoader->tileset == mLoader->map.GetNumTilesets())
      {
        // Reset tileset value and proceed to LoadStage2
        mLoader->tileset = 0;
        mLoader->stage = TileStage;
      }
    }
    else
    {
      // Move on to next stage, no tilesets available
      mLoader->stage = TileStage;
    }
  }
}

void LevelSystem::LoadStage2(void)
{
  // Sanity check our mLoader value
  if(mLoader != NULL)
  {
    // Sanity check our boundaries
    if(mLoader->map.GetNumLayers() > 0)
    {
      // Update our loader percent complete value which ranges from 0.0 to 1.0
      mLoader->percent = (float)(mLoader->layer * mLoader->map.GetWidth() * mLoader->map.GetHeight() +
          mLoader->x * mLoader->map.GetHeight() + mLoader->y) / mLoader->total;

      // Tmx::Layer pointer constant for the current layer
      const Tmx::Layer* anLayer = mLoader->map.GetLayer(mLoader->layer);

      // Tmx::MapTile at the x and y coordinate specified
      const Tmx::MapTile anMapTile = anLayer->GetTile(mLoader->x, mLoader->y);

      // If tilesetId is not >= 0 then it is an empty tile, move on
      if(anMapTile.tilesetId >= 0)
      {
        // Tmx::Tileset to use for this tile
        const Tmx::Tileset *anTileset = mLoader->map.GetTileset(anMapTile.tilesetId);

        // Create a GQE::Instance to represent this tile
        GQE::Instance* anInstance = mTile.MakeInstance();

        if(anInstance != NULL)
        {
          const GQE::Uint32 anScreen = (mLoader->x / mScreenTileWidth) +
            ((mLoader->y / mScreenTileHeight) * mScreenWidth);

          // Set our z-order to the same as our layer
          anInstance->SetOrder(mLoader->layer);

          // Add this instance to our queue for this screen
          mScreens[anScreen].tiles[mLoader->layer].push_back(anInstance);

          // Add the tile ID as a special property of anInstance
          anInstance->mProperties.Add<GQE::Uint32>("uTileID", anMapTile.id);

          // Add the map position and screen relative position for this tile as properties
          // This is the tile in Map coordinates
          anInstance->mProperties.Add<sf::Vector2u>("wMap",
            sf::Vector2u(mLoader->x, mLoader->y));

          // This is the screen that the tile will display on
          anInstance->mProperties.Add<sf::Vector2u>("wScreen",
            sf::Vector2u(mLoader->x / mScreenTileWidth, mLoader->y / mScreenTileHeight));

          anInstance->mProperties.Add<bool>("bAnimation", false);
          anInstance->mProperties.Add<bool>("bTreasure", false);
          anInstance->mProperties.Add<bool>("bWall", false);

          // Load a texture into our Sprite for this tile
          anInstance->mProperties.Set<sf::Sprite>("Sprite",
              sf::Sprite(mLoader->tilesets[anMapTile.tilesetId].GetAsset()));

#if SFML_VERSION_MAJOR<2
          const sf::Uint32 anWidth = mLoader->tilesets[anMapTile.tilesetId].GetAsset().GetWidth();
          if(anWidth > 0)
          {
            anInstance->mProperties.Set<sf::IntRect>("rSpriteRect",
                sf::IntRect(
                  (anMapTile.id%int(anWidth/mLoader->map.GetTileWidth()))*mLoader->map.GetTileWidth(),
                  (anMapTile.id/int(anWidth/mLoader->map.GetTileHeight()))*mLoader->map.GetTileHeight(),
                  (anMapTile.id%int(anWidth/mLoader->map.GetTileWidth()))*mLoader->map.GetTileWidth+mTileWidth(),
                  (anMapTile.id/int(anWidth/mLoader->map.GetTileHeight()))*mLoader->map.GetTileHeight+mTileHeight()));
          }
          //anInstance->mProperties.Set<sf::IntRect>("rBoundingBox",
          //    sf::IntRect(1,1,mLoader->map.GetTileWidth(),mLoader->map.GetTileHeight()));
#else
          const sf::Vector2u anSize = mLoader->tilesets[anMapTile.tilesetId].GetAsset().getSize();
          if(anSize.x > 0)
          {
            anInstance->mProperties.Set<sf::IntRect>("rSpriteRect",
                sf::IntRect(anMapTile.id%int(anSize.x/mLoader->map.GetTileWidth())*mLoader->map.GetTileWidth(),
                  anMapTile.id/int(anSize.x/mLoader->map.GetTileHeight())*mLoader->map.GetTileHeight(),
                  mLoader->map.GetTileWidth(),mLoader->map.GetTileHeight()));
          }
          //anInstance->mProperties.Set<sf::IntRect>("rBoundingBox",
          //    sf::IntRect(1,1,mLoader->map.GetTileWidth()-1,mLoader->map.GetTileHeight()-1));
#endif
          // Set the position for this tile
          anInstance->mProperties.Set<sf::Vector2f>("vPosition",
              sf::Vector2f((float)(mLoader->x % mScreenTileWidth)*mTileWidth,
                (float)(mLoader->y % mScreenTileHeight)*mTileHeight));

          // First load the Layer properties into this tile
          LoadProperties(anLayer->GetProperties().GetList(), anInstance);

          // Tmx::Tile type for the given Map Tile ID value
          const Tmx::Tile *anTile = anTileset->GetTile(anMapTile.id);

          // Now override any layer properties with specific tile properties
          if(anTile != NULL)
          {
            // Load the tile properties into anInstance
            LoadProperties(anTile->GetProperties().GetList(), anInstance);
          }

          // Is this a treasure tile, then add it to our list of treasures
          if(anInstance->mProperties.Get<bool>("bTreasure"))
          {
            mScreens[anScreen].treasures.push_back(anInstance);
          }

          // Is this a wall, then add it to our list of walls
          if(anInstance->mProperties.Get<bool>("bWall"))
          {
            mScreens[anScreen].walls.push_back(anInstance);
          }
        } // if(anInstance != NULL)
      } // if(anMapTile.tilesetId>=0)

      // Increment our counters for the next call to LoadStage3
      if(++mLoader->y == mLoader->map.GetHeight())
      {
        // Reset y value and increment x value
        mLoader->y = 0;
        if(++mLoader->x == mLoader->map.GetWidth())
        {
          // Reset x value and increment layer value
          mLoader->x = 0;
          if(++mLoader->layer == mLoader->map.GetNumLayers())
          {
            // Reset layer value and proceed to LoadStage4
            mLoader->layer = 0;
            mLoader->stage = ObjectStage;
          }
        }
      }
    }
    else
    {
      // Move on to next stage, no layers available
      mLoader->stage = ObjectStage;
    }
  } // if(mLoader != NULL)
}

void LevelSystem::LoadStage3(void)
{
  // Sanity check our mLoader value
  if(mLoader != NULL)
  {
    // Sanity check our boundaries
    if(mLoader->map.GetNumObjectGroups() > 0)
    {
      // Update our loader percent complete value which ranges from 0.0 to 1.0
      mLoader->percent = (float)mLoader->group / mLoader->map.GetNumObjectGroups();

      // The Tmx::ObjectGroup to look through
      const Tmx::ObjectGroup* anObjectGroup = mLoader->map.GetObjectGroup(mLoader->group);

      // The Tmx::Object in the current ObjectGroup
      const Tmx::Object* anObject = anObjectGroup->GetObject(mLoader->object);

      if(anObject->GetName()=="Player")
      {
        // Create anInstance for this Tmx::Object
        GQE::Instance* anInstance = mObject.MakeInstance();

        if(anInstance != NULL)
        {
          // Set our z-order to the same as our group
          anInstance->SetOrder(mLoader->group);

          // Store the ObjectName in anInstance as a string property
          anInstance->mProperties.Add<std::string>("ObjectName", anObject->GetName());

          // Add the tile ID as a special property of anInstance
          anInstance->mProperties.Add<std::string>("ObjectType", anObject->GetType());

          //mControlSystem.AddProperties(anInstance);
          //mControlSystem.AddEntity(anInstance);
#if SFML_VERSION_MAJOR<2
          //anInstance->mProperties.Set<sf::Sprite>("Sprite",
          //  sf::Sprite(anObjectImage.GetAsset()));
          //anInstance->mProperties.Set<sf::IntRect>("BoundingBox",sf::IntRect(2,2,16,16));
#else
          //anInstance->mProperties.Set<sf::Sprite>("Sprite",
          //  sf::Sprite(anObjectImage.GetAsset(),sf::IntRect(0,32,16,16)));
          //anInstance->mProperties.Set<sf::IntRect>("rBoundingBox",sf::IntRect(2,2,14,14));
#endif

          //anInstance->mProperties.Set<sf::IntRect>("rSpriteRect",sf::IntRect(0,0,16,16));
          anInstance->mProperties.Set<sf::Vector2f>("vPosition",
              sf::Vector2f((float)anObject->GetX(),(float)anObject->GetY()));
          //anInstance->mProperties.Set<float>("fFrameDelay",0.5f);
          //anInstance->mProperties.Set<sf::Vector2u>("vFrameModifier",sf::Vector2u(1,0));
          //anInstance->mProperties.Set<sf::IntRect>("rFrameRect",sf::IntRect(0,0,32,64));
          //anInstance->mProperties.Set<bool>("Player",true);
          anInstance->mProperties.Set<sf::Vector2f>("vStartPosition",
              sf::Vector2f((float)anObject->GetX(),(float)anObject->GetY()));
        }
      }

      // Increment our counters for the next call to LoadStage4
      if(++mLoader->object == anObjectGroup->GetNumObjects())
      {
        // Reset object value and increment group value
        mLoader->object = 0;
        if(++mLoader->group == mLoader->map.GetNumObjectGroups())
        {
          // Reset group value and proceed to LoadStage5
          mLoader->group = 0;
          mLoader->stage = WaitingStage;
        }
      }
    }
    else
    {
      // Move on to next stage, no object groups available
      mLoader->stage = WaitingStage;
    }
  } // if(mLoader != NULL)
}

void LevelSystem::LoadStage4(void)
{
  // How many players have committed keyboard state information?
  unsigned int anCount = 0;
  unsigned int anTotal = 0;

  // Make sure a load is actually in process
  if(mLoader != NULL)
  {
    // Reset all our registered IEntity properties
    //ResetProperties(true);

    // Make each player visible now
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

        // Player is local
        if(anEntity->mProperties.Get<bool>("bNetworkLocal"))
        {
          // Set our LevelSystem properties
          anEntity->mProperties.Set<std::string>("sLevelMap", mMapFilename);
          anEntity->mProperties.Set<std::string>("sLevelLoading", mLoadingFilename);

          // Set our bLoading property to false
          anEntity->mProperties.Set<bool>("bLoading", false);
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

    if(anCount == anTotal)
    {
      // LoadScreen now
      // TODO: This won't work with spawn points, fix it!
      LoadScreen(mScreen);

      // Move on to next stage, everyone has loaded their maps
      mLoader->stage = CleanupStage;
    }
  } // if(mLoader != NULL)
}

void LevelSystem::LoadStage5(void)
{
  // Make sure a load is actually in process
  if(mLoader != NULL)
  {
    // Did we have a previous tileset in use? then delete it now
    if(mTilesets != NULL)
    {
      // By deleting it here we can refrain from reloading identical tilesets
      // already in memory
      delete[] mTilesets;
    }

    // Make note of the new mTilesets
    mTilesets = mLoader->tilesets;

    // Reset all our registered IEntity properties
    //ResetProperties(true);

    // Make each player visible now
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

        // Player is local
        if(anEntity->mProperties.Get<bool>("bNetworkLocal"))
        {
          // Make our player visible
          anEntity->mProperties.Set<bool>("bVisible", true);
        }
        else
        {
          // Network players should disappear if they are not on the same screen as local players
          sf::Vector2u anScreen = anEntity->mProperties.Get<sf::Vector2u>("wScreen");
          anEntity->mProperties.Set<bool>("bVisible", (anScreen == mScreen));
        }
      } // while(anQueue != anIter->second.end())

      // Increment map iterator
      anIter++;
    } //while(anIter != mEntities.end())

    // Now remove our mLoader value since we are finally done
    delete mLoader;

    // Don't keep addresses we have deleted around
    mLoader = NULL;
  } // if(mLoader != NULL)
}

void LevelSystem::LoadProperties(std::map<std::string, std::string> theProperties,
    GQE::IEntity* theEntity)
{
  if(theEntity != NULL && theProperties.empty() == false)
  {
    // Loop through each property and add them as a number, boolean, or string
    std::map<std::string, std::string>::iterator anIter = theProperties.begin();
    while(anIter != theProperties.end())
    {
      // Is this a boolean property value? then its name will start with b or B
      if(anIter->first.at(0) == 'b' || anIter->first.at(0) == 'B')
      {
        // If the property already exists then replace it by setting a new property value
        if(theEntity->mProperties.HasID(anIter->first))
        {
          // Get the previous value and use it as the default
          bool anPrevious = theEntity->mProperties.Get<bool>(anIter->first);
          // Now set the new value by parsing the string provided
          theEntity->mProperties.Set<bool>(anIter->first,
              GQE::ParseBool(anIter->second, anPrevious));
        }
        // Otherwise just add the new property value
        else
        {
          // Create a boolean property by associating 0 as a false value, otherwise true
          theEntity->mProperties.Add<bool>(anIter->first,
              GQE::ParseBool(anIter->second, false));
        }
      }
      // Is this a color value? then its name will start with c or C
      else if(anIter->first.at(0) == 'c' || anIter->first.at(0) == 'C')
      {
        // If the property already exists then replace it by setting a new property value
        if(theEntity->mProperties.HasID(anIter->first))
        {
          // Get the previous value and use it as the default
          sf::Color anPrevious = theEntity->mProperties.Get<sf::Color>(anIter->first);
          // Now set the new value by parsing the string provided
          theEntity->mProperties.Set<sf::Color>(anIter->first,
              GQE::ParseColor(anIter->second, anPrevious));
        }
        // Otherwise just add the new property value
        else
        {
          // Create a sf::Color property value
          theEntity->mProperties.Add<sf::Color>(anIter->first,
              GQE::ParseColor(anIter->second, sf::Color(0,0,0,0)));
        }
      }
      // Is this a float value? then its name will start with f or F
      else if(anIter->first.at(0) == 'f' || anIter->first.at(0) == 'F')
      {
        // If the property already exists then replace it by setting a new property value
        if(theEntity->mProperties.HasID(anIter->first))
        {
          // Get the previous value and use it as the default
          float anPrevious = theEntity->mProperties.Get<float>(anIter->first);
          // Now set the new value by parsing the string provided
          theEntity->mProperties.Set<float>(anIter->first,
              GQE::ParseFloat(anIter->second, anPrevious));
        }
        // Otherwise just add the new property value
        else
        {
          // Create a float property value
          theEntity->mProperties.Add<float>(anIter->first,
              GQE::ParseFloat(anIter->second, 0.0f));
        }
      }
      // Is this a signed numeric property value? then its name will start with i or I
      else if(anIter->first.at(0) == 'i' || anIter->first.at(0) == 'I')
      {
        // If the property already exists then replace it by setting a new property value
        if(theEntity->mProperties.HasID(anIter->first))
        {
          // Get the previous value and use it as the default
          GQE::Int32 anPrevious = theEntity->mProperties.Get<GQE::Int32>(anIter->first);
          // Now set the new value by parsing the string provided
          theEntity->mProperties.Set<GQE::Int32>(anIter->first,
              GQE::ParseInt32(anIter->second, anPrevious));
        }
        // Otherwise just add the new property value
        else
        {
          // Create a boolean property by associating 0 as a false value, otherwise true
          theEntity->mProperties.Add<GQE::Int32>(anIter->first,
              GQE::ParseInt32(anIter->second, 0));
        }
      }
      // Is this an sf::IntRect property value? then its name will start with r or R
      if(anIter->first.at(0) == 'r' || anIter->first.at(0) == 'R')
      {
        // If the property already exists then replace it by setting a new property value
        if(theEntity->mProperties.HasID(anIter->first))
        {
          // Get the previous value and use it as the default
          sf::IntRect anPrevious = theEntity->mProperties.Get<sf::IntRect>(anIter->first);
          // Now set the new value by parsing the string provided
          theEntity->mProperties.Set<sf::IntRect>(anIter->first,
              GQE::ParseIntRect(anIter->second, anPrevious));
        }
        // Otherwise just add the new property value
        else
        {
          // Create a boolean property by associating 0 as a false value, otherwise true
          theEntity->mProperties.Add<sf::IntRect>(anIter->first,
              GQE::ParseIntRect(anIter->second, sf::IntRect(0,0,0,0)));
        }
      }
      // Is this an unsigned numeric property value? then its name will start with u or U
      if(anIter->first.at(0) == 'u' || anIter->first.at(0) == 'U')
      {
        // If the property already exists then replace it by setting a new property value
        if(theEntity->mProperties.HasID(anIter->first))
        {
          // Get the previous value and use it as the default
          GQE::Uint32 anPrevious = theEntity->mProperties.Get<GQE::Uint32>(anIter->first);
          // Now set the new value by parsing the string provided
          theEntity->mProperties.Set<GQE::Uint32>(anIter->first,
              GQE::ParseUint32(anIter->second, anPrevious));
        }
        // Otherwise just add the new property value
        else
        {
          // Create a boolean property by associating 0 as a false value, otherwise true
          theEntity->mProperties.Add<GQE::Uint32>(anIter->first,
              GQE::ParseUint32(anIter->second, 0));
        }
      }
      // Is this a sf::Vector2f property value? then its name will start with v or V
      if(anIter->first.at(0) == 'v' || anIter->first.at(0) == 'V')
      {
        // If the property already exists then replace it by setting a new property value
        if(theEntity->mProperties.HasID(anIter->first))
        {
          // Get the previous value and use it as the default
          sf::Vector2f anPrevious = theEntity->mProperties.Get<sf::Vector2f>(anIter->first);
          // Now set the new value by parsing the string provided
          theEntity->mProperties.Set<sf::Vector2f>(anIter->first,
              GQE::ParseVector2f(anIter->second, anPrevious));
        }
        // Otherwise just add the new property value
        else
        {
          // Create a boolean property by associating 0 as a false value, otherwise true
          theEntity->mProperties.Add<sf::Vector2f>(anIter->first,
              GQE::ParseVector2f(anIter->second, sf::Vector2f(0.0f,0.0f)));
        }
      }
      // Is this a sf::Vector2u property value? then its name will start with w or W
      if(anIter->first.at(0) == 'w' || anIter->first.at(0) == 'W')
      {
        // If the property already exists then replace it by setting a new property value
        if(theEntity->mProperties.HasID(anIter->first))
        {
          // Get the previous value and use it as the default
          sf::Vector2u anPrevious = theEntity->mProperties.Get<sf::Vector2u>(anIter->first);
          // Now set the new value by parsing the string provided
          theEntity->mProperties.Set<sf::Vector2u>(anIter->first,
              GQE::ParseVector2u(anIter->second, anPrevious));
        }
        // Otherwise just add the new property value
        else
        {
          // Create a boolean property by associating 0 as a false value, otherwise true
          theEntity->mProperties.Add<sf::Vector2u>(anIter->first,
              GQE::ParseVector2u(anIter->second, sf::Vector2u(0,0)));
        }
      }
      // Is this a sf::Vector3f property value? then its name will start with z or Z
      if(anIter->first.at(0) == 'z' || anIter->first.at(0) == 'Z')
      {
        // If the property already exists then replace it by setting a new property value
        if(theEntity->mProperties.HasID(anIter->first))
        {
          // Get the previous value and use it as the default
          sf::Vector3f anPrevious = theEntity->mProperties.Get<sf::Vector3f>(anIter->first);
          // Now set the new value by parsing the string provided
          theEntity->mProperties.Set<sf::Vector3f>(anIter->first,
              GQE::ParseVector3f(anIter->second, anPrevious));
        }
        // Otherwise just add the new property value
        else
        {
          // Create a boolean property by associating 0 as a false value, otherwise true
          theEntity->mProperties.Add<sf::Vector3f>(anIter->first,
              GQE::ParseVector3f(anIter->second, sf::Vector3f(0.0f,0.0f,0.0f)));
        }
      }
      // Otherwise assume its a string property
      else
      {
        // If the property already exists then replace it by setting a new property value
        if(theEntity->mProperties.HasID(anIter->first))
        {
          // Set the new value as the string value provided
          theEntity->mProperties.Set<std::string>(anIter->first, anIter->second);
        }
        // Otherwise just add the new property value
        else
        {
          // Create a boolean property by associating 0 as a false value, otherwise true
          theEntity->mProperties.Add<std::string>(anIter->first, anIter->second);
        }
      }

      // Iterate to the next property
      anIter++;
    }
  } // if(theTile->GetProperties().Empty() == false)
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
