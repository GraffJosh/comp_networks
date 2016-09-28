#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include "project2.h"

enum fsm_state a_state = wait_for_call_0;
struct msg *ack,*nack;
struct buffer *a_buffer,*a_buffer_last,*newbuffer;
int a_buffer_length=0,a_new_received = FALSE,a_timed_out = FALSE;
char debugmsg[MESSAGE_LENGTH],a_data_received[MESSAGE_LENGTH];
int a_ack_received,a_seq_received,a_chk_received,last_chk_received,last_seq_sent;
struct pkt *a_send_packet;



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

	newbuffer = malloc(sizeof(struct buffer));
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
/*	printf("newbuffer: %p newbuffer->prev: %p newbuffer->next: %p\n",newbuffer,newbuffer->prev,newbuffer->next );
	printf("a_buffer_last: %p a_buffer_last->prev: %p a_buffer_last->next: %p\n",a_buffer_last,a_buffer_last->prev,a_buffer_last->next );

*/}

//pops a message from the a queue
//returns that message (with data)
char* get_message()
{/*
	struct msg pop_msg;
	memcpy(&pop_msg,&a_buffer[0],sizeof(struct msg));
	return pop_msg;*/
	//free(a_buffer);

	sprintf(debugmsg,"get: %s, chk: ,\n",a_buffer->message.data);//,calc_checksum(a_buffer->message.data,MESSAGE_LENGTH));
	debug(debugmsg,3);
	return a_buffer->message.data;
}

//deletes the top of the queue when we receive an ACK
void delete_message()
{
	struct buffer *delete_buffer;
	sprintf(debugmsg,"delete: %s bufferlen: %d\n",a_buffer->message.data, a_buffer_length);
	debug(debugmsg,3);
	a_buffer_length = a_buffer_length - 1;
	if(a_buffer_length > 1)
	{
		delete_buffer = a_buffer;
		a_buffer = a_buffer->next;
		a_buffer->prev = NULL;
		//printf("a_buffer: %p delete_buffer: %p a_buffer_last: %p\n",a_buffer,delete_buffer,a_buffer_last );
		free(delete_buffer);
	}else if(a_buffer_length == 1)
	{
		delete_buffer = a_buffer;
		a_buffer = a_buffer->next;
		//printf("a_buffer: %p delete_buffer: %p a_buffer_last: %p\n",a_buffer,delete_buffer,a_buffer_last );
		free(delete_buffer);
	}else if(a_buffer_length == 0)
	{
		//printf("a_buffer: %p delete_buffer: %p a_buffer_last: %p\n",a_buffer,delete_buffer,a_buffer_last );
		free(a_buffer);
		a_buffer = NULL;
	}

}

int a_send_pkt(int seqnum, int acknum, char* data)
{
	a_send_packet = realloc(a_send_packet, sizeof(struct pkt));
	memset(a_send_packet->payload, 0,MESSAGE_LENGTH);
	memcpy(a_send_packet->payload, data,MESSAGE_LENGTH);
	a_send_packet->seqnum 		= seqnum;
	a_send_packet->acknum 		= acknum;
	a_send_packet->checksum 	= calc_checksum(a_send_packet->payload,a_send_packet->seqnum,a_send_packet->acknum, MESSAGE_LENGTH);
	tolayer3(AEntity,*a_send_packet);
	sprintf(debugmsg,"A SENT %d data: %s, A_chk: %d \n",a_send_packet->seqnum,a_send_packet->payload, a_send_packet->checksum);
	debug(debugmsg,3);
	startTimer(AEntity,TIMEOUT_LENGTH);
}

//FSM handler for a_receive
void a_receive_pkt(struct pkt packet)
{

	stopTimer(AEntity);
	last_chk_received = a_chk_received;
	memcpy(a_data_received, packet.payload,MESSAGE_LENGTH);
	a_seq_received = packet.seqnum;
	a_ack_received = packet.acknum;	
	a_chk_received = packet.checksum - calc_checksum(packet.payload,packet.seqnum,packet.acknum, MESSAGE_LENGTH);
	sprintf(debugmsg,"A Received checksum: %d\n",a_chk_received);
	debug(debugmsg,5);
	sprintf(debugmsg,"data Areceved: %s seq: %d chk: %d vs calc: %d\n",a_data_received, a_seq_received, packet.checksum, calc_checksum(packet.payload,packet.seqnum,packet.acknum, MESSAGE_LENGTH));
	debug(debugmsg,5);
	a_new_received = TRUE;
}

