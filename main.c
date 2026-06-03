#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

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

Dns *init_dns()
{
    Header *header = malloc(sizeof(Header));
    Quest *quest = malloc(sizeof(Quest));

    Dns *dns = malloc(sizeof(Dns));
    dns->header = header;
    dns->quest = quest;

    return dns;
}

void print_screen()
{
    printf("==============================================\n");
    printf("  DNS Server running on port 8053\n");
    printf("==============================================\n");
    printf("  Test with dig:\n");
    printf("  dig @127.0.0.1 -p 8053 <domain>\n");
    printf("\n");
    printf("  Examples:\n");
    printf("  dig @127.0.0.1 -p 8053 google.com\n");
    printf("  dig @127.0.0.1 -p 8053 facebook.com\n");
    printf("==============================================\n");
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

void free_dns(Dns *dns)
{
    free(dns->header->domaine);
    free(dns->header);
    free(dns->quest);
    free(dns);
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

unsigned char *resolve(Record *table, char *domaine)
{
    for (int i = 0; table[i].domaine != NULL; i++)
        if (strcmp(table[i].domaine, domaine) == 0)
            return table[i].ip;
    return NULL;
}

void build_resp(Record *table, unsigned char *resp, int *pos, char *domaine)
{
    unsigned char *d = resolve(table, domaine);

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

    if (d != NULL)
    {
        resp[*pos] = d[0];
        resp[*pos + 1] = d[1];
        resp[*pos + 2] = d[2];
        resp[*pos + 3] = d[3];
    }
    else
    {
        resp[*pos] = 0x00;
        resp[*pos + 1] = 0x00;
        resp[*pos + 2] = 0x00;
        resp[*pos + 3] = 0x00;
    }
    *pos += 4;
}

void send_resp(Record *table, Dns *dns, unsigned char *buff, int sockfd, struct sockaddr_in addr, socklen_t addr_len)
{
    unsigned char resp[512];
    int cmt = parse_name(buff, 12, dns->header);
    save_quest(dns->quest, buff, &cmt);
    int pos = build_header(resp, dns->header);
    memcpy(resp + pos, buff + 12, cmt - 12);
    pos += cmt - 12;

    build_resp(table, resp, &pos, dns->header->domaine);
    sendto(sockfd, resp, pos, 0, (struct sockaddr *)&addr, addr_len);
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

int main()
{
    Record table[4] = {
        {"google.com", {142, 250, 74, 46}},
        {"facebook.com", {0, 0, 0, 0}},
        {"youtube.com", {172, 217, 20, 174}},
        {NULL, {0}}};

    int sockfd = init_sockfd();
    struct sockaddr_in addr = init_addr();
    socklen_t addr_len = sizeof(addr);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        return 1;
    }
    else
    {
        print_screen();
    }

    while (1)
    {
        Dns *dns = init_dns();
        unsigned char buff[512];
        recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&addr, &addr_len);
        save_head(dns->header, buff);

        if ((int)dns->header->qdcount == 1)
            send_resp(table, dns, buff, sockfd, addr, addr_len);
        free_dns(dns);
    }
}
