
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define  LINE_SIZE   16
#define  OUT_SUFFIX  ".out"

int main(int argc, char *argv[])
  {
  int            fix;
  FILE          *infile, *outfile;
  char          *outname = new char[1];
  int            toreturn = 0;
  int            byte, subbyte, bytesread;
  unsigned char  line_buffer[LINE_SIZE];

  strcpy(outname, "");
  for (fix = 1; fix < argc; fix++)
    {
    infile = fopen(argv[fix], "rb");
    if (infile == NULL)
      {
      fprintf(stderr, "xd: cannot open \"%s\"\n", argv[fix]);
      toreturn = -1;
      continue;
      }

    if (strlen(outname) < strlen(argv[fix]) + strlen(OUT_SUFFIX))
      {
      delete outname;
      outname = new char [strlen(argv[fix]) + strlen(OUT_SUFFIX) + 1];
      }
    sprintf(outname, "%s%s", argv[fix], OUT_SUFFIX);
    outfile = fopen(outname, "w");
    if (outfile == NULL)
      {
      fprintf(stderr, "xd: cannot open \"%s\"\n", outname);
      fclose(infile);
      toreturn = -1;
      continue;
      }

    for (byte = 0; ; byte += LINE_SIZE)
      {
      bytesread = fread(line_buffer, 1, LINE_SIZE, infile);
      if (bytesread > 0)
        {
        fprintf(outfile, "%08x |", byte);
        for (subbyte = 0; subbyte < bytesread; subbyte++)
          fprintf(outfile, " %02x", line_buffer[subbyte]);
        for (; subbyte < LINE_SIZE; subbyte++)
          fprintf(outfile, "   ");
        fprintf(outfile, " | ");
        for (subbyte = 0; subbyte < bytesread; subbyte++)
          fprintf(outfile, "%c", isprint(line_buffer[subbyte]) ? line_buffer[subbyte] : '.');
        fprintf(outfile, "\n");
        }

      if (feof(infile))
        break;
      if (ferror(infile))
        {
        fprintf(stderr, "xd: cannot read \"%s\"\n", argv[fix]);
        break;
        }
      if (ferror(outfile))
        {
        fprintf(stderr, "xd: cannot write \"%s\"\n", outname);
        break;
        }
      }

    fclose(infile);
    fclose(outfile);
    if (ferror(outfile))
      fprintf(stderr, "xd: cannot write \"%s\"\n", outname);
    }

  delete outname;

  return toreturn;
  }
