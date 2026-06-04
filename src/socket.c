#include "../dns.h"

Dns *init_dns()
{
  Header *header = malloc(sizeof(Header));
  Quest *quest = malloc(sizeof(Quest));

  Dns *dns = malloc(sizeof(Dns));
  dns->header = header;
  dns->quest = quest;

  return dns;
}

int init_sockfd()
{
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1)
  {
    perror("sockfd");
    exit(1);
  }
  return sockfd;
}

struct sockaddr_in init_addr()
{
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8053);
  addr.sin_addr.s_addr = INADDR_ANY;

  return addr;
}
