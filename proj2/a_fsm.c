#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include "project2.h"
#define JPGTRACE 3

enum fsm_state a_state = wait_for_call_0;
struct msg *ack,*nack;
struct buffer *a_buffer,*a_buffer_last,*newbuffer;
int a_buffer_length=0;
char debugmsg[MESSAGE_LENGTH],a_data_received[MESSAGE_LENGTH];
int a_ack_received,a_seq_recevied,a_chk_received,last_chk_received,last_seq_sent;




//adds a message to the A_queue
//returns nothign
void a_push_message(struct msg message)
{		
	/*if(a_buffer_length < MAX_QUEUED)
	{
		sprintf(debugmsg,"A push message, queue size: %d\n",a_buffer_length);
		debug(debugmsg,5);
		memcpy(&a_buffer[a_buffer_length],&message,sizeof(struct msg));

		a_buffer_length = a_buffer_length + 1;
	}*/
	newbuffer = realloc(newbuffer,sizeof(struct buffer));
	if(a_buffer_length>0)
	{
		a_buffer_last->next = newbuffer;
	}else if(a_buffer_length == 0){
		a_buffer = newbuffer;
	}
	newbuffer->prev = a_buffer_last;
	newbuffer->next = NULL;
	a_buffer_last = newbuffer;
	memcpy(&newbuffer->message,&message, sizeof(struct msg));
	a_buffer_length = a_buffer_length + 1;
}

//pops a message from the a queue
//returns that message (with data)
struct msg get_message()
{/*
	struct msg pop_msg;
	memcpy(&pop_msg,&a_buffer[0],sizeof(struct msg));
	return pop_msg;*/
	//free(a_buffer);

	printf("get: %s, chk: %d\n",a_buffer->message.data,calc_checksum(a_buffer->message.data));
	return a_buffer->message;
}

//deletes the top of the queue when we receive an ACK
void delete_message()
{
	struct buffer *delete_buffer;
	printf("delete: %s\n",a_buffer->message.data );

	a_buffer_length = a_buffer_length - 1;
	if(a_buffer_length > 1)
	{
		delete_buffer = a_buffer;

		a_buffer = a_buffer->next;
		a_buffer->prev = NULL;

		free(delete_buffer);
	}else if(a_buffer_length == 1)
	{
		delete_buffer = a_buffer;
		a_buffer = a_buffer->next;
		a_buffer->prev = NULL;
		free(delete_buffer);
	}else if(a_buffer_length == 0)
	{
		free(a_buffer);
		a_buffer = NULL;
	}

}

int a_send_pkt(int seqnum, int acknum, struct msg data)
{
	struct pkt packet;
	memset(packet.payload, 0,MESSAGE_LENGTH-1);
	packet.seqnum 		= seqnum;
	packet.acknum 		= acknum;
	packet.checksum 	= calc_checksum(data.data);
	memcpy(&packet.payload, &data.data,MESSAGE_LENGTH);
	tolayer3(AEntity,packet);
	sprintf(debugmsg,"A SENT: %s\n",data.data);
	debug(debugmsg,5);
}

//FSM handler for a_receive
void a_receive_pkt(struct pkt packet)
{

	last_chk_received = a_chk_received;
	a_seq_recevied = packet.seqnum;
	a_ack_received = packet.acknum;
	a_chk_received = packet.checksum;
	memcpy(&a_data_received, packet.payload,MESSAGE_LENGTH);
	sprintf(debugmsg,"A Received checksum: %d\n",a_chk_received);
	debug(debugmsg,3);
	printf("data Areceved: %s\n",a_data_received);
}

void a_fsm()
{
	switch(a_state)
	{
		case wait_for_call_0:
			if (a_buffer_length >= 1)
			{
				struct msg message;
				memset(message.data, 0,MESSAGE_LENGTH-1);
				message = get_message();
				a_send_pkt(0,0,message);			//send a message
				last_seq_sent = 0;	
				sprintf(debugmsg,"A SENT SEQNUM: %d\n",0);
				debug(debugmsg,5);
				a_state = wait_for_ack_0;
			}
		break;
		case wait_for_ack_0:	
			sprintf(debugmsg,"A0 ack seq Expected: %d | seq Received: %d | Ack Received: %d\n", last_seq_sent, a_seq_recevied, a_ack_received);
			debug(debugmsg,3);
			sprintf(debugmsg,"A0 checksum Expected: %d | checksum Received: %d \n", a_chk_received, calc_checksum(a_data_received));
			debug(debugmsg,3);	
			if(a_seq_recevied == 0 && a_ack_received == 1 && a_chk_received == calc_checksum(a_data_received))
			{
				if(a_buffer_length)
					delete_message();
				a_state = wait_for_call_1;
			}else{
				sprintf(debugmsg,"resending 0!");
				debug(debugmsg,3);
				a_state = wait_for_call_0;
				a_fsm();
			}
		break;
		case wait_for_call_1:
			if (a_buffer_length >= 1)
			{
				struct msg message;
				memset(message.data, 0,MESSAGE_LENGTH-1);
				message = get_message();
				a_send_pkt(1,0,message);			//send a message
				last_seq_sent = 1;
				sprintf(debugmsg,"A SENT SEQNUM: %d\n",1);
				debug(debugmsg,5);
				a_state = wait_for_ack_1;
			}
		break;
		case wait_for_ack_1:
			sprintf(debugmsg,"A1 ack seq Expected: %d | seq Received: %d | Ack Received: %d\n", last_seq_sent, a_seq_recevied, a_ack_received);
			debug(debugmsg,3);
			sprintf(debugmsg,"A1 checksum Expected: %d | checksum Received: %d \n", a_chk_received, calc_checksum(a_data_received));
			debug(debugmsg,3);	
			if(a_seq_recevied == 1 && a_ack_received == 1 && a_chk_received == calc_checksum(a_data_received))
			{
				if(a_buffer_length)
					delete_message();
				a_state = wait_for_call_0;
			}else{
				sprintf(debugmsg,"resending 1!");
				debug(debugmsg,3);
				a_state = wait_for_call_1;
				a_fsm();
			}

		break;
		default:
		break;
	}
}

//initializes values for a.
void init_a()
{
	a_buffer = realloc(a_buffer, sizeof(struct msg));
	a_buffer_last = a_buffer;
	ack = (struct msg*)malloc(sizeof(struct msg));
	nack = (struct msg*)malloc(sizeof(struct msg));
	memcpy(ack->data, ack_str,MESSAGE_LENGTH);
	memcpy(nack->data, nack_str,MESSAGE_LENGTH);
}