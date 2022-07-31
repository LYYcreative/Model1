// C++ code
const int LED_PIN = 13;
const int PIR_PIN = 2;

int PIR_STATE = 0;

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  
  Serial.begin(9600);
}

void loop()
{
  
  PIR_STATE = digitalRead(PIR_PIN);
  
  if(PIR_STATE == HIGH)
  {
    digitalWrite(LED_PIN, HIGH);
  }
  else
  {
    digitalWrite(LED_PIN, LOW);
  }
  int LED_STATE = digitalRead(LED_PIN);
  Serial.print(LED_STATE);
  Serial.print(PIR_STATE);
  Serial.println("");
  delay(500);
}