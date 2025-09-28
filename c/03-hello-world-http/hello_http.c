/* hello_http */
/* Based off minIP - https://github.com/IanSeyler/minIP */

/* Global Includes */
#include "libBareMetal.h"

/* Global functions */
u16 checksum(u8* data, u16 bytes);
u16 checksum_tcp(u8* data, u16 bytes, u16 protocol, u16 length);
int net_init();
void* memset(void* s, int c, int n);
void* memcpy(void* d, const void* s, int n);
int strlen(const char* s);
char* b_to_s(char* buffer, unsigned char byte);
void display_ip(u8* ip);

/* Global defines */
#define swap16(x) __builtin_bswap16(x)
#define swap32(x) __builtin_bswap32(x)
#undef ETH_FRAME_LEN
#define ETH_FRAME_LEN 1518
#define ETHERTYPE_ARP 0x0806
#define ETHERTYPE_IPv4 0x0800
#define ETHERTYPE_IPv6 0x86DD
#define ARP_REQUEST 1
#define ARP_REPLY 2
#define PROTOCOL_IP_ICMP 1
#define PROTOCOL_IP_TCP 6
#define PROTOCOL_IP_UDP 11
#define ICMP_ECHO_REPLY 0
#define ICMP_ECHO_REQUEST 8
#define TCP_ACK 16
#define TCP_PSH 8
#define TCP_RST 4
#define TCP_SYN 2
#define TCP_FIN 1
#define INTERFACE 0

/* Global variables */
u8 src_MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
u8 dst_broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
u8 src_IP[4] = {192, 168, 4, 250};
u8 src_SN[4] = {255, 255, 255, 0};
u8 src_GW[4] = {192, 168, 4, 1};
#ifndef NO_DHCP
u8 dhcpdst[4] = {255, 255, 255, 255};
u8 dhcpsrc[4] = {0, 0, 0, 0};
#endif
unsigned char *buffer = (unsigned char *)0x11C000;
unsigned char tosend[ETH_FRAME_LEN];
int running = 1, recv_packet_len;

/* Global structs */
#pragma pack(1)
typedef struct eth_header {
	u8 dest_mac[6];
	u8 src_mac[6];
	u16 type;
} eth_header; // 14 bytes
typedef struct arp_packet {
	eth_header ethernet;
	u16 hardware_type;
	u16 protocol;
	u8 hardware_size;
	u8 protocol_size;
	u16 opcode;
	u8 sender_mac[6];
	u8 sender_ip[4];
	u8 target_mac[6];
	u8 target_ip[4];
} arp_packet; // 28 bytes
typedef struct ipv4_packet {
	eth_header ethernet;
	u8 version;
	u8 dsf;
	u16 total_length;
	u16 id;
	u16 flags;
	u8 ttl;
	u8 protocol;
	u16 checksum;
	u8 src_ip[4];
	u8 dest_ip[4];
} ipv4_packet; // 20 bytes since we don't support options
typedef struct icmp_packet {
	ipv4_packet ipv4;
	u8 type;
	u8 code;
	u16 checksum;
	u16 id;
	u16 sequence;
	u64 timestamp;
	u8 data[2]; // Set to 2 so can be used as pointer
} icmp_packet;
typedef struct udp_packet {
	ipv4_packet ipv4;
	u16 src_port;
	u16 dest_port;
	u16 length;
	u16 checksum;
	u8 data[2]; // Set to 2 so can be used as pointer
} udp_packet;
typedef struct tcp_packet {
	ipv4_packet ipv4;
	u16 src_port;
	u16 dest_port;
	u32 seqnum;
	u32 acknum;
	u8 data_offset;
	u8 flags;
	u16 window;
	u16 checksum;
	u16 urg_pointer;
	// Options and data
//	u8 data[2]; // Set to 2 so can be used as pointer
} tcp_packet;

