#include "../dns.h"

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

unsigned char *resolve(Record *table, char *domaine)
{
  for (int i = 0; table[i].domaine != NULL; i++)
    if (strcmp(table[i].domaine, domaine) == 0)
      return table[i].ip;
  return NULL;
}
