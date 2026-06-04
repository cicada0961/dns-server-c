#ifndef DNS_H
#define DNS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 50

typedef struct Record
{
  char *domaine;
  unsigned char ip[4];
} Record;

typedef struct Header
{
  uint16_t id;
  uint16_t flags;
  uint16_t qdcount;
  uint16_t ancount;
  uint16_t nscount;
  uint16_t arcount;

  char *domaine;
} Header;

typedef struct Quest
{
  uint16_t qtype;
  uint16_t qclass;
} Quest;

typedef struct Resp
{
  uint32_t ttl;
  uint32_t rdata;
  uint16_t rdlength;
} Resp;

typedef struct Dns
{
  Header *header;
  Quest *quest;
  Resp *resp;

} Dns;

// SOCKET
Dns *init_dns();
int init_sockfd();
struct sockaddr_in init_addr();

// TABLE
int load_table(Record *table, char *filename);
unsigned char *resolve(Record *table, char *domaine);

// PARSER
int parse_name(unsigned char *buff, int pos, Header *d);
void save_head(Header *header, unsigned char *buff);
void save_quest(Quest *quest, unsigned char *buff, int *n);

// BUILDER
int build_header(unsigned char *resp, Header *h);
int build_resp(Record *table, unsigned char *resp, int *pos, char *domaine, unsigned char *buff, int query_len);

// NETWORK
int forward(unsigned char *query, int query_len, unsigned char *response);
void send_resp(Record *table, Dns *dns, unsigned char *buff, int sockfd, struct sockaddr_in addr, socklen_t addr_len, int query_len);

// UTILS
void print_screen();
void free_dns(Dns *dns);

#endif
