//Attach coinInt to Interrupt Pin 0 (Digital Pin 2). Pin 3 = Interrpt Pin 1.
const int coinInt = 1; 

//Set the coinsValue to a Volatile float
//Volatile as this variable changes any time the Interrupt is triggered
volatile int coinsValue = 0;

//A Coin has been inserted flag
volatile int coinsChange = 0;

// 50 cent is a 1 that comes from the acceptor
int coinConst = 50;
volatile unsigned long lastPulse = 0;
unsigned long pulseTimeout = 250;
volatile boolean coinFinished = true;
boolean waitForCoinFinish = false;
volatile int currentCoin = 0;

int hopperSensor = A5;
int hopperPin = 3;
int hopperSensorTreshold = 50;
unsigned long coinAt = 0;
unsigned long hopperTimeout = 1000;

boolean hopper = false;

void setup() {
  //Start Serial Communication
  Serial.begin(9600);                 
  
  pinMode(6, INPUT);

  //If coinInt goes HIGH (a Pulse), call the coinInserted function
  //An attachInterrupt will always trigger, even if your using delays
  attachInterrupt(coinInt, coinInserted, RISING);   

  // Set motor direction to forward
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);
  // Disable break
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);

  // Empty hopper on startup
  emptyHopper();
}

//The function that is called every time it recieves a pulse
void coinInserted() {
  //As we set the Pulse to represent 5p or 5c we add this to the coinsValue
  coinsValue = coinsValue + coinConst;
  //Flag that there has been a coin inserted
  coinsChange = 1;

  // Our machine only recognizes 50cent/1 Euro.
  // To avoid failures we will hardcode that
  currentCoin += coinConst;
  if(currentCoin > 100) {
    currentCoin = 100;
    Serial.println("d:too much money detected");
  }
  
  coinFinished = false;

  lastPulse = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  int read = analogRead(hopperSensor);
  if(read < hopperSensorTreshold) {
    // Coin wurde ausgeworfen
    coinAt = millis();
    // elapsed millis ???
    //Serial.println("COIN Sensor triggered");
    //Serial.println(read);
  }

  // Read input from Serial client
  char input = (char)Serial.read();

  // Command e = eject
  if(input == 'e') {
    emptyHopper();
  }

  // Command p = give current credit
  if(input == 'p') {
    Serial.println("p:"+String(coinsValue));
  }

  // Needed for millis calculation (why?)
  delay(1);
  
  // Hopper is empty
  if ((unsigned long)(millis() - coinAt) >= hopperTimeout) {
    // If hopper is switchched on
    if(hopper == true) {
      coinAt = millis();
      hopper = false;
      coinsValue = 0;
      // Tell client that hopper is empty
      Serial.println("e:1");
    }
  }

  // Tell client which type of coin was inserted
  // and reset coin counting
  if(((unsigned long)(millis() - lastPulse) >= pulseTimeout) && currentCoin > 0) {
    // prevent this command from sending nonsense
    if(currentCoin != 50 && currentCoin != 100) {
      if(currentCoin <= 50) {
        currentCoin = 50;
      } else {
        currentCoin = 100;
      }
    }
    Serial.println("c:"+String(currentCoin));
    coinFinished = true;
    currentCoin = 0;
  }

  // Activate or deactivate hopper
  if(hopper == false) {
    analogWrite(hopperPin, 0);
  } else {
    analogWrite(hopperPin, 255);
  }
}

// Emptying the hopper
void emptyHopper() {
  // Activate hopper
  if(hopper == false) {
    coinAt = millis();
    hopper = true;
    // Tell client that hopper started
    Serial.println("e:0");
  }
}
