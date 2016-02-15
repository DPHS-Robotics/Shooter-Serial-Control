// Pins
const int loaderPins[2] = { 7, 6 }; // Pins controlling the large cylinder
const int latchPins[2] = { 5, 4 }; // Pins controlling the latch cylinder
const int backLimitSwitchPin = 10;
const int frontLimitSwitchPin = 11;
const int NUM_POWER_PINS = 2; // How many power pins to include for the limit switches
const int POWER_PINS[NUM_POWER_PINS] = { 8, 9 }; // Power pins for the limit switches

// Commands
const int commandLoader = 1;
const int commandLatch = 2;
const int commandReset = 3;
const int commandFireSequence = 5;
const int commandStateRequest = 7;
const int commandAssistedRelease = 9;

const int assistedReleaseLoaderTime = 2000; // How long to wait for the loader to move to the back during an assisted release
const int assistedReleaseVacateTime = 500; // How long to wait for the bucket to vacate the latch during an assisted release
const int firingSequenceVacateTime = 200; // How long to wait for the bucket to vacate the latch during a firing sequence
const int relayHoldTime = 20; // How long to hold the relay on (ms)

int fromSerial;
int loaderState;
int latchState;

void setup()
{
  // Pin modes
  for (int i = 0; i < NUM_POWER_PINS; i++)
  {
    pinMode(POWER_PINS[i], OUTPUT);
    digitalWrite(POWER_PINS[i], HIGH);
  }
  for (int i = 0; i < 2; i++)
  {
    pinMode(loaderPins[i], OUTPUT);
    pinMode(latchPins[i], OUTPUT);
  }
  pinMode(backLimitSwitchPin, INPUT);
  pinMode(frontLimitSwitchPin, INPUT);

  // Resetting the system
  loaderState = loaderPins[0];
  latchState = latchPins[0];
  updateRelay(0); // Updates both relays

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
  fromSerial = Serial.parseInt();
  if (fromSerial < 10) runCommand(fromSerial);
  else
  {
    Serial.print("Beginning timed action...\n...enter time now: ");
    delay(3000);
    int wait = Serial.parseInt();
    Serial.print(wait);
    Serial.print("...\n");
    if (wait == 0) Serial.println("...no wait time entered!!");
    else
    {
      Serial.print("...waiting ");
      Serial.print(wait);
      Serial.print(" ms...\n");
      delay(wait);
      runCommand(fromSerial % 10);
    }
  }
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
    Serial.println("...locking latch...");
    absoluteRelay(commandLatch, 0);
    Serial.println("...pushing loading slide...");
    absoluteRelay(commandLoader, 1);
    while (!digitalRead(backLimitSwitchPin))
    {
      if (Serial.parseInt() != 0)
      {
        Serial.println("...canceled!!");
        return;
      }
    }
    Serial.println("...resetting loading slide...");
    absoluteRelay(commandLoader, 0);
    while (!digitalRead(frontLimitSwitchPin))
    {
      if (Serial.parseInt() != 0)
      {
        Serial.println("...canceled!!");
        return;
      }
    }
    Serial.println("...releasing latch...");
    absoluteRelay(commandLatch, 1);
    Serial.println("...waiting for bucket to vacate...");
    delay(firingSequenceVacateTime);
    Serial.println("...locking latch...");
    absoluteRelay(commandLatch, 0);
    Serial.println("...firing sequence complete.");
  }
  if (command == commandStateRequest)                      // 7 -> State request
  {
    Serial.print("Loader: ");
    Serial.print(loaderState == loaderPins[0] ? "Reset\n" : "Loading\n");
    Serial.print("Latch: ");
    Serial.print(latchState == latchPins[0] ? "Locked\n" : "Released\n");
    Serial.print("Back switch: ");
    Serial.print(digitalRead(backLimitSwitchPin) == HIGH ? "Pressed" : "Not pressed");
    Serial.print("Front switch: ");
    Serial.print(digitalRead(frontLimitSwitchPin) == HIGH ? "Pressed" : "Not pressed");
  }
  if (command == commandAssistedRelease)                   // 9 -> Assisted release
  {
    Serial.println("Beginning assisted release...");
    absoluteRelay(commandLoader, 1);
    Serial.println("...pushing loading slide and waiting...");
    delay(assistedReleaseLoaderTime);
    Serial.println("...releasing latch...");
    absoluteRelay(commandLatch, 1);
    Serial.println("...releasing loader...");
    absoluteRelay(commandLoader, 0);
    Serial.println("...waiting for loader to vacate...");
    delay(assistedReleaseVacateTime);
    Serial.println("...locking latch...");
    absoluteRelay(commandLatch, 0);
    Serial.println("...assisted release complete.");
  }
}

void allRelaysLow()
{
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(loaderPins[i], LOW);
    digitalWrite(latchPins[i], LOW);
  }
}

void toggle(int cylinder)
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

  updateRelay(cylinder);
}

void updateRelay(int cylinder)
{
  if (cylinder == 0 || cylinder == commandLoader) digitalWrite(loaderState, HIGH);
  if (cylinder == 0 || cylinder == commandLatch) digitalWrite(latchState, HIGH);
  delay(relayHoldTime);
  allRelaysLow();
}

void absoluteRelay(int cylinder, int state)
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

void reset()
{
  absoluteRelay(commandLoader, 0);
  absoluteRelay(commandLatch, 0);
}
