#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/types.h>
#include "project2.h"
#define JPGTRACE 3
 
/* ***************************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for unidirectional or bidirectional
   data transfer protocols from A to B and B to A.
   Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets may be delivered out of order.

   Compile as gcc -g project2.c student2.c -o p2
**********************************************************************/
int  num_queued;
int a_pkt_received,a_seq_recevied,last_seq_sent;
char *ack_str = "THIS IS AN ACK";
char *nack_str = "THIS IS A NACK";
char queued_message[MAX_QUEUED][sizeof(struct msg)];
int ack_len = 14;

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* 
 * The routines you will write are detailed below. As noted above, 
 * such procedures in real-life would be part of the operating system, 
 * and would be called by other procedures in the operating system.  
 * All these routines are in layer 4.
 */

/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to insure that the data in such a message is delivered 
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message) {
	int i = 0;
	struct pkt packet;
	if(JPGTRACE >=3)
		printf("seq Expected: %d | seq Received: %d\n", last_seq_sent, a_seq_recevied);
	if(a_seq_recevied == last_seq_sent)
	{
		last_seq_sent = last_seq_sent + 1;
		a_send_pkt(last_seq_sent,1,0,message.data);
		if (JPGTRACE >= 3)
		{
			printf("A SENT SEQNUM: %d\n",last_seq_sent );
		}
	}
}

/*
 * Just like A_output, but residing on the B side.  USED only when the 
 * implementation is bi-directional.
 */
void B_output(struct msg message)  {


}

/* 
 * A_input(packet), where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the B-side (i.e., as a result 
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side. 
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
void A_input(struct pkt packet) {
	if (JPGTRACE >=5)
	{
		printf("RECEIVED  A\n");
	}
	
		
	if (strstr(packet.payload, ack_str))
	{

		a_seq_recevied = packet.seqnum;
		a_pkt_received = packet.acknum;
		if(JPGTRACE >=3)
			printf("A RECEIVED SEQNUM: %d\n", packet.seqnum);
	}

}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission	 of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {

}  

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
	a_seq_recevied = 0;
	last_seq_sent = 0;
	num_queued = 0;
	
}


/* 
 * Note that with simplex transfer from A-to-B, there is no routine  B_output() 
 */

/*
 * B_input(packet),where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the A-side (i.e., as a result 
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
void B_input(struct pkt packet) {
	struct msg message;

	memcpy(&message.data,&packet.payload,sizeof(struct msg));
	tolayer5(BEntity,message);

	if (JPGTRACE >= 3)
	{
		printf("B RECEIVED SEQNUM: %d\n", packet.seqnum);
	}
	b_send_pkt(packet.seqnum,packet.acknum,0,ack_str);
}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init() {
}

