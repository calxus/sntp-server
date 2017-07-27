#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFLEN 48 // size of SNTP message excluding optional authenticator
#define PORT 8888
#define NTP_DELTA 2208988800 // difference between 01/01/1900 and 01/01/1970

int main(void) {

	struct sockaddr_in si_me, si_other;
	int s, i, slen = sizeof(si_other), recv_len;
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	char buf[48];
	time_t rawtime;
	uint32_t ref_time = (uint32_t)time(&rawtime) + NTP_DELTA; // dummy value to simulate time since clock was adjusted
	uint32_t recv_time;
	uint32_t trans_time;

	memset((char *) &si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(s, (struct sockaddr*)&si_me, sizeof(si_me));

	while(1) {
		printf("Waiting for request... \n");
		fflush(stdout);
		recv_len = recvfrom(s, buf, 48, 0, (struct sockaddr *) &si_other, &slen);
		recv_time = (uint32_t)time(&rawtime) + NTP_DELTA; // the time the request was received
		printf("Received request from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		if((buf[0] & 0x18) != 0x18) { // drops request if not from a client
			printf("Request dropped\n");
			continue;
		}
		// updates header to indicate message from server
		buf[0] = buf[0] ^ 0x38;

		// inserts the reference timestamp
		buf[16] = (ref_time >> 24) & 0xff;
		buf[17] = (ref_time >> 16) & 0xff;
		buf[18] = (ref_time >> 8) & 0xff;
		buf[19] = ref_time & 0xff;

		// copies the clients timestamp to the originate timestamp
		buf[24] = buf[40];
		buf[25] = buf[41];
		buf[26] = buf[42];
		buf[27] = buf[43];

		// inserts the receive timestamp
		buf[32] = (recv_time >> 24) & 0xff;
                buf[33] = (recv_time >> 16) & 0xff;
                buf[34] = (recv_time >> 8) & 0xff;
                buf[35] = recv_time & 0xff;

		// generates and inserts the transmit timestamp
		trans_time = (uint32_t)time(&rawtime) + NTP_DELTA;
		buf[40] = (trans_time >> 24) & 0xff;
                buf[41] = (trans_time >> 16) & 0xff;
                buf[42] = (trans_time >> 8) & 0xff;
                buf[43] = trans_time & 0xff;

		// sends the generated message
		sendto(s, buf, 48, 0, (struct sockaddr*) &si_other, slen);
	}
}





