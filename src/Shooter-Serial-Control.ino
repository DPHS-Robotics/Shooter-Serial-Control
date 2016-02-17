// Pins
const int loaderPins[2] = { 7, 6 }; // Loader solenoid (7 retracts, 6 extends)
const int latchPins[2] = { 5, 4 }; // Latch solenoid (5 extends, 4 retracts)
const int backLimitSwitchPin = 10; // Data pin for the back limit switch (triggers as cam locks)
const int frontLimitSwitchPin = 11; // Data pin for the front limit switch (triggers as loader finishes retracting)
const int sideLimitSwitchPin = 12; // Data pin for the side limit switch (triggers while bucket is in back)
const int limitSwitchPowerPins[2] = { 8, 9 }; // Power pins for the limit switches
const int pressureReaderPowerPin = A2; // Power pin for the pressure gauge
const int pressureReaderDataPin = A1; // Data pin for the pressure gauge
const int pressureReaderGroundPin = A0; // Ground pin for the pressure gauge

// Commands
const int commandLoader = 1; // Toggles the loader between extension and retraction
const int commandLatch = 2; // Toggles the latch between extension and retraction
const int commandReset = 3; // Resets the loader and latch to their initial states
const int commandFireSequence = 5; // Prepares and initiates a firing sequence
const int commandStateRequest = 7; // Reports on the states of certain relays and switches
const int commandAssistedRelease = 9; // Initiates an assisted release, or unload

// Wait times
const int relayHoldTime = 20; // Time to power solenoids in all cases

// Pressures
const int fireAtPressure = 70; // Pressure necessary to remove latch from cam (unadjusted for volume)
const int loadAtPressure = 100; // Pressure necessary for loader to load bucket (unadjusted for volume)

int fromSerial; // The parsed integer received from the serial connection
int loaderState; // The most recently powered pin for the loader
int latchState; // The most recently powered pin for the latch

void setup()
{
  // Pin modes and power/ground pin writes
  for (int i = 0; i < 2; i++) // Limit switch power/ground pins
  {
    pinMode(limitSwitchPowerPins[i], OUTPUT);
    digitalWrite(limitSwitchPowerPins[i], HIGH);
  }
  for (int i = 0; i < 2; i++) // Relay pins
  {
    pinMode(loaderPins[i], OUTPUT);
    pinMode(latchPins[i], OUTPUT);
  }
  pinMode(backLimitSwitchPin, INPUT); // Back limit switch data
  pinMode(frontLimitSwitchPin, INPUT); // Front limit switch data
  pinMode(sideLimitSwitchPin, INPUT); // Side limit switch data
  pinMode(pressureReaderGroundPin, OUTPUT); // Pressure gauge ground pin
  pinMode(pressureReaderDataPin, INPUT); // Pressure gauge data pin
  pinMode(pressureReaderPowerPin, OUTPUT); // Pressure gauge power pin
  digitalWrite(pressureReaderGroundPin, LOW); // Pressure gauge ground pin write
  digitalWrite(pressureReaderPowerPin, HIGH); // Pressure gauge power pin write

  // Resetting the system
  loaderState = loaderPins[0]; // Initial loader state defaults to index 0 of its two relays
  latchState = latchPins[0]; // Initial latch state defaults to index 0 of its two relays
  updateRelay(0); // Powers both relays with the new states

  // User instructions
  Serial.begin(9600);
  Serial.print(commandLoader);
  Serial.print(" toggles the state of the loader.\n");
  Serial.print(commandLatch);
  Serial.print(" toggles the state of the latch.\n");
  Serial.print(commandReset);
  Serial.print(" resets the system.\n");
  Serial.print(commandFireSequence);
  Serial.print(" initiates a firing sequence.\n");
  Serial.print(commandStateRequest);
  Serial.print(" returns the relays' most recent commands.\n");
  Serial.print(commandAssistedRelease);
  Serial.print(" initiates an assisted release.\n\n");
  Serial.println("A '..' after feedback indicates that the action will complete promptly.");
  Serial.println("A '...' before feedback indicates that the action follows an automatic action.");
  Serial.println("A '...' after feedback indicates that an automatic action will follow.");
  Serial.println("A '!!' after feedback indicates that no action was performed.\n");
}

void loop()
{
  fromSerial = Serial.parseInt(); // Parses an integer from the serial connection
  runCommand(fromSerial); // Checks against the index of commands and runs the correct one
}

