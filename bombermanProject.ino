#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <LedControl.h>
#define INIT_MENU_SELECTION 0
#define MAX_NAME_SIZE 3
#define PLAYER_CENTER 3
#define DEFAULT_AXIS_POS 7
#define MAX_ARENA_SIZE 16
#define DIRECTIONS 4
#define BASE_NUMBER_ENEMIES 10
#define SCORE_REMOVE_WALL 1
#define SCORE_REMOVE_ENEMY 3

// Declare the enum of the game status
enum GameStateEnum {
  HELLO_SCREEN,
  INTRO_SCREEN,
  SETTINGS_SCREEN,
  ABOUT_SCREEN,
  GAME_SCREEN,
  HIGHSCORE_SCREEN,
  PLAYER_NAME_SCREEN,
  END_GAME_SCREEN
};
GameStateEnum gameState = HELLO_SCREEN, oldGameState = HELLO_SCREEN;
byte volumeToggle = EEPROM.read(18);
int menuSelection = INIT_MENU_SELECTION;
const unsigned int menuMessageScrollTime = 500, menuScrollTime = 500;
unsigned long currentMessageScrollTime, currentScrollTime;

// Declare the variables for the LCD
const unsigned int rs = 9,
                   en = 8,
                   d4 = 7,
                   d5 = 6,
                   d6 = 5,
                   d7 = 4,
                   pwmPin = 3;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
byte bombChar[8] = {
  0b00110,
  0b01000,
  0b00100,
  0b01110,
  0b11111,
  0b10111,
  0b11111,
  0b01110
};
byte heartChar[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};
byte emptyHeartChar[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b10101,
  0b10001,
  0b01010,
  0b00100,
  0b00000
};
byte clockChar[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b01110,
  0b10101,
  0b10111,
  0b10001,
  0b01110
};
bool showOnceMessage = false;
byte lcdBrightness = EEPROM.read(4);

// Declare the variables for the led matrix
const unsigned int dinPin = 12,
                   clockPin = 11,
                   loadPin = 10;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);
byte matrixBrightness = EEPROM.read(3);

// Declare the variables for the joystick
const unsigned int xAxis = A4,
                   yAxis = A5,
                   swPin = 1;
unsigned int xValue, yValue;
byte lastSwButtonState = LOW, swButtonState = HIGH;
unsigned long lastSwDebounceTime = 0, debounceDelay = 50;

// Declare the variables for the game
static unsigned long gameSeconds = 300,
                     health = 3,
                     score = 0,
                     szEnemy = 0;
bool arena[MAX_ARENA_SIZE][MAX_ARENA_SIZE] = {
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};
int enemies[BASE_NUMBER_ENEMIES + MAX_ARENA_SIZE][2], totalEnemies,
  playerPos[2] = { MAX_ARENA_SIZE / 2, MAX_ARENA_SIZE / 2 }, fovTopLeft[2], fovBotRight[2],
  di[3 * DIRECTIONS] = { -1, 0, 1, 0, -2, 0, 2, 0, -1, 1, -1, 1 }, dj[3 * DIRECTIONS] = { 0, -1, 0, 1, 0, -2, 0, 2, -1, -1, 1, 1 };
bool gamemodeInit = false, playerBlink = false, enemyBlink = false, alreadyHit = false;
unsigned long playerMillisBlink = 0, delayPlayerBlink = 250,
              enemyMillisBlink = 0, delayEnemyBlink = 10,
              currentGameMillis = 0, delayGame = 1000, highscorePodium = 0;
byte playerNamePersistable[3] = { 
  EEPROM.read(0) < (byte)'A' || EEPROM.read(0) > (byte)'Z' ? 'A' : EEPROM.read(0), 
  EEPROM.read(1) < (byte)'A' || EEPROM.read(1) > (byte)'Z' ? 'A' : EEPROM.read(1),  
  EEPROM.read(2) < (byte)'A' || EEPROM.read(2) > (byte)'Z' ? 'A' : EEPROM.read(2), 
};

