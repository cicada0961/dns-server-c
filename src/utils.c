#include "../dns.h"

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

void free_dns(Dns *dns)
{
  free(dns->header->domaine);
  free(dns->header);
  free(dns->quest);
  free(dns);
}
