#include "packeteer.h"
#include <stddef.h>

#define PACKETEER_HEADER    0xAF

//extern void printit(char * str);

#if defined(PACKETEER_ADDRESSING_ENABLED)

void packeteer_init(packeteer_t * pteer, uint8_t addr, packeteer_send_func sfunc, 
                    packeteer_recv_func rfunc, packeteer_avail_func afunc) 
{
    pteer->addr = addr;
    pteer->sfunc = sfunc;
    pteer->rfunc = rfunc;
    pteer->afunc = afunc;
    pteer->yfunc = NULL;
    pteer->read_state = PACKETEER_STATE_READ_HEADER;
}

// Writes a single packet
uint16_t packeteer_send(packeteer_t * pteer, const void * data, uint16_t len, uint8_t to_addr) {
    if (len == 0 || pteer->sfunc == NULL)               // no zero-length writes allowed
        return PACKETEER_READ_INCOMPLETE;
    
    uint8_t to_write, chksum = 0;
    pteer->obuf[0] = PACKETEER_HEADER;
    
    to_write = (len > PACKETEER_MAX_PKT_LEN) ? PACKETEER_MAX_PKT_LEN : len;

    pteer->obuf[1] = to_write;
    pteer->obuf[2] = to_addr;
    
    chksum = (PACKETEER_HEADER + to_write + to_addr) & 0xff;
    
    uint8_t * ptr = (uint8_t *)data;
    for (uint16_t i = 0; i < to_write; i++) {
        pteer->obuf[3 + i] = ptr[i];
        chksum += ptr[i];
    }
    
    chksum += pteer->addr;
    pteer->obuf[to_write + 3] = pteer->addr;
    pteer->obuf[to_write + 3 + 1] = ~chksum + 1;
    
    pteer->sfunc(pteer->obuf, to_write + 5);

    return to_write;
}

int16_t packeteer_recv(packeteer_t * pteer, void * data, uint16_t len, uint8_t * from_addr, uint8_t * to_addr) {
    if (len == 0 || pteer->rfunc == NULL || pteer->afunc == NULL)
        return PACKETEER_READ_INCOMPLETE;
    
    while (1) {
        switch (pteer->read_state) {
            case PACKETEER_STATE_READ_HEADER:
                if (pteer->afunc() < 3)
                    return PACKETEER_READ_INCOMPLETE;
                
                pteer->rfunc(pteer->ibuf, 1);
                if (pteer->ibuf[0] != PACKETEER_HEADER)
                    return PACKETEER_READ_INCOMPLETE;
                
                pteer->rfunc(pteer->ibuf, 1);
                pteer->to_read = pteer->ibuf[0];
                // check the length, but could miss a header this way
                if (pteer->to_read > PACKETEER_MAX_PKT_LEN) {
                    if (pteer->yfunc != NULL) pteer->yfunc();
                    continue;
                }
                
                pteer->rfunc(pteer->ibuf, 1);
                pteer->dest_addr = pteer->ibuf[0];
                
                pteer->read_state = PACKETEER_STATE_READ_CONTENTS;
                break;
                
            case PACKETEER_STATE_READ_CONTENTS:
                if (pteer->afunc() < pteer->to_read + 2)            // +2 for src address + chksum
                    return PACKETEER_READ_INCOMPLETE;
                if (pteer->to_read > len)
                    return 0;
                
                pteer->read_state = PACKETEER_STATE_READ_HEADER;     // not coming back here again
                
                pteer->rfunc(pteer->ibuf, pteer->to_read + 2);
                uint8_t chksum = (PACKETEER_HEADER + pteer->to_read + pteer->dest_addr) & 0xff;
                
                uint8_t * ptr = (uint8_t *)data;
                for (uint16_t i = 0; i < pteer->to_read; i++) {
                    ptr[i] = pteer->ibuf[i];
                    chksum += pteer->ibuf[i];
                }
                
                pteer->src_addr = pteer->ibuf[pteer->to_read];
                
                chksum += pteer->src_addr;                 // add src addr
                chksum += pteer->ibuf[pteer->to_read + 1];        // finally add embedded chksum
                
                if (chksum != 0)                    // abort and try to read another
                    continue;
                    
                if (from_addr != NULL)
                    *from_addr = pteer->src_addr;
                if (to_addr != NULL)
                    *to_addr = pteer->dest_addr;
                
                return pteer->to_read;
            default:
                break;
        }
        if (pteer->yfunc != NULL) pteer->yfunc();
    }
}