void computeFovBasedOnPlayerPosition() {
  fovTopLeft[0] = playerPos[0] - PLAYER_CENTER < 0 ? 0 : (playerPos[0] - PLAYER_CENTER > (MAX_ARENA_SIZE / 2) ? 8 : playerPos[0] - PLAYER_CENTER);
  fovTopLeft[1] = playerPos[1] - PLAYER_CENTER < 0 ? 0 : (playerPos[1] - PLAYER_CENTER > (MAX_ARENA_SIZE / 2) ? 8 : playerPos[1] - PLAYER_CENTER);

  fovBotRight[0] = fovTopLeft[0] + (MAX_ARENA_SIZE / 2 - 1) >= MAX_ARENA_SIZE ? MAX_ARENA_SIZE - 1 : fovTopLeft[0] + (MAX_ARENA_SIZE / 2 - 1);
  fovBotRight[1] = fovTopLeft[1] + (MAX_ARENA_SIZE / 2 - 1) >= MAX_ARENA_SIZE ? MAX_ARENA_SIZE - 1 : fovTopLeft[1] + (MAX_ARENA_SIZE / 2 - 1);
}

// Declare a function for game initialization walls, enemies, etc..
bool isProtectionZone(unsigned int i, unsigned int j) {
  for (unsigned int x = 0; x < DIRECTIONS; x++) {
    if (i + di[x] == MAX_ARENA_SIZE / 2 && j + dj[x] == MAX_ARENA_SIZE / 2) {
      return true;
    }
  }
  return false;
}

void onGameFinish() {
  gameSeconds = 0;
  highscorePodium = 0;
  if (EEPROM.read(9) <= score) {
    EEPROM.update(6, EEPROM.read(0));
    EEPROM.update(7, EEPROM.read(1));
    EEPROM.update(8, EEPROM.read(2));
    EEPROM.update(9, score);
    highscorePodium = 1;
  } else if (EEPROM.read(9) > score && EEPROM.read(13) <= score) {
    EEPROM.update(10, EEPROM.read(0));
    EEPROM.update(11, EEPROM.read(1));
    EEPROM.update(12, EEPROM.read(2));
    EEPROM.update(13, score);
    highscorePodium = 2;
  } else if (EEPROM.read(13) > score && EEPROM.read(17) <= score) {
    EEPROM.update(14, EEPROM.read(0));
    EEPROM.update(15, EEPROM.read(1));
    EEPROM.update(16, EEPROM.read(2));
    EEPROM.update(17, score);
    highscorePodium = 3;
  }

  gameState = END_GAME_SCREEN;
  lcdShowMenu();
}

void onGameInit() {
  bool resetArena[MAX_ARENA_SIZE][MAX_ARENA_SIZE] = {
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
  };

  for (unsigned int i = 0; i < MAX_ARENA_SIZE; i++) {
    for (unsigned int j = 0; j < MAX_ARENA_SIZE; j++) {
      arena[i][j] = resetArena[i][j];
    }
  }

  for (unsigned int i = 1; i < MAX_ARENA_SIZE - 1; i++) {
    for (unsigned int j = 1; j < MAX_ARENA_SIZE - 1; j++) {
      if (isProtectionZone(i, j)) {
        continue;
      }

      arena[i][j] = (random(5) == 0) ? 1 : 0;
    }
  }

  totalEnemies = BASE_NUMBER_ENEMIES + random(MAX_ARENA_SIZE);
  for (unsigned int i = 1; i < MAX_ARENA_SIZE - 1 && szEnemy <= totalEnemies; i++) {
    for (unsigned int j = 1; j < MAX_ARENA_SIZE - 1 && szEnemy <= totalEnemies; j++) {
      if (arena[i][j] || random(2) == 0 || isProtectionZone(i, j)) {
        continue;
      }

      enemies[szEnemy][0] = i, enemies[szEnemy][1] = j, szEnemy++;
    }
  }

  health = 3, score = 0, gameSeconds = 300;
  playerPos[0] = playerPos[1] = MAX_ARENA_SIZE / 2;
  gamemodeInit = true;
}

void onEnemyMove() {
  for (unsigned int i = 0; i < szEnemy; i++) {
    int randomPosition = random(DIRECTIONS),
        currentEnemyX = enemies[i][0],
        currentEnemyY = enemies[i][1],
        newEnemyX = currentEnemyX + di[randomPosition],
        newEnemyY = currentEnemyY + dj[randomPosition];

    bool hasFoundPositionX = false, hasFoundPositionY = false;
    for (unsigned int j = 0; j < szEnemy && (!hasFoundPositionX || !hasFoundPositionY); j++) {
      hasFoundPositionX = hasFoundPositionY = false;
      int otherEnemyX = enemies[j][0],
          otherEnemyY = enemies[j][1];

      hasFoundPositionX = otherEnemyX == newEnemyX;
      hasFoundPositionY = otherEnemyY == newEnemyY;
    }

    if (arena[newEnemyX][newEnemyY] || (hasFoundPositionX && hasFoundPositionY)) {
      continue;
    }

    if (playerPos[0] == newEnemyX && playerPos[1] == newEnemyY) {
      alreadyHit = true;
      health--;
    }

    enemies[i][0] = newEnemyX, enemies[i][1] = newEnemyY;
  }
}

