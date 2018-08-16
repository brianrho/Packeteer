/* Packeteer example over Serial, with addressing
 *  Ensure PACKETEER_ADDRESSING_ENABLED is uncommented in packeteer.h
 */
 
#include <SoftwareSerial.h>
#include <packeteer.h>

#define SEND_INTERVAL   1000  
#define MSG_BUF_SZ      32

SoftwareSerial softy(4, 5);

#define SENDER        1
#define RECVER        2

/* Make one device SENDER and the other a RECVER
 * by commenting out one of the following
 */ 
 
//const int role = SENDER;
const int role = RECVER;

uint16_t send_func(uint8_t * data, uint16_t len);
uint16_t recv_func(uint8_t * data, uint16_t len);
uint16_t avail_func(void);

packeteer_t swhandle;

char obuf[] = "Hello world\r\n";
char ibuf[MSG_BUF_SZ];

unsigned long last_send = 0;
uint8_t own_addr, other_addr;

void setup() {
  Serial.begin(9600);
  softy.begin(9600);

  if (role == SENDER) {
    own_addr = 0x1;
    other_addr = 0x2;
    Serial.println("ROLE: Sender");
    Serial.print("Address: 0x"); Serial.println(own_addr);
    Serial.println();
  }
  else if (role == RECVER) {
    own_addr = 0x2;
    Serial.println("ROLE: Receiver");
    Serial.print("Address: 0x"); Serial.println(own_addr);
    Serial.println();
  }

  packeteer_init(&swhandle, own_addr, send_func, recv_func, avail_func);
}

void loop() {
  if (role == SENDER) {
    if (millis() - last_send > SEND_INTERVAL) {         // send msgs every second to other device
      last_send = millis();
      packeteer_send(&swhandle, obuf, strlen(obuf), other_addr);
      Serial.print("TO 0x"); Serial.print(other_addr); 
      Serial.print(": "); Serial.print(obuf);
    }
  }
  else if (role == RECVER) {
    if (packeteer_avail(&swhandle)) {
      uint8_t src, dest;
      int16_t ret = packeteer_recv(&swhandle, ibuf, MSG_BUF_SZ, &src, &dest); // recv msgs and print everything
      if (ret > 0) {
        Serial.print("FROM 0x"); Serial.print(src); 
        Serial.print(", TO 0x"); Serial.print(dest);
        Serial.print(": "); Serial.write(ibuf, ret);
      }
    }
  }
}

uint16_t send_func(uint8_t * data, uint16_t len) {
  return softy.write(data, len);
}

uint16_t recv_func(uint8_t * data, uint16_t len) {
  return softy.readBytes(data, len);
}

uint16_t avail_func(void) {
  return softy.available();
}

