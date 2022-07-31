// C++ code

const int LED_PIN = 13;
const int PIR_PIN = 2;

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), Switch, CHANGE);
  Serial.begin(9600);
}

void loop()
{
}

void Switch()
{
  int PIR_STATE = digitalRead(PIR_PIN);
  digitalWrite(LED_PIN, PIR_STATE);
  int LED_STATE = digitalRead(LED_PIN);
  Serial.print(PIR_STATE);
  Serial.print(LED_STATE);
  Serial.println("");
}