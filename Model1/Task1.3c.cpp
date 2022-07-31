volatile byte r,g,b = 0;

const int LED_r = 11;
const int LED_g = 10;
const int LED_b = 9;

void setup()
{
  pinMode(LED_r, OUTPUT);
  pinMode(LED_g, OUTPUT);
  pinMode(LED_b, OUTPUT);
  Serial.begin(9600);
  
  PCICR |= 0b00000111;
  PCMSK2 |= 0b00011100;
  
}

void loop()
{
  
}

ISR(PCINT2_vect)
{
  r = PIND & B00010000;
  g = PIND & B00001000;
  b = PIND & B00000100;
  
  digitalWrite(LED_r, r);
  digitalWrite(LED_g, g);
  digitalWrite(LED_b, b);
  
  Serial.print("LED_r: ");
  Serial.print(digitalRead(LED_r));
  Serial.print(", ");
  Serial.print("PIN4: ");
  Serial.print(digitalRead(4));
  Serial.print(", ");
  Serial.print("LED_g: ");
  Serial.print(digitalRead(LED_g));
  Serial.print(", ");
  Serial.print("PIN3: ");
  Serial.print(digitalRead(3));
  Serial.print(", ");
  Serial.print("LED_b: ");
  Serial.print(digitalRead(LED_b));
  Serial.print(", ");
  Serial.print("PIN2: ");
  Serial.print(digitalRead(2));
  
  Serial.println("");
}