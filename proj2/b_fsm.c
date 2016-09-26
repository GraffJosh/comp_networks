#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include "project2.h"

#define JPGTRACE 3

enum fsm_state b_state = wait_for_call_0;
char debugmsg[MESSAGE_LENGTH],*b_data_received;
int b_seq_received,b_ack_received,b_chk_received,last_chk_received,ack_len;
char *nack,*ack;
struct pkt bsendpkt;

int b_send_pkt(int seqnum, int acknum, char* data)
{
/*	bsendpkt.seqnum 		= seqnum;
	bsendpkt.acknum 		= acknum;
	bsendpkt.checksum 	= calc_checksum(data->data);
	printf("BSenddata: %s\n",data->data );
	memmove(bsendpkt.payload, data->data,sizeof(data->data));
	tolayer3(BEntity,bsendpkt);	

	sprintf(debugmsg,"B send seqnum: %d\n",bsendpkt.seqnum);
	debug(debugmsg,5);
*/
	struct pkt *packet;
	packet = malloc(sizeof(struct pkt));
	memset(packet, 0,sizeof(struct pkt));
	memset(packet->payload, 0, MESSAGE_LENGTH);
	packet->seqnum 		= seqnum;
	packet->acknum 		= acknum;
	packet->checksum 	= calc_checksum(data,14);
	strncpy(packet->payload, data,14);
	tolayer3(BEntity,*packet);
	sprintf(debugmsg,"B SENT: %s Seq: %d\n",data, packet->seqnum);
	debug(debugmsg,3);
	free(packet);
}

void b_receive_pkt(struct pkt packet)
{
	last_chk_received = b_chk_received;
	strncpy(b_data_received, packet.payload,MESSAGE_LENGTH);
	b_seq_received = packet.seqnum;
	b_ack_received = packet.acknum;
	b_chk_received = packet.checksum - calc_checksum(b_data_received,MESSAGE_LENGTH);
	sprintf(debugmsg,"B Received data: %s , b_chk: %d vs %d\n",b_data_received, packet.checksum, calc_checksum(b_data_received,MESSAGE_LENGTH));
	debug(debugmsg,3);

}

void deliver(char *data_received)
{
	struct msg received_message;
	memcpy(&received_message.data,data_received,MESSAGE_LENGTH);
	tolayer5(BEntity,received_message);
}

void b_fsm()
{
	switch(b_state)
	{
		case wait_for_call_0:
			if(b_seq_received ==0 && (b_chk_received == 0)|| b_chk_received == last_chk_received){
				deliver(b_data_received);
				sprintf(debugmsg,"B0 received correct: checksum: %d\n", b_chk_received);
				debug(debugmsg,3);
				b_send_pkt(0,1,ack);
				b_state = wait_for_call_1;
			}else if(b_seq_received ==0 && b_chk_received != 0){
				struct msg message;
				sprintf(debugmsg,"B0 received corrupt packet in 0!chk: %d\n", b_chk_received);
				debug(debugmsg,3);
				b_send_pkt(b_seq_received,2,nack);
				b_state = wait_for_call_0;
			}else if(b_seq_received ==1 && b_chk_received == 0)
			{
				struct msg message;
				memcpy(&message.data, &b_data_received,MESSAGE_LENGTH);
				sprintf(debugmsg,"B0 received duplicate: %d\n", b_chk_received);
				debug(debugmsg,3);
				b_send_pkt(1,1,ack);
				b_state = wait_for_call_0;
			}

			sprintf(debugmsg,"B0 expected seqnum: %d || received seqnum: %d \n", 1, b_seq_received);
			debug(debugmsg,5);
			sprintf(debugmsg,"B0 expected checksum: %d || received checksum: %d || last checksum: %d\n", calc_checksum(b_data_received,MESSAGE_LENGTH), b_chk_received,last_chk_received);
			debug(debugmsg,5);
		break;
		case wait_for_call_1:
			
			if(b_seq_received ==1 && (b_chk_received == 0 )){
				deliver(b_data_received);
				sprintf(debugmsg,"B1 received correct checksum: %d\n", b_chk_received);
				debug(debugmsg,3);
				b_send_pkt(1,1,ack);
				b_state = wait_for_call_0;
			}else if(b_seq_received ==1 && b_chk_received != 0){
				struct msg message;
				sprintf(debugmsg,"B1 received corrupt packet in 1!chk: %d\n", b_chk_received);
				debug(debugmsg,3);
				b_send_pkt(b_seq_received,2,nack);
				//b_state = wait_for_call_1;
			}else if(b_seq_received ==0 && b_chk_received == 0)
			{
				struct msg message;
				memcpy(&message.data, &b_data_received,MESSAGE_LENGTH);
				sprintf(debugmsg,"B1 received duplicate: %d\n", b_chk_received);
				debug(debugmsg,3);
				b_send_pkt(0,1,ack);
				b_state = wait_for_call_1;
			}
			sprintf(debugmsg,"B1 expected seqnum: %d || received seqnum: %d \n", 1, b_seq_received);
			debug(debugmsg,5);
			sprintf(debugmsg,"B1 expected checksum: %d || received checksum: %d || last checksum: %d\n", calc_checksum(b_data_received,MESSAGE_LENGTH), b_chk_received,last_chk_received);
			debug(debugmsg,5);

		break;
		default:
		break;
	}
}

void init_b(){
	//ack = (struct msg)malloc(sizeof(struct msg));
	//nack = (struct msg)malloc(sizeof(struct msg));
/*	ack_len = 14;
	memcpy(ack.data, ack_str,ack_len);
	memcpy(nack.data, nack_str,ack_len);
*/
	b_data_received = realloc(b_data_received,MESSAGE_LENGTH);
	ack_len = MESSAGE_LENGTH;
	ack = realloc(ack, ack_len);
	nack = realloc(nack, ack_len);
	memcpy(ack,"THIS IS AN ACK",ack_len);
	memcpy(nack,"THIS IS A NACK",ack_len);
}