void onBombExplodes(unsigned int i, unsigned int j) {
  for (unsigned int x = 0; x < 2 * DIRECTIONS; x++) {
    int currentX = i + di[x], currentY = j + dj[x];
    if (currentX < MAX_ARENA_SIZE - 1 && currentX > 0 && currentY > 0 && currentY < MAX_ARENA_SIZE - 1) {
      if (arena[currentX][currentY]) {
        arena[currentX][currentY] = 0;
        score += SCORE_REMOVE_WALL;
        lcdShowMenu();
      }
    }

    int foundEnemyIndex = -1;
    for (unsigned int y = 0; y < szEnemy && foundEnemyIndex == -1; y++) {
      if (enemies[y][0] == currentX && enemies[y][1] == currentY) {
        foundEnemyIndex = j;
      }
    }

    if (foundEnemyIndex != -1) {
      for (unsigned int y = foundEnemyIndex + 1; y < szEnemy; y++) {
        enemies[y - 1][0] = enemies[y][0], enemies[y - 1][1] = enemies[y][1];
      }
      szEnemy--;
      score += SCORE_REMOVE_ENEMY;
      lcdShowMenu();
    }
  }
}

// Declare a function to show a menu according to the screen
void lcdShowMenu() {
  if (oldGameState != gameState || gameState == HELLO_SCREEN) {
    lcd.begin(16, 2);
    lcd.clear();
    lcd.setCursor(0, 0);
  }

  switch (gameState) {
    case PLAYER_NAME_SCREEN:
      {
        if (!showOnceMessage) {
          lcd.clear();
          lcd.noBlink();
          lcd.setCursor(0, 0);
          lcd.print("Enter name");

          showOnceMessage = true;
        }

        switch (menuSelection) {
          case 3:
            {
              lcd.noBlink();
              clearLcdLine(1);
              lcd.setCursor(0, 1);
              lcd.print("Back");
              lcd.setCursor(0, 1);
              break;
            }
          default:
            {
              lcd.blink();
              lcd.setCursor(0, 1);
              char message[16];
              sprintf(message, "> %c%c%c", playerNamePersistable[0], playerNamePersistable[1], playerNamePersistable[2]);
              lcd.print(message);
              lcd.setCursor(menuSelection + 2, 1);
              break;
            }
        }

        break;
      }
    case END_GAME_SCREEN:
      {
        char endGameMessage[32];
        sprintf(endGameMessage, "Congrats %c%c%c! Score: %03d", EEPROM.read(0), EEPROM.read(1), EEPROM.read(2), score);

        lcd.clear();
        lcd.noBlink();
        lcd.setCursor(0, 0);
        lcd.print(endGameMessage);
        size_t length = strlen(endGameMessage);
        while (length != (1 << 4)) {
          if ((millis() - currentMessageScrollTime) > menuMessageScrollTime) {
            currentMessageScrollTime = millis();
            length--;

            lcd.scrollDisplayLeft();
          }
        }
        delay(250);

        lcd.clear();
        lcd.noBlink();
        lcd.setCursor(0, 0);
        if (highscorePodium) {
          char highscoreMessage[32];
          sprintf(highscoreMessage, "NEW HIGHSCORE PLACE: %d", highscorePodium);
          lcd.print(highscoreMessage);

          length = strlen(highscoreMessage);
          while (length != (1 << 4)) {
            if ((millis() - currentMessageScrollTime) > menuMessageScrollTime) {
              currentMessageScrollTime = millis();
              length--;

              lcd.scrollDisplayLeft();
            }
          }
        }
        delay(250);

        lc.clearDisplay(0);
        for (unsigned int i = 0; i < 8; i++) {
          for (unsigned int j = 0; j < 8; j++) {
            lc.setLed(0, i, j, HIGH);
          }
        }

        gameState = INTRO_SCREEN;
        showOnceMessage = false;
        break;
      }
    case GAME_SCREEN:
      {
        if (!showOnceMessage) {
          lcd.clear();
          lcd.noBlink();
          lcd.setCursor(0, 0);
          lcd.print("Lives: ");

          lcd.setCursor(11, 0);
          lcd.write(byte(3));
          lcd.print(":");
          showOnceMessage = true;

          lcd.setCursor(0, 1);
          lcd.print("Score: ");
        }

        lcd.setCursor(13, 0);
        char timer[3];
        sprintf(timer, "%03d", gameSeconds);
        lcd.print(timer);

        lcd.setCursor(7, 0);
        for (unsigned int i = 1; i <= health; i++) {
          lcd.write(byte(1));
        }
        for (unsigned int i = 1; i <= 3 - health; i++) {
          lcd.write(byte(2));
        }

        for (unsigned int i = 7; i <= 15; i++) {
          lcd.setCursor(i, 1);
          lcd.print(" ");
        }
        lcd.setCursor(7, 1);
        lcd.print(score);
        break;
      }
    case HELLO_SCREEN:
      {
        char *playerName = new char[MAX_NAME_SIZE];
        sprintf(playerName, "%c%c%c", EEPROM.read(0), EEPROM.read(1), EEPROM.read(2));

        if (!strcmp(playerName, "000")) {
          const char welcomeMessage[] = "Hello! Welcome to the bomberman game.";
          size_t welcomeMessageLength = strlen(welcomeMessage);

          lcd.print(welcomeMessage);

          currentMessageScrollTime = millis();
          while (welcomeMessageLength != (1 << 4)) {
            if ((millis() - currentMessageScrollTime) > menuMessageScrollTime) {
              currentMessageScrollTime = millis();
              welcomeMessageLength--;

              lcd.scrollDisplayLeft();
            }
          }
        } else {
          char customPlayerMessage[32];
          sprintf(customPlayerMessage, "Good to see you back, %s!", playerName);
          size_t customPlayerMessageLength = strlen(customPlayerMessage);

          lcd.print(customPlayerMessage);

          currentMessageScrollTime = millis();
          while (customPlayerMessageLength != (1 << 4)) {
            if ((millis() - currentMessageScrollTime) > menuMessageScrollTime) {
              currentMessageScrollTime = millis();
              customPlayerMessageLength--;

              lcd.scrollDisplayLeft();
            }
          }
        }

        if (playerName != NULL) {
          delete[] playerName;
        }
        break;
      }
    case INTRO_SCREEN:
      {
        if (!showOnceMessage) {
          lcd.setCursor(2, 0);
          lcd.write(byte(0));
          lcd.print("Bomberman");
          lcd.write(byte(0));

          showOnceMessage = true;
        }

        lcd.blink();
        switch (menuSelection) {
          case 0:
            {
              clearLcdLine(1);
              lcd.setCursor(0, 1);
              lcd.print("> Start new game");
              lcd.setCursor(0, 1);
              break;
            }
          case 1:
            {
              clearLcdLine(1);
              lcd.setCursor(0, 1);
              lcd.print("> Settings");
              lcd.setCursor(0, 1);
              break;
            }
          case 2:
            {
              clearLcdLine(1);
              lcd.setCursor(0, 1);
              lcd.print("> About the game");
              lcd.setCursor(0, 1);
              break;
            }
          case 3:
            {
              clearLcdLine(1);
              lcd.setCursor(0, 1);
              lcd.print("> Highscores");
              lcd.setCursor(0, 1);
              break;
            }
          case 4:
            {
              clearLcdLine(1);
              lcd.setCursor(0, 1);
              lcd.print("> Enter name");
              lcd.setCursor(0, 1);
              break;
            }
        }
        break;
      }
    case ABOUT_SCREEN:
      {
        lcd.clear();
        lcd.noBlink();
        lcd.setCursor(0, 0);
        lcd.print("Get more info:");
        lcd.setCursor(0, 1);
        lcd.print("link: t.ly/vVdh_");
        break;
      }
    case SETTINGS_SCREEN:
      {
        switch (menuSelection) {
          case 0:
            {
              if (!showOnceMessage) {
                lcd.clear();
                lcd.noBlink();
                lcd.setCursor(0, 0);
                lcd.print("LCD Brightness");

                showOnceMessage = true;
              }
              clearLcdLine(1);
              lcd.setCursor(0, 1);
              lcd.print("> ");
              lcd.print(lcdBrightness);
              break;
            }
          case 1:
            {
              if (!showOnceMessage) {
                lcd.clear();
                lcd.noBlink();
                lcd.setCursor(0, 0);
                lcd.print("LED Brightness");

                showOnceMessage = true;
              }
              clearLcdLine(1);
              lcd.setCursor(0, 1);
              lcd.print("> ");
              lcd.print(matrixBrightness);
              break;
            }
          case 2:
            {
              char toggleVolume[16];
              sprintf(toggleVolume, "> Tog volume %s", (!volumeToggle ? "ON" : "OFF"));

              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print(toggleVolume);
              lcd.setCursor(0, 0);
              break;
            }
          case 3:
            {
              lcd.clear();
              lcd.noBlink();
              lcd.setCursor(0, 0);
              lcd.print("Back");
              break;
            }
        }
        break;
      }
    case HIGHSCORE_SCREEN:
      {
        if (!showOnceMessage) {
          lcd.clear();
          lcd.noBlink();
          lcd.setCursor(2, 0);
          lcd.print("Hall of fame");

          showOnceMessage = true;
        }

        char highscoreSelection[16];
        sprintf(highscoreSelection, "%d. %c%c%c       %03d", menuSelection + 1, EEPROM.read(6 + 4 * menuSelection), EEPROM.read(7 + 4 * menuSelection), EEPROM.read(8 + 4 * menuSelection), EEPROM.read(9 + 4 * menuSelection));
        clearLcdLine(1);
        lcd.setCursor(0, 1);
        lcd.print(highscoreSelection);
        lcd.setCursor(0, 1);
        break;
      }
  }
}

