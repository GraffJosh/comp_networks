#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include "project2.h"


void push_message(struct msg message)
{

}

struct msg pop_message()
{

}

int a_send_pkt(int seqnum, int acknum, int checksum, char* data)
{
	struct pkt packet;

	packet.seqnum 		= seqnum;
	packet.acknum 		= acknum;
	packet.checksum 	= checksum;
	memcpy(&packet.payload, data,sizeof(char)*MESSAGE_LENGTH);
	tolayer3(AEntity,packet);
}



void A_FSM()
{

}
