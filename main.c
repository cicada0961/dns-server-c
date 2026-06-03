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

struct sockaddr_in init_addr()
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8053);
    addr.sin_addr.s_addr = INADDR_ANY;

    return addr;
}

int load_table(Record *table, char *filename)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL)
    {
        perror("fopen");
        return -1;
    }

    char line[256];
    int cmt = 0;
    while (fgets(line, sizeof(line), f) != NULL && cmt < MAX)
    {
        char *domaine = strtok(line, " ");
        char *ip_str = strtok(NULL, " ");
        char *token = strtok(ip_str, ".");
        for (int i = 0; i < 4; i++)
        {
            table[cmt].ip[i] = atoi(token);
            token = strtok(NULL, ".");
        }
        table[cmt].domaine = malloc(strlen(domaine) + 1);
        strcpy(table[cmt].domaine, domaine);
        cmt++;
    }
    fclose(f);
    table[cmt].domaine = NULL;
    printf("Chargement du fichier reussi.\n");
    return cmt;
}

int main()
{
    Record table[MAX];
    char *fname = "blocklist.txt";
    int nb_enregistrement = load_table(table, fname);

    if (nb_enregistrement == -1)
    {
        printf("Erreur au chargement du fichier.\n");
        return 1;
    }

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
        int query_len = recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&addr, &addr_len);
        save_head(dns->header, buff);

        if ((int)dns->header->qdcount == 1)
            send_resp(table, dns, buff, sockfd, addr, addr_len, query_len);
        free_dns(dns);
    }
}