#else
   
void packeteer_init(packeteer_t * pteer, packeteer_send_func sfunc, 
                    packeteer_recv_func rfunc, packeteer_avail_func afunc) 
{
    pteer->sfunc = sfunc;
    pteer->rfunc = rfunc;
    pteer->afunc = afunc;
    pteer->yfunc = NULL;
    pteer->read_state = PACKETEER_STATE_READ_HEADER;
}

uint16_t packeteer_send(packeteer_t * pteer, const void * data, uint16_t len) {
    if (len == 0 || pteer->sfunc == NULL)               // no zero-length writes allowed
        return PACKETEER_READ_INCOMPLETE;
    
    uint8_t to_write, chksum = 0;
    pteer->obuf[0] = PACKETEER_HEADER;
    
    to_write = (len > PACKETEER_MAX_PKT_LEN) ? PACKETEER_MAX_PKT_LEN : len;

    pteer->obuf[1] = to_write;
    
    chksum = (PACKETEER_HEADER + to_write) & 0xff;
    
    uint8_t * ptr = (uint8_t *)data;
    for (uint16_t i = 0; i < to_write; i++) {
        pteer->obuf[2 + i] = ptr[i];
        chksum += ptr[i];
    }
    
    pteer->obuf[to_write + 2] = ~chksum + 1;
    
    pteer->sfunc(pteer->obuf, to_write + 3);

    return to_write;
}
 
int16_t packeteer_recv(packeteer_t * pteer, void * data, uint16_t len) {
    if (len == 0 || pteer->rfunc == NULL)
        return PACKETEER_READ_INCOMPLETE;
    
    while (1) {
        switch (pteer->read_state) {
            case PACKETEER_STATE_READ_HEADER:
                if (pteer->afunc() == 0)
                    return PACKETEER_READ_INCOMPLETE;
                
                pteer->rfunc(pteer->ibuf, 1);
                if (pteer->ibuf[0] != PACKETEER_HEADER) {
                    if (pteer->yfunc != NULL) pteer->yfunc();
                    continue;
                }
                
                pteer->read_state = PACKETEER_STATE_READ_LENGTH;
                break;
                
            case PACKETEER_STATE_READ_LENGTH:
                if (pteer->afunc() == 0)
                    return PACKETEER_READ_INCOMPLETE;
                
                pteer->rfunc(pteer->ibuf, 1);
                pteer->to_read = pteer->ibuf[0];
                if (pteer->to_read > PACKETEER_MAX_PKT_LEN) {          // check the length
                    if (pteer->yfunc != NULL) pteer->yfunc();
                    pteer->read_state = PACKETEER_STATE_READ_HEADER;
                    continue;
                }
                
                pteer->read_state = PACKETEER_STATE_READ_CONTENTS;
                break;
                
            case PACKETEER_STATE_READ_CONTENTS:
                if (pteer->afunc() < pteer->to_read + 1)                   // +1 for chksum
                    return PACKETEER_READ_INCOMPLETE;
                if (pteer->to_read > len)
                    return 0;
                
                pteer->read_state = PACKETEER_STATE_READ_HEADER;       // not coming back here again
                
                pteer->rfunc(pteer->ibuf, pteer->to_read + 1);                   // could miss a packet this way too
                uint8_t chksum = (PACKETEER_HEADER + pteer->to_read) & 0xff;
                
                uint8_t * ptr = (uint8_t *)data;
                for (uint16_t i = 0; i < pteer->to_read; i++) {
                    ptr[i] = pteer->ibuf[i];
                    chksum += pteer->ibuf[i];
                }
                
                chksum += pteer->ibuf[pteer->to_read];
                if (chksum != 0)                    // abort and try to read another
                    continue;
                return pteer->to_read;
            default:
                break;
        }
        if (pteer->yfunc != NULL) pteer->yfunc();
    }
}

#endif

void packeteer_set_yield_func(packeteer_t * pteer, packeteer_yield_func yfunc) {
    pteer->yfunc = yfunc;
}

uint16_t packeteer_avail(packeteer_t * pteer) {
    return pteer->afunc();
}

void packeteer_reset_state(packeteer_t * pteer) {
    pteer->read_state = PACKETEER_STATE_READ_HEADER;
}
