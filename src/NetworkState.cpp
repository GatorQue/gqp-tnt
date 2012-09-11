/**
 * Provides the Network Game Join and Start State for Traps and Treasures.
 *
 * @file src/NetworkState.cpp
 * @author Ryan Lindeman
 * @date 20120712 - Initial Release
 * @date 20120730 - Improved network synchronization for multiplayer game play
 * @date 20120910 - Fix SFML v1.6 issues
 */
#include "NetworkState.hpp"
#include <SFML/Graphics.hpp>
#include <GQE/Core/assets/ImageAsset.hpp>
#include <GQE/Core/interfaces/IApp.hpp>
#include <GQE/Entity/classes/Instance.hpp>
#include "TnTApp.hpp"

NetworkState::NetworkState(TnTApp& theApp) :
  GQE::IState("Game",theApp),
  mTnTApp(theApp),
  mAnimationSystem(theApp),
  mRenderSystem(theApp),
  mPlayer("player", 255),
  mPlayerImage(""),
  mPlayerCount(0),
  mBackground("resources/images/network.png", GQE::AssetLoadNow),
  mServerActive(false)
{
  // Bind our game server socket
#if (SFML_VERSION_MAJOR < 2)
  mServerActive = mServer.Bind(GAME_SERVER_PORT);
#else
  sf::Socket::Status anStatus = mServer.bind(GAME_SERVER_PORT);

  // See if we successfully bound the Game Server Port
  mServerActive = (anStatus != sf::Socket::Error);
#endif

  if(mServerActive)
  {
#if (SFML_VERSION_MAJOR < 2)
    // Set our server as non-blocking
    mServer.SetBlocking(false);
#else
    // Set our server as non-blocking
    mServer.setBlocking(false);
#endif

    ILOG() << "NetworkState::ctor() Server Active!" << std::endl;
  }
  else
  {
    ILOG() << "NetworkState::ctor() Server Inactive!" << std::endl;
  }
}

NetworkState::~NetworkState(void)
{
  ILOG() << "NetworkState::dtor()" << std::endl;
}

void NetworkState::DoInit(void)
{
  // First call our base class implementation
  IState::DoInit();

  // Enable graphics and game performance statistics
  mApp.mStatManager.SetShow(true);

  // Register all ISystems for the Player prototype
  mPlayer.AddSystem(&mAnimationSystem);
  mPlayer.AddSystem(&mRenderSystem);

  // Retrieve the character this player has selected in CharacterState
  mPlayerImage = mApp.mProperties.Get<GQE::typeAssetID>("sCharacter");

  // What ID and port were we assigned?
#if (SFML_VERSION_MAJOR < 2)
  ILOG() << "NetworkState::DoInit() ClientID=" << mTnTApp.mClientID << ", port="
    << mTnTApp.mClient.GetPort() << std::endl;

  // Now add this player as the first player
  AddPlayer(mTnTApp.mClientID, sf::IPAddress::GetLocalAddress(),
    mTnTApp.mClient.GetPort(), mPlayerImage);
#else
  ILOG() << "NetworkState::DoInit() ClientID=" << mTnTApp.mClientID << ", port="
    << mTnTApp.mClient.getLocalPort() << std::endl;

  // Now add this player as the first player
  AddPlayer(mTnTApp.mClientID, sf::IpAddress::getLocalAddress(),
    mTnTApp.mClient.getLocalPort(), mPlayerImage);
#endif
}

void NetworkState::ReInit()
{
}

void NetworkState::HandleEvents(sf::Event theEvent)
{
  // Call our Base class implementation
  IState::HandleEvents(theEvent);

  // Switch to the next character when they press the Space key
  if(
#if (SFML_VERSION_MAJOR < 2)
    (theEvent.Type == sf::Event::KeyReleased) && (theEvent.Key.Code == sf::Key::Space)
#else
    (theEvent.type == sf::Event::KeyReleased) && (theEvent.key.code == sf::Keyboard::Space)
#endif
    )
  {
    // Make note of the number of players for this game
    mTnTApp.mProperties.Add<GQE::Uint32>("uPlayerCount", mPlayers.size());

    // Drop this active state
    mTnTApp.mStateManager.DropActiveState();
  }
}

