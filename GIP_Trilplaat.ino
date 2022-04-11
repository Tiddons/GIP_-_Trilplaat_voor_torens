// configuration of variables
int PIN_ANALOGSCHAKELAAR = A0;
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
  pinMode(PIN_ANALOGSCHAKELAAR, INPUT);
  pinMode(INB, OUTPUT);
  pinMode(HALLSENSOR, INPUT);
  attachInterrupt(digitalPinToInterrupt(HALLSENSOR), calculateFrequenty, FALLING);
  Serial.begin(9600);
  Serial.println();
  delay(2000);
  Serial.println("--GIP V0.1-- code by Tiddo Nees - 6IW");
  delay(2000);
  Serial.println();
  Serial.println();
  boolean VoltCheck = false;
  while (VoltCheck == false)
  {
    while (Serial.available())
    {
      Serial.read();
    }
    Serial.println("How many volts is the source input?");
    Serial.println();
    while (Serial.available() == 0)
    {
    }
    String StringInput = Serial.readString();
    Serial.println();
    INPUT_VOLTAGE = StringInput.toInt();
    String volts = String(INPUT_VOLTAGE, 1);
    Serial.println("Input is " + volts + "V");
    Serial.println();
    delay(200);
    if (INPUT_VOLTAGE > 0.0)
    {
      VoltCheck = true;
    }
  }
}

