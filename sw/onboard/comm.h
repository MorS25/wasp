#ifndef COMM_H
#define COMM_H

#include "std.h"
#include "generated/messages.h"

#define STX 0x99
#define ACID 120
#define NUM_NON_PAYLOAD_BYTES 6

#define PPRZ_PAYLOAD_LEN 256

typedef enum {
    COMM_1,
    COMM_2,
    COMM_NB
} CommChannel_t;

typedef enum {
    STATE_UNINIT,
    STATE_GOT_STX,
    STATE_GOT_LENGTH,
    STATE_GOT_ACID,
    STATE_GOT_MSGID,
    STATE_GOT_PAYLOAD,
    STATE_GOT_CRC1
} ParseState_t;
    
typedef struct __CommStatus {
    uint8_t ck_a;
    uint8_t ck_b;
    uint8_t msg_received;
    uint8_t buffer_overrun;
    uint8_t parse_error;
    ParseState_t parse_state;
} CommStatus_t;

typedef bool_t (*CommMessageCallback_t)(CommMessage_t *message);

extern CommMessageCallback_t    comm_callback[COMM_NB];
extern CommMessage_t            comm_message[COMM_NB];
extern CommStatus_t             comm_status[COMM_NB];
extern bool_t                   comm_channel_used[COMM_NB];

void
comm_init ( CommChannel_t chan );

void
comm_periodic_task ( CommChannel_t chan );

bool_t
comm_event_task ( CommChannel_t chan );

bool_t
comm_ch_available ( CommChannel_t chan );

void 
comm_send_ch ( CommChannel_t chan, uint8_t c );

void
comm_send_message ( CommChannel_t chan, CommMessage_t *message );

bool_t
comm_send_message_by_id (CommChannel_t chan, uint8_t msgid);

uint8_t
comm_get_ch( CommChannel_t chan );

bool_t
comm_check_free_space ( CommChannel_t chan, uint8_t len );

void
comm_start_message ( CommChannel_t chan, uint8_t id, uint8_t len );

void
comm_end_message ( CommChannel_t chan );

bool_t
comm_parse ( CommChannel_t chan );

#endif /* COMM_H */