void NetworkState::UpdateFixed(void)
{
  // Process clients if we are an active server
  if(mServerActive)
  {
    ProcessClients();
  }

  // Allow AnimationSystem to perform its regularly scheduled update
  mAnimationSystem.UpdateFixed();

  // Send Join information to the network
  SendJoinRequest();

  // Process any messages received from the server
  ProcessMessages();
}

void NetworkState::UpdateVariable(float theElapsedTime)
{
}

void NetworkState::Draw(void)
{
  // Get our Background screen which provides instructions for waiting,
  // joining, and starting a game of Traps and Treasures
  sf::Sprite anBackground(mBackground.GetAsset());

#if (SFML_VERSION_MAJOR < 2)
  // Draw our Background screen
  mTnTApp.mWindow.Draw(anBackground);
#else
  // Draw our Background screen
  mTnTApp.mWindow.draw(anBackground);
#endif

  // Allow our RenderSystem to draw our character
  mRenderSystem.Draw();
}

void NetworkState::HandleCleanup(void)
{
#if (SFML_VERSION_MAJOR < 2)
  // Unbind our server socket
  mServer.Unbind();
#else
  // Unbind our server socket
  mServer.unbind();
#endif

  // Now remove all of our images we have collected
#if (SFML_VERSION_MAJOR < 2)
  std::vector<sf::Image*>::iterator anIter;
#else
  std::vector<sf::Texture*>::iterator anIter;
#endif
  anIter = mPlayerImages.begin();
  while(anIter != mPlayerImages.end())
  {
    // Get the image to be removed
#if (SFML_VERSION_MAJOR < 2)
    sf::Image* anImage = (*anIter);
#else
    sf::Texture* anImage = (*anIter);
#endif

    // Increment iterator to next image
    anIter++;
    
    // Delete the image
    delete anImage;
  }

  // Now clear the vector list of images
  mPlayerImages.clear();
}

#if (SFML_VERSION_MAJOR < 2)
void NetworkState::AddPlayer(GQE::Uint32 theID, sf::IPAddress theAddress,
  unsigned short thePort, GQE::typeAssetID theAssetID)
#else
void NetworkState::AddPlayer(GQE::Uint32 theID, sf::IpAddress theAddress,
  unsigned short thePort, GQE::typeAssetID theAssetID)
