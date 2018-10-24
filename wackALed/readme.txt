This is the Wack-A-LED's game source code!

It implements a game modelled as a finite state machine, where user reflexes are measured by storing the time it takes the user to press the correct button when one of the game's LEDs is switched on. It takes into account errors and timeouts, adding the corresponding penalties to the stored time. After the preset number of rounds are finished or the maximum number of mistakes are reached, the game displays the user's results via serial interface.
