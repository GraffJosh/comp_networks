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
char debugmsg[MESSAGE_LENGTH];
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

	a_push_message(message);

	a_fsm();
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

	sprintf(debugmsg,"RECEIVED A\n");
	debug(debugmsg,5);
	a_receive_pkt(packet);
	a_fsm();
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
	ack_str = "THIS IS AN ACK01";
 	nack_str= "THIS IS A NACK01";
 	init_a();
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
	sprintf(debugmsg,"RECEIVED B\n");
	debug(debugmsg,5);
	b_receive_pkt(packet);
	b_fsm();
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
	ack_str = "THIS IS AN ACK01";
 	nack_str= "THIS IS A NACK01";
 	init_b();  
}

//calculates the checksum for a given set of data
int calc_checksum(char* message)
{	
	int checksum,i;
	checksum = 0;
	unsigned char tempmsg[(sizeof(char)*MESSAGE_LENGTH)];
	memcpy(tempmsg,message,(sizeof(char)*MESSAGE_LENGTH));
	unsigned char *messageptr = &tempmsg;
	for(i = 0;i<(sizeof(char)*MESSAGE_LENGTH);i++)
	{

		checksum += messageptr[i];
		checksum += messageptr[(sizeof(char)*MESSAGE_LENGTH)-1];
	}
	sprintf(debugmsg,"message: %s, checksum: %d\n",messageptr, checksum );
	debug(debugmsg,5);
	return checksum;
}

//prints debug messages if the trace level is high enough
void debug(char* debug_message, int trace_level)
{
	if (JPGTRACE >= trace_level)
	{
		printf("debug: %s\n", debug_message);
		/* code */
	}
	
}