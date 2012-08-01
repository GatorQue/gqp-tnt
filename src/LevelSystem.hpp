/**
 * Provides the LevelSystem class for handing level loading and interaction in
 * a game.
 *
 * @file src/LevelSystem.hpp
 * @author Ryan Lindeman
 * @date 20120712 - Initial Release
 * @date 20120728 - Game Control fixes needed for multiplayer to work correctly
 * @date 20120730 - Improved network synchronization for multiplayer game play
 * @date 20120731 - Add sound effects and player spawn points
 */
#ifndef LEVEL_SYSTEM_HPP_INCLUDED
#define LEVEL_SYSTEM_HPP_INCLUDED

#include <map>
#include <deque>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <GQE/Entity/interfaces/ISystem.hpp>
#include <GQE/Entity/classes/Prototype.hpp>
#include <GQE/Entity/Entity_types.hpp>
#include <GQE/Core/Core_types.hpp>
#include <TmxParser/TmxMap.h>
#include <TmxParser/TmxTile.h>
#include "TmxAsset.hpp"

class LevelSystem : public GQE::ISystem
{
  public:
    /**
     * LevelSystem default ctor
     * @param[in] theApp address to IApp based class
     * @param[in] theAnimationSystem pointer which will be used for each Prototype
     * @param[in] theMapFilename of the map to load initially
     * @param[in] theLoadingFilename of the Loading...Please Wait screen
     * @param[in] theFontFilename to use when showing the percent complete value
     * @param[in] theScreenTileWidth is the number of tiles to display to the screen
     * @param[in] theScreenTileHeight is the number of tiles to display to the screen
     * @param[in] theLoaderCount is the number of consecutive loader calls in Draw
     */
    LevelSystem(GQE::IApp& theApp,
        GQE::ISystem* theAnimationSystem,
        const GQE::typeAssetID theMapFilename = "",
        const GQE::typeAssetID theLoadingFilename = "",
        const GQE::typeAssetID theFontFilename = "resources/arial.ttf",
        GQE::Uint32 theScreenTileWidth = 32,
        GQE::Uint32 theScreenTileHeight = 24,
        GQE::Uint32 theLoaderCount = 100);

    /**
     * LevelSystem dtor
     */
    virtual ~LevelSystem();

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

    /**
     * SwitchScreen provides a way to switch to a different screen in the level
     * being shown right now. It will first remove each animated tile from the
     * AnimationSystem and then add each new animated tile on the new screen to
     * the AnimationSystem. It will only perform this task if theScreen values
     * provided are within mScreenWith and mScreenHeight.
     * @param[in] theScreen to switch to in the current level/map
     */
    void SwitchScreen(sf::Vector2u theScreen);

    /**
     * SwitchPosition provides a way to move each IEntity registered with this
     * LevelSystem (usually only the local player) to thePosition provided on
     * the same screen in the level being shown right now.
     * @param[in] thePosition to move all registered IEntity classes to
     */
    //void SwitchPosition(sf::Vector2f thePosition);

    /**
     * LoadMap can be called to attempt to load theFilename map provided. If
     * a load of the map is currently in progress than false will be returned
     * otherwise the loading of theFilename provided will begin. The Draw
     * method is responsible for showing a Loading...please wait image during
     * the loading of the level.
     * @param[in] theMapFilename to open and load
     * @param[in] theLoadingFilename to open and load
     */
    bool LoadMap(const GQE::typeAssetID theMapFilename,
        const GQE::typeAssetID theLoadingFilename);

  protected:
    /**
     * UpdateCoordinates is responsible for updating theEntity provided using
     * its vPosition, wScreen, and rBoundingBox properties and the following
     * equations.
     * TileCenter.x = ((vPosition.x + rBoundingBox.left + rBoundingBox.width / 2) /
     *           Tile.width) % ScreenTile.width
     * TileCenter.y = ((vPosition.y + rBoundingBox.top + rBoundingBox.height / 2) / 
     *           Tile.height) % ScreenTile.height
     * TileLeft = ((vPosition.x + vVelocity.x + rBoundingBox.left) /
     *           Tile.width) % ScreenTile.width
     * TileRight = ((vPosition.x + vVelocity.x + rBoundingBox.left + rBoundingBox.width) /
     *           Tile.width) % ScreenTile.width
     * TileUp = ((vPosition.y + vVelocity.y + rBoundingBox.top) / 
     *           Tile.height) % ScreenTile.height
     * TileUp = ((vPosition.y + vVelocity.y + rBoundingBox.top + rBoundingBox.height) / 
     *           Tile.height) % ScreenTile.height
     * MapC.x = Tile.x + wScreen.x * ScreenTile.width
     * MapC.y = Tile.y + wScreen.y * ScreenTile.height
     * @param[in] theEntity to update the map coordinates for
     */
    void UpdateCoordinates(GQE::IEntity* theEntity);

