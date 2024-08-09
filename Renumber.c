#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>#include <string.h>
#include <ctype.h>
#include <stdbool.h>

int Renumber(char *infilename, char *outfilename, int increment/*, int startlinenumber, int rangestart, int rangeend*/) {
   FILE *infile, *outfile;
   char inputline[32768], tokenbuffer[65536], c, *dest;
   int i, currentchar = 0, currentlinenumber = 1, tokenchar = 0;
   int oldlinenumber[10000], newlinenumber[10000], oldline, newline, result, newgoto;

   memset(oldlinenumber, 0, 10000);
   memset(newlinenumber, 0, 10000);
   infile = fopen(infilename, "r");
   if (infile == NULL) {
      fprintf(stderr, "Cannot open input file %s\n", infilename);
      return 1;
   }

   // First pass - collect line numbers
   while (fgets(inputline, 32768, infile) != NULL) {
      // Skip leading whitespace
      while (isspace(inputline[currentchar]))
         currentchar++;
      // Blank lines ignored
      if (currentchar < strlen(inputline)) {
         // Not a blank line - check first non-whitespace character
         c = inputline[currentchar++];
         if (!isdigit(c)) {
            fprintf(stderr, "Error in line %d - must begin with a line number\n", currentlinenumber);
            fclose(infile);
            return 1;
         }
         else {
            // Get line number
            tokenbuffer[tokenchar++] = c;
            c = inputline[currentchar++];
            while (isdigit(c)) {
               tokenbuffer[tokenchar++] = c;
               c = inputline[currentchar++];
            }
            tokenbuffer[tokenchar] = 0;
            oldlinenumber[currentlinenumber] = atoi(tokenbuffer);
            newlinenumber[currentlinenumber] = currentlinenumber * increment;
            currentlinenumber++;
         }
      }
      currentchar = 0;
      tokenchar = 0;
   }

   // Second pass - output file
   outfile = fopen(outfilename, "w");
   if (outfile == NULL) {
      fclose(infile);
      fprintf(stderr, "Cannot open output file %s\n", outfilename);
      return 2;
   }
   rewind(infile);
   currentlinenumber = 1;
   while (fgets(inputline, 32768, infile) != NULL) {
      // Skip leading whitespace
      while (isspace(inputline[currentchar]))
         currentchar++;
      // Blank lines ignored
      if (currentchar < strlen(inputline)) {
         // Not a blank line - check first non-whitespace character
         c = inputline[currentchar];
         while (isdigit(c)) {
            c = inputline[currentchar++];
         }
         dest = strstr(inputline, "Goto");
         if (dest != NULL) {
            fprintf(outfile, "%d ", newlinenumber[currentlinenumber++]);
            for (i = currentchar;i < dest - inputline + 4;i++)
               fprintf(outfile, "%c", inputline[i]);
            result = dest - inputline + 4;
            c = inputline[result++];
            while (isdigit(c)) {
               tokenbuffer[tokenchar++] = c;
               c = inputline[result++];
            }
            tokenbuffer[tokenchar] = 0;
            newgoto = atoi(tokenbuffer);
            for (i = 1; i < 100; i++) {
               if (oldlinenumber[i] == newgoto) {
                  newgoto = newlinenumber[i];
                  break;
               }
            }
            if (i == 100) {
               fprintf(stderr, "error in line %d - line number %d not found\n", currentlinenumber, newgoto);
               fclose(outfile);
               fclose(infile);
               exit(1);
            }
            fprintf(outfile, "%d\n", newgoto);
         }
         else {
            dest = strstr(inputline, "Gos");
            if (dest != NULL) {
               fprintf(outfile, "%d ", newlinenumber[currentlinenumber++]);
               for (i = currentchar;i < dest - inputline + 3;i++)
                  fprintf(outfile, "%c", inputline[i]);
               result = dest - inputline + 3;
               c = inputline[result++];
               while (isdigit(c)) {
                  tokenbuffer[tokenchar++] = c;
                  c = inputline[result++];
               }
               tokenbuffer[tokenchar] = 0;
               newgoto = atoi(tokenbuffer);
               for (i = 1; i < 100; i++) {
                  if (oldlinenumber[i] == newgoto) {
                     newgoto = newlinenumber[i];
                     break;
                  }
               }
               if (i == 100) {
                  fprintf(stderr, "error in line %d - line number %d not found\n", currentlinenumber, newgoto);
                  fclose(outfile);
                  fclose(infile);
                  exit(1);
               }
               fprintf(outfile, "%d\n", newgoto);
            }
            else {               
               fprintf(outfile, "%d ", newlinenumber[currentlinenumber++]);
               dest = strstr(inputline, "\n");
               for (i = currentchar; i < strlen(inputline) - (dest > 0 ? 2 : 0); i++)
                  fprintf(outfile, "%c", inputline[i]);
               fprintf(outfile, "\n");
            }
         }
      }
      currentchar = 0;
      tokenchar = 0;
   }
   fclose(outfile);
   fclose(infile);
}

int main(int argc, char** argv) {
   int ret = EXIT_SUCCESS, increment = 1;

   if (argc == 4)
      increment = atoi(argv[3]);
   if (argc < 3)
      printf("Usage: Renumber infilename outfilename [increment]\n"
             "infilename is the name of the file to be renumbered\n"
             "outfilename is the renumbered file\n"
             "increment is the increment to be used in the new number sequence\n"
             "increment defaults to 1 if not specified\n");      
   else
      Renumber(argv[1], argv[2], increment);
   
   exit(ret);
}
