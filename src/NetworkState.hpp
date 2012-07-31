/**
 * Provides the Network Game Join and Start State for Traps and Treasures.
 *
 * @file src/NetworkState.hpp
 * @author Ryan Lindeman
 * @date 20120712 - Initial Release
 * @date 20120730 - Improved network synchronization for multiplayer game play
 */

#ifndef   NETWORK_STATE_HPP_INCLUDED
#define   NETWORK_STATE_HPP_INCLUDED

#include <list>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <GQE/Core/Core_types.hpp>
#include <GQE/Core/interfaces/IState.hpp>
#include <GQE/Entity/systems/AnimationSystem.hpp>
#include <GQE/Entity/systems/RenderSystem.hpp>
#include <GQE/Entity/classes/Prototype.hpp>

// Forward declare our TnTApp class
class TnTApp;

class NetworkState : public GQE::IState
{
  public:
    /**
     * NetworkState constructor
     * @param[in] theApp is an address to the App class.
     * @param[in] theCharacter is the IEntity for the current player
     */
    NetworkState(TnTApp& theApp);

    /**
     * NetworkState deconstructor
     */
    virtual ~NetworkState(void);

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
    /// Constant representing the game port to use for network game
    static const unsigned int  GAME_SERVER_PORT = 55000;
    typedef struct {
      sf::IpAddress    addr;
      unsigned short   port;
      GQE::typeAssetID assetID;
    } typeClientInfo;
    /// The TnTApp address used by this state
    TnTApp&                    mTnTApp;
    /// The animation system for our players and treasures
    GQE::AnimationSystem       mAnimationSystem;
    /// The render system for handling rendering of tiles, players, etc
    GQE::RenderSystem          mRenderSystem;
    /// The prototype system for creating players
    GQE::Prototype             mPlayer;
    /// The list of players that will be in our game
    std::map<const GQE::Uint32, typeClientInfo> mPlayers;
    /// The player image for the local player
    GQE::typeAssetID           mPlayerImage;
    /// The container to hold each players images as they join the game
#if (SFML_VERSION_MAJOR < 2)
    std::vector<sf::Image*>    mPlayerImages;
#else
    std::vector<sf::Texture*>  mPlayerImages;
#endif
    /// Keep track of the number of players as they join while we wait
    GQE::Uint32                mPlayerCount;
    /// The background image giving instructions on waiting, joining, and starting the game
    GQE::ImageAsset            mBackground;

    /// True if the server socket is bound and active
    bool                       mServerActive;
    /// The server socket if no one on this PC has already bound it
    sf::UdpSocket              mServer;

    /**
     * AddPlayer is responsible for adding each player as they join the network
     * game waiting for players. After each player has joined the network game
     * during the waiting period someone presses the Spacebar to start the game.
     * @param[in] theID is the ID of the new network player
     * @param[in] theAddress is the IP Address of the new network player
     * @param[in] thePort is the IP Address port of the new network player
     * @param[in] theAssetID to use to represent this network player in the game
     */
    void AddPlayer(GQE::Uint32 theID, sf::IpAddress theAddress,
      unsigned short thePort, GQE::typeAssetID theAssetID);

    /**
     * ProcessClients is responsible for processing all client messages and
     * echoing them to all other clients that have registered with the active
     * server.
     */
    void ProcessClients(void);

    /**
     * SendJoinRequest is responsible for sending a broadcast message to the game
     * server port requesting to join a game.
     */
    void SendJoinRequest(void);

    /**
     * ProcessMessages is responsible for processing messages sent from the
     * server informing us of each new client.
     */
    void ProcessMessages(void);
}; // class NetworkState

#endif // NETWORK_STATE_HPP_INCLUDED

/**
 * @class NetworkState
 * @ingroup Examples
 * The NetworkState class provides an opportunity to wait for each network
 * player to join the network game. Once each player has joined each player
 * should press the space bar to prevent any other players from joining and
 * begin the game.
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
