### 1. Tie the ground rails together
Connect one of the Arduino/ESP32 GND pins to the breadboard’s negative (blue) rail. This gives you a common ground for both LEDs.

### 2. Position your LEDs
Insert each LED so that its long leg (anode, +) and short leg (cathode, –) occupy two different rows.
Keep at least one empty column between the two LEDs to avoid accidental shorts.

### 3. Add the series resistors
For each LED, place a 220 Ω (or 330 Ω) resistor in series with the anode:
One end of the resistor goes into the same row as the LED’s anode.
The other end goes into a separate, empty row.

### 4. Wire the digital pins
Use a jumper wire to connect GPIO 25 to the free end of the first resistor (which is tied to the red LED’s anode).
Similarly, connect GPIO 26 to the free end of the second resistor (green LED’s anode).

### 5. Connect the cathodes to ground
Run a jumper from each LED’s cathode row back to the breadboard’s negative rail.
Since the negative rail is already tied to GND, this completes each LED circuit.

That’s it! When your sketch sets pin 25 HIGH and pin 26 LOW, the red LED will light; swapping them will light the green LED, and so on. Just make sure your resistors are sized so you don’t exceed the 12 mA per pin (typical safe limit on many boards).

## 🔁 A simple way to picture it:
Part	                     Role
GPIO pin (set to HIGH)	  +3.3V power source
Resistor	  Limits current (protects LED)
LED	    Lights up when current flows through it
GND pin	  Return path—completes the circuit