void clearLcdLine(int line) {
  lcd.setCursor(0, line);
  for (unsigned int i = 0; i < 20; i++) {
    lcd.print(" ");
  }
}

void debounceSwitchButton() {
  int reading = digitalRead(swPin);
  Serial.print("Reading switch Button: ");
  Serial.println(reading);

  if (reading != lastSwButtonState) {
    lastSwDebounceTime = millis();
  }

  if ((millis() - lastSwDebounceTime) > debounceDelay && lastSwDebounceTime != 0) {
    if (reading != swButtonState) {
      swButtonState = reading;

      if (swButtonState == HIGH) {
        switch (gameState) {
          case GAME_SCREEN:
            {
              onBombExplodes(playerPos[0], playerPos[1]);
              break;
            }
          case INTRO_SCREEN:
            {
              switch (menuSelection) {
                case 0:
                  {
                    gameState = GAME_SCREEN;
                    menuSelection = INIT_MENU_SELECTION;
                    showOnceMessage = false;
                    break;
                  }
                case 1:
                  {
                    gameState = SETTINGS_SCREEN;
                    menuSelection = INIT_MENU_SELECTION;
                    break;
                  }
                case 2:
                  {
                    gameState = ABOUT_SCREEN;
                    break;
                  }
                case 3:
                  {
                    gameState = HIGHSCORE_SCREEN;
                    showOnceMessage = false;
                    menuSelection = INIT_MENU_SELECTION;
                    break;
                  }
                case 4:
                  {
                    gameState = PLAYER_NAME_SCREEN;
                    showOnceMessage = false;
                    menuSelection = INIT_MENU_SELECTION;
                    break;
                  }
              }
              break;
            }
          case PLAYER_NAME_SCREEN:
            {
              switch (menuSelection) {
                case 3:
                  {
                    gameState = INTRO_SCREEN;
                    showOnceMessage = false;
                    menuSelection = INIT_MENU_SELECTION;
                    break;
                  }
                default:
                  {
                    EEPROM.update(0, playerNamePersistable[0]);
                    EEPROM.update(1, playerNamePersistable[1]);
                    EEPROM.update(2, playerNamePersistable[2]);
                  }
              }
              break;
            }
          case HIGHSCORE_SCREEN:
            {
              gameState = INTRO_SCREEN;
              showOnceMessage = false;
              menuSelection = INIT_MENU_SELECTION;
              break;
            }
          case ABOUT_SCREEN:
            {
              gameState = INTRO_SCREEN;
              break;
            }
          case SETTINGS_SCREEN:
            {
              switch (menuSelection) {
                case 0:
                  {
                    EEPROM.update(4, lcdBrightness);
                    break;
                  }
                case 1:
                  {
                    EEPROM.update(3, matrixBrightness);
                    break;
                  }
                case 2:
                  {
                    volumeToggle = (!volumeToggle) ? HIGH : LOW;
                    EEPROM.update(18, volumeToggle);
                    lcdShowMenu();
                    break;
                  }
                case 3:
                  {
                    showOnceMessage = false;
                    gameState = INTRO_SCREEN;
                    menuSelection = 1;
                    break;
                  }
              }
              break;
            }
        }
      }
    }
  }
  lastSwButtonState = reading;
}

