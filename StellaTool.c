#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>

#define LINUX 1

const int STATE_FILE_SIZE = 3292;
const int NTSC = 0;
const int PAL = 1;
const char* characters[] = {
   "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "", "", "", "", "", "",
   "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P",
   "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "", "", "", "", "", " ",
   "+", "-", "*", "/", ".", "[", "]", "=", ",", "$", "?", "\"", "<", ">", "Rnd", "",
   "New", "", "List", "", "Run", "", "Dele", "", "Load", "", "Save", "", "Brk", "", "Cont", "",
   "Then", "", "For", "", "Next", "", "Gos", "", "Rtn", "", "Step", "", "", "", "Int", "",
   "Goto", "", "Rem", "", "", "", "Let", "", "Inp", "",  "Prt", "", "If", "To", "", "",
   "Rt", "Me_", "Fa_", "So_", "La_", "Te_", "Do", "Re", "Me", "Fa", "So", "La", "Te", "_Do", "_Re", "_Me",
   "", "Sixteenth", "Eighth", "Quarter", "Dottedquarter", "Half", "Dottedhalf", "Whole"
};

// Check if program is too long
void CheckBufferPointer(int bufferpointer, int linenumber, FILE *infile) {
   if (bufferpointer > 3189) {
      fprintf(stderr, "Error in line %d - program too long", linenumber);
      fprintf(stderr, "Error code 1\n");
      fclose(infile);
      exit(1);
   }
}

int GetStateFileName(char *n, char *filename, int filetype) {
   char *configfolder;
   int ret;
#ifdef LINUX
   struct stat buf;
   configfolder = getenv("HOME");
   ret = sprintf(filename, "%s/.config/stella/state/", configfolder);
   ret = stat(filename, &buf);
   if (ret == 0 && S_ISDIR(buf.st_mode))
      ret = sprintf(filename, "%s/.config/stella/state/CompuMate (1983) (Spectravideo)%s.st%s", configfolder, filetype == NTSC ? "" : " (PAL)", n);
   else {
      fprintf(stderr, "Cannot open state folder %s\n", filename);
      return -1;
   }
#else
   struct _stat buf;
   configfolder = getenv("APPDATA");
   ret = sprintf(filename, "%s\\Stella\\state\\", configfolder);
   ret = _stat(filename, &buf);
   if ((buf.st_mode & _S_IFDIR) == _S_IFDIR)
      ret = sprintf(filename, "%sCompuMate (1983) (Spectravideo)%s.st%s", filename, filetype == NTSC ? "" : " (PAL)", n);
   else {
      fprintf(stderr, "Cannot open state folder %s\n", filename);
      return -1;
   }
#endif
   return EXIT_SUCCESS;
}

int ReadBasicTextFile(char *n, char *infilename, int filetype) {
   FILE *infile, *outfile;
   uint16_t linepointer;
   unsigned char memorybuffer[STATE_FILE_SIZE];
   char filename[32767], tokenbuffer[65536], inputline[32768], searchtoken[32768], c;
   int i, ret, bufferpointer = 0x0775, tokenchar = 0, currentchar = 0, currentlinenumber, linenumber = 1;
   bool foundtoken, quotes = false, remark = false;

   // Read empty state file into memory buffer
   ret = sprintf(filename, "blank%s.st", filetype == NTSC ? "" : "pal");
   infile = fopen(filename, "rb");
   if (infile == NULL) {
      fprintf(stderr, "Cannot open blank state file %s\n", filename);
      return -1;
   }
   fread(memorybuffer, sizeof(unsigned char), STATE_FILE_SIZE, infile);
   fclose(infile);

   // Open Basic source file
   infile = fopen(infilename, "r");
   if (infile == NULL) {
      fprintf(stderr, "Cannot open input file %s\n", infilename);
      return -1;
   }
   else {
      while (fgets(inputline, 32768, infile) != NULL) {
         // Handle carriage return in Linux
         if (inputline[strlen(inputline) - 2] == '\r')
            inputline[strlen(inputline) - 2] = 0;

         // Skip leading whitespace
         while (isspace(inputline[currentchar]))
            currentchar++;
         // Blank lines ignored
         if (currentchar < strlen(inputline)) {
            // Not a blank line - check first non-whitespace character
            c = inputline[currentchar++];
            if (!isdigit(c)) {
               fprintf(stderr, "Error in line %d - must begin with a line number\n", linenumber);
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
               currentlinenumber = atoi(tokenbuffer);
               if (currentlinenumber < 0 || currentlinenumber > 99) {
                  fprintf(stderr, "Error in line %d - line number must be between 1 and 99\n", linenumber);
                  fclose(infile);
                  return 1;
               }

               // Work around bug in Compumate BASIC
               // If a line starts on a 256 byte boundary then it is not able to be listed or run
               // Leave a gap of one byte in this case
               if ((bufferpointer + 0x7353) % 256 == 0)
                  bufferpointer++;

               // Write line number to buffer
               memorybuffer[0x6AD + 2 * currentlinenumber] = (bufferpointer + 0x7353) % 256;
               memorybuffer[0x6AD + 2 * currentlinenumber + 1] = (bufferpointer + 0x7353) / 256;
               memorybuffer[bufferpointer] = (unsigned char)currentlinenumber;
               bufferpointer++;
               CheckBufferPointer(bufferpointer, linenumber, infile);

               // Parse rest of line and write to buffer
               c = inputline[currentchar];
               while(c != '\n' && c != 0) {
                  // Get next token
                  tokenchar = 0;
                  tokenbuffer[tokenchar] = c;
                  if (isupper(c)) {
                     // Letter or keyword starting with letter
                     c = inputline[++currentchar];
                     while (islower(c) || c == '_') {
                        tokenbuffer[++tokenchar] = c;
                        c = inputline[++currentchar];
                     }
                     currentchar--;
                  }
                  else if (c == '_') {
                     // Keyword starting with _
                     c = inputline[++currentchar];
                     while (isalpha(c)) {
                        tokenbuffer[++tokenchar] = c;
                        c = inputline[++currentchar];
                     }
                     currentchar--;
                  }
                  else if (c == '\"')
                     quotes = !quotes;
                  tokenbuffer[tokenchar + 1] = 0;
                  currentchar++;

                  // Check if next token is valid.  Will exclude spaces outside of quotes or remarks
                  foundtoken = false;
                  for (i = 0; i < 136; i++) {
                     if (strcmp(tokenbuffer, characters[i]) == 0) {
                        c = characters[i][0];
                        if (c != ' ' || (c == ' ' && (quotes == true || remark == true)))
                           memorybuffer[bufferpointer++] = i;
                        CheckBufferPointer(bufferpointer, linenumber, infile);
                        foundtoken = true;
                        if (i == 98) {
                           remark = true;
                           quotes = false;
                        }
                        break;
                     }
                  }
                  if (foundtoken == false) {
                     printf("Error in line %d - invalid token %s\n", linenumber, tokenbuffer);
                     fclose(infile);
                     return 1;
                  }
                  c = inputline[currentchar];
               }

               // Write end of line marker to buffer
               memorybuffer[bufferpointer++] = 0xDA;
               CheckBufferPointer(bufferpointer, linenumber, infile);
            }
            currentchar = 0;
            tokenchar = 0;
         }
         else
            currentchar--;
         linenumber++;
      }
      fclose(infile);

      // Set end of basic pointer
      memorybuffer[0xA3] = (bufferpointer + 0x7353) % 256;
      memorybuffer[0xA4] = (bufferpointer + 0x7353) / 256;
      printf("Program length - %d bytes\n", bufferpointer - 0x0775);
   }

   // Save state file in Stella folder
   ret = GetStateFileName(n, filename, filetype);
   if (ret == 0) {
      outfile = fopen(filename, "wb");
      if (outfile == NULL) {
         fprintf(stderr, "Cannot open output file %s\n", filename);
         return 2;
      }
      else {
         fwrite(memorybuffer, sizeof(unsigned char), STATE_FILE_SIZE, outfile);
         fclose(outfile);
      }

      return EXIT_SUCCESS;
   }
   else
      return ret;
}

