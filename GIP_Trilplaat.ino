#include <PID_v1.h>

// configuration of variables
int HALLSENSOR = 2;
int PWMA = 3;
int INB = 4;

int state = 0;
unsigned long startRotation = 0L;
unsigned long count = 0L;
float currentFrequency = 0.0f;
float INPUT_VOLTAGE;
float PRECISION;
int TEST_DELAY;
char endMarker = '\n';
char receivedChars[32];
String myString;

void setup()
{
  pinMode(PWMA, OUTPUT);
  pinMode(INB, OUTPUT);
  pinMode(HALLSENSOR, INPUT);
  attachInterrupt(digitalPinToInterrupt(HALLSENSOR), calculateFrequenty, FALLING);
  digitalWrite(INB, HIGH); // Needs to be high to get motor moving. Make sure INA is shorted to ground. For diagram, see: https://i.imgur.com/AAdZkGg.png
  Serial.begin(9600);
  Serial.println();
  delay(2000);
  Serial.println("--Trilplaat V0.1-- code by Tiddo Nees - 6IW");
  delay(2000);
  Serial.println('\n');
  boolean VoltCheck = false;
  while (VoltCheck == false)
  {
    while (Serial.available())
    {
      Serial.read();
    }
    Serial.println("How many volts is the source input?\n");
    while (Serial.available() == 0)
    {
    }
    String StringInput = Serial.readString();
    INPUT_VOLTAGE = StringInput.toFloat();
    String volts = String(INPUT_VOLTAGE, 1);
    Serial.println("\nInput is " + volts + "V\n");
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
    boolean modeCheck = false;
    while (modeCheck == false)
    {
      Serial.flush();
      while (Serial.available())
      {
        Serial.read();
      }
      Serial.println("What mode do you want to use?\n\n[1] Manual or [2] Testing mode");
      while (Serial.available() == 0)
      {
      }
      int StringInput = Serial.read();
      Serial.println();
      if (StringInput == 49)
      {
        Serial.println("_MANUAL MODE ENABLED_");
        state = 1;
        modeCheck = true;
      }
      else if (StringInput == 50)
      {
        Serial.println("_TESTING MODE ENABLED_");
        state = 2;
        modeCheck = true;
      }
      else
      {
        Serial.println("Please enter valid input");
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
      Serial.println();
      delay(500);
      Serial.println();
      Serial.flush();
      while (Serial.available())
      {
        Serial.read();
      }
      Serial.println("How do you want to control the speed?\n\n[1] Voltage based or [2] Frequency based");
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
        Serial.println("Please enter valid input");
        delay(200);
      }
    }
    if (ManualState == true) // Voltage controlled mode
    {
      Serial.println();
      delay(1000);
      Serial.println("*You can stop the mode by sending any text*");
      delay(2500);
      Serial.println();
      boolean manualcheck = false;
      boolean FirstcharCheck = false;
      byte ndx = 0;
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
        String percentS = String(percent, 2); // using a float and the decimal places
        String voltageS = String(voltage, 2);
        String frequencyS = String(currentFrequency, 2);
        Serial.println("Current output: " + percentS + "%  (" + voltageS + "V) (" + currentFrequency + "Hz)");
        if (receivedChars[0] != '\0' && (receivedChars[0] < '0' || receivedChars[0] > '9'))
        {
          manualcheck = true;
          state = 0;
          Serial.println();
        }
      }
    }
    else if (ManualState == false) // Frequency controlled mode // TODO testing
    {
      Serial.println();
      delay(1000);
      Serial.println("*You can stop the mode by sending any text*");
      delay(2500);
      Serial.println();
      boolean manualcheck = false;
      boolean FirstcharCheck = false;
      byte ndx = 0;
      String frequencyString;
      float tempFrequency;
      float output = 0.0;
      float wantedFrequency = 0;
      double Setpoint, Input, Output;

      PID myPID(&Input, &Output, &Setpoint, 10, 10, 10, DIRECT);
      myPID.SetSampleTime(50);

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
        myPID.Compute();
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
    Serial.println();
    boolean checkPrecision = false;
    while (checkPrecision == false)
    {
      Serial.flush();
      while (Serial.available())
      {
        Serial.read();
      }
      Serial.println("How many tests do you want to perform?\n");
      while (Serial.available() == 0)
      {
      }
      String StringInput = Serial.readString();
      Serial.println('\n');
      PRECISION = StringInput.toInt();
      delay(500);
      if (PRECISION > 0)
      {
        checkPrecision = true;
      }
    }
    boolean checkDelay = false;
    while (checkDelay == false)
    {
      Serial.flush();
      while (Serial.available())
      {
        Serial.read();
      }
      Serial.println("What should the interval between tests be? [ms]\n");
      while (Serial.available() == 0)
      {
      }
      String StringInput = Serial.readString();
      Serial.println('\n');
      TEST_DELAY = StringInput.toInt();
      delay(500);
      if (TEST_DELAY > 0)
      {
        checkDelay = true;
      }
    }
    Serial.println();
    String amountOfTests = String((int)PRECISION);
    delay(2000);
    Serial.println("Precision has been succesfully set to " + amountOfTests + " tests\n");
    delay(1000);
    String amountOfMinutes = String((PRECISION * TEST_DELAY) / 60000, 2);
    Serial.println("This will approximatly take: " + amountOfMinutes + " minutes\n");
    delay(1000);
    for (int i = 10; i > 0; i--)
    {
      Serial.flush();
      int charInput = Serial.read();
      analogWrite(PWMA, 255);
      String countdown = String(i);
      Serial.println("Testing starts in: " + countdown + "       *Make sure the motor is spinning!*  [A]bort / [S]kip");
      if (charInput == 65)
      {
        i = 0;
        PRECISION = 0;
        Serial.println("\n---Test aborted---\n");
      }
      if (charInput == 83)
      {
        i = 0;
      }
      delay(1000);
    }
    Serial.println('\n');
    for (float i = 0; i < PRECISION; i++)
    {
      unsigned long testingCount = millis();
      unsigned long testingDifference;
      float currentOutput = (255.0 / PRECISION) * i;
      analogWrite(PWMA, currentOutput);
      float percent = (currentOutput / 255.0) * 100.0;
      float voltage = (INPUT_VOLTAGE * ((float)percent / 100.0));

      do
      { 
        unsigned long testingCurrentCount = millis();
        testingDifference = testingCount - testingCurrentCount;
        String percentS = String(percent, 2); // using a float and the decimal places
        String voltageS = String(voltage, 2);
        String totalTests = String((int)PRECISION);
        String currentTest = String((int)PRECISION - (int)i + 1);
        String frequencyS = String(currentFrequency, 2);
        Serial.println("Currently testing with output: [" + percentS + "%]  (" + voltageS + "V)  " + " (" + currentFrequency + "Hz)" + currentTest + "/" + totalTests);

      } while (testingDifference < TEST_DELAY);
    }
    Serial.println("\n---------------------------------------------------------------------\n_TESTS DONE_\n");
    delay(200);
    boolean TestAgainCheck = false;
    while (TestAgainCheck == false)
    {
      Serial.flush();
      while (Serial.available())
      {
        Serial.read();
      }
      Serial.println("\nDo you want to test again?\n[1] Yes or [2] No");
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
          Serial.print("Do you want to enter [1] Manual mode or [2] Exit?");
          delay(100);
          Serial.flush();
          while (Serial.available() == 0)
          {
          }
          int StringInput2 = Serial.read();
          Serial.println();
          if (StringInput2 == 50)
          {
            Serial.println("\nGoodbye.");
            Serial.end();
            while (true)
            {
              delay(10000);
            }
          }
          else if (StringInput2 == 49)
          {
            Serial.println("\n_MANUAL MODE ENABLED_");
            StringInput = 0;
            state = 1;
            switchMode = true;
            TestAgainCheck = true;
          }
          else
          {
            Serial.println("Please enter valid input");
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
        Serial.println("Please enter valid input");
        delay(200);
      }
    }
    while (Serial.available())
    {
      Serial.read();
    }
  }
}

void calculateFrequenty()
{
  count = millis();
  unsigned long difference = count - startRotation;
  currentFrequency = 1.0f / ((float)difference / 1000);
  // Serial.println("state has been changed");
  // Serial.println(currentFrequency, 5);
  startRotation = millis();
}