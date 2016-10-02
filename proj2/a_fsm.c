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
/*	printf("newbuffer: %p newbuffer->prev: %p newbuffer->next: %p\n",newbuffer,newbuffer->prev,newbuffer->next );
	printf("a_buffer_last: %p a_buffer_last->prev: %p a_buffer_last->next: %p\n",a_buffer_last,a_buffer_last->prev,a_buffer_last->next );

*/}

//pops a message from the a queue
//returns that message (with data)
char* get_message(int pop_seq)
{

	int i;
	struct buffer *pop_msg;
	pop_msg = a_buffer;
	for(i=0;i<a_buffer_length;i++)
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
	printf("del_seq: %d\n", del_seq);
	struct buffer *delete_buffer;
	delete_buffer = a_buffer;
	while(a_buffer->seq < del_seq)
	{
		sprintf(debugmsg,"delete: %d bufferlen: %d\n",a_buffer->seq, a_buffer_length);
		debug(debugmsg,3);
		delete_buffer = a_buffer;
		a_buffer_length = a_buffer_length - 1;
		if(a_buffer_length > 1)
		{
			
			a_buffer = a_buffer->next;
			a_buffer->prev = NULL;
			//printf("a_buffer: %p delete_buffer: %p a_buffer_last: %p\n",a_buffer,delete_buffer,a_buffer_last );
			free(delete_buffer);
		}else if(a_buffer_length == 1)
		{
			
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
	debug(debugmsg,3);
	a_new_received = TRUE;

	if(!a_chk_received && a_seq_received >= base)
	{
		printf("base: %d\n", base);
		base = a_seq_received + 1;
		num_in_flight--;
		delete_message(a_seq_received);
		stopTimer(AEntity);
	}
	a_fsm();
}

void a_fsm()
{
	while(num_in_flight < max_num_in_flight && curr_seq-base < a_buffer_length)
	{
		if(a_buffer_length && curr_seq-base < max_num_in_flight)
		{
			if(get_message(curr_seq))
				a_send_pkt(curr_seq,0,get_message(curr_seq));			//send a message
				curr_seq++;
				num_in_flight++;
		}
	}
}

void a_timeout(){
	printf("timeout\n");
	num_in_flight = 0;
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
	max_num_in_flight 	= 10;
	base = 0;
	curr_seq = 0;
	push_seq = 0;
}