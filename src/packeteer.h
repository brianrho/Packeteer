#ifndef PACKETEER_H_
#define PACKETEER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define PACKETEER_MAX_PKT_LEN       32

//#define PACKETEER_ADDRESSING_ENABLED

#define PACKETEER_READ_INCOMPLETE   -1

typedef uint16_t (*packeteer_send_func) (uint8_t *, uint16_t);
typedef uint16_t (*packeteer_recv_func) (uint8_t *, uint16_t);
typedef uint16_t (*packeteer_avail_func) (void);
typedef void (*packeteer_yield_func) (void);
// may need a peek function eventually

typedef enum {
	PACKETEER_STATE_READ_HEADER,
	PACKETEER_STATE_READ_CONTENTS,
} packeteer_state_e;

typedef struct {
    uint8_t addr;
    packeteer_send_func sfunc;
    packeteer_recv_func rfunc;
    packeteer_avail_func afunc;
    packeteer_yield_func yfunc;
    
    // variables used for reading, maintained between states
    packeteer_state_e read_state;
    uint8_t src_addr;
    uint8_t dest_addr;
    uint16_t to_read;
    
    // buffers for formatting data before sending and for receiving
    uint8_t ibuf[PACKETEER_MAX_PKT_LEN + 5];
    uint8_t obuf[PACKETEER_MAX_PKT_LEN + 5];
} packeteer_t;

#if defined(PACKETEER_ADDRESSING_ENABLED)
    void packeteer_init(packeteer_t * pteer, uint8_t addr, packeteer_send_func sfunc, 
                        packeteer_recv_func rfunc, packeteer_avail_func afunc);
    uint16_t packeteer_send(packeteer_t * pteer, const void * data, uint16_t len, uint8_t to_addr);
    int16_t packeteer_recv(packeteer_t * pteer, void * data, uint16_t len, uint8_t * from_addr, uint8_t * to_addr);
#else
    void packeteer_init(packeteer_t * pteer, packeteer_send_func sfunc, 
                        packeteer_recv_func rfunc, packeteer_avail_func afunc);
    uint16_t packeteer_send(packeteer_t * pteer, const void * data, uint16_t len);
    uint16_t packeteer_recv(packeteer_t * pteer, void * data, uint16_t len);
#endif

void packeteer_set_yield_func(packeteer_t * pteer, packeteer_yield_func yfunc);
uint16_t packeteer_avail(packeteer_t * pteer);
void packeteer_reset_state(packeteer_t * pteer);

#ifdef __cplusplus
}
#endif

#endif
