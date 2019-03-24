/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 *                                 client.ino                                  *                                 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 *
 * This program is to be loaded onto a Particle Photon functioning as the 
 * remote control for an RC Trebuchet. The photon has 3 external inputs:
 *  - Joystick X-direction ADC reading 
 *  - Joystick Y-direction ADC reading
 *  - Joystick switch ON/OFF
 * 
 * It must wirelessly communicate the state of the joystick to a Raspberry Pi, 
 * sitting on the Trebuchet. The Raspberry Pi is running a TCP server, and 
 * the Photon acts as a client. A 25 bit stream is sent to the server:
 *  { 1 bit FIRE, 12 bits JS_X, 12 bits JS_Y }
 * 
 * and the server sends a 1 bit acknowledgment back. The Photon hangs until
 * reading the acknowledgment in order to sync communications. 
 * 
 * The joystick's switch is attached to an interrupt, and will forcefully 
 * send the fire code (MSB = 1 => fire, otherwise nothing). This is to 
 * ensure that button pushes which occur during the hanging periods are still
 * handled. 
 */

#define PORT 8080
#define SEND_PER 10
#define JX_PIN A0
#define JY_PIN A1
#define JSW_PIN D1 // Interrupts not supported on D1
#define ADC_RES 12 // in bits
#define BUF_SIZE (2 * ADC_RES + 1)

/* Declaring global variables */
TCPClient client;
//byte server[] = {128, 237, 162, 159}; // Joe PC
byte server[] = {128, 237, 184, 183};
byte send_buf[BUF_SIZE];
byte fire_code[BUF_SIZE];
unsigned char read_buf[1];

void setup() {
  delay(3000);

  // Make sure your Serial Terminal app is closed before powering your device
  Serial.begin(9600);

  fire_code[BUF_SIZE - 1] = 1;
  send_buf[BUF_SIZE - 1] = 0;

  client.connect(server, PORT);  
  Serial.println("Connection success.");

  attachSystemInterrupt(JSW_PIN, handle_fire, UP);
}

/* 
 * loop(): Execute the main application loop.
 * 
 * First gathers ADC readings from joystick. Then, fills the sending buffer
 * with the joystick data and sends. 
 */
void loop() {
  /* Reading JS vals */
  int x_dc, y_dc;

  x_dc = analogRead(JX_PIN); // ADC resolution is 12 bits => maxval 4095
  y_dc = analogRead(JY_PIN);

  /* Communication with server */

  // Filling the sending buffer with new x and y vals
  for(int a = 0; a < ADC_RES; a++) {
    send_buf[a] = (y_dc >> a) & 0x1;
    send_buf[a + ADC_RES] = (x_dc >> a) & 0x1;
  }

  if(!client.connected()) {
    Serial.println("Server disconnected. ")
    client.stop();
  }

  else {
    client.write(send_buf, (size_t)BUF_SIZE);
    Serial.println("Sent x = %d, y = %d", x_dc, y_dc);
    // Wait for server ack before proceeding
    while(client.read(read_buf, 1) != -1);
  }
  delay(SEND_PER);
}

/*
 * Handles a positive edge on D1 (JSW_PIN) by forcefully sending a fire signal.
 */
void handle_fire() {
  if(!client.connected()) {
    client.stop();
    Serial.println("Error sending fire: Disconnected from server");
  }

  else {
    client.write(fire_code, BUF_SIZE);
    Serial.println("FIRE!");
  }
}