void setup() {
  Serial.begin(9600);

  lc.shutdown(0, false);
  lc.setIntensity(0, matrixBrightness);
  lc.clearDisplay(0);
  for (unsigned int i = 0; i < 8; i++) {
    for (unsigned int j = 0; j < 8; j++) {
      lc.setLed(0, i, j, HIGH);
    }
  }

  pinMode(swPin, INPUT_PULLUP);
  pinMode(pwmPin, OUTPUT);
  analogWrite(pwmPin, lcdBrightness);
  lcd.createChar(0, bombChar);
  lcd.createChar(1, heartChar);
  lcd.createChar(2, emptyHeartChar);
  lcd.createChar(3, clockChar);

  gameState = HELLO_SCREEN;
  lcdShowMenu();
  delay(250);  // It doesn't matter if the whole system freezes because it's for the hello message screen not needed for complex delaying with millis

  gameState = INTRO_SCREEN;
}

void loop() {
  xValue = analogRead(xAxis);
  yValue = analogRead(yAxis);

  // Correct the interval of the joystick axis
  xValue = map(xValue, 0, (1 << 10) - 1, 0, (1 << 4) - 1);
  yValue = map(yValue, 0, (1 << 10) - 1, 0, (1 << 4) - 1);

  // Show new LCD screen based on the menu setup
  if (gameState != oldGameState) {
    showOnceMessage = false;
    lcdShowMenu();

    oldGameState = gameState;
  }

  switch (gameState) {
    case PLAYER_NAME_SCREEN:
      {
        if ((millis() - currentScrollTime) > menuScrollTime && (xValue < DEFAULT_AXIS_POS - 1 || xValue > DEFAULT_AXIS_POS + 1)) {
          menuSelection += (xValue < 7) ? -1 : 1, currentScrollTime = millis();

          if (menuSelection < 0) {
            menuSelection = 0;
          } else if (menuSelection > 3) {
            menuSelection = 3;
          }
          lcdShowMenu();
        } else if ((millis() - currentScrollTime) > menuScrollTime && (yValue < DEFAULT_AXIS_POS - 1 || yValue > DEFAULT_AXIS_POS + 1)) {
          if (menuSelection != 3) {
            playerNamePersistable[menuSelection] += (yValue < 7) ? 1 : -1;
            currentScrollTime = millis();
            if (playerNamePersistable[menuSelection] < 'A') {
              playerNamePersistable[menuSelection] = 'A';
            } else if (playerNamePersistable[menuSelection] > 'Z') {
              playerNamePersistable[menuSelection] = 'Z';
            }
          }
          lcdShowMenu();
        }
        break;
      }
    case INTRO_SCREEN:
      {
        if ((millis() - currentScrollTime) > menuScrollTime && (xValue < DEFAULT_AXIS_POS - 1 || xValue > DEFAULT_AXIS_POS + 1)) {
          menuSelection += (xValue < 7) ? -1 : 1, currentScrollTime = millis();

          if (menuSelection < 0) {
            menuSelection = 0;
          } else if (menuSelection > 4) {
            menuSelection = 4;
          }
          lcdShowMenu();
        }
        break;
      }
    case SETTINGS_SCREEN:
      {
        if ((millis() - currentScrollTime) > menuScrollTime && (yValue < DEFAULT_AXIS_POS - 1 || yValue > DEFAULT_AXIS_POS + 1)) {
          switch (menuSelection) {
            case 0:
              {
                matrixBrightness += (yValue < 7) ? 1 : -1, currentScrollTime = millis();
                matrixBrightness = matrixBrightness % 256;
                lc.setIntensity(0, matrixBrightness);

                lcdShowMenu();
                break;
              }
            case 1:
              {
                lcdBrightness += (yValue < 7) ? 1 : -1, currentScrollTime = millis();
                lcdBrightness = lcdBrightness % 256;

                lcdShowMenu();
                break;
              }
          }
        } else if ((millis() - currentScrollTime) > menuScrollTime && (xValue < DEFAULT_AXIS_POS - 1 || xValue > DEFAULT_AXIS_POS + 1)) {
          showOnceMessage = false;
          menuSelection += (xValue < 7) ? -1 : 1, currentScrollTime = millis();

          if (menuSelection < 0) {
            menuSelection = 0;
          } else if (menuSelection > 3) {
            menuSelection = 3;
          }
          lcdShowMenu();
        }
        break;
      }
    case HIGHSCORE_SCREEN:
      {
        if ((millis() - currentScrollTime) > menuScrollTime && (xValue < DEFAULT_AXIS_POS - 1 || xValue > DEFAULT_AXIS_POS + 1)) {
          showOnceMessage = false;
          menuSelection += (xValue < 7) ? -1 : 1, currentScrollTime = millis();

          if (menuSelection < 0) {
            menuSelection = 0;
          } else if (menuSelection > 2) {
            menuSelection = 2;
          }
          lcdShowMenu();
        }
        break;
      }
    case GAME_SCREEN:
      {
        if (!gamemodeInit) {
          onGameInit();
        }
        if (gameSeconds == 0 || !health) {
          onGameFinish();
        }

        computeFovBasedOnPlayerPosition();

        if ((millis() - currentGameMillis) > delayGame && gameSeconds > 0) {
          alreadyHit = false;
          currentGameMillis = millis();
          gameSeconds--;

          lcdShowMenu();
          onEnemyMove();
        }

        if ((millis() - playerMillisBlink) > delayPlayerBlink) {
          playerBlink = !playerBlink;
          playerMillisBlink = millis();
        }
        if ((millis() - enemyMillisBlink) > delayEnemyBlink) {
          enemyMillisBlink = millis();
          enemyBlink = !enemyBlink;
        }

        lc.setLed(0, playerPos[0] - fovTopLeft[0], playerPos[1] - fovTopLeft[1], playerBlink);
        for (unsigned int s = fovTopLeft[0]; s <= fovBotRight[0]; s++) {
          for (unsigned int t = fovTopLeft[1]; t <= fovBotRight[1]; t++) {
            if (s == playerPos[0] && t == playerPos[1]) {
              continue;
            }
            lc.setLed(0, s - fovTopLeft[0], t - fovTopLeft[1], arena[s][t]);

            for (unsigned int e = 0; e < szEnemy; e++) {
              if (enemies[e][0] != s || enemies[e][1] != t) {
                continue;
              }
              lc.setLed(0, s - fovTopLeft[0], t - fovTopLeft[1], enemyBlink);
            }
          }
        }

        if ((millis() - currentScrollTime) > menuScrollTime && (yValue < DEFAULT_AXIS_POS - 1 || yValue > DEFAULT_AXIS_POS + 1)) {
          int coef = (yValue < 7) ? 1 : -1,
              newCoord = playerPos[1] + coef;
          currentScrollTime = millis();

          if (newCoord > 0 && newCoord < MAX_ARENA_SIZE - 1 && !arena[playerPos[0]][newCoord]) {
            playerPos[1] = newCoord;
            for (unsigned int e = 0; e < szEnemy; e++) {
              if (enemies[e][0] == playerPos[0] && enemies[e][1] == playerPos[1]) {
                health -= !alreadyHit;
              }
            }
          }
        } else if ((millis() - currentScrollTime) > menuScrollTime && (xValue < DEFAULT_AXIS_POS - 1 || xValue > DEFAULT_AXIS_POS + 1)) {
          int coef = (xValue < 7) ? -1 : 1,
              newCoord = playerPos[0] + coef;
          currentScrollTime = millis();

          if (newCoord > 0 && newCoord < MAX_ARENA_SIZE - 1 && !arena[newCoord][playerPos[1]]) {
            playerPos[0] = newCoord;
            for (unsigned int e = 0; e < szEnemy; e++) {
              if (enemies[e][0] == playerPos[0] && enemies[e][1] == playerPos[1]) {
                health -= !alreadyHit;
              }
            }
          }
        }
        break;
      }
  }

  // Debounce joystick button
  debounceSwitchButton();
}
