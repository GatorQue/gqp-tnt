/**
 * This is the starting point for all new projects.  This file's purpose is
 * pretty small, but important.  In here we create our application, give the
 * application an opportunity to process command line arguments and begin the
 * application initialization and game loop.
 *
 * @file src/main.cpp
 * @author Ryan Lindeman
 * @date 20120712 - Initial Release
 */

#include <assert.h>
#include <stddef.h>
#include <GQE/Core.hpp>
#include <GQE/Entity.hpp>
#include "TnTApp.hpp"

/**
 * The starting point of the Traps and Treasures LPC application
 * @param[in] argc the number of command line arguments provided
 * @param[in] argv[] the array of command line arguments provided as an array
 * @return the result returned by the application
 */
int main(int argc, char* argv[])
{
  // Default anExitCode to a specific value
  int anExitCode = GQE::StatusNoError;

  // Create a FileLogger and make it the default logger before creating our App
  GQE::FileLogger anLogger("output.txt", true);

  // Create our TnT application.
  GQE::IApp* anApp = new(std::nothrow) TnTApp();
  assert(NULL != anApp && "main() Can't create Application");

  // Process command line arguments
  anApp->ProcessArguments(argc, argv);

  // Start the action application:
  // Initialize the action application
  // Enter the Game Loop where the application will remain until it is shutdown
  // Cleanup the action application
  // Exit back to here
  anExitCode = anApp->Run();

  // Cleanup ourselves by deleting the action application
  delete anApp;

  // Don't keep pointers to objects we have just deleted
  anApp = NULL;

  // return our exit code
  return anExitCode;
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

