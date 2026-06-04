#include "../dns.h"

int parse_name(unsigned char *buff, int pos, Header *d)
{
  int j = 0;
  char out[512];
  while (buff[pos] != 0)
  {
    int len = buff[pos];
    pos++;

    for (int i = 0; i < len; i++)
    {
      out[j] = buff[pos];
      j++;
      pos++;
    }
    if (buff[pos] != 0)
    {
      out[j] = '.';
      j++;
    }
  }
  out[j] = '\0';
  d->domaine = malloc(strlen(out) + 1);
  strcpy(d->domaine, out);
  return pos + 1;
}

void save_head(Header *header, unsigned char *buff)
{
  header->id = (buff[0] << 8) | buff[1];
  header->flags = (buff[2] << 8 | buff[3]);
  header->qdcount = (buff[4] << 8 | buff[5]);
  header->ancount = (buff[6] << 8 | buff[7]);
  header->nscount = (buff[8] << 8 | buff[9]);
  header->arcount = (buff[10] << 8 | buff[11]);
}

void save_quest(Quest *quest, unsigned char *buff, int *n)
{
  quest->qtype = (buff[*n] << 8 | buff[(*n) + 1]);
  (*n) += 2;
  quest->qclass = (buff[*n] << 8 | buff[(*n) + 1]);
  (*n) += 2;
}