/* Default HTTP page with HTTP headers */
const char webpage[] =
"HTTP/1.0 200 OK\n"
"Server: BareMetal\n"
"Content-type: text/html\n"
"\n"
"<!DOCTYPE html>\n"
"<html>\n"
"\t<head>\n"
"\t\t<title>Hello</title>\n"
"\t</head>\n"
"\t<body>\n"
"\t\t<h1>Hello world, from BareMetal!</h1>\n"
"\t</body>\n"
"</html>\n";
const char version_string[] = "minIP v0.9.0 (2025 08 31)\n";

/* Main code */
int main()
{
	b_output(version_string, (unsigned long)strlen(version_string));
	net_init();

	while(running == 1)
	{
		recv_packet_len = b_net_rx(buffer, INTERFACE);
		eth_header* rx = (eth_header*)buffer;

		if (recv_packet_len > 0) // Make sure we received a packet
		{
			b_output(".", 1); // display a dot when there is a packet
			memset(tosend, 0, ETH_FRAME_LEN); // clear the send buffer
			if (swap16(rx->type) == ETHERTYPE_ARP)
			{
				//b_output(arp, (unsigned long)strlen(arp));
				arp_packet* rx_arp = (arp_packet*)buffer;
				if (swap16(rx_arp->opcode) == ARP_REQUEST)
				{
					if (*(u32*)rx_arp->target_ip == *(u32*)src_IP)
					{
						arp_packet* tx_arp = (arp_packet*)tosend;
						// Ethernet
						memcpy(tx_arp->ethernet.dest_mac, rx_arp->sender_mac, 6);
						memcpy(tx_arp->ethernet.src_mac, src_MAC, 6);
						tx_arp->ethernet.type = swap16(ETHERTYPE_ARP);
						// ARP
						tx_arp->hardware_type = swap16(1); // Ethernet
						tx_arp->protocol = swap16(ETHERTYPE_IPv4);
						tx_arp->hardware_size = 6;
						tx_arp->protocol_size = 4;
						tx_arp->opcode = swap16(ARP_REPLY);
						memcpy(tx_arp->sender_mac, src_MAC, 6);
						memcpy(tx_arp->sender_ip, rx_arp->target_ip, 4);
						memcpy(tx_arp->target_mac, rx_arp->sender_mac, 6);
						memcpy(tx_arp->target_ip, rx_arp->sender_ip, 4);
						// Send the reply
						b_net_tx(tosend, 42, INTERFACE);
					}
				}
				else if (buffer[21] == ARP_REPLY)
				{
					// TODO - Responses to our requests
				}
			}
			else if (swap16(rx->type) == ETHERTYPE_IPv4)
			{
				//b_output(ipv4, (unsigned long)strlen(ipv4));
				ipv4_packet* rx_ipv4 = (ipv4_packet*)buffer;
				if(rx_ipv4->protocol == PROTOCOL_IP_ICMP)
				{
					icmp_packet* rx_icmp = (icmp_packet*)buffer;
					if(rx_icmp->type == ICMP_ECHO_REQUEST)
					{
						if (*(u32*)rx_icmp->ipv4.dest_ip == *(u32*)src_IP)
						{
							//b_output(ping, (unsigned long)strlen(ping));
							// Reply to the ping request
							icmp_packet* tx_icmp = (icmp_packet*)tosend;
							// Ethernet
							memcpy(tx_icmp->ipv4.ethernet.dest_mac, rx_icmp->ipv4.ethernet.src_mac, 6);
							memcpy(tx_icmp->ipv4.ethernet.src_mac, src_MAC, 6);
							tx_icmp->ipv4.ethernet.type = swap16(ETHERTYPE_IPv4);
							// IPv4
							tx_icmp->ipv4.version = rx_icmp->ipv4.version;
							tx_icmp->ipv4.dsf = rx_icmp->ipv4.dsf;
							tx_icmp->ipv4.total_length = rx_icmp->ipv4.total_length;
							tx_icmp->ipv4.id = rx_icmp->ipv4.id;
							tx_icmp->ipv4.flags = rx_icmp->ipv4.flags;
							tx_icmp->ipv4.ttl = rx_icmp->ipv4.ttl;
							tx_icmp->ipv4.protocol = rx_icmp->ipv4.protocol;
							tx_icmp->ipv4.checksum = rx_icmp->ipv4.checksum; // No need to recalculate checksum
							memcpy(tx_icmp->ipv4.src_ip, rx_icmp->ipv4.dest_ip, 4);
							memcpy(tx_icmp->ipv4.dest_ip, rx_icmp->ipv4.src_ip, 4);
							// ICMP
							tx_icmp->type = ICMP_ECHO_REPLY;
							tx_icmp->code = rx_icmp->code;
							tx_icmp->checksum = 0;
							tx_icmp->id = rx_icmp->id;
							tx_icmp->sequence = rx_icmp->sequence;
							tx_icmp->timestamp = rx_icmp->timestamp;
							memcpy (tx_icmp->data, rx_icmp->data, (swap16(rx_icmp->ipv4.total_length)-20-16)); // IP length - IPv4 header - ICMP header
							tx_icmp->checksum = checksum(&tosend[34], recv_packet_len-14-20); // Frame length - MAC header - IPv4 header
							// Send the reply
							b_net_tx(tosend, recv_packet_len, INTERFACE);
						}
					}
					else if (rx_icmp->type == ICMP_ECHO_REPLY)
					{
						// Ignore these for now.
					}
					else
					{
						// Do nothing
					}
				}
				else if(rx_ipv4->protocol == PROTOCOL_IP_TCP)
				{
					tcp_packet* rx_tcp = (tcp_packet*)buffer;
					if (rx_tcp->flags == TCP_SYN && *(u32*)rx_tcp->ipv4.dest_ip == *(u32*)src_IP && rx_tcp->dest_port == swap16(80))
					{
						tcp_packet* tx_tcp = (tcp_packet*)tosend;
						memcpy((void*)tosend, (void*)buffer, ETH_FRAME_LEN); // make a copy of the original frame
						// Ethernet
						memcpy(tx_tcp->ipv4.ethernet.dest_mac, rx_tcp->ipv4.ethernet.src_mac, 6);
						memcpy(tx_tcp->ipv4.ethernet.src_mac, src_MAC, 6);
						tx_tcp->ipv4.ethernet.type = swap16(ETHERTYPE_IPv4);
						// IPv4
						tx_tcp->ipv4.version = rx_tcp->ipv4.version;
						tx_tcp->ipv4.dsf = rx_tcp->ipv4.dsf;
						tx_tcp->ipv4.total_length = rx_tcp->ipv4.total_length;
						tx_tcp->ipv4.id = rx_tcp->ipv4.id;
						tx_tcp->ipv4.flags = rx_tcp->ipv4.flags;
						tx_tcp->ipv4.ttl = rx_tcp->ipv4.ttl;
						tx_tcp->ipv4.protocol = rx_tcp->ipv4.protocol;
						tx_tcp->ipv4.checksum = 0;
						memcpy(tx_tcp->ipv4.src_ip, rx_tcp->ipv4.dest_ip, 4);
						memcpy(tx_tcp->ipv4.dest_ip, rx_tcp->ipv4.src_ip, 4);
						tx_tcp->ipv4.checksum = checksum(&tosend[14], 20);
						// TCP
						tx_tcp->src_port = rx_tcp->dest_port;
						tx_tcp->dest_port = rx_tcp->src_port;
						tx_tcp->seqnum = rx_tcp->seqnum;
						tx_tcp->acknum = swap32(swap32(rx_tcp->seqnum)+1);
						tx_tcp->data_offset = rx_tcp->data_offset;
						tx_tcp->flags = TCP_SYN|TCP_ACK;
						tx_tcp->window = rx_tcp->window;
						tx_tcp->checksum = 0;
						tx_tcp->urg_pointer = rx_tcp->urg_pointer;
						tx_tcp->checksum = checksum_tcp(&tosend[34], recv_packet_len-34, PROTOCOL_IP_TCP, recv_packet_len-34);
						// Send the reply
						b_net_tx(tosend, recv_packet_len, INTERFACE);
					}
					else if (rx_tcp->flags == TCP_ACK)
					{
						// Ignore these for now.
					}
					else if (rx_tcp->flags == (TCP_PSH|TCP_ACK) && *(u32*)rx_tcp->ipv4.dest_ip == *(u32*)src_IP && rx_tcp->dest_port == swap16(80))
					{
						tcp_packet* tx_tcp = (tcp_packet*)tosend;
						memcpy((void*)tosend, (void*)buffer, ETH_FRAME_LEN); // make a copy of the original frame
						// Ethernet
						memcpy(tx_tcp->ipv4.ethernet.dest_mac, rx_tcp->ipv4.ethernet.src_mac, 6);
						memcpy(tx_tcp->ipv4.ethernet.src_mac, src_MAC, 6);
						tx_tcp->ipv4.ethernet.type = swap16(ETHERTYPE_IPv4);
						// IPv4
						tx_tcp->ipv4.version = rx_tcp->ipv4.version;
						tx_tcp->ipv4.dsf = rx_tcp->ipv4.dsf;
						tx_tcp->ipv4.total_length = swap16(52);
						tx_tcp->ipv4.id = rx_tcp->ipv4.id;
						tx_tcp->ipv4.flags = rx_tcp->ipv4.flags;
						tx_tcp->ipv4.ttl = rx_tcp->ipv4.ttl;
						tx_tcp->ipv4.protocol = rx_tcp->ipv4.protocol;
						tx_tcp->ipv4.checksum = 0;
						memcpy(tx_tcp->ipv4.src_ip, rx_tcp->ipv4.dest_ip, 4);
						memcpy(tx_tcp->ipv4.dest_ip, rx_tcp->ipv4.src_ip, 4);
						tx_tcp->ipv4.checksum = checksum(&tosend[14], 20);
						// TCP
						tx_tcp->src_port = rx_tcp->dest_port;
						tx_tcp->dest_port = rx_tcp->src_port;
						tx_tcp->seqnum = rx_tcp->seqnum;
						tx_tcp->acknum = swap32(swap32(rx_tcp->seqnum)+(recv_packet_len-14-20-32)); // Add the bytes received
						tx_tcp->data_offset = rx_tcp->data_offset;
						tx_tcp->flags = TCP_ACK;
						tx_tcp->window = rx_tcp->window;
						tx_tcp->checksum = 0;
						tx_tcp->urg_pointer = rx_tcp->urg_pointer;
						tx_tcp->checksum = checksum_tcp(&tosend[34], 32, PROTOCOL_IP_TCP, 32);
						// Send the reply
						b_net_tx(tosend, 66, INTERFACE);
						// Send the webpage
						tx_tcp->ipv4.total_length = swap16(52+strlen(webpage));
						tx_tcp->ipv4.checksum = 0;
						tx_tcp->ipv4.checksum = checksum(&tosend[14], 20);
						tx_tcp->flags = TCP_PSH|TCP_ACK;
						tx_tcp->checksum = 0;
						memcpy((char*)tosend+66, (char*)webpage, strlen(webpage));
						tx_tcp->checksum = checksum_tcp(&tosend[34], 32+strlen(webpage), PROTOCOL_IP_TCP, 32+strlen(webpage));
						b_net_tx(tosend, 66+strlen(webpage), INTERFACE);
						// Disconnect the client
						tx_tcp->ipv4.total_length = swap16(52);
						tx_tcp->ipv4.checksum = 0;
						tx_tcp->ipv4.checksum = checksum(&tosend[14], 20);
						tx_tcp->seqnum = swap32(swap32(tx_tcp->seqnum)+strlen(webpage));
						tx_tcp->flags = TCP_FIN|TCP_ACK;
						tx_tcp->checksum = 0;
						tx_tcp->checksum = checksum_tcp(&tosend[34], 32, PROTOCOL_IP_TCP, 32);
						b_net_tx(tosend, 66, INTERFACE);
					}
					else if (rx_tcp->flags == (TCP_FIN|TCP_ACK))
					{
						tcp_packet* tx_tcp = (tcp_packet*)tosend;
						memcpy((void*)tosend, (void*)buffer, ETH_FRAME_LEN); // make a copy of the original frame
						// Ethernet
						memcpy(tx_tcp->ipv4.ethernet.dest_mac, rx_tcp->ipv4.ethernet.src_mac, 6);
						memcpy(tx_tcp->ipv4.ethernet.src_mac, src_MAC, 6);
						tx_tcp->ipv4.ethernet.type = swap16(ETHERTYPE_IPv4);
						// IPv4
						tx_tcp->ipv4.version = rx_tcp->ipv4.version;
						tx_tcp->ipv4.dsf = rx_tcp->ipv4.dsf;
						tx_tcp->ipv4.total_length = swap16(52);
						tx_tcp->ipv4.id = rx_tcp->ipv4.id;
						tx_tcp->ipv4.flags = rx_tcp->ipv4.flags;
						tx_tcp->ipv4.ttl = rx_tcp->ipv4.ttl;
						tx_tcp->ipv4.protocol = rx_tcp->ipv4.protocol;
						tx_tcp->ipv4.checksum = 0;
						memcpy(tx_tcp->ipv4.src_ip, rx_tcp->ipv4.dest_ip, 4);
						memcpy(tx_tcp->ipv4.dest_ip, rx_tcp->ipv4.src_ip, 4);
						tx_tcp->ipv4.checksum = checksum(&tosend[14], 20);
						// TCP
						tx_tcp->src_port = rx_tcp->dest_port;
						tx_tcp->dest_port = rx_tcp->src_port;
						tx_tcp->seqnum = rx_tcp->acknum;
						tx_tcp->acknum = swap32(swap32(rx_tcp->seqnum)+1);
						tx_tcp->data_offset = rx_tcp->data_offset;
						tx_tcp->flags = TCP_ACK;
						tx_tcp->window = rx_tcp->window;
						tx_tcp->checksum = 0;
						tx_tcp->urg_pointer = rx_tcp->urg_pointer;
						tx_tcp->checksum = checksum_tcp(&tosend[34], 32, PROTOCOL_IP_TCP, 32);
						// Send the reply
						b_net_tx(tosend, 66, INTERFACE);
					}
				}
				else if (rx_ipv4->protocol == PROTOCOL_IP_UDP)
				{
					// TODO - UDP
				}
				else
				{
					// Do nothing
				}
			}
			else if (swap16(rx->type) == ETHERTYPE_IPv6)
			{
				// TODO - IPv6
			}
		}
	}

	b_output("\n", 1);
	return 0;
}


/* checksum - Calculate a checksum value */
// Returns 16-bit checksum
u16 checksum(u8* data, u16 bytes)
{
	u32 sum = 0;
	u16 i;

	for (i=0; i<bytes-1; i+=2) // Add up the words
		sum += *(u16 *) &data[i];

	if (bytes & 1) // Add the left-over byte if there is one
		sum += (u8) data[i];

	while (sum >> 16) // Fold total to 16-bits
		sum = (sum & 0xFFFF) + (sum >> 16);

	return ~sum; // Return 1's complement
}


/* checksum_tcp - Calculate a TCP checksum value */
// Returns 16-bit checksum
u16 checksum_tcp(u8* data, u16 bytes, u16 protocol, u16 length)
{
	u32 sum = 0;
	u16 i;
	data -= 8; // Start at the source and dest IPs
	bytes += 8;

	for (i=0; i<bytes-1; i+=2) // Add up the words
		sum += *(u16 *) &data[i];

	if (bytes & 1) // Add the left-over byte if there is one
		sum += (u8) data[i];

	sum += swap16(protocol);
	sum += swap16(length);

	while (sum >> 16) // Fold total to 16-bits
		sum = (sum & 0xFFFF) + (sum >> 16);

	return ~sum; // Return 1's complement
}


/* net_init */
int net_init()
{
	/* Populate the MAC Address */
	/* Pulls the MAC from the OS sys var table... so gross */
	char * os_MAC = (void*)0x11A008;
	memcpy(src_MAC, os_MAC, 6); // Copy MAC address

	#ifndef NO_DHCP
	// Send a DHCP Discover packet
	udp_packet* tx_udp = (udp_packet*)tosend;
	memset(tosend, 0, 1500);
	// Ethernet
	memcpy(tx_udp->ipv4.ethernet.dest_mac, dst_broadcast, 6);
	memcpy(tx_udp->ipv4.ethernet.src_mac, src_MAC, 6);
	tx_udp->ipv4.ethernet.type = swap16(ETHERTYPE_IPv4);
	// IPv4
	tx_udp->ipv4.version = 0x45;
	tx_udp->ipv4.dsf = 0;
	tx_udp->ipv4.total_length = swap16(312);
	tx_udp->ipv4.id = 0;
	tx_udp->ipv4.flags = swap16(0x4000);
	tx_udp->ipv4.ttl = 0x40;
	tx_udp->ipv4.protocol = 0x11;
	tx_udp->ipv4.checksum = 0;
	memcpy(tx_udp->ipv4.src_ip, dhcpsrc, 4);
	memcpy(tx_udp->ipv4.dest_ip, dhcpdst, 4);
	tx_udp->ipv4.checksum = checksum(&tosend[14], 20);
	// UDP
	tx_udp->src_port = swap16(68);
	tx_udp->dest_port = swap16(67);
	tx_udp->length = swap16(292);
	tx_udp->checksum = 0;
//	tx_udp->checksum = checksum_tcp(&tosend[34], 32, PROTOCOL_IP_TCP, 32);
	// DHCP
	tosend[42] = 0x01;
	tosend[43] = 0x01;
	tosend[44] = 0x06;
	tosend[45] = 0x00;
	tosend[46] = 0x35;
	tosend[47] = 0xBA;
	tosend[48] = 0x16;
	tosend[49] = 0x81;
	memcpy(&tosend[70], src_MAC, 6);
	// DHCP magic value
	tosend[278] = 0x63;
	tosend[279] = 0x82;
	tosend[280] = 0x53;
	tosend[281] = 0x63;

	tosend[282] = 0x35; // message type
	tosend[283] = 0x01; // length
	tosend[284] = 0x01; // discover

	tosend[285] = 0x3d; // client id
	tosend[286] = 0x07; // length
	tosend[287] = 0x01;
	memcpy(&tosend[288], src_MAC, 6);
	tosend[294] = 0x37; // Parameter Request List
	tosend[295] = 0x11; // Length
	tosend[296] = 0x01; // Subnet Mask
	tosend[297] = 0x02; // Time Offset
	tosend[298] = 0x06; // Domain Name Server
	tosend[299] = 0x0c; // Host Name
	tosend[300] = 0x0f; // Domain Name
	tosend[301] = 0x1a; // Interface MTU
	tosend[302] = 0x1c; // Broadcast Address
	tosend[303] = 0x79; // Classless Static Route
	tosend[304] = 0x03; // Router
	tosend[305] = 0x21; // Static Route
	tosend[306] = 0x28; // Network Information Service Domain
	tosend[307] = 0x29; // Network Information Service Servers
	tosend[308] = 0x2a; // Network Time Protocol Servers
	tosend[309] = 0x77; // Domain Search
	tosend[310] = 0xf9; // Private/Classless Static Route
	tosend[311] = 0xfc; // Private/Proxy Autodiscovery
	tosend[312] = 0x11; // Root Path
	tosend[313] = 0x39; // Maximum DHCP Message Size
	tosend[314] = 0x02; // Length
	tosend[315] = 0x02; // Size (0x240 - 576 bytes)
	tosend[316] = 0x40;
	tosend[317] = 0x0c; // Host Name
	tosend[318] = 0x06; // Length
	tosend[319] = 'b';
	tosend[320] = 'm';
	tosend[321] = 'e';
	tosend[322] = 't';
	tosend[323] = 'a';
	tosend[324] = 'l';
	tosend[325] = 0xFF; // End

	// Send the reply
	b_net_tx(tosend, 326, INTERFACE);

	// Wait for a DHCP Offer Packet
	int dhcp = 0;
	while (dhcp == 0)
	{
		recv_packet_len = b_net_rx(buffer, INTERFACE);
		eth_header* rx = (eth_header*)buffer;
		if (swap16(rx->type) == ETHERTYPE_IPv4)
		{
			udp_packet* rx_udp = (udp_packet*)buffer;
			if (swap16(rx_udp->dest_port) == 68)
			{
				unsigned int index = 282;
				u8 tval = 0, tlen = 0;
				memcpy(src_IP, buffer + 58, 4);
				dhcp = 1;
				b_output("DHCP - IP: ", 11);
				display_ip(src_IP);

				// Parse options
				while (1)
				{
					tval = buffer[index];
					if (tval == 0xFF)
						break;
					tlen = buffer[index+1];
					if (tval == 0x01) // Subnet
					{
						memcpy(src_SN, buffer + index + 2, 4);
						b_output(", SN: ", 6);
						display_ip(src_SN);
					}
					else if (tval == 0x03) // Router
					{
						memcpy(src_GW, buffer + index + 2, 4);
						b_output(", GW: ", 6);
						display_ip(src_GW);
					}
					index = index + tlen + 2;
				}
				b_output("\n", 1);
			}
		}
	}

	if (dhcp == 1)
	{
		// Send a DHCP Request packet
		tx_udp->ipv4.total_length = swap16(324);
		tx_udp->ipv4.checksum = 0;
		tx_udp->ipv4.checksum = checksum(&tosend[14], 20);
		tx_udp->length = swap16(304);
		tx_udp->checksum = 0;
		tosend[282] = 0x35; // message type
		tosend[283] = 0x01; // length
		tosend[284] = 0x03; // request

		tosend[317] = 0x32; // requested IP Address
		tosend[318] = 0x04; // length
		memcpy(tosend + 319, buffer + 58, 4); // requested IP Address value

		tosend[323] = 0x36; // dhcp server identifier
		tosend[324] = 0x04; // length
		memcpy(tosend + 325, buffer + 26, 4); // dhcp server identifier value

		tosend[329] = 0x0c; // Host Name
		tosend[330] = 0x06; // Length
		tosend[331] = 'b';
		tosend[332] = 'm';
		tosend[333] = 'e';
		tosend[334] = 't';
		tosend[335] = 'a';
		tosend[336] = 'l';
		tosend[337] = 0xFF; // End

		// Send the reply
		b_net_tx(tosend, 338, INTERFACE);
	}

	// Ignore the DHCP ACK for now.
	#endif

	return 0;
}


