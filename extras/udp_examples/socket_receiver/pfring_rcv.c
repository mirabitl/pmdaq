//#define _GNU_SOURCE
#include <signal.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/poll.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <locale.h>
#include <getopt.h>
#include <openssl/md5.h>

#include "pfring.h"
#include "time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/udp.h>
#include <netinet/ether.h>
#include "utils.hh"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <json/json.h>

int nrec = 0, nloss = 0;
int nbytes = 0;
int n0 = 0;
double totalbytes = 0;
time_t t0;
struct timespec clock_start, clock_end;

// do stuff
const char *device;
const char *filter_ip_source;
static unsigned short filter_dest_port;
static struct
{
  std::string s_ip_address;
  std::string s_device;
  uint16_t port;
} rd_conf;
Json::Value jsonFile2Value(std::string config)
{
  std::ifstream ifs(config.c_str(), std::ifstream::in);

  // Let's parse it

  Json::Reader reader;
  Json::Value rootd = Json::Value::null;
  bool parsedSuccess = reader.parse(ifs,
                                    rootd,
                                    false);

  if (not parsedSuccess)
  {
    // Report failures and their locations
    // in the document.
    std::cerr<<"Failed to parse JSON file [" << config << "]"
                                                    << " error: " << reader.getFormattedErrorMessages()<<std::endl;
    return Json::Value::null;
  }

  return rootd;
}

/////////////////////////////////////////////////

uint8_t recb[65535];
uint8_t event_b[0x100000];
int event_len = 0;
int first_id = -1;

void processCompleteUDPPacket(uint8_t *data, int total_size)
{
	struct udphdr *udp_header = (struct udphdr *)(data);
	int udp_data_len = total_size - sizeof(struct udphdr);
	uint32_t *l = (uint32_t *)&data[sizeof(struct udphdr)];
	uint32_t id = ntohl(l[0]);
	uint32_t offset = ntohl(l[1]);
	uint32_t elen = ntohl(l[2]);
	uint32_t gtc = ntohl(l[3]);
	uint32_t tokens = ntohl(l[4]);
	// printf("%d %d %d %d %d \n",id,offset,elen,gtc,tokens);
	if (first_id < 0)
		first_id = id;

	nrec++;
	totalbytes += udp_data_len;
	if ((id - first_id + 1) != nrec)
	  {
		nloss++;
		if (nloss==1 && nrec> (id+100))
		  {
		    first_id=id;
		    nrec=1;
		    nloss=0;
		  }
		if (nloss%10000==1)
		  printf("Loss %d Id %d first_id %d nrec %d \n",nloss,id,first_id,nrec);
	  }
	if (offset == 0 && gtc > 1)
	{
		int *el = (int *)event_b;
		//int eid = ntohl(el[0]);
		//int eoffset = ntohl(el[1]);
		//int eelen = ntohl(el[2]);
		//int egtc = ntohl(el[3]);
		//int etokens = ntohl(el[4]);
		// printf("%d ---->%d %d %d %d %d \n",event_len,eid,eoffset,eelen,egtc,etokens);
		uint64_t bxid = gtc;
		utils::store(201, 10, gtc, bxid, event_b, event_len, "/dev/shm/events");
		event_len = 0;
	}
	memcpy(&event_b[offset], &data[sizeof(struct udphdr)], 1472);
	event_len += 1472;

	if (id % 50000 == 0 && id > 1)
	{
		clock_gettime(CLOCK_MONOTONIC_RAW, &clock_end);

		uint64_t delta_us = (clock_end.tv_sec - clock_start.tv_sec) * 1000000 + (clock_end.tv_nsec - clock_start.tv_nsec) / 1000;
		// clock_gettime(CLOCK_MONOTONIC_RAW, &clock_start);
		double dt = delta_us * 1E-6;
		double mbs = totalbytes / 1024. / 1024 / dt;
		printf("✅ Received %d %d  Id count %d  Lost %d Complete UDP Packet Received: Src Port=%d, Dst Port=%d, Packet Data Size=%d ID=%d Total Size %.2f MBytes time %.2f s  Rate %.2f Mbit/s  Packet rate %.2f kHz %d %d \n", nrec, gtc, id - first_id + 1, nloss,
			   ntohs(udp_header->source), ntohs(udp_header->dest), udp_data_len, id, totalbytes / 1024 / 1024., dt, mbs * 8, nrec / dt / 1000, elen, tokens);
	}
}

void processPmdaqPacket(const struct pfring_pkthdr *h, const u_char *p, const u_char *user_bytes)
{
  bool debug=false;
	struct ethhdr *eth = (struct ethhdr *)p; // Extract Ethernet header
	// int etherType = ntohs(eth->h_proto);
	// int port = (h->extended_hdr.parsed_pkt.l4_dst_port);
	// if (port!=8765) return;

	if (ntohs(eth->h_proto) != ETH_P_IP)
		return; // Only handle IPv4 packets
	// fprintf(stderr,"EtherType: %x %d  \n", etherType,port);
	struct iphdr *ip_header = (struct iphdr *)(p + sizeof(struct ethhdr)); // Extract IP header
	uint16_t ip_id = ntohs(ip_header->id);
	uint16_t frag_offset = ntohs(ip_header->frag_off) & 0x1FFF;		 // Extract fragment offset (in 8-byte blocks)
	int more_fragments = (ntohs(ip_header->frag_off) & 0x2000) != 0; // Extract MF flag
	int ip_header_length = ip_header->ihl * 4;
	int data_size = h->caplen - sizeof(struct ethhdr) - ip_header_length;
	int payload_offset = frag_offset * 8;

	if (debug)
	  fprintf(stderr,"📦 Fragment Received: ID=%d, Offset=%d, MF=%d, Size=%d\n", ip_id, frag_offset, more_fragments, data_size);
	memcpy(&recb[frag_offset * 8], p + sizeof(struct ethhdr) + ip_header_length, data_size);
	if (more_fragments == 0)
	{
		processCompleteUDPPacket(recb, frag_offset * 8 + data_size);
		return;
	}
	return;
}

