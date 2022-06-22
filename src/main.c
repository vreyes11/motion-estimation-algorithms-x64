/* main.c - the program's UNIX interface */
#include <assert.h> /* for assert() */
#include <libgen.h> /* for basename() */
#include <getopt.h> /* for external optarg, opterr, optind, getopt() */
#include <stdio.h> /* for FILE, fprint, fread, stdin, stdout, stderr */
#include <errno.h> /* for external errno variable */
#include <string.h> /* for memcpy, memset, strlen */
#include <stdio.h> /* for FILE */
#include <stdlib.h> /* for stroul, exit() */
#include <stdint.h> /* for uint32_t */
#include <stddef.h> /* for size_t */
#include "main.h"

extern int errno;
extern char *optarg; /* for use with getopt() */
extern int opterr, optind;

static char *DEFAULT_SRC = "../colors.bmp";
static char *DEFAULT_DEST = "../RESULT.bmp";
static int BMP_MAGIC = 0x4D42;

void usage(char *progname) {
   fprintf(stderr, USAGE_FMT, progname ? progname : DEFAULT_PROGNAME);
}

// Assumes Little-endian, 24bit bmp
int handle_image(char *src, char *dest) {
   if (!src || !dest) {
      errno = EINVAL;
      return -1;
   }

   printf("src: %s, dest: %s\n", src, dest);
   FILE *fp = NULL;
   if (!(fp = fopen(src, "rb")))
      return -1;

   // first read bfheader bytes from file
   struct bmp_fheader bfheader;
   fread(&bfheader.ftype, sizeof(bfheader.ftype), 1, fp);   
   fread(&bfheader.fsize, sizeof(bfheader.fsize), 1, fp);
   fread(&bfheader.reserved1, sizeof(bfheader.reserved1), 1, fp);
   fread(&bfheader.reserved2, sizeof(bfheader.reserved2), 1, fp);
   fread(&bfheader.offset, sizeof(bfheader.offset), 1, fp);

   if (bfheader.ftype == BMP_MAGIC)
      printf("Your image is a BMP file.\n");

   // second read biheader bytes from file
   // NOTE: Assuming biheader follows immediately after bfheader
   struct bmp_iheader biheader;
   fread(&biheader.size, sizeof(biheader.size), 1, fp);
   fread(&biheader.imageWidth, sizeof(biheader.imageWidth), 1, fp);
   fread(&biheader.imageHeight, sizeof(biheader.imageHeight), 1, fp);
   fread(&biheader.planes, sizeof(biheader.planes), 1, fp);
   fread(&biheader.bitsPerPxl, sizeof(biheader.bitsPerPxl), 1, fp);
   fread(&biheader.compressionType, sizeof(biheader.compressionType), 1, fp);
   fread(&biheader.imageSize, sizeof(biheader.imageSize), 1, fp);
   fread(&biheader.XpxlsPerMeter, sizeof(biheader.XpxlsPerMeter), 1, fp);
   fread(&biheader.YpxlsPerMeter, sizeof(biheader.YpxlsPerMeter), 1, fp);
   fread(&biheader.colorsUsed, sizeof(biheader.colorsUsed), 1, fp);
   fread(&biheader.colorsImportant, sizeof(biheader.colorsImportant), 1, fp);

   printf("image dimensions: %d x %d (w x h px)\n", biheader.imageWidth, biheader.imageHeight);
   printf("image size: %d bytes\n", biheader.imageSize);
   printf("offset to data: %#01X\n", bfheader.offset);
   fseek(fp, bfheader.offset, SEEK_SET);

   printf("location of fp, AFTER SEEK: %#01lX\n", ftell(fp));

   char image_buffer[biheader.imageSize];
   size_t read = fread(image_buffer, 1, biheader.imageSize, fp);
   if (feof(fp))
      printf("Hit EOF\n");
   printf("amount read: %ld bytes\n", read); 

   printf("--------------------------\n");
   printf("printing raw image BUFFER:\n");
   for (size_t i = 0; i < sizeof(image_buffer); ++i) {
      // printf("%#01X ", image_buffer[i]);
   }
   printf("\n--------------------------\n");

   printf("bytes in x: %d\n", biheader.imageWidth * biheader.bitsPerPxl / 8);
   printf("padding in x: %d\n", 8 % (biheader.imageWidth * (biheader.bitsPerPxl / 8)));
   printf("pixels in y: %d\n", biheader.imageHeight);

   printf("--------------------------\n");
   printf("printing image BUFFER, ignoring padding\n");
   int x_image_bytes = biheader.imageWidth * biheader.bitsPerPxl / 8;
   int x_total_bytes = x_image_bytes - (x_image_bytes % 4); // padding
   for (int y = 0; y < (int)biheader.imageHeight; ++y) {
      for (int x = 0; x < x_image_bytes; ++x) {
         int i = x + (y * x_total_bytes);
         // printf("%08X ", image_buffer[i]);

         // invert pixels
         // image_buffer[i] = 255 - image_buffer[i];
         // if (x == 2) printf("|| ");
      }
      // printf("\n");
   }
   printf("--------------------------\n");

   // write image_buffer to dest file
   printf("writing to dest file: '%s'\n", dest);
   FILE *dest_fp = fopen(dest, "wb");
   if (!dest_fp) return -1;

   // write bfheader and biheader
   fwrite(&bfheader.ftype, sizeof(bfheader.ftype), 1, dest_fp);   
   fwrite(&bfheader.fsize, sizeof(bfheader.fsize), 1, dest_fp);
   fwrite(&bfheader.reserved1, sizeof(bfheader.reserved1), 1, dest_fp);
   fwrite(&bfheader.reserved2, sizeof(bfheader.reserved2), 1, dest_fp);
   fwrite(&bfheader.offset, sizeof(bfheader.offset), 1, dest_fp);

   // NOTE: biheader is properly aligned, so no padding, so just write the whole struct
   fwrite(&biheader, sizeof(biheader), 1, dest_fp);

   // actually write the buffer now
   fseek(dest_fp, bfheader.offset, SEEK_SET);
   size_t bytes_written = fwrite(&image_buffer, 1, biheader.imageSize, dest_fp);
   assert(bytes_written == biheader.imageSize && "fwrite failed somehow");

   fclose(fp);
   fclose(dest_fp);
   return -1;
}

int main(int argc, char *argv[]) {
   int opt;
   opterr = 0;
   char *src = DEFAULT_SRC;
   char *dest = DEFAULT_DEST;

   while ((opt = getopt(argc, argv, OPTSTR)) != EOF)
      switch(opt) {
         case 'i': src = optarg; break;
         case 'o': dest = optarg; break;
         case 'h':
         default: usage(basename(argv[0])); exit(0);
      }

   if (handle_image(src, dest) != 0) {
      perror(ERR_HANDLEIMAGE); // error message is chosen based on value of errno before this
      exit(EXIT_FAILURE);
   }

   return EXIT_SUCCESS;
}

