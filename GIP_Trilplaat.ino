//#include <PID_v1.h>

#define HALLSENSOR 2
#define PWMA 3
#define INA2 4
#define endMarker '\n'

// configuration of variables

byte state = 0;
byte ndx = 0;
unsigned long startRotation = 0L;
unsigned long count = 0L;
float currentFrequency = 0.0f;
float INPUT_VOLTAGE;
char receivedChars[32];
String StringInput;

void setup()
{
  pinMode(PWMA, OUTPUT);
  pinMode(INA2, OUTPUT);
  pinMode(HALLSENSOR, INPUT);
  attachInterrupt(digitalPinToInterrupt(HALLSENSOR), calculateFrequenty, FALLING);
  digitalWrite(INA2, HIGH); // Needs to be high to get motor moving. Make sure INA is shorted to ground. For diagram, see: https://i.imgur.com/AAdZkGg.png
  Serial.begin(9600);
  for (int i = 0; i < 20; i++)
    Serial.println(F(""));
  delay(2000);
  Serial.println(F("--Trilplaat V0.6-- GIP 6IW 2021-2022"));
  delay(2000);
  boolean VoltCheck = false;
  while (VoltCheck == false)
  {
    while (Serial.available())
    {
      Serial.read();
    }
    Serial.println(F("\nHow many volts is the source input?\n"));
    while (Serial.available() == 0)
    {
    }
    StringInput = Serial.readString();
    INPUT_VOLTAGE = StringInput.toFloat();
    Serial.print("\nInput is " + String(INPUT_VOLTAGE) + "V\n");
    delay(200);
    if (INPUT_VOLTAGE > 0.0)
    {
      VoltCheck = true;
    }
  }
}

