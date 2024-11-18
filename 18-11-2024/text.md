**Algorithm Diagram for ESP32 Motor Control Project**

### Explanation of the Algorithm:

1. **Initialization**:
   - Initialize GPIO pins for the three buttons and rotary encoder.
   - Setup the LCD display.
   - Establish connection with Firebase.

2. **Main Loop**:
   - Continuously check the state of each button and the rotary encoder.

3. **Button 1: Motor On/Off Control**:
   - If Button 1 is pressed:
     - Toggle the motor state between ON and OFF.
     - Update the LCD to show the current motor state.

4. **Button 2: Speed Setting and Conditional Control**:
   - If Button 1 is OFF:
     - If Button 2 is held for 3 seconds:
       - Allow speed adjustment using the rotary encoder.
       - Display current speed setting on the LCD.
   - If Button 1 is ON:
     - If Button 2 is pressed:
       - Check Button 2’s state (ON/OFF):
         - **If ON**:
           - Stop the motor.
           - Push the recent motor data (speed, direction, etc.) to Firebase.
           - Update the LCD with a “Paused” status.
         - **If OFF**:
           - Resume motor operation at the last set speed.

5. **Button 3: Rotation Direction Control**:
   - If Button 3 is pressed:
     - Toggle motor rotation direction (clockwise/counterclockwise).
     - Update the LCD with the new direction.

6. **Firebase Data Push**:
   - Ensure data (speed, direction, status) is pushed to Firebase when Button 2 pauses the motor.

### Detailed Flow Explanation:

1. **Start**
   - Begin by initializing the ESP32 components (GPIOs, LCD, Firebase).

2. **Read Button 1 State**:
   - If pressed, toggle motor state (ON/OFF).

3. **Check Button 2 State**:
   - If Button 1 is OFF:
     - Hold Button 2 for 3 seconds to activate speed adjustment via rotary encoder.
   - If Button 1 is ON:
     - Press Button 2 to pause/resume the motor and save data to Firebase.

4. **Monitor Button 3**:
   - On press, change motor rotation direction and display the change.

5. **Update LCD**:
   - Update the display based on current motor status, speed, and direction.

6. **Push to Firebase**:
   - Ensure motor data is pushed to Firebase during pause events or relevant changes.

### Diagram Outline:

```
START
  |
[Initialize ESP32 components]
  |
[Read Button 1 State] -- YES --> [Toggle Motor ON/OFF] --> [Update LCD]
  |                                   |                        |
  NO                                  v                        v
  |                             [Read Button 2 State]   [Read Button 3 State]
  |                                   |                        |
  |                                   |                        [Toggle Direction]
  |                                   |                              |
  |                             [Button 1 OFF]                  [Update LCD]
  |                                   |
  |              [Hold Button 2 for 3s] --> [Set Speed via Rotary Encoder]
  |                                   |
  |                             [Button 1 ON]
  |                                   |
  |             [Press Button 2] --> [Pause Motor, Save Data to Firebase]
  |                                   |
  |                             [Resume Motor]
  |
  v
END
```