    /**
     * CheckTreasure is responsible for checking theEntity provided against
     * each treasure tile to see if they can pick it up.
     * @param[in] theEntity to check treasure against
     */
    void CheckTreasure(GQE::IEntity* theEntity);

    /**
     * CheckWalls is responsible for checking theEntity provided against the
     * 2D boolean wall index to determine if they will hit a wall and zero
     * the vVelocity values to prevent collisions with the wall.
     * @param[in] theEntity to check 2D boolean wall index against
     */
    void CheckWalls(GQE::IEntity* theEntity);

    /**
     * CheckScreenEdges is responsible for checking theEntity provided against
     * the edge of the screen to determine if the player should switch to a new
     * screen.
     * @param[in] theEntity to check screen edges against
     */
    void CheckScreenEdges(GQE::IEntity* theEntity);

    /**
     * DrawBar is responsible for drawing the percent complete bar using
     * thePercent value provided. A default text implementation is provided if
     * no class derives from LevelSystem and overrides this method.
     * @param[in] thePercent complete as a floating point value from 0.0 - 1.0
     */
    virtual void DrawBar(void);

    /**
     * DrawTiles is responsible for drawing the tiles for the screen of the
     * local player.
     */
    virtual void DrawTiles(void);

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
    // Enumerations and Typedefs
    /////////////////////////////////////////////////////////////////////////
    enum LoadStage {
      UnknownStage = 0, ///< Unknown loading stage
      TilesetStage = 1, ///< The tileset image loading stage
      TileStage    = 2, ///< The tile stage
      ObjectStage  = 3, ///< The Object layer stage
      WaitingStage = 4, ///< The Waiting stage before the game begins
      CleanupStage = 5  ///< The Cleanup stage as the game begins
    };

    // Struct to hold all values needed to load a map
    typedef struct sLoadContext {
      LoadStage          stage;    ///< The current stage we are processing now
      TmxAsset           asset;    ///< The map file which is of type Tmx
      Tmx::Map&          map;      ///< The Tmx::Map object from TmxAsset above
      GQE::ImageAsset    loading;  ///< The Loading, Please Wait background screen to display
      GQE::ImageAsset*   tilesets; ///< An array of ImageAssets for each Tileset in the map
      int                tileset;  ///< Which tileset we are loading right now
      int                layer;    ///< Which layer we are loading right now
      int                group;    ///< Which group we are loading right now
      int                object;   ///< Which object we are loading right now
      int                x;        ///< Which x coordinate for the tile we are loading right now
      int                y;        ///< Which y coordinate for the tile we are loaidng right now
      GQE::Uint32        total;    ///< The total used to determine percent complete
      float              percent;  ///< The computed percent complete for each stage
      sLoadContext(GQE::typeAssetID theMapFilename,
          GQE::typeAssetID theLoadingFilename) :
        stage(UnknownStage),
        asset(theMapFilename, GQE::AssetLoadNow),
        map(asset.GetAsset()),
        loading(theLoadingFilename, GQE::AssetLoadNow),
        tilesets(NULL),
        tileset(0),
        layer(0),
        group(0),
        object(0),
        x(0),
        y(0),
        total(1),
        percent(0.0f)
      {
      }
    } LoadContext;

    typedef struct sScreenInfo {
      std::map<const GQE::Uint32, std::deque<GQE::IEntity*> > tiles;
      std::deque<GQE::IEntity*> walls;
      std::deque<GQE::IEntity*> treasures;
    } ScreenInfo;