#endif
{
  // Is this player not found? then add him now
  if(mPlayers.find(theID) == mPlayers.end())
  {
#if (SFML_VERSION_MAJOR < 2)
    // What ID and port were we assigned?
    ILOG() << "NetworkState::AddPlayer() ID=" << theID << ", addr="
      << theAddress.ToString() << ", port=" << thePort
      << ", assetID=" << theAssetID << std::endl;
#else
    // What ID and port were we assigned?
    ILOG() << "NetworkState::AddPlayer() ID=" << theID << ", addr="
      << theAddress.toString() << ", port=" << thePort
      << ", assetID=" << theAssetID << std::endl;
#endif

    // Create an Instance to represent this player and set its various properties
    GQE::Instance* anInstance = mPlayer.MakeInstance();

    // Did we get a valid Instance? then set some of its properties now
    if(anInstance != NULL)
    {
      // Start with our selected image
#if (SFML_VERSION_MAJOR < 2)
      sf::Image* anImage = new(std::nothrow) sf::Image();
#else
      sf::Texture* anImage = new(std::nothrow) sf::Texture();
#endif

#if (SFML_VERSION_MAJOR < 2)
      anImage->LoadFromFile(theAssetID);
#else
      anImage->loadFromFile(theAssetID);
#endif

      // Add the character to our list of character images chosen
      mPlayerImages.push_back(anImage);

      // Set the player image
      anInstance->mProperties.Set<sf::Sprite>("Sprite", sf::Sprite(*anImage));

      // Get the SpriteRect property from our instance
#if (SFML_VERSION_MAJOR < 2)
      sf::IntRect anSpriteRect(0,64*2,64,64*(2+1));
#else
      sf::IntRect anSpriteRect(0,64*2,64,64);
#endif
      anInstance->mProperties.Set<sf::IntRect>("rSpriteRect", anSpriteRect);

      // Set our visible property
      anInstance->mProperties.Set<bool>("bVisible", true);

      // Set our animation properties
      anInstance->mProperties.Set<float>("fFrameDelay", 0.08f);
      anInstance->mProperties.Set<sf::Vector2u>("wFrameModifier", sf::Vector2u(1,0));
#if (SFML_VERSION_MAJOR < 2)
      anInstance->mProperties.Set<sf::IntRect>("rFrameRect",
          sf::IntRect(0,0,anImage->GetWidth(), anImage->GetHeight()));
#else
      anInstance->mProperties.Set<sf::IntRect>("rFrameRect",
          sf::IntRect(0,0,anImage->getSize().x, anImage->getSize().y));
#endif

      // Compute some valuable constants for placing each player image
#if (SFML_VERSION_MAJOR < 2)
      const GQE::Uint32 anMaxWidth = mApp.mWindow.GetWidth() / anSpriteRect.GetWidth();
#else
      const GQE::Uint32 anMaxWidth = mApp.mWindow.getSize().x / anSpriteRect.width;
#endif
      const GQE::Uint32 anImageX = mPlayerCount % anMaxWidth;
      const GQE::Uint32 anImageY = mPlayerCount / anMaxWidth;

      // Set initial position on the screen to the middle of the screen
#if (SFML_VERSION_MAJOR < 2)
      anInstance->mProperties.Set<sf::Vector2f>("vPosition",
          sf::Vector2f((float)(anImageX * anSpriteRect.GetWidth()),
          (float)((mApp.mWindow.GetHeight() - anSpriteRect.GetHeight()) / 2) +
          (anImageY * anSpriteRect.GetHeight())));
#else
      anInstance->mProperties.Set<sf::Vector2f>("vPosition",
        sf::Vector2f((float)(anImageX * anSpriteRect.width),
        (float)((mApp.mWindow.getSize().y - anSpriteRect.height) / 2) +
        (anImageY * anSpriteRect.height)));
#endif

      // Create a string stream to create our player id tag
      std::string anPlayerID("sPlayerID");
      std::string anPlayerAddr("sPlayerAddr");
      std::string anPlayerPort("uPlayerPort");
      std::string anPlayerAsset("sPlayerAssetID");
      std::ostringstream anPlayerNumber;

      // Create a string version of our player number to append to the above strings
      anPlayerNumber << ++mPlayerCount;

      // Append player number to each player
      anPlayerID.append(anPlayerNumber.str());
      anPlayerAddr.append(anPlayerNumber.str());
      anPlayerPort.append(anPlayerNumber.str());
      anPlayerAsset.append(anPlayerNumber.str());

      // Now register these with our IApp properties
      mTnTApp.mProperties.Add<GQE::Uint32>(anPlayerID, theID);
#if (SFML_VERSION_MAJOR < 2)
      mTnTApp.mProperties.Add<std::string>(anPlayerAddr, theAddress.ToString());
#else
      mTnTApp.mProperties.Add<std::string>(anPlayerAddr, theAddress.toString());
#endif
      mTnTApp.mProperties.Add<unsigned short>(anPlayerPort, thePort);
      mTnTApp.mProperties.Add<GQE::typeAssetID>(anPlayerAsset, theAssetID);

      // Add this new player to our list of players
      mPlayers[theID].addr = theAddress;
      mPlayers[theID].port = thePort;
      mPlayers[theID].assetID = theAssetID;
    }
    else
    {
      // Signal the application to exit
      mApp.Quit(GQE::StatusError);
    }
  }
}

