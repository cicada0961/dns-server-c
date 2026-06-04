#include "../dns.h"

int forward(unsigned char *query, int query_len, unsigned char *response)
{
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1)
  {
    perror("sockfd");
    return -1;
  }

  struct sockaddr_in dest;
  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(53);
  dest.sin_addr.s_addr = inet_addr("8.8.8.8");

  sendto(sockfd, query, query_len, 0, (struct sockaddr *)&dest, sizeof(dest));
  socklen_t dest_len = sizeof(dest);
  int n = recvfrom(sockfd, response, 512, 0, (struct sockaddr *)&dest, &dest_len);
  close(sockfd);

  return n;
}

void send_resp(Record *table, Dns *dns, unsigned char *buff, int sockfd, struct sockaddr_in addr, socklen_t addr_len, int query_len)
{
  unsigned char resp[512];
  int cmt = parse_name(buff, 12, dns->header);
  save_quest(dns->quest, buff, &cmt);
  int pos = build_header(resp, dns->header);
  memcpy(resp + pos, buff + 12, cmt - 12);
  pos += cmt - 12;

  int size = build_resp(table, resp, &pos, dns->header->domaine, buff, query_len);
  if (size == -1)
    sendto(sockfd, resp, pos, 0, (struct sockaddr *)&addr, addr_len);

  else
    sendto(sockfd, resp, size, 0, (struct sockaddr *)&addr, addr_len);
}
