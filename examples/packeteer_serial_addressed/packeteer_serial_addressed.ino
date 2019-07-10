/* Packeteer example over UART, with addressing.
 * Ensure PACKETEER_ADDRESSING_ENABLED is uncommented in packeteer.h
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
    
    /* init packeteer */
    packeteer_init(&swhandle, own_addr, send_func, recv_func, avail_func);
}

void loop() {
    /* send "hello world" every second to other_addr, as a single packet */
    if (role == SENDER) {
        if (millis() - last_send > SEND_INTERVAL) {
            last_send = millis();
            packeteer_send(&swhandle, obuf, strlen(obuf), other_addr);
            Serial.print("TO 0x"); Serial.print(other_addr); 
            Serial.print(": "); Serial.print(obuf);
        }
    }
    /* print whatever's recved */
    else if (role == RECVER) {
        if (packeteer_avail(&swhandle) == 0)
            return;
        
        uint8_t src, dest;
        int16_t ret = packeteer_recv(&swhandle, ibuf, MSG_BUF_SZ, &src, &dest);
        if (ret <= 0)
            return;
        Serial.print("FROM 0x"); Serial.print(src); 
        Serial.print(", TO 0x"); Serial.print(dest);
        Serial.print(": "); Serial.write(ibuf, ret);
    }
}

/* functions used by packeteer */
uint16_t send_func(uint8_t * data, uint16_t len) {
    return softy.write(data, len);
}

uint16_t recv_func(uint8_t * data, uint16_t len) {
    return softy.readBytes(data, len);
}

uint16_t avail_func(void) {
    return softy.available();
}

