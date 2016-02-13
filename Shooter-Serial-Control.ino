const int loaderPins[2] = { 7, 6 }; // Pins controlling the large cylinder
const int latchPins[2] = { 5, 4 }; // Pins controlling the latch cylinder
const int commandLoader = 1;
const int commandLatch = 2;
const int commandReset = 3;
const int commandFireSequence = 5;
const int commandAssistedRelease = 9;
const int backLimitSwitchPin = 10; // ENTER PIN VALUE
const int frontLimitSwitchPin = 11; // ENTER PIN VALUE
const int assistedReleaseLoaderTime = 2000; // How long to wait for the loader to move back
                                            // during an assisted release
const int assistedReleaseVacateTime = 500; // How long to wait for the loader to vacate
                                           // during an assisted release
const int firingSequenceVacateTime = 200; // How long to wait for the loader to vacate
                                          // during a firing sequence
const int RELAY_HOLD = 25; // How long to hold the relay on (ms)
const int NUM_POWER_PINS = 2;
const int POWER_PINS[NUM_POWER_PINS] = { 8, 9 };

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
  Serial.print(commandAssistedRelease);
  Serial.print(" initiates an assisted release.\n");
  Serial.print("\nBe sure that the line ending option below is not 'No line ending'.\n\n");
}

void loop()
{
  fromSerial = Serial.parseInt();

  if (fromSerial == commandReset) // Reset
  {
    reset();
    Serial.println("Reset.");
  }
  if (fromSerial == commandLoader || fromSerial == commandLatch) toggle(fromSerial);
  if (fromSerial == commandFireSequence)
  {
    Serial.println("Beginning firing sequence...");
    Serial.println("...raising latch...");
    absoluteRelay(commandLatch, 0);
    Serial.println("...loading bucket...");
    absoluteRelay(commandLoader, 1);
    while (!digitalRead(backLimitSwitchPin));
    Serial.println("...resetting loading slide...");
    absoluteRelay(commandLoader, 0);
    while (!digitalRead(frontLimitSwitchPin));
    Serial.println("...dropping latch...");
    absoluteRelay(commandLatch, 1);
    Serial.println("...waiting for bucket to vacate...");
    delay(firingSequenceVacateTime);
    Serial.println("...raising latch...");
    absoluteRelay(commandLatch, 0);
    Serial.println("...firing sequence complete.");
  }
  if (fromSerial == commandAssistedRelease)
  {
    Serial.println("Beginning assisted release...");
    absoluteRelay(commandLoader, 1);
    Serial.println("...waiting for loader...");
    delay(assistedReleaseLoaderTime);
    Serial.println("...dropping latch...");
    absoluteRelay(commandLatch, 1);
    Serial.println("...releasing loader...");
    absoluteRelay(commandLoader, 0);
    Serial.println("...waiting for loader to vacate...");
    delay(assistedReleaseVacateTime);
    Serial.println("...raising latch...");
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
      "Loading bucket.."
    );
  }

  if (cylinder == commandLatch)
  {
    latchState = latchPins[latchState == latchPins[0] ? 1 : 0];
    Serial.println(
      latchState == latchPins[0] ?
      "Raising latch.." :
      "Dropping latch.."
    );
  }

  updateRelay(cylinder);
}

void updateRelay(int cylinder)
{
  if (cylinder == 0 || cylinder == commandLoader) digitalWrite(loaderState, HIGH);
  if (cylinder == 0 || cylinder == commandLatch) digitalWrite(latchState, HIGH);
  delay(RELAY_HOLD);
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