void loop()
{

  if (state == 0)
  { // __________________________________ Menu
    boolean modeCheck = false;
    while (modeCheck == false)
    {
      Serial.flush();
      while (Serial.available())
      {
        Serial.read();
      }
      Serial.println("What mode do you want to use?");
      Serial.println();
      Serial.println("[1] Manual or [2] Testing mode");
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

  else if (state == 1)
  {                      // _________________________ Manual state
    boolean ManualState; // True is serial, false is analog
    boolean ManualModeCheck = false;
    while (ManualModeCheck == false)
    {
      Serial.println();
      delay(500);
      Serial.println();
      Serial.flush();
      while (Serial.available())
      {
        Serial.read();
      }
      Serial.println("What manual mode do you want to use?");
      Serial.println();
      Serial.println("[1] Serial controlled or [2] Analog controlled");
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
    if (ManualState == true)
    { // serial manual mode
      Serial.println();
      delay(1000);
      Serial.println("*You can stop the mode by sending any text*");
      delay(2500);
      Serial.println();
      boolean manualcheck = false;
      boolean FirstcharCheck = false;
      byte ndx = 0;
      int schakelaarEersteToestand = 0;
      float percent;
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
        if (receivedChars[0] != 0 && (receivedChars[0] >= '1' || receivedChars[0] <= '9'))
        {
          String percentString = String(receivedChars);
          int tempPercent = percentString.toInt();
          if (tempPercent <= 100.0)
          {
            percent = tempPercent;
            schakelaarEersteToestand = 1023.0 * (percent / 100.0);
          }
        }
        analogWrite(PWMA, schakelaarEersteToestand / 4);
        float voltage = (INPUT_VOLTAGE * ((float)percent / 100.0));
        String percentS = String(percent, 2); // using a float and the decimal places
        String voltageS = String(voltage, 2);
        Serial.println("Current output: " + percentS + "%  (" + voltageS + "V)");
        if (receivedChars[0] != 0 && (receivedChars[0] < '1' || receivedChars[0] > '9'))
        {
          manualcheck = true;
          state = 0;
          Serial.println();
        }
      }
    }
    else if (ManualState == false)
    { // analog manual mode
      Serial.println();
      delay(1000);
      Serial.println("*You can stop the mode by sending any text*");
      delay(2500);
      Serial.println();
      boolean manualcheck = false;
      boolean FirstcharCheck = false;
      while (manualcheck == false)
      {
        Serial.flush();
        int charInput = Serial.read();
        int schakelaarEersteToestand = analogRead(PIN_ANALOGSCHAKELAAR);
        analogWrite(PWMA, schakelaarEersteToestand / 4);
        float percent = (schakelaarEersteToestand / 1023.0) * 100.0;
        float voltage = (INPUT_VOLTAGE * ((float)percent / 100.0));
        String percentS = String(percent, 2); // using a float and the decimal places
        String voltageS = String(voltage, 2);
        Serial.println("Current output: " + percentS + "%  (" + voltageS + "V)");
        if (charInput > 10)
        {
          manualcheck = true;
          state = 0;
          Serial.println();
        }
      }
    }
  }
  else if (state == 2)
  { // _____________________ Testing mode
    Serial.println();
    boolean checkPrecision = false;
    while (checkPrecision == false)
    {
      Serial.flush();
      while (Serial.available())
      {
        Serial.read();
      }
      Serial.println("How many tests do you want to perform?");
      Serial.println();
      while (Serial.available() == 0)
      {
      }
      String StringInput = Serial.readString();
      Serial.println();
      PRECISION = StringInput.toInt();
      Serial.println();
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
      Serial.println("What should the interval between tests be? [ms]");
      Serial.println();
      while (Serial.available() == 0)
      {
      }
      String StringInput = Serial.readString();
      Serial.println();
      TEST_DELAY = StringInput.toInt();
      Serial.println();
      delay(500);
      if (TEST_DELAY > 0)
      {
        checkDelay = true;
      }
    }
    Serial.println();
    String amountOfTests = String((int)PRECISION);
    delay(2000);
    Serial.println("Precision has been succesfully set to " + amountOfTests + " tests");
    Serial.println();
    delay(1000);
    String amountOfMinutes = String((PRECISION * TEST_DELAY) / 60000, 2);
    Serial.println("This will approximatly take: " + amountOfMinutes + " minutes");
    Serial.println();
    delay(1000);
    for (int i = 10; i > 0; i--)
    {
      Serial.flush();
      int charInput = Serial.read();
      analogWrite(PWMA, 1023);
      String countdown = String(i);
      Serial.println("Testing starts in: " + countdown + "       *Make sure the motor is spinning!*  [A]bort / [S]kip");
      if (charInput == 65)
      {
        i = 0;
        PRECISION = 0;
        Serial.println();
        Serial.println("---Test aborted---");
        Serial.println();
      }
      if (charInput == 83)
      {
        i = 0;
      }
      delay(1000);
    }
    Serial.println();
    Serial.println();
    for (float i = PRECISION; i > 0; i--)
    {
      float currentOutput = ((1023.0 / PRECISION) * i) / 4.011764;
      analogWrite(PWMA, currentOutput);
      float percent = (currentOutput / 255.0) * 100.0;
      float voltage = (INPUT_VOLTAGE * ((float)percent / 100.0));
      String percentS = String(percent, 2); // using a float and the decimal places
      String voltageS = String(voltage, 2);
      String totalTests = String((int)PRECISION);
      String currentTest = String((int)PRECISION - (int)i + 1);
      Serial.println("Currently testing with output: [" + percentS + "%]  (" + voltageS + "V)  " + currentTest + "/" + totalTests);
      delay(TEST_DELAY);
    }
    Serial.println();
    Serial.println("---------------------------------------------------------------------");
    Serial.println();
    Serial.println("_TESTS DONE_");
    delay(200);
    boolean TestAgainCheck = false;
    while (TestAgainCheck == false)
    {
      Serial.flush();
      while (Serial.available())
      {
        Serial.read();
      }
      Serial.println();
      Serial.println("Do you want to test again?");
      Serial.println("[1] Yes or [2] No");
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
            Serial.println();
            Serial.println("Goodbye.");
            Serial.end();
            while (true)
            {
              delay(10000);
            }
          }
          else if (StringInput2 == 49)
          {
            Serial.println();
            Serial.println("_MANUAL MODE ENABLED_");
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
  Serial.println("state has been changed");
  count = millis();
  unsigned long difference = count - startRotation;
  currentFrequency = 1.0f / ((float)difference / 1000);
  Serial.println(currentFrequency, 5);
  startRotation = millis();
}