void* memset(void* s, int c, int n)
{
	char* _src;

	_src = (char*)s;

	while (n--)
	{
		*_src++ = c;
	}

	return s;
}


void* memcpy(void* d, const void* s, int n)
{
	char* dest;
	char* src;

	dest = (char*)d;
	src = (char*)s;

	while (n--)
	{
		*dest++ = *src++;
	}

	return d;
}


int strlen(const char* s)
{
	int r = 0;

	for(; *s++ != 0; r++) { }

	return r;
}


char* b_to_s(char* buffer, unsigned char byte)
{
	int i = 0;
	int temp = byte;
	char digits[4];
	int digit_count = 0;

	// Check if the byte was 0 and set the string if so
	if (byte == 0)
	{
		buffer[0] = '0';
		buffer[1] = '\0';
		return buffer;
	}

	// Extract the individual digits
	while (temp > 0)
	{
		digits[digit_count] = (temp % 10) + '0';
		temp /= 10;
		digit_count++;
	}

	// Put digits in the correct order
	for (i=0; i < digit_count; i++)
	{
		buffer[i] = digits[digit_count - 1 - i];
	}

	// Null terminate the string
	buffer[digit_count] = '\0';

	return buffer;
}


void display_ip(u8* ip)
{
	char tstring[] = "xxx";
	b_to_s(tstring, ip[0]);
	b_output(tstring, (unsigned long)strlen(tstring));
	b_output(".", 1);
	b_to_s(tstring, ip[1]);
	b_output(tstring, (unsigned long)strlen(tstring));
	b_output(".", 1);
	b_to_s(tstring, ip[2]);
	b_output(tstring, (unsigned long)strlen(tstring));
	b_output(".", 1);
	b_to_s(tstring, ip[3]);
	b_output(tstring, (unsigned long)strlen(tstring));
}

/* EOF */
