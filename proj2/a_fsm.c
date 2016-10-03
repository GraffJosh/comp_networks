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
int num_in_flight, max_num_in_flight,base,curr_seq,push_seq;
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
	newbuffer->seq 	= push_seq;
	push_seq++;
	a_buffer_last = newbuffer;
	memcpy(&newbuffer->message,&message, sizeof(struct msg));
	a_buffer_length = a_buffer_length + 1;
	//a_fsm();
}

//pops a message from the a queue
//returns that message (with data)
char* get_message(int pop_seq)
{

	int i;
	struct buffer *pop_msg;
	pop_msg = a_buffer;
	while(pop_msg->seq <= pop_seq)
	{
		if(pop_msg->seq == pop_seq)
		{
			sprintf(debugmsg,"pop: %s, chk: ,\n",pop_msg->message.data);//,calc_checksum(a_buffer->message.data,MESSAGE_LENGTH));
			debug(debugmsg,5);
			return pop_msg->message.data;
		}
		pop_msg = pop_msg->next;
	}
	return NULL;
}

//deletes the top of the queue when we receive an ACK
void delete_message(int del_seq)
{
	struct buffer *delete_buffer;
	delete_buffer = a_buffer;
	while(a_buffer->seq <= del_seq)
	{
		sprintf(debugmsg,"delete: %d bufferlen: %d\n",a_buffer->seq, a_buffer_length);
		debug(debugmsg,6);
		delete_buffer = a_buffer;
		a_buffer_length = a_buffer_length - 1;
		if(a_buffer_length > 1)
		{
			
			a_buffer = a_buffer->next;
			a_buffer->prev = NULL;
			free(delete_buffer);
		}else if(a_buffer_length == 1)
		{
			a_buffer = a_buffer->next;
			free(delete_buffer);
		}else if(a_buffer_length == 0)
		{
			free(a_buffer);
			a_buffer = NULL;
			break;
		}
	}
}

int a_send_pkt(int seqnum, int acknum, char* data)
{
	stopTimer(AEntity);
	a_send_packet = realloc(a_send_packet, sizeof(struct pkt));
	memset(a_send_packet->payload, 0,MESSAGE_LENGTH);
	memcpy(a_send_packet->payload, data,MESSAGE_LENGTH);
	a_send_packet->seqnum 		= seqnum;
	a_send_packet->acknum 		= acknum;
	a_send_packet->checksum 	= calc_checksum(a_send_packet->payload,a_send_packet->seqnum,a_send_packet->acknum, MESSAGE_LENGTH);
	tolayer3(AEntity,*a_send_packet);
	sprintf(debugmsg,"A SENT %d data: %s, A_chk: %d \n",a_send_packet->seqnum,a_send_packet->payload, a_send_packet->checksum);
	debug(debugmsg,5);
	return a_send_packet->seqnum;
}

//FSM handler for a_receive
void a_receive_pkt(struct pkt packet)
{

	last_chk_received = a_chk_received;
	memcpy(a_data_received, packet.payload,MESSAGE_LENGTH);
	a_seq_received = packet.seqnum;
	a_ack_received = packet.acknum;	
	a_chk_received = packet.checksum - calc_checksum(packet.payload,packet.seqnum,packet.acknum, MESSAGE_LENGTH);
	sprintf(debugmsg,"A Received sequence: %d, chk: %d base: %d\n",a_seq_received,a_chk_received,base);
	debug(debugmsg,5);
	a_new_received = TRUE;

	if(!a_chk_received && a_seq_received >= base)
	{
		stopTimer(AEntity);
		base = a_seq_received + 1;
		delete_message(a_seq_received);
		a_timed_out = 0;
	}else{
		startTimer(AEntity,TIMEOUT_LENGTH);
	}
	//a_fsm();
}

void a_fsm()
{
	while(a_buffer_length > curr_seq-base && curr_seq-base < max_num_in_flight)
	{
		if(get_message(curr_seq))
		{
			curr_seq = a_send_pkt(curr_seq,curr_seq-base,get_message(curr_seq)) + 1;			//send a message
		}
		if(curr_seq-base == 1)
		{
			startTimer(AEntity, TIMEOUT_LENGTH);
		}
	}

}

void a_timeout(){
	a_timed_out ++;
	curr_seq = base;
	a_fsm();
}

//initializes values for a.
void init_a()
{
	a_send_packet = realloc(a_send_packet,MESSAGE_LENGTH);
	a_buffer = realloc(a_buffer, sizeof(struct msg));
	a_buffer_last = a_buffer;
	num_in_flight 		= 0;
	max_num_in_flight 	= 20;
	base = 0;
	curr_seq = 0;
	push_seq = 0;
}