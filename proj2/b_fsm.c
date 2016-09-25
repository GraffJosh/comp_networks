#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include "project2.h"

#define JPGTRACE 3

enum fsm_state b_state = wait_for_call_0;
char debugmsg[MESSAGE_LENGTH];


int b_send_pkt(int seqnum, int acknum, int checksum, char* data)
{

	struct pkt packet;

	packet.seqnum 		= seqnum;
	packet.acknum 		= acknum;
	packet.checksum 	= checksum;
	memcpy(&packet.payload, data,sizeof(char)*MESSAGE_LENGTH);
	tolayer3(BEntity,packet);	

	sprintf(debugmsg,"B send seqnum: %d\n",packet.seqnum);
	debug(debugmsg,5);
}

void b_receive_pkt(struct pkt packet)
{
	struct msg message;
	memcpy(&message.data,&packet.payload,MESSAGE_LENGTH);
	printf(" messagedata: %s\n", message.data);
	//if the checksum of the data is the same as the received checksum
	if(calc_checksum(message.data) == packet.checksum)
	{
		tolayer5(BEntity,message);	
		sprintf(debugmsg,"B received seqnum: %d\n", packet.seqnum);
		debug(debugmsg,5);
		b_send_pkt(packet.seqnum,1,calc_checksum(ack_str),ack_str);
	}else{	
		sprintf(debugmsg,"B received corrupt packet!: %d\n", packet.seqnum);
		debug(debugmsg,3);
		b_send_pkt(packet.seqnum,2,calc_checksum(nack_str),nack_str);
	}
	sprintf(debugmsg,"B expected checksum: %d || received checksum: %d\n", calc_checksum(message.data), packet.checksum);
	debug(debugmsg,3);
}


void b_fsm()
{
	switch(b_state)
	{
		case wait_for_call_0:

		break;
		case wait_for_call_1:

		break;
		default:
		break;
	}
}