    // Variables
    /////////////////////////////////////////////////////////////////////////
    GQE::ISystem*      mAnimationSystem;
    GQE::Prototype     mTile;
    GQE::Prototype     mObject;
    GQE::ImageAsset*   mTilesets;
    GQE::SoundAsset*   mSounds;
    GQE::Uint32        mScreenTileWidth;
    GQE::Uint32        mScreenTileHeight;
    GQE::Uint32        mScreenWidth;
    GQE::Uint32        mScreenHeight;
    GQE::Uint32        mTileWidth;
    GQE::Uint32        mTileHeight;
    sf::Vector2f       mTileScale;
    GQE::typeAssetID   mMapFilename;
    GQE::typeAssetID   mLoadingFilename;
    sf::Vector2u       mScreen;
    sf::Font           mFont;
    sf::Sound          mBump;
    sf::Sound          mCoin;
    LoadContext*       mLoader;
    GQE::Uint32        mLoaderCount;
    // Map of screens to each z-ordered deque of IEntity* tiles for rendering purposes
    std::map<const GQE::Uint32, ScreenInfo> mScreens;
    std::vector<sf::Vector2f> mPositions;

    /**
     * ResetProperties will set the LevelSystem properties of all IEntity
     * classes to the current values. This is typically done when the level
     * has been unloaded through some other interface than the IEntity
     * interface.
     * @param[in] theVisible value to use for each registered IEntity value
     */
    //void ResetProperties(bool theVisible);

    /**
     * DropAllScreens is responsible for unloading and deleting each screen
     * deque. This is called when loading a new map or by the destructor.
     */
    void DropAllScreens(void);

    /**
     * LoadScreen is responsible for adding each Instance to the RenderSystem
     * to theScreen specified.
     * @param[in] theScreen to load by adding each Instance to mRenderSystem
     */
    void LoadScreen(sf::Vector2u theScreen);

    /**
     * UnloadScreen is responsible for dropping each Instance from the
     * RenderSystem for theScreen specified.
     * @param[in] theScreen to unload
     */
    void UnloadScreen(sf::Vector2u theScreen);

    /**
     * LoadStage1 will be called by the Draw method to perform stage 1 of the
     * loading process. This stage is responsible for loading each tileset
     * which is used to represent each tile in the map. By providing loading in
     * a multi-stage technique we can provided a Loading...please wait
     * mechanism with a progress bar.
     */
    void LoadStage1(void);

    /**
     * LoadStage2 will be called by the Draw method to perform stage 2 of the
     * loading process. This stage is responsible for loading each tile or
     * object that will be drawn. 
     */
    void LoadStage2(void);

    /**
     * LoadStage3 will be called by the Draw method to perform stage 3 of the
     * loading process. This stage is responsible for completing any final
     * steps after the tiles and objects have been loaded.
     * @return true if this loading stage has been completed
     */
    void LoadStage3(void);

    /**
     * LoadStage4 will be called by the Draw method to perform stage 4 of the
     * loading process. This stage is responsible for informing all of the other
     * players that we have finished loading our map and waiting for them to
     * complete loading their maps.
     */
    void LoadStage4(void);

    /**
     * LoadStage5 will be called by the Draw method to perform stage 5 of the
     * loading process. This stage is responsible for enabling the game to
     * begin since all players have finally loaded their maps.
     */
    void LoadStage5(void);

    /**
     * LoadProperties is responsible for adding each property as a property of
     * theEntity provided. Property names begin with a special letter
     * that determines the type of each property. These include i=GQE::Int32,
     * u=GQE::Uint32, f=float, b=boolean, s=string, etc.
     * @param[in] theProperties to processs
     * @param[in] theEntity to store each property into
     */
    void LoadProperties(std::map<std::string, std::string> theProperties,
        GQE::IEntity* theEntity); 
};
#endif // LEVEL_CONTROL_SYSTEM_HPP_INCLUDED

/**
 * @class GQE::LevelSystem
 * The LevelSystem class is used to manage the current level/map that has been
 * loaded. The properties provided by this ISystem are as follows:
 * - sLevelMap is the current map level loaded or the desired map level to load
 * - sLevelLoading is the image loading screen filename to use when loading a map level
 * - sLevelFont is the font that will be used when displaying the percent complete
 * - uLevelScreen is the screen to display after loading the map level
 * - vLevelPosition is the position to put this IEntity after loading the map level
 * The map wide properties value provided may override properties set by other
 * registered ISystems for each IEntity registered with the LevelSystem
 * (typically the player IEntity classes). The layer wide properties will be
 * loaded into each tile of that layer but may be overrided by a tile specific
 * value for the same property. Each object group property will be loaded into
 * each object of that object group but may be overrided by an object specific
 * value for the same property.
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