int WriteBasicTextFile(char *n, char *outfilename, int filetype) {
   FILE *infile, *outfile;
   int filesize, i, bufferpointer, ret;
   uint16_t linepointer;
   unsigned char memorybuffer[STATE_FILE_SIZE], inchar;
   char *infilename, filename[32767];

   ret = GetStateFileName(n, filename, filetype);
   if (ret != 0)
      return ret;
   infile = fopen(filename, "rb");
   if (infile == NULL) {
      fprintf(stderr, "Cannot open state file %s\n", filename);
      return 2;
   }
   else {
      fseek(infile, 0L ,SEEK_END);
      filesize = ftell(infile);
      if (filesize != STATE_FILE_SIZE) {
         fprintf(stderr, "Not a CompuMate state file %s\n", filename);
         fclose(infile);
         return 3;
      }
      outfile = fopen(outfilename, "w");
      if (outfile == NULL) {
         fprintf(stderr, "Cannot open output file %s\n", outfilename);
         fclose(infile);
         return 4;
      }
      rewind(infile);
      fread(memorybuffer, sizeof(unsigned char), STATE_FILE_SIZE, infile);
      for (i = 0; i < 99; i++) {
         linepointer = *(uint16_t*) &memorybuffer[0x6AF + i * 2];
         if (linepointer > 0) {
            bufferpointer = linepointer - 0x7353;
            inchar = memorybuffer[bufferpointer];
            fprintf(outfile, "%d ", inchar);
            while ((inchar = memorybuffer[++bufferpointer]) != 0xDA) {
               if (inchar > 135)
                  fprintf(outfile, "{%d}", inchar);
               else
                  fprintf(outfile, "%s", characters[inchar]);
            }
            fprintf(outfile, "\n");
         }
      }
      fclose(outfile);
      fclose(infile);
      printf("Program length - %d bytes\n", bufferpointer - 0x0774);
      return EXIT_SUCCESS;
   }
}

int main(int argc, char** argv) {
   int ret = EXIT_SUCCESS;

   if (argc > 3)
      if (strcmp(argv[1], "writetxt") == 0)
         ret = WriteBasicTextFile(argv[2], argv[3], NTSC);
      else if (strcmp(argv[1], "writetxtpal") == 0)
         ret = WriteBasicTextFile(argv[2], argv[3], PAL);
      else if (strcmp(argv[1], "readtxt") == 0)
         ret = ReadBasicTextFile(argv[2], argv[3], NTSC);
      else if (strcmp(argv[1], "readtxtpal") == 0)
         ret = ReadBasicTextFile(argv[2], argv[3], PAL);
      else
         ret = 1;
   else {
      printf("Usage: StellaTool readtxt/writetxt/readtxtpal/writetxtpal n filename\n"
             "readtxt reads text file into state file number n\n"
             "readtxtpal reads text file into PAL state file number n\n"
             "writetxt writes text file from state file number n\n"
             "writetxtpal writes text file from PAL state file number n\n"
             "filename is a Compumate BASIC file in text format\n");
      ret = 1;
   }
   if (ret != EXIT_SUCCESS)
      fprintf(stderr, "Error code %d\n", ret);
   exit(ret);
}
