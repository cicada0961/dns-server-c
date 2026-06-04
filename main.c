#include "dns.h"

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
