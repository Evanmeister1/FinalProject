#include <Adafruit_CircuitPlayground.h> // Include the Adafruit Circuit Playground library

int gameState = 0; // Game state variable: 0: IDLE, 1: PLAY_SEQUENCE, 2: USER_INPUT, 3: VICTORY, 4: END
int sequenceLength = 2; // Initial sequence length
int sequence[100]; // Array to store the sequence, with a maximum length of 100
int userInput[100]; // Array to store the user's input
int userInputIndex = 0; // Index for tracking the user's input position
int score = 0; // Variable to track the user's score
int totalRounds = 5; // Total number of rounds to play

const int LEFT_TONE = 262; // Frequency for the left tone
const int RIGHT_TONE = 294; // Frequency for the right tone
const int VICTORY_TONE_1 = 523; // Frequency for the first victory tone
const int VICTORY_TONE_2 = 587; // Frequency for the second victory tone
const int VICTORY_TONE_3 = 659; // Frequency for the third victory tone
const int WIN_TUNE[] = {700, 700, 700, 700, 700}; // Array for the win tune

const int SWITCH_PIN = 7; // Pin number for the switch

volatile bool switchPressed = false; // Flag to indicate if the switch is pressed
volatile bool tonePlaying = false; // Flag to indicate if a tone is currently playing

void setup() {
    pinMode(SWITCH_PIN, INPUT_PULLUP); // Set the switch pin as input with internal pull-up resistor
    CircuitPlayground.begin(); // Initialize the Circuit Playground library
    Serial.begin(9600); 
    randomSeed(analogRead(0)); // Seed the random number generator using an analog read
}

void loop() {
    // Check the state of the switch
    if (digitalRead(SWITCH_PIN) == LOW) { // If the switch is pressed (LOW)
        if (tonePlaying) { // If a tone is currently playing
            CircuitPlayground.playTone(0, 1); // Play a silent tone to stop any tones
            tonePlaying = false; // Set the tonePlaying flag to false
        }
        CircuitPlayground.clearPixels(); // Turn off all LEDs
        delay(50); // Debounce delay for the switch
        switchPressed = true; // Set the switchPressed flag to true
    } else {
        switchPressed = false; // Set the switchPressed flag to false if the switch is not pressed
    }

    // Game logic only runs if the game is not ended by the switch
    if (!switchPressed) {
        if (gameState == 0) { // If the game is in IDLE state
            if (CircuitPlayground.leftButton()) { // If the left button is pressed
                startGame(); // Start the game
            }
        } else if (gameState == 1) { // If the game is in PLAY_SEQUENCE state
            playSequence(); // Play the sequence
        } else if (gameState == 2) { // If the game is in USER_INPUT state
            checkUserInput(); // Check the user's input
        } else if (gameState == 3) { // If the game is in VICTORY state
            if (!CircuitPlayground.leftButton()) { // If the left button is not pressed
                playVictoryTone(); // Play the victory tone
                if (score < totalRounds) { // If the score is less than the total rounds
                    startGame(); // Start the next round
                } else {
                    endGame(); // End the game if all rounds are completed
                }
            }
        } else if (gameState == 4) { // If the game is in END state
            endGame(); // End the game
        }
    }
}

// Start the game and generate a new sequence
void startGame() {
    gameState = 1; // Set the game state to PLAY_SEQUENCE
    if (score < totalRounds) { // If the score is less than the total rounds
        sequenceLength = 2 + score; // Increase the sequence length with each round
    } else {
        sequenceLength = 2; // Reset the sequence length after reaching the desired rounds
    }
    for (int i = 0; i < sequenceLength; ++i) { // Generate the sequence
        sequence[i] = random(2); // 0 for LEFT, 1 for RIGHT
    }
}

// Play the generated sequence
void playSequence() {
    for (int i = 0; i < sequenceLength; ++i) { // Loop through the sequence
        if (sequence[i] == 0) { // If the sequence element is 0
            CircuitPlayground.playTone(LEFT_TONE, 250); // Play the left tone
        } else { // If the sequence element is 1
            CircuitPlayground.playTone(RIGHT_TONE, 250); // Play the right tone
        }
        tonePlaying = true; // Set the tonePlaying flag to true
        for (int j = 0; j < 100; j++) { // Check the switch state periodically during delay
            if (digitalRead(SWITCH_PIN) == LOW) { // If the switch is pressed
                switchPressed = true; // Set the switchPressed flag to true
                break; // Break out of the loop
            }
            delay(5); // Short delay for responsiveness
        }
        if (switchPressed) { // If the switch is pressed during the delay
            tonePlaying = false; // Set the tonePlaying flag to false
            return; // Exit the function
        }
    }
    tonePlaying = false; // Set the tonePlaying flag to false
    userInputIndex = 0; // Reset the user input index
    gameState = 2; // Set the game state to USER_INPUT
}

// Check the user's input against the sequence
void checkUserInput() {
    if (CircuitPlayground.leftButton()) { // If the left button is pressed
        if (!checkInput(0)) return; // Check the input and return if it's incorrect
    } else if (CircuitPlayground.rightButton()) { // If the right button is pressed
        if (!checkInput(1)) return; // Check the input and return if it's incorrect
    }
}

// Validate the user's input and handle game state transitions
bool checkInput(int input) {
    userInput[userInputIndex] = input; // Store the input in the userInput array
    userInputIndex++; // Increment the user input index

    if (input == 0) { // If the input is 0 (left button)
        CircuitPlayground.playTone(LEFT_TONE, 250); // Play the left tone
    } else { // If the input is 1 (right button)
        CircuitPlayground.playTone(RIGHT_TONE, 250); // Play the right tone
    }

    // If the user's input doesn't match the sequence, end the game
    if (input != sequence[userInputIndex - 1]) { // Compare the input to the sequence
        gameState = 4; // Set the game state to END
        return false; // Return false for incorrect input
    }

    // If the user has completed the sequence correctly, proceed to victory
    if (userInputIndex == sequenceLength) { // If the user input matches the sequence length
        score++; // Increment the score
        Serial.print("Score: "); // Print the score to the serial monitor
        Serial.println(score); // Print the score value to the serial monitor
        gameState = 3; // Set the game state to VICTORY
    }
    return true; // Return true for correct input
}

// Play a victory tone sequence
void playVictoryTone() {
    if (!switchPressed) { // Check if switch is not pressed
        delay(500);
        CircuitPlayground.playTone(VICTORY_TONE_1, 200); 
        delay(100);
        CircuitPlayground.playTone(VICTORY_TONE_2, 200); 
        delay(100);
        CircuitPlayground.playTone(VICTORY_TONE_3, 200); 
        delay(1000);
        // Play a win tune if the user has completed all rounds
        if (score == totalRounds) { // If the user has reached the total rounds
            for (int i = 0; i < 3; ++i) { // Loop to play the win tune
                CircuitPlayground.playTone(WIN_TUNE[i], 200); // Play each note in the win tune
                delay(300);
            }
            Serial.println("You Win!"); // Print "You Win!" to the serial monitor
        }
    }
}

// End the game, reset the state, and clear any previous sequences
void endGame() {
    if (score == totalRounds) { // If the user has reached the total rounds
        for (int i = 0; i < 10; ++i) { // Loop to set all LEDs to green
            CircuitPlayground.setPixelColor(i, 0, 255, 0); 
        }
        delay(2000);
        CircuitPlayground.clearPixels(); 
        gameState = 0; // Reset game state to IDLE
        sequenceLength = 2; // Reset sequence length to 2 for round 
