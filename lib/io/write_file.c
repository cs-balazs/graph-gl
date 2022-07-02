#include <stdio.h>

void write_file(const char *dest, char *content)
{
  FILE *fp;

  fp = fopen(dest, "w");
  fprintf(fp, content);
  fclose(fp);
}