void debugPmdaqPacket(const struct pfring_pkthdr *h,
						  const u_char *p, const u_char *user_bytes)
{
  //int i;
  // static unsigned int cnt = 0;
	
	nrec++;

	int *l = (int *)&p[h->extended_hdr.parsed_pkt.offset.payload_offset];
	if (nrec == 1)
		n0 = ntohl(l[0]);
	int id = ntohl(l[0]);
	int nbytesrec = h->len - h->extended_hdr.parsed_pkt.offset.payload_offset;
	nbytes += nbytesrec;
	// if (id%1000==1)
	printf("%d %d  %d len %d total %.2f \n", nrec, id - n0 + 1, id, h->len - h->extended_hdr.parsed_pkt.offset.payload_offset, nbytes / 1024. / 1024.);
}

int add_rule(pfring *pd, int id)//, int port, char *ipaddr)
{
  
  
  std::cout<<"Address "<<rd_conf.s_ip_address.c_str()<<std::endl;
  
	int rc;
	filtering_rule rule;
	memset(&rule, 0, sizeof(rule));
	rule.rule_id = id;
	rule.rule_action = forward_packet_and_stop_rule_evaluation;
	rule.core_fields.proto = 17; /* UDP */
	rule.core_fields.shost.v4 = ntohl(inet_addr((char*) rd_conf.s_ip_address.c_str()));
	rule.core_fields.shost_mask.v4 = 0xfffffeff;
	rule.core_fields.dport_low = rule.core_fields.dport_high = rd_conf.port;
	rule.extended_fields.tunnel.tunnel_id = NO_TUNNEL_ID; /* Ignore the tunnel */

	if ((rc = pfring_add_filtering_rule(pd, &rule)) < 0)
		fprintf(stderr, "pfring_add_filtering_rule(id=%d) failed: rc=%d\n", rule.rule_id, rc);
	else
		printf("Rule %.3d added successfully [ip:%s port:%d]\n", rule.rule_id, rd_conf.s_ip_address.c_str(), rd_conf.port);

	return rc;
}
void useJson(std::string confname)
{
  auto jr=jsonFile2Value(confname);
	std::cout<<jr<<std::endl;
	if (jr.isMember("interface"))
	  {
	    rd_conf.s_device=jr["interface"].asString();
	  }
	else
	  {
	    fprintf(stderr,"Missing interface tag\n");
	    exit(-1);
	  }
	if (jr.isMember("ip-from"))
	  {
	  rd_conf.s_ip_address=jr["ip-from"].asString();
	  }
	else
	  {
	    fprintf(stderr,"Missing ip-from tag\n");
	    exit(-1);
	  }
	if (jr.isMember("port-from"))
	  rd_conf.port=jr["port-from"].asUInt();
	else
	  {
	    fprintf(stderr,"Missing port-from tag\n");
	    exit(-1);
	  }
	


}
int main(int argc, char *argv[])
{
	pfring *pd;
	int rc;
	// packet_direction direction = rx_and_tx_direction;
	packet_direction direction = rx_only_direction;

	useJson("/etc/socket_receiver.json");


	pd = pfring_open((char*) rd_conf.s_device.c_str(), 64 * 1024, PF_RING_LONG_HEADER);
	if (pd == NULL)
	{
		printf("pfring_open error(%s) [%s]\n", device, strerror(errno));
		return (-1);
	}
	else
	{
		u_int32_t version;

		pfring_set_application_name(pd, "zino");
		pfring_version(pd, &version);

		printf("Using PF_RING v.%d.%d.%d on %s\n",
			   (version & 0xFFFF0000) >> 16,
			   (version & 0x0000FF00) >> 8,
			   version & 0x000000FF,
		       rd_conf.s_device.c_str());
	}

	if ((rc = pfring_set_direction(pd, direction)) != 0)
		fprintf(stderr, "pfring_set_direction returned %d (perhaps you use a direction other than rx only with ZC?)\n", rc);

	if ((rc = pfring_set_socket_mode(pd, recv_only_mode)) != 0)
		fprintf(stderr, "pfring_set_socket_mode returned [rc=%d]\n", rc);

	/*
	 * Aggiunta filtri
	 */
	// filter_dest_port=0; // The port of following packets is 0
		     //, filter_dest_port, (char*) filter_ip_source)) != 0)
	if ((rc = add_rule(pd, 5))!=0)
		fprintf(stderr, "add_rule returned [rc=%d]\n", rc);

	pfring_toggle_filtering_policy(pd, 0); /* Default to drop */
	if (pfring_enable_ring(pd) != 0)
	{
		fprintf(stderr, "Unable to enable ring :-(\n");
		pfring_close(pd);
		return (-1);
	}
	t0 = time(0);
	clock_gettime(CLOCK_MONOTONIC_RAW, &clock_start);
	// pfring_set_reassembly(pd, 1); // Enable reassembly
	pfring_loop(pd, processPmdaqPacket, (u_char *)NULL, 1);

	pfring_close(pd);

	return (0);
}