void runCommand(int command)
{
  if (command == commandLoader) toggle(commandLoader);     // 1 -> Toggle loader
  if (command == commandLatch) toggle(commandLatch);       // 2 -> Toggle latch
  if (command == commandReset)                             // 3 -> Reset
  {
    Serial.println("Resetting..");
    reset();
  }
  if (command == commandFireSequence)                      // 5 -> Firing sequence
  {
    Serial.println("Beginning firing sequence...");
    Serial.println("...locking latch..."); // Latch must be locked in order to load the bucket
    absoluteRelay(commandLatch, 0);
    Serial.println("...waiting for pressure..."); // We must have loadAtPressure PSI in order to successfully load
    holdForPressure(loadAtPressure);
    Serial.println("...pushing loading slide..."); // Once we are at pressure, we can load
    absoluteRelay(commandLoader, 1);
    while (!digitalRead(backLimitSwitchPin)); // Wait while the loader has not finished loading the bucket
    Serial.println("...resetting loading slide..."); // The bucket has been loaded and the loader can retract
    absoluteRelay(commandLoader, 0);
    while (!digitalRead(frontLimitSwitchPin)); // Wait while the loader has not finished retracting
    Serial.println("...waiting for pressure..."); // We must have fireAtPressure PSI in order to successfully release the latch
    holdForPressure(fireAtPressure);
    Serial.println("...releasing latch..."); // We can now pull the latch from the cam
    absoluteRelay(commandLatch, 1);
    Serial.println("...waiting for bucket to vacate..."); // Wait for the bucket to leave the back end
    while(digitalRead(sideLimitSwitchPin));
    Serial.println("...locking latch..."); // Relock the latch for the next firing sequence
    absoluteRelay(commandLatch, 0);
    Serial.println("...firing sequence complete.");
  }
  if (command == commandStateRequest)                      // 7 -> State request
  {
    // This command makes heavy use of the ternary operator (http://bit.ly/ternary-op)
    Serial.print("Loader: ");
    Serial.print(loaderState == loaderPins[0] ? "Reset\n" : "Loading\n");
    Serial.print("Latch: ");
    Serial.print(latchState == latchPins[0] ? "Locked\n" : "Released\n");
    Serial.print("Back switch: ");
    Serial.print(digitalRead(backLimitSwitchPin) == HIGH ? "Pressed\n" : "Not pressed\n");
    Serial.print("Front switch: ");
    Serial.print(digitalRead(frontLimitSwitchPin) == HIGH ? "Pressed\n" : "Not pressed\n");
  }
  if (command == commandAssistedRelease)                   // 9 -> Assisted release
  {
    Serial.println("Beginning assisted release...");
    Serial.println("...waiting for pressure..."); // We need loadAtPressure PSI to ensure loader strength
    holdForPressure(loadAtPressure);
    Serial.println("...pushing loading slide..."); // Move the loader to hold the bucket
    absoluteRelay(commandLoader, 1);
    while(!digitalRead(backLimitSwitchPin)); // Wait for the bucket to be held back by the loader
    Serial.println("...waiting for pressure..."); // We need fireAtPressure PSI to ensure latch release
    holdForPressure(fireAtPressure);
    Serial.println("...releasing latch..."); // Release the latch
    absoluteRelay(commandLatch, 1);
    Serial.println("...releasing loader..."); // Let the loader carry the bucket back to the front
    absoluteRelay(commandLoader, 0);
    Serial.println("...waiting for bucket to vacate..."); // Wait for the bucket to leave the back end
    while (digitalRead(sideLimitSwitchPin));
    Serial.println("...locking latch..."); // Relock the latch for the next fire or unload
    absoluteRelay(commandLatch, 0);
    Serial.println("...assisted release complete.");
  }
}

void allRelaysLow() // Bring down all relays (usually used after holding a relay up for relayHoldTime)
{
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(loaderPins[i], LOW);
    digitalWrite(latchPins[i], LOW);
  }
}

void toggle(int cylinder) // Switch the state of a certain solenoid and report
{
  if (cylinder == commandLoader)
  {
    loaderState = loaderPins[loaderState == loaderPins[0] ? 1 : 0];
    Serial.println(
      loaderState == loaderPins[0] ?
      "Resetting loading slide.." :
      "Pushing loading slide.."
    );
  }

  if (cylinder == commandLatch)
  {
    latchState = latchPins[latchState == latchPins[0] ? 1 : 0];
    Serial.println(
      latchState == latchPins[0] ?
      "Locking latch.." :
      "Releasing latch.."
    );
  }

  updateRelay(cylinder); // Update the relay pair (the state of which we have just modified)
}

void updateRelay(int cylinder) // Update a relay pair to match its recorded state
{
  if (cylinder == 0 || cylinder == commandLoader) digitalWrite(loaderState, HIGH);
  if (cylinder == 0 || cylinder == commandLatch) digitalWrite(latchState, HIGH);
  delay(relayHoldTime); // Give the solenoid time to register the power
  allRelaysLow(); // Finish by bringing all relays back down
}

void absoluteRelay(int cylinder, int state) // Fire a change to a solenoid, regardless of what its current state is
{
  if (cylinder == commandLoader)
  {
    loaderState = loaderPins[state];
    updateRelay(commandLoader);
  }

  if (cylinder == commandLatch)
  {
    latchState = latchPins[state];
    updateRelay(commandLatch);
  }
}

void reset() // Bring both cylinders back to their default states
{
  absoluteRelay(commandLoader, 0);
  absoluteRelay(commandLatch, 0);
}

void holdForPressure(int PSIG) // Run a loop to wait until a given pressure is read
{
  while(0.255 * (analogRead(pressureReaderDataPin)) - 25.427 < PSIG); // Experimental equation
}
