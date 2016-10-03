#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include "project2.h"


enum fsm_state b_state = wait_for_call_0,b_last_state;
char debugmsg[MESSAGE_LENGTH],*b_data_received;
int b_new_received = FALSE,b_timed_out = FALSE;
int b_seq_received,b_ack_received,b_chk_received,last_seq_received,ack_len;
char *nack,*ack;
struct pkt bsendpkt;
struct pkt *b_send_packet;

void deliver(char *data_received)
{
	struct msg received_message;
	memcpy(&received_message.data,data_received,MESSAGE_LENGTH);
	tolayer5(BEntity,received_message);
}

int b_send_pkt(int seqnum, int acknum, char* data)
{
	b_send_packet = realloc(b_send_packet, sizeof(struct pkt));
	memset(b_send_packet->payload, 0,MESSAGE_LENGTH);
	memcpy(b_send_packet->payload, data,MESSAGE_LENGTH);
	b_send_packet->seqnum 			= seqnum;
	b_send_packet->acknum 			= acknum;
	b_send_packet->checksum 		= calc_checksum(b_send_packet->payload,b_send_packet->seqnum,b_send_packet->acknum, MESSAGE_LENGTH);
	tolayer3(BEntity,*b_send_packet);
	sprintf(debugmsg,"B SENT seq: %d\n",b_send_packet->seqnum);
	debug(debugmsg,6);
}

void b_receive_pkt(struct pkt packet)
{
	strncpy(b_data_received, packet.payload,MESSAGE_LENGTH);
	b_seq_received = packet.seqnum;
	b_ack_received = packet.acknum;
	b_chk_received = packet.checksum - calc_checksum(packet.payload,packet.seqnum,packet.acknum, MESSAGE_LENGTH);
	sprintf(debugmsg,"B Received %d , b_chk: %d vs %d == %d\n",b_seq_received, packet.checksum, calc_checksum(packet.payload,packet.seqnum,packet.acknum, MESSAGE_LENGTH), b_chk_received);
	debug(debugmsg,5);
	b_new_received = TRUE;

	if(!b_chk_received && b_seq_received == last_seq_received+1)
	{
		deliver(b_data_received);
		last_seq_received = b_seq_received;
	}

	b_send_pkt(last_seq_received,1,ack);
}



void init_b(){
	//ack = (struct msg)malloc(sizeof(struct msg));
	//nack = (struct msg)malloc(sizeof(struct msg));
/*	ack_len = 14;
	memcpy(ack.data, ack_str,ack_len);
	memcpy(nack.data, nack_str,ack_len);
*/
	b_send_packet = realloc(b_send_packet,MESSAGE_LENGTH);
	b_data_received = realloc(b_data_received,MESSAGE_LENGTH);
	ack_len = MESSAGE_LENGTH;
	ack = realloc(ack, ack_len);
	nack = realloc(nack, ack_len);
	memcpy(ack,"THIS IS AN ACK",ack_len);
	memcpy(nack,"THIS IS A NACK",ack_len);
	last_seq_received = -1;

}