void NetworkState::ProcessClients(void)
{
  // Data packet received from client
  sf::Packet anData;
#if (SFML_VERSION_MAJOR < 2)
  // The IP address of the client
  sf::IPAddress anRemoteAddr;
#else
  // The IP address of the client
  sf::IpAddress anRemoteAddr;
#endif
  // The port of the client
  unsigned short anRemotePort;

#if (SFML_VERSION_MAJOR < 2)
  // Read from the server to get the client information
  sf::Socket::Status anResult = mServer.Receive(anData, anRemoteAddr, anRemotePort);
#else
  // Read from the server to get the client information
  sf::Socket::Status anResult = mServer.receive(anData, anRemoteAddr, anRemotePort);
#endif

  if(anResult == sf::Socket::Done)
  {
    // The client ID that is speaking to us
    GQE::Uint32 anClientID;
    // The client IP address as a string
    std::string anClientAddr;
    // The client port
    unsigned short anClientPort;
    // The client asset ID to the character they want to use
    GQE::typeAssetID anAssetID;
    // Retrieve the data from the prospective client
    anData >> anClientID;
    anData >> anClientAddr;
    anData >> anClientPort;
    anData >> anAssetID;

    // What ID and port were we assigned?
    //ILOG() << "NetworkState::ProcessClients() received ID=" << anClientID
    //  << ", addr=" << anClientAddr << ", port=" << anClientPort
    //  << ", assetID=" << anAssetID << std::endl;

    // Send all previously registered players to the new player joining
    std::map<const sf::Uint32, typeClientInfo>::iterator anIter;
    anIter = mPlayers.begin();
    while(anIter != mPlayers.end())
    {
      // Prepare a reply for each previously registered client
      sf::Packet anReply;
      anReply << anIter->first;
#if (SFML_VERSION_MAJOR < 2)
      anReply << anIter->second.addr.ToString();
#else
      anReply << anIter->second.addr.toString();
#endif
      anReply << anIter->second.port;
      anReply << anIter->second.assetID;

      //ILOG() << "NetworkState::ProcessClients() sending ID=" << anIter->first
      //  << ", addr=" << anIter->second.addr.toString()
      //  << ", port=" << anIter->second.port
      //  << ", assetID=" << anIter->second.assetID << std::endl;

#if (SFML_VERSION_MAJOR < 2)
      // Send an acknowlegement message back to the client
      mServer.Send(anReply, anRemoteAddr, anRemotePort);
#else
      // Send an acknowlegement message back to the client
      mServer.send(anReply, anRemoteAddr, anRemotePort);
#endif

      // Move on to the next client registered
      anIter++;
    }

    // If this is not a local player, try and add him now
    if(anClientID != mTnTApp.mClientID)
    {
      AddPlayer(anClientID, anClientAddr, anClientPort, anAssetID);
    }
  }
}

void NetworkState::SendJoinRequest(void)
{
  // Data packet for Join request sent from client
  sf::Packet anJoin;

  // Prepare the Join request packet
  anJoin << mTnTApp.mClientID;    // Start with our personal client ID value
#if (SFML_VERSION_MAJOR < 2)
  anJoin << sf::IPAddress::GetLocalAddress().ToString(); // Local IP Address
  anJoin << mTnTApp.mClient.GetPort(); // Local port that was randomly assigned to us
#else
  anJoin << sf::IpAddress::getLocalAddress().toString(); // Local IP Address
  anJoin << mTnTApp.mClient.getLocalPort(); // Local port that was randomly assigned to us
#endif
  anJoin << mPlayerImage; // Add the player image we have chosen for ourselves

#if (SFML_VERSION_MAJOR < 2)
  // Send a broadcast packet to everyone about ourselves
  mTnTApp.mClient.Send(anJoin, 0xffffffff, GAME_SERVER_PORT);
#else
  // Send a broadcast packet to everyone about ourselves
  mTnTApp.mClient.send(anJoin, sf::IpAddress::Broadcast, GAME_SERVER_PORT);
#endif
}

void NetworkState::ProcessMessages(void)
{
  // Data packet for each message received from the server
  sf::Packet anData;
#if (SFML_VERSION_MAJOR < 2)
  // The senders IP address
  sf::IPAddress anSenderAddr;
#else
  // The senders IP address
  sf::IpAddress anSenderAddr;
#endif
  // The senders port
  unsigned short anSenderPort;

#if (SFML_VERSION_MAJOR < 2)
  // Process the messages from our server
  sf::Socket::Status anResult = mTnTApp.mClient.Receive(anData, anSenderAddr, anSenderPort);
#else
  // Process the messages from our server
  sf::Socket::Status anResult = mTnTApp.mClient.receive(anData, anSenderAddr, anSenderPort);
#endif

  // Did we get a reply? was it from our sever?
  if(anResult == sf::Socket::Done && anSenderPort == GAME_SERVER_PORT)
  {
    // Client ID of another player
    GQE::Uint32 anClientID;
    // IP Address of another player
    std::string anClientAddr;
    // Port of another player
    unsigned short anClientPort;
    // Asset ID the other player will be using
    GQE::typeAssetID anAssetID;

    // Retrieve the data from the prospective client
    anData >> anClientID;
    anData >> anClientAddr;
    anData >> anClientPort;
    anData >> anAssetID;

    // Add network player to our list of players
    AddPlayer(anClientID, anClientAddr, anClientPort, anAssetID);
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
