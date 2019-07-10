#ifndef PACKETEER_H_
#define PACKETEER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* uncomment this to enable addressing */
#define PACKETEER_ADDRESSING_ENABLED

/* maximum payload length that can be sent or recvd by an application */
#define PACKETEER_MAX_PKT_LEN       32

/* returned whenever only a portion of a packet is available to be read */
#define PACKETEER_READ_INCOMPLETE   -1

/* packeteer machine states, only used internally */
typedef enum {
	PACKETEER_STATE_READ_HEADER,
    PACKETEER_STATE_READ_LENGTH,
	PACKETEER_STATE_READ_CONTENTS,
} packeteer_state_e;

/* signatures for send, recv and avail functions
 * to be supplied by application code
 */
typedef uint16_t (*packeteer_send_func) (uint8_t *, uint16_t);
typedef uint16_t (*packeteer_recv_func) (uint8_t *, uint16_t);
typedef uint16_t (*packeteer_avail_func) (void);
typedef void (*packeteer_yield_func) (void);
/* may need a peek function eventually */

/* struct for each packeteer handle created by user application,
 * holds all the data needed to maintain state between function calls 
 */
typedef struct {
    /* send, recv, etc. functions for a given packeteer handle */
    uint8_t addr;
    packeteer_send_func sfunc;
    packeteer_recv_func rfunc;
    packeteer_avail_func afunc;
    packeteer_yield_func yfunc;
    
    /* variables used for reading, maintained between states */
    packeteer_state_e read_state;
    uint8_t src_addr;
    uint8_t dest_addr;
    uint16_t to_read;
    
    /* buffers for formatting data before sending and for receiving */
    uint8_t ibuf[PACKETEER_MAX_PKT_LEN + 5];
    uint8_t obuf[PACKETEER_MAX_PKT_LEN + 5];
} packeteer_t;

/* function names and arguments are self-explanatory, see examples for usage */
#if defined(PACKETEER_ADDRESSING_ENABLED)
    void packeteer_init(packeteer_t * pteer, uint8_t addr, packeteer_send_func sfunc, 
                        packeteer_recv_func rfunc, packeteer_avail_func afunc);
    uint16_t packeteer_send(packeteer_t * pteer, const void * data, uint16_t len, uint8_t to_addr);
    int16_t packeteer_recv(packeteer_t * pteer, void * data, uint16_t len, uint8_t * from_addr, uint8_t * to_addr);
#else
    void packeteer_init(packeteer_t * pteer, packeteer_send_func sfunc, 
                        packeteer_recv_func rfunc, packeteer_avail_func afunc);
    uint16_t packeteer_send(packeteer_t * pteer, const void * data, uint16_t len);
    int16_t packeteer_recv(packeteer_t * pteer, void * data, uint16_t len);
#endif

/* set a yield function, this is optional, unless you have a WDT */
void packeteer_set_yield_func(packeteer_t * pteer, packeteer_yield_func yfunc);
uint16_t packeteer_avail(packeteer_t * pteer);

/* reset the packeteer state machine */
void packeteer_reset_state(packeteer_t * pteer);

#ifdef __cplusplus
}
#endif

#endif
