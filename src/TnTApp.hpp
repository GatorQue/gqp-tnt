/**
 * Provides the TnTApp class which implements the Traps and Treasures example
 * game for the GQP/GQE projects.
 *
 * @file src/TnTApp.hpp
 * @author Ryan Lindeman
 * @date 20120707 - Initial Release
 * @date 20120730 - Improved network synchronization for multiplayer game play
 */
#ifndef   T_N_T_APP_HPP_INCLUDED
#define   T_N_T_APP_HPP_INCLUDED

#include <SFML/Network.hpp>
#include <GQE/Core/interfaces/IApp.hpp>

/// Provides the core game loop algorithm for all game engines.
class TnTApp : public GQE::IApp
{
  public:
    // Variables
    /////////////////////////////////////////////////////////////////////////
    /// The client socket for this application
    sf::UdpSocket mClient;
    /// Randomly selected client ID value for this client
    GQE::Uint32   mClientID;

    /**
     * TnTApp constructor
     * @param[in] theTitle is the title of the window
     */
    TnTApp(const std::string theTitle = "Traps and Treasures");

    /**
     * TnTApp deconstructor
     */
    virtual ~TnTApp();

  protected:
    /**
     * InitAssetHandlers is responsible for registering custom IAssetHandler
     * derived classes for a specific game application.
     */
    virtual void InitAssetHandlers(void);

    /**
     * InitScreenFactory is responsible for initializing any IScreen derived
     * classes with the ScreenManager class that will be used to create new
     * IScreen derived classes as requested.
     */
    virtual void InitScreenFactory(void);

    /**
     * HandleCleanup is responsible for performing any custom last minute
     * Application cleanup steps before exiting the Application.
     */
    virtual void HandleCleanup(void);

  private:
}; // class RedRubyApp

#endif // T_N_T_APP_HPP_INCLUDED
/**
 * @class TnTApp
 * @ingroup Examples
 * @section DESCRIPTION
 * The TnTApp class is the App class for the Traps and Treasures LPC contest
 * submission and example game for the GQP and GQE projects.
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
