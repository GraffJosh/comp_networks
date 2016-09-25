#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include "project2.h"
#define JPGTRACE 3

enum fsm_state a_state = wait_for_call_0;
struct msg *a_buffer;
int a_buffer_length=0;
char debugmsg[MESSAGE_LENGTH];
int a_ack_received,a_seq_recevied,last_seq_sent;




//adds a message to the A_queue
//returns nothign
void a_push_message(struct msg *message)
{		
	sprintf(debugmsg,"A push message, queue size: %d\n",a_buffer_length);
	debug(debugmsg,5);
	memcpy(&a_buffer[a_buffer_length],message,sizeof(struct msg));
	if(a_buffer_length < MAX_QUEUED)
		a_buffer_length = a_buffer_length + 1;


}

//pops a message from the a queue
//returns that message (with data)
struct msg a_pop_message()
{
	struct msg pop_msg;
	memcpy(&pop_msg,&a_buffer[0],sizeof(struct msg));
	return pop_msg;
}

int a_send_pkt(int seqnum, int acknum, int checksum, char* data)
{
	struct pkt packet;
	printf("Amessagedata: %s\n",data);
	packet.seqnum 		= seqnum;
	packet.acknum 		= acknum;
	packet.checksum 	= checksum;
	memcpy(&packet.payload, data,MESSAGE_LENGTH);
	tolayer3(AEntity,packet);
}

//FSM handler for a_receive
void a_receive_pkt(struct pkt packet)
{
	if (strstr(packet.payload, ack_str)||strstr(packet.payload, nack_str))
	{
		a_seq_recevied = packet.seqnum;
		a_ack_received = packet.acknum;
		sprintf(debugmsg,"A Received seqnum: %d\n",a_seq_recevied);
		debug(debugmsg,5);
	}
}

void a_fsm()
{
	switch(a_state)
	{
		case wait_for_call_0:
			if (a_buffer_length >= 1)
			{
				struct msg message;
				message = a_pop_message();
				a_send_pkt(0,0,calc_checksum(message.data),message.data);			//send a message
				last_seq_sent = 0;	
				sprintf(debugmsg,"A SENT SEQNUM: %d\n",0);
				debug(debugmsg,5);
				a_state = wait_for_ack_0;
			}
		break;
		case wait_for_ack_0:	
			sprintf(debugmsg,"A seq Expected: %d | seq Received: %d | Ack Received: %d\n", last_seq_sent, a_seq_recevied, a_ack_received);
			debug(debugmsg,3);
			if(a_seq_recevied == last_seq_sent && a_ack_received == 1)
			{
				a_buffer_length = a_buffer_length - 1;
				a_state = wait_for_call_1;
			}else{
				a_state = wait_for_call_0;
			}
		break;
		case wait_for_call_1:
			if (a_buffer_length >= 1)
			{
				struct msg message;
				message = a_pop_message();
				a_send_pkt(1,0,calc_checksum(message.data),message.data);			//send a message
				last_seq_sent = 1;
				sprintf(debugmsg,"A SENT SEQNUM: %d\n",1);
				debug(debugmsg,5);
				a_state = wait_for_ack_1;
			}
		break;
		case wait_for_ack_1:
			sprintf(debugmsg,"A seq Expected: %d | seq Received: %d | Ack Received: %d\n", last_seq_sent, a_seq_recevied, a_ack_received);
			debug(debugmsg,3);
			if(a_seq_recevied == last_seq_sent)
			{
				a_buffer_length = a_buffer_length - 1;
				a_state = wait_for_call_0;
			}else{
				a_state = wait_for_call_0;
			}

		break;
		default:
		break;
	}
}

//initializes values for a.
void init_a()
{
	a_buffer = realloc(a_buffer, sizeof(struct msg)*MAX_QUEUED);
}