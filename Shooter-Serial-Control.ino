const int loaderPins[2] = { 7, 6 }; // Pins controlling the large cylinder
const int latchPins[2] = { 5, 4 }; // Pins controlling the latch cylinder
const int RELAY_HOLD = 25; // How long to hold the relay on (ms)

int fromSerial;
int loaderState;
int latchState;

void setup()
{
  // Pin modes
  for (int i = 0; i < 2; i++)
  {
    pinMode(loaderPins[i], OUTPUT);
    pinMode(latchPins[i], OUTPUT);
  }

  // Resetting the system
  loaderState = loaderPins[0];
  latchState = latchPins[0];
  updateRelay(0); // Updates both relays

  Serial.begin(9600);
  Serial.println("Enter 1 to toggle the state of the loader.");
  Serial.println("Enter 2 to toggle the state of the latch.");
  Serial.println("Enter 3 to reset the system.");
  Serial.println("Be sure that the line ending option below is not 'No line ending'.\n");
}

void loop()
{
  fromSerial = Serial.parseInt();

  if (fromSerial == 3) // Reset
  {
    loaderState = loaderPins[0];
    latchState = latchPins[0];
    updateRelay(0);
  }
  else
  {
    if (fromSerial == 1 || fromSerial == 2) toggle(fromSerial);
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
  if (cylinder == 1)
  {
    loaderState = loaderPins[loaderState == loaderPins[0] ? 1 : 0];
    Serial.println(
      loaderState == loaderPins[0] ?
      "Resetting loading slide.." :
      "Loading bucket.."
    );
  }

  if (cylinder == 2)
  {
    latchState = latchPins[latchState == latchPins[0] ? 1 : 0];
    Serial.println(
      loaderState == loaderPins[0] ?
      "Raising latch.." :
      "Dropping latch.."
    );
  }

  updateRelay(cylinder);
}

void updateRelay(int cylinder)
{
  if (cylinder == 0 || cylinder == 1) digitalWrite(loaderState, HIGH);
  if (cylinder == 0 || cylinder == 2) digitalWrite(latchState, HIGH);
  delay(RELAY_HOLD);
  allRelaysLow();
}

