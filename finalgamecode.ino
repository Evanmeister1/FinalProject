#include <Adafruit_CircuitPlayground.h>

int gameState = 0; // 0: IDLE, 1: PLAY_SEQUENCE, 2: USER_INPUT, 3: VICTORY, 4: END
int sequenceLength = 2;
int sequence[100]; // Maximum sequence length is 100
int userInput[100];
int userInputIndex = 0;
int score = 0;
int totalRounds = 5; // Number of rounds to play

const int LEFT_TONE = 262;  // C4
const int RIGHT_TONE = 294; // D4
const int VICTORY_TONE_1 = 523; // C5
const int VICTORY_TONE_2 = 587; // D5
const int VICTORY_TONE_3 = 659; // E5
const int WIN_TUNE[] = {700, 700, 700, 700, 700}; // A4, C5, E5

const int SWITCH_PIN = 7; // Assuming the switch is connected to pin 7

volatile bool switchPressed = false; // Flag to indicate switch state
volatile bool tonePlaying = false; // Flag to indicate if a tone is currently playing

void setup() {
    pinMode(SWITCH_PIN, INPUT_PULLUP); // Set switch pin as input with internal pull-up resistor
    CircuitPlayground.begin();
    Serial.begin(9600);
    randomSeed(analogRead(0)); // Seed the random generator
}

void loop() {
    // Check the state of the switch
    if (digitalRead(SWITCH_PIN) == LOW) {
        // Interrupt any playing tone if the switch is pressed
        if (tonePlaying) {
            CircuitPlayground.playTone(0, 1); // Play silent tone to stop any tones
            tonePlaying = false;
        }
        CircuitPlayground.clearPixels(); // Turn off lights
        delay(50); // Debounce delay
        switchPressed = true; // Set switch flag
    } else {
        switchPressed = false;
    }

    // Game logic only runs if the game is not ended by the switch
    if (!switchPressed) {
        if (gameState == 0) { // IDLE
            if (CircuitPlayground.leftButton()) {
                startGame();
            }
        } else if (gameState == 1) { // PLAY_SEQUENCE
            playSequence();
        } else if (gameState == 2) { // USER_INPUT
            checkUserInput();
        } else if (gameState == 3) { // VICTORY
            if (!CircuitPlayground.leftButton()) { // Check if the switch is still pressed
                playVictoryTone();
                if (score < totalRounds) {
                    startGame();
                } else {
                    endGame();
                }
            }
        } else if (gameState == 4) { // END
            endGame();
        }
    }
}

void startGame() {
    gameState = 1; // PLAY_SEQUENCE
    if (score < totalRounds) {
        sequenceLength = 2 + score; // Increase sequence length with each round
    } else {
        sequenceLength = 2; // Reset sequence length after reaching the desired rounds
    }
    for (int i = 0; i < sequenceLength; ++i) {
        sequence[i] = random(2); // 0 for LEFT, 1 for RIGHT
    }
}

void playSequence() {
    for (int i = 0; i < sequenceLength; ++i) {
        if (sequence[i] == 0) {
            CircuitPlayground.playTone(LEFT_TONE, 250); 
        } else {
            CircuitPlayground.playTone(RIGHT_TONE, 250); 
        }
        tonePlaying = true; // Set tone playing flag
        for (int j = 0; j < 100; j++) { // Check switch state periodically during delay
            if (digitalRead(SWITCH_PIN) == LOW) {
                switchPressed = true;
                break;
            }
            delay(5); // Short delay for responsiveness
        }
        if (switchPressed) { // Check if switch is pressed during delay
            tonePlaying = false;
            return;
        }
    }
    tonePlaying = false; // Reset tone playing flag
    userInputIndex = 0;
    gameState = 2; // USER_INPUT
}

void checkUserInput() {
    if (CircuitPlayground.leftButton()) { // Check if the left touch pad is pressed
        if (!checkInput(0)) return;
    } else if (CircuitPlayground.rightButton()) { // Check if the right touch pad is pressed
        if (!checkInput(1)) return;
    }
}

bool checkInput(int input) {
    userInput[userInputIndex] = input;
    userInputIndex++;

    if (input == 0) {
        CircuitPlayground.playTone(LEFT_TONE, 250);
    } else {
        CircuitPlayground.playTone(RIGHT_TONE, 250); 
    }

    if (input != sequence[userInputIndex - 1]) {
        gameState = 4; // END
        return false;
    }

    if (userInputIndex == sequenceLength) {
        score++;
        Serial.print("Score: ");
        Serial.println(score);
        gameState = 3; // VICTORY
    }
    return true;
}

void playVictoryTone() {
    if (!switchPressed) { // Check if switch is not pressed
        CircuitPlayground.playTone(VICTORY_TONE_1, 200); 
        delay(100);
        CircuitPlayground.playTone(VICTORY_TONE_2, 200);
        delay(100);
        CircuitPlayground.playTone(VICTORY_TONE_3, 200);
        delay(100);
        if (score == totalRounds) {
            for (int i = 0; i < 3; ++i) {
                CircuitPlayground.playTone(WIN_TUNE[i], 200); // Softer tone, shorter duration
                delay(300);
            }
            Serial.println("You Win!");
        }
    }
}

void endGame() {
    if (score == totalRounds) {
        for (int i = 0; i < 10; ++i) {
            CircuitPlayground.setPixelColor(i, 0, 255, 0); // Set all LEDs to green
        }
        delay(2000);
        CircuitPlayground.clearPixels();
        gameState = 0; // Start the next round
        sequenceLength = 2; // Reset sequence length to 2 for round 1
        score = 0; // Reset score for the next round
        Serial.println("Starting Round 1.");
    } else {
        for (int i = 0; i < 10; ++i) {
            CircuitPlayground.setPixelColor(i, 255, 0, 0); // Set all LEDs to red
        }
        delay(2000);
        CircuitPlayground.clearPixels();
        gameState = 0; // Reset game state to IDLE
        sequenceLength = 2; // Reset sequence length
        score = 0; // Reset score
        Serial.println("Game Over. Score reset to 0.");
    }
    for (int i = 0; i < 100; ++i) {
        sequence[i] = -1;
        userInput[i] = -1; // Reset userInput array
    }
}
