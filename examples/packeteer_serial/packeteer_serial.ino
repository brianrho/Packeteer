/* Packeteer example over UART.
*/

#include <SoftwareSerial.h>
#include <packeteer.h>

#define MSG_BUF_SZ    32

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

void setup() {
    Serial.begin(9600);
    softy.begin(9600);

    if (role == SENDER) {
        Serial.println("ROLE: Sender");
        Serial.println();
    }
    else if (role == RECVER) {
        Serial.println("ROLE: Receiver");
        Serial.println();
    }

    /* init packeteer */
    packeteer_init(&swhandle, send_func, recv_func, avail_func);
}

void loop() {
    /* send "hello world" every second, as a single packet */
    if (role == SENDER) {
        if (millis() - last_send > 1000) {
            last_send = millis();
            packeteer_send(&swhandle, obuf, strlen(obuf));
            Serial.print("Sent: "); Serial.print(obuf);
        }
    }
    /* print whatever's recved */
    else if (role == RECVER) {
        if (packeteer_avail(&swhandle) == 0)
            return;
        
        int16_t ret = packeteer_recv(&swhandle, ibuf, MSG_BUF_SZ);
        if (ret > 0) {
            Serial.print("Recved: "); Serial.write(ibuf, ret);
        }
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

