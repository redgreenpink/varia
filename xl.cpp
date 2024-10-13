
#include <stdio.h>
#include <string.h>

extern "C"
  {
  #include "regex.h"
  }

#define  LINE_SIZE   257
#define  BUFFER_SIZE  16
#define  OUT_SUFFIX  ".out"

int main(int argc, char *argv[])
  {
  int            fix;
  FILE          *infile, *outfile;
  char          *outname = new char[1], dummy;
  int            toreturn = 0;
  int            byte, index, bytesread;
  unsigned char  byte_buffer[BUFFER_SIZE];
  char           line_buffer[LINE_SIZE];

#define HEX_NUMBER  "[0-9A-Fa-f]"
#define WHITE_SPACE "[ \t]+"
  struct re_pattern_buffer HexNumber, WhiteSpace;
  memset(&HexNumber, 0, sizeof(HexNumber));
  memset(&WhiteSpace, 0, sizeof(WhiteSpace));
  re_set_syntax(RE_SYNTAX_AWK);
  const char *pMessage =
    re_compile_pattern(HEX_NUMBER, sizeof(HEX_NUMBER) - 1, &HexNumber);
  if (pMessage != NULL)
    {
    printf("regex error: %s\n", pMessage);
    return -1;
    }
  pMessage =
    re_compile_pattern(WHITE_SPACE, sizeof(WHITE_SPACE) - 1, &WhiteSpace);
  if (pMessage != NULL)
    {
    printf("regex error: %s\n", pMessage);
    return -1;
    }

  strcpy(outname, "");
  for (fix = 1; fix < argc; fix++)
    {
    infile = fopen(argv[fix], "r");
    if (infile == NULL)
      {
      fprintf(stderr, "xl: cannot open \"%s\"\n", argv[fix]);
      toreturn = -1;
      continue;
      }

    if (strlen(outname) < strlen(argv[fix]) + strlen(OUT_SUFFIX))
      {
      delete outname;
      outname = new char [strlen(argv[fix]) + strlen(OUT_SUFFIX) + 1];
      }
    sprintf(outname, "%s%s", argv[fix], OUT_SUFFIX);
    outfile = fopen(outname, "wb");
    if (outfile == NULL)
      {
      fprintf(stderr, "xl: cannot open \"%s\"\n", outname);
      fclose(infile);
      toreturn = -1;
      continue;
      }

    for (index = 0; ; index += bytesread)
      {
      char  *fgets_returned =
        fgets(line_buffer, sizeof(line_buffer) - 1, infile);
      if (fgets_returned != NULL)
        {
        if (strcmp(line_buffer, "")     == 0 ||
            strcmp(line_buffer, "\n")   == 0 ||
            strcmp(line_buffer, "\r")   == 0 ||
            strcmp(line_buffer, "\n\r") == 0 ||
            strcmp(line_buffer, "\r\n") == 0)
          continue;
        if (strlen(line_buffer) >= sizeof(line_buffer) - 2)
          {
          printf("xl: line to long: \"%s\"\n", line_buffer);
          continue;
          }
        if (sscanf(line_buffer, "%*s %c", &dummy) != 1)
          {
          printf("xl: bad line: \"%s\"\n", line_buffer);
          continue;
          }
        if (dummy != '|')
          {
          printf("xl: bad line: \"%s\"\n", line_buffer);
          continue;
          }
        char *charptr = strstr(line_buffer, "|") + 1;
        for (bytesread = 1; bytesread < LINE_SIZE; bytesread++)
          {
          if (sscanf(charptr, "%x", &byte) != 1)
            {
            bytesread--;
            break;
            }
          byte_buffer[bytesread - 1] = byte;
          int nOffset =
            re_search(
              &HexNumber, charptr, strlen(charptr), 0,
              strlen(charptr) - 1, NULL);
          charptr += nOffset;
          nOffset =
            re_search(
              &WhiteSpace, charptr, strlen(charptr), 0,
              strlen(charptr) - 1, NULL);
          if (nOffset == -1)
            {
            if (sscanf(charptr, "%*x %c", &dummy) > 0)
              {
              printf("xl: line to long: \"%s\"\n", line_buffer);
              goto CONTINUE;
              }
            else
              break;
            }
          charptr += nOffset;
          }
        if (bytesread == 0)
          {
          printf("xl: bad line: \"%s\"\n", line_buffer);
          continue;
          }
        fwrite(byte_buffer, 1, bytesread, outfile);
        }

      if (feof(infile))
        break;
      if (ferror(infile))
        {
        fprintf(stderr, "xl: cannot read \"%s\"\n", argv[fix]);
        break;
        }
      if (ferror(outfile))
        {
        fprintf(stderr, "xl: cannot write \"%s\"\n", outname);
        break;
        }
      CONTINUE:
        ;
      }

    fclose(infile);
    fclose(outfile);
    if (ferror(outfile))
      fprintf(stderr, "xl: cannot write \"%s\"\n", outname);
    }

  delete outname;

  return toreturn;
  }
