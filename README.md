# Bomberman Arduino Game
This is an Arduino-based implementation of the classic game Bomberman. The game features an LCD display for the user interface, a LED matrix for the game field, and a joystick for player control. Players can navigate the menu, set options, and play the Bomberman game.

## Features
- **Game States**: The game has various states, including Hello Screen, Intro Screen, Settings Screen, About Screen, Game Screen, Highscore Screen, Player Name Screen, and End Game Screen.

- **LCD Display**: Utilizes the LiquidCrystal library for displaying information, including player lives, score, and other game details.

- **LED Matrix**: Implements a LED matrix using the LedControl library to represent the game field and entities such as walls, enemies, and the player.

- **Joystick Control**: Uses a joystick for player control, allowing movement and interaction within the game.

- **Settings**: Provides settings for LCD brightness, LED matrix brightness, and volume toggle.

- **Highscore**: Keeps track of the player's score and displays the highscore podium at the end of the game.

## Game Logic
- **Game Initialization**: The game initializes the game field, walls, enemies, and the player's position.

- **Player Movement**: The player's movement is controlled by the joystick. The field of view is calculated based on the player's position.

- **Enemy Movement**: Enemies move randomly within the game field, and collisions with the player result in health reduction.

- **Bomb Mechanism**: Players can place bombs that explode and remove walls and enemies within their blast radius.

- **Game Over**: The game ends when the player's health reaches zero, and the player's score is compared to the highscores.

## Usage
- **Hello Screen**: Upon starting, a welcome message is displayed.

- **Intro Screen**: Players can navigate through the menu options: Start New Game, Settings, About the Game, Highscores, and Enter Name.

- **Player Name Screen**: Allows players to enter their name for highscore tracking.

- **Settings Screen**: Adjusts settings such as LCD brightness, LED matrix brightness, and volume toggle.

- **Game Screen**: The main gameplay screen where players navigate the field, place bombs, and avoid enemies.

- **End Game Screen**: Displays the player's score and, if applicable, indicates a new highscore.

## Installation
Download and install the required libraries: EEPROM, LiquidCrystal, and LedControl.

Connect the components (LCD, LED matrix, joystick) to the Arduino board.

Upload the code to your Arduino board.

Enjoy playing Bomberman on your Arduino device!

## Credits
This Bomberman game is created by [Radu Vrinceanu](https://www.linkedin.com/in/radu-vrinceanu/). Check out the repository for updates and additional information.
