#include "../dns.h"

int build_header(unsigned char *resp, Header *h)
{
  resp[0] = h->id >> 8;
  resp[1] = h->id & 0xFF;
  resp[2] = (char)(0x8180 >> 8);
  resp[3] = (char)(0x8180 & 0xFF);
  resp[4] = h->qdcount >> 8;
  resp[5] = h->qdcount & 0xFF;
  resp[6] = 0;
  resp[7] = 1;
  resp[8] = 0 >> 8;
  resp[9] = 0 & 0xFF;
  resp[10] = 0 >> 8;
  resp[11] = 0 & 0xFF;

  return 12;
}

int build_resp(Record *table, unsigned char *resp, int *pos, char *domaine, unsigned char *buff, int query_len)
{
  unsigned char *d = resolve(table, domaine);

  if (d == NULL)
  {
    printf("Aucun domaine trouvé sur nos serveurs. Demande en cours aux serveurs Google.\n");
    return forward(buff, query_len, resp);
  }
  else
  {
    printf("Domaine trouvé : \n");
    resp[*pos] = 0xC0;
    resp[*pos + 1] = 0x0C;
    *pos += 2;

    resp[*pos] = 0x00;
    resp[*pos + 1] = 0x01;
    *pos += 2;

    resp[*pos] = 0x00;
    resp[*pos + 1] = 0x01;
    *pos += 2;

    resp[*pos] = 0x00;
    resp[*pos + 1] = 0x00;
    resp[*pos + 2] = 0x01;
    resp[*pos + 3] = 0x2C;
    *pos += 4;

    resp[*pos] = 0x00;
    resp[*pos + 1] = 0x04;
    *pos += 2;

    resp[*pos] = d[0];
    resp[*pos + 1] = d[1];
    resp[*pos + 2] = d[2];
    resp[*pos + 3] = d[3];

    *pos += 4;
    return -1;
  }
}