void loop()
{

  if (state == 0) // ___________________ Menu
  {
    memset(receivedChars, 0, sizeof(receivedChars));
    boolean modeCheck = false;
    while (modeCheck == false)
    {
      Serial.flush();
      while (Serial.available())
      {
        Serial.read();
      }
      Serial.flush();
      Serial.read();
      Serial.println(F("What mode do you want to use?\n\n[1] Manual or [2] Testing mode or [3] Info "));
      while (Serial.available() == 0)
      {
      }
      Serial.flush();
      int StringInput = Serial.parseInt();
      Serial.println();
      if (StringInput == 1)
      {
        Serial.println(F("_MANUAL MODE ENABLED_\n"));
        state = 1;
        modeCheck = true;
      }
      else if (StringInput == 2)
      {
        Serial.println(F("_TESTING MODE ENABLED_\n"));
        state = 2;
        modeCheck = true;
      }
      else if (StringInput == 3)
      {
        state = 3;
        modeCheck = true;
      }
      else
      {
        Serial.println(F("Please enter valid input\n"));
        delay(200);
      }
    }
  }

  else if (state == 1) // ______________ Manual state
  {
    boolean ManualState; // True is Voltage, false is frequency based control
    boolean ManualModeCheck = false;
    while (ManualModeCheck == false) // Decides wether to use serial or analog input
    {
      Serial.flush();
      while (Serial.available())
      {
        Serial.read();
      }
      Serial.println(F("\nHow do you want to control the speed?\n\n[1] Voltage based or [2] Frequency based (Doesn't work yet)"));
      while (Serial.available() == 0)
      {
      }
      int StringInput = Serial.read();
      Serial.println();
      if (StringInput == 49)
      {
        ManualModeCheck = true;
        ManualState = true;
      }
      else if (StringInput == 50)
      {
        ManualModeCheck = true;
        ManualState = false;
      }
      else
      {
        Serial.println(F("Please enter valid input"));
        delay(200);
      }
    }
    if (ManualState == true) // Voltage controlled mode
    {
      Serial.println();
      delay(1000);
      Serial.println(F("*You can stop the mode by sending any text*"));
      delay(2500);
      Serial.println();
      boolean manualcheck = false;
      boolean FirstcharCheck = false;
      ndx = 0;
      float schakelaarEersteToestand = 0.0;
      float percent = 0;
      while (manualcheck == false)
      {
        memset(receivedChars, 0, sizeof(receivedChars));
        boolean newData = false;
        Serial.flush();
        while (Serial.available() > 0 && newData == false)
        {
          char rc = Serial.read();
          if (rc != endMarker)
          {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= 32)
            {
              ndx = 32 - 1;
            }
          }
          else
          {
            receivedChars[ndx] = '\0';
            ndx = 0;
            newData = true;
          }
        }
        if ((receivedChars[0] >= '0' && receivedChars[0] <= '9'))
        {
          String percentString = String(receivedChars);
          float tempPercent = percentString.toFloat();
          if (tempPercent <= 100.0)
          {
            percent = tempPercent;
            schakelaarEersteToestand = 255.0 * (percent / 100.0);
          }
        }
        analogWrite(PWMA, schakelaarEersteToestand);
        float voltage = (INPUT_VOLTAGE * (percent / 100.0));
        Serial.println("Current output: " + String(percent, 2) + "%  (" + String(voltage, 2) + "V) (" + String(currentFrequency, 2) + "Hz)");
        if (receivedChars[0] != '\0' && (receivedChars[0] < '0' || receivedChars[0] > '9'))
        {
          manualcheck = true;
          analogWrite(PWMA, 0);
          state = 0;
          Serial.println();
        }
      }
    }
    else if (ManualState == false) // Frequency controlled mode // TODO troubleshooting
    {
      Serial.println();
      delay(1000);
      Serial.println(F("*You can stop the mode by sending any text*"));
      delay(2500);
      Serial.println();
      boolean manualcheck = false;
      boolean FirstcharCheck = false;
      ndx = 0;
      String frequencyString;
      float tempFrequency;
      float output = 0.0;
      float wantedFrequency = 0;
      double Setpoint, Input, Output;

      // PID myPID(&Input, &Output, &Setpoint, 1000, 1000, 1000, DIRECT);
      // myPID.SetSampleTime(50);

      while (manualcheck == false)
      {
        memset(receivedChars, 0, sizeof(receivedChars));
        boolean newData = false;
        Serial.flush();
        while (Serial.available() > 0 && newData == false)
        {
          char rc = Serial.read();
          if (rc != endMarker)
          {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= 32)
            {
              ndx = 32 - 1;
            }
          }
          else
          {
            receivedChars[ndx] = '\0';
            ndx = 0;
            newData = true;
          }
        }
        if ((receivedChars[0] >= '0' && receivedChars[0] <= '9'))
        {
          frequencyString = String(receivedChars);
          tempFrequency = frequencyString.toFloat();
          if (tempFrequency <= 100.0)
          {
            wantedFrequency = tempFrequency;
          }
        }
        Setpoint = wantedFrequency;
        Input = currentFrequency;
        // myPID.Compute();
        analogWrite(PWMA, Output);
        float percentage = (Output / 255.0) * 100;
        float voltage = INPUT_VOLTAGE * (percentage / 100);
        String percentageS = String(percentage, 2);
        String wantedFrequencyS = String(wantedFrequency, 2); // using a float and the decimal places
        String voltageS = String(voltage, 2);
        String currentFrequencyS = String(currentFrequency, 2);
        Serial.println("Wanted frequency: " + wantedFrequencyS + "Hz (" + percentageS + "%) (" + voltageS + "V) (" + currentFrequencyS + "Hz)");
        if (receivedChars[0] != '\0' && (receivedChars[0] < '0' || receivedChars[0] > '9'))
        {
          manualcheck = true;
          state = 0;
          Serial.println();
        }
      }
    }
  }

  else if (state == 2) // ______________ Testing mode
  {
    boolean abortTest = false;
    boolean checkMin = false;
    boolean checkMax = false;
    boolean checkPrecision = false;
    boolean checkDelay = false;
    String StringInput;
    int TEST_DELAY;
    int PRECISION;
    float TEST_MAX;
    float TEST_MIN;

    while (checkMin == false)
    {
      Serial.flush();
      while (Serial.available())
      {
        Serial.read();
      }
      Serial.println(F("What should the minimum percentage be? [%]\n"));
      while (Serial.available() == 0)
      {
      }
      StringInput = Serial.readString();
      TEST_MIN = StringInput.toFloat();
      delay(250);
      if (TEST_MIN >= 0)
      {
        checkMin = true;
      }
    }
    Serial.print("Minimum percentage set to: " + String(TEST_MIN, 1) + "% or " + String(INPUT_VOLTAGE * (TEST_MIN / 100.0), 1) + "V\n\n\n");
    while (checkMax == false)
    {
      Serial.flush();
      while (Serial.available())
      {
        Serial.read();
      }
      Serial.println(F("What should the maximum percentage be? [%]\n"));
      while (Serial.available() == 0)
      {
      }
      StringInput = Serial.readString();
      TEST_MAX = StringInput.toFloat();
      delay(250);
      if (TEST_MAX > 0)
      {
        checkMax = true;
      }
    }
    Serial.print(("Maximum percentage set to: " + String(TEST_MAX, 1) + "% or " + String(INPUT_VOLTAGE * (TEST_MAX / 100.0)) + "V\n\n\n"));
    while (checkPrecision == false)
    {
      Serial.flush();
      while (Serial.available())
      {
        Serial.read();
      }
      Serial.println(F("How many tests do you want to perform?\n"));
      while (Serial.available() == 0)
      {
      }
      StringInput = Serial.readString();
      PRECISION = StringInput.toInt();
      delay(250);
      if (PRECISION > 0)
      {
        checkPrecision = true;
      }
    }
    Serial.println("Amount of tests set to: " + String(PRECISION) + "\n\n");
    while (checkDelay == false)
    {
      Serial.flush();
      while (Serial.available())
      {
        Serial.read();
      }
      Serial.println(F("What should the interval between tests be? [ms]\n"));
      while (Serial.available() == 0)
      {
      }
      StringInput = Serial.readString();
      TEST_DELAY = StringInput.toInt();
      delay(250);
      if (TEST_DELAY > 0)
      {
        checkDelay = true;
      }
    }
    Serial.println("Amount of time in between tests set to " + String(TEST_DELAY) + "ms\n\n\n");
    String amountOfTests = String(PRECISION);
    delay(1000);
    Serial.println("Precision has been succesfully set to " + amountOfTests + " tests\n");
    delay(500);
    String amountOfMinutes = String(float(PRECISION * TEST_DELAY) / 60000.0f, 2);
    Serial.println("This will approximatly take: " + amountOfMinutes + " minutes\n");
    delay(500);
    for (int i = 10; i > 0; i--)
    {
      Serial.flush();
      int charInput = Serial.read();
      analogWrite(PWMA, 0);
      String countdown = String(i);
      Serial.println("Testing starts in: " + countdown + "s  [A]bort / [S]kip");
      if (charInput == 65)
      {
        i = 0;
        PRECISION = -1;
        Serial.println(F("\n---Test aborted---\n"));
      }
      if (charInput == 83)
      {
        i = 0;
      }
      delay(1000);
    }

    if (PRECISION > 0)
    {
      Serial.println(F("*You can stop the test by sending any text*\n"));
    }

    for (byte i = 1; i < (PRECISION + 1); i++)
    {
      ndx = 0;
      unsigned long testingCount = millis();
      unsigned long testingDifference = 0;
      float currentOutput = (((((float)TEST_MAX - (float)TEST_MIN) / 100.0) * 255.0) / float(PRECISION - 1)) * float(i - 1) + ((float)TEST_MIN / 100.0) * 255.0;
      // float currentOutput = ((TEST_MAX - TEST_MIN))
      analogWrite(PWMA, currentOutput);
      float percent = (currentOutput / 255.0) * 100.0;
      float voltage = (INPUT_VOLTAGE * ((float)percent / 100.0));

      memset(receivedChars, 0, sizeof(receivedChars));
      boolean newData = false;
      Serial.flush();
      while (Serial.available() > 0 && newData == false)
      {
        char rc = Serial.read();
        if (rc != endMarker)
        {
          receivedChars[ndx] = rc;
          ndx++;
          if (ndx >= 32)
          {
            ndx = 32 - 1;
          }
        }
        else
        {
          receivedChars[ndx] = '\0';
          ndx = 0;
          newData = true;
        }
      }
      if (receivedChars[0] != '\0')
      {
        String abortTestS = String(receivedChars);
        abortTest = true;
        i = PRECISION;
      }

      do
      {
        unsigned long testingCurrentCount = millis();
        testingDifference = testingCurrentCount - testingCount;
        Serial.print("Testing: [" + String(percent, 2) + "%] (" + String(voltage, 2) + "V) (");
        Serial.print(String(currentFrequency, 2) + "Hz) " + String(i) + "/" + String(PRECISION) + " (" + String(testingDifference) + "ms)\n");

      } while ((int)testingDifference < TEST_DELAY && abortTest == false);
    }
    analogWrite(PWMA, 0);
    Serial.println(F("\n-------------------------\n_TESTS DONE_\n"));
    delay(200);
    boolean TestAgainCheck = false;
    while (TestAgainCheck == false)
    {
      Serial.flush();
      while (Serial.available())
      {
        Serial.read();
      }
      Serial.println(F("\nDo you want to test again?\n[1] Yes or [2] No"));
      while (Serial.available() == 0)
      {
      }
      int StringInput = Serial.read();
      Serial.println();
      if (StringInput == 50)
      {
        boolean switchMode = false;
        while (switchMode == false)
        {
          Serial.flush();
          Serial.println();
          do
          {
            Serial.read();
          } while (Serial.available());
          Serial.print(F("Do you want to enter [1] Menu or [2] Exit?"));
          delay(100);
          Serial.flush();
          while (Serial.available() == 0)
          {
          }
          int StringInput2 = Serial.read();
          Serial.println();
          if (StringInput2 == 50)
          {
            Serial.println(F("\nGoodbye."));
            Serial.end();
            while (true)
            {
              delay(10000);
            }
          }
          else if (StringInput2 == 49)
          {
            StringInput = 0;
            state = 0;
            switchMode = true;
            TestAgainCheck = true;
            Serial.println(F("\nEntering menu: \n\n"));

          }
          else
          {
            Serial.println(F("Please enter valid input"));
            delay(200);
          }
        }
      }
      else if (StringInput == 49)
      {
        TestAgainCheck = true;
      }
      else
      {
        Serial.println(F("Please enter valid input"));
        delay(200);
      }
    }
    while (Serial.available())
    {
      Serial.read();
    }
  }
  else if (state == 3)
  {
    Serial.println(F("--Trilplaat--\n--Technische scholen mechelen - 6IW 2021-2022--\n--Made by Tiddo Nees with lots of blood sweat and tears--\n--Made to control the trilplaat prototype, make sure all \n  cables are connected as shown on the diagram in the thesis--\n\n"));
    state = 0;
  }
}

void calculateFrequenty()
{
  count = millis();
  unsigned long difference = count - startRotation;
  currentFrequency = 1.0f / ((float)difference / 1000.0);
  startRotation = millis();
}

float checkPrompt(String a)
{
  boolean inputCheck = false;
  while (inputCheck == false)
  {
    memset(receivedChars, 0, sizeof(receivedChars));
    boolean newData = false;
    byte ndx = 0;
    Serial.flush();
    while (Serial.available() > 0 && newData == false)
    {
      char rc = Serial.read();
      if (rc != endMarker)
      {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= 32)
        {
          ndx = 32 - 1;
        }
      }
      else
      {
        receivedChars[ndx] = '\0';
        ndx = 0;
        newData = true;
      }
    }
    if (receivedChars[0] != '\0')
    {
      state = 0;
      inputCheck = true;
    }
  }
}