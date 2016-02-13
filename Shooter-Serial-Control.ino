int relayNumbers[4] = {1,2,3,4};
int solenoidPin[4] = {7,6,5,4};
int solenoidAt[2];
int solenoidAtIndex[2];
int fromSerial;

const int DELAY_AFTER_FIRE = 25;

void setup() {
  for (int i = 0; i < 4; i++) pinMode(solenoidPin[i], OUTPUT);
  solenoidAt[0] = solenoidPin[0];
  solenoidAtIndex[0] = 0;
  solenoidAt[1] = solenoidPin[2];
  solenoidAtIndex[1] = 2;
  digitalWrite(solenoidAt[0], HIGH);
  digitalWrite(solenoidAt[1], HIGH);
  delay(25);
  for (int i = 0; i < 4; i++)
    {
      digitalWrite(solenoidPin[i], LOW);
    }
  Serial.begin(9600);
  Serial.println("Enter 1 to toggle the state of the large cylinder.\nEnter 2 to toggle the state of the small cylinder.");
}

void loop() {
  fromSerial = Serial.parseInt();

  if (fromSerial == 1)
  {
    if (solenoidAtIndex[0] == 0)
    {
      solenoidAtIndex[0] = 1;
      solenoidAt[0] = solenoidPin[1];
    }
    else
    {
      solenoidAtIndex[0] = 0;
      solenoidAt[0] = solenoidPin[0];
    }
  }
  if (fromSerial == 2)
  {
    if (solenoidAtIndex[1] == 2)
    {
      solenoidAtIndex[1] = 3;
      solenoidAt[1] = solenoidPin[3];
    }
    else
    {
      solenoidAtIndex[1] = 2;
      solenoidAt[1] = solenoidPin[2];
    }
  }
      
  if (fromSerial == 1 || fromSerial == 2)
  {
    /*Serial.print("#");
    Serial.print(fromSerial);
    Serial.print(" -> ");
    Serial.print(solenoidAt[fromSerial - 1]);
    Serial.print("(");
    Serial.print(relayNumbers[solenoidAtIndex[fromSerial - 1]]);
    Serial.print(")..");
    */
    for (int i = 0; i < 4; i++)
    {
      if (solenoidAt[fromSerial - 1] == solenoidPin[i])
      {
        digitalWrite(solenoidPin[i], HIGH);
        switch (relayNumbers[i])
        {
          case 1:
            Serial.println("Pushing bucket back..");
            break;
          case 2:
            Serial.println("Resetting loading slide..");
            break;
          case 3:
            Serial.println("Raising latch..");
            break;
          case 4:
            Serial.println("Lowering latch..");
            break;
        }
      }
      else
     {
       digitalWrite(solenoidPin[i], LOW);
      }
    }
    delay(DELAY_AFTER_FIRE);
    for (int i = 0; i < 4; i++)
    {
      digitalWrite(solenoidPin[i], LOW);
    }
    Serial.print("!\n");
  }
}