void a_fsm()
{
	switch(a_state)
	{
		case wait_for_call_0:
			if (a_buffer_length >= 1)
			{
				a_send_pkt(0,0,get_message());			//send a message
				last_seq_sent = 0;	
				sprintf(debugmsg,"A SENT SEQNUM: %d\n",0);
				debug(debugmsg,5);
				a_state = wait_for_ack_0;
			}
		break;
		case wait_for_ack_0:	
			if(a_new_received == TRUE)
			{
				sprintf(debugmsg,"A0 ack seq Expected: %d | seq Received: %d | Ack Received: %d\n", last_seq_sent, a_seq_received, a_ack_received);
				debug(debugmsg,5);
				sprintf(debugmsg,"A0 checksum Expected: %d | checksum Received: %d \n", a_chk_received, calc_checksum(a_data_received,a_seq_received,a_ack_received,MESSAGE_LENGTH));
				debug(debugmsg,5);	
				if(a_seq_received == 0 && a_ack_received == 1 && a_chk_received == 0)
				{
					if(a_buffer_length)
						delete_message();
					a_state = wait_for_call_1;
					if(a_buffer_length)
					{
						sprintf(debugmsg,"CONFIRMED RECEIVED SEND ANOTHER: %d\n",1);
						debug(debugmsg,3);
						a_fsm();
					}
				}else{

					sprintf(debugmsg,"Resend0 because: Seq: %d, Ack: %d, Chk: %d\n", a_seq_received,a_ack_received,a_chk_received );
					debug(debugmsg,3);
					a_state = wait_for_call_0;
					a_fsm();
				}
				a_new_received = FALSE;
			}
			if(a_timed_out)
			{
				a_timed_out = FALSE;
				sprintf(debugmsg,"Resend0 because: timeout\n");
				debug(debugmsg,3);
				a_state = wait_for_call_0;
				a_fsm();
			}
		break;
		case wait_for_call_1:
			if (a_buffer_length >= 1)
			{
				a_send_pkt(1,0,get_message());			//send a message
				last_seq_sent = 1;
				sprintf(debugmsg,"A SENT SEQNUM: %d\n",1);
				debug(debugmsg,5);
				a_state = wait_for_ack_1;
			}
		break;
		case wait_for_ack_1:
			if(a_new_received == TRUE)
			{
				sprintf(debugmsg,"A1 ack seq Expected: %d | seq Received: %d | Ack Received: %d\n", last_seq_sent, a_seq_received, a_ack_received);
				debug(debugmsg,5);
				sprintf(debugmsg,"A1 checksum Expected: %d | checksum Received: %d \n", a_chk_received, calc_checksum(a_data_received,a_seq_received,a_ack_received,MESSAGE_LENGTH));
				debug(debugmsg,5);	
				if(a_seq_received == 1 && a_ack_received == 1 && a_chk_received == 0)
				{
					if(a_buffer_length)
						delete_message();
					a_state = wait_for_call_0;
					if(a_buffer_length)
					{
						sprintf(debugmsg,"CONFIRMED RECEIVED SEND ANOTHER: %d\n",1);
						debug(debugmsg,3);
						a_fsm();
					}
				}else{
					sprintf(debugmsg,"Resend0 because: Seq: %d, Ack: %d, Chk: %d\n", a_seq_received,a_ack_received,a_chk_received );
					debug(debugmsg,3);
					a_state = wait_for_call_1;
					a_fsm();
				}
				a_new_received = FALSE;
			}
			if(a_timed_out)
			{						
				a_timed_out = FALSE;		
				sprintf(debugmsg,"Resend0 because: timeout\n");
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
	a_send_packet = realloc(a_send_packet,MESSAGE_LENGTH);
	a_buffer = realloc(a_buffer, sizeof(struct msg));
	a_buffer_last = a_buffer;
}