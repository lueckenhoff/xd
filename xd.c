/*
 * hd.c -- enhanced hex dump
 * bruce lueckenhoff, 10/28/98
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>


#define LINE_SIZE	16
#define CHUNK		256

char printable_repr[256];       /* lookup table of printable representations */

void init_lookup_table (void)
{
    int             ix;

    for (ix = 0; ix < 256; ix++) {
        printable_repr[ix] = isprint((ix)) ? ix : '.';
    }
}


void print_full_line (unsigned int offset, unsigned char * buf)
{
    printf("%07x:"
           " %02x%02x %02x%02x %02x%02x %02x%02x"
           " %02x%02x %02x%02x %02x%02x %02x%02x"
           "  %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n"
           , offset
           , buf[0],  buf[1],  buf[2],  buf[3]
           , buf[4],  buf[5],  buf[6],  buf[7]
           , buf[8],  buf[9],  buf[10], buf[11]
           , buf[12], buf[13], buf[14], buf[15]
           , printable_repr[buf[0]]
           , printable_repr[buf[1]]
           , printable_repr[buf[2]]
           , printable_repr[buf[3]]
           , printable_repr[buf[4]]
           , printable_repr[buf[5]]
           , printable_repr[buf[6]]
           , printable_repr[buf[7]]
           , printable_repr[buf[8]]
           , printable_repr[buf[9]]
           , printable_repr[buf[10]]
           , printable_repr[buf[11]]
           , printable_repr[buf[12]]
           , printable_repr[buf[13]]
           , printable_repr[buf[14]]
           , printable_repr[buf[15]]
          );
}


void print_line (unsigned int offset, unsigned char * buf, int howmany)
{
    int                 ix;

    printf("%07x: ", offset);
    for (ix = 0; ix < LINE_SIZE; ix++) {
        if (ix < howmany) {
            printf("%02x", buf[ix]);
        } else {
            fputs("  ", stdout);
        }
        if (1 == (ix & 1)) {
            putchar(' ');
        }
    }
    putchar(' ');
    for (ix = 0; ix < howmany; ix++) {
        putchar(printable_repr[buf[ix]]);
    }
    putchar('\n');
}


/* things to always do when leaving a run of duplicate lines... */
#define LEAVING_DUPLICATE_RUN                           \
if (ina_dup) {                                          \
    if (offset - dup_start_offset > LINE_SIZE) {        \
        fputs("*\n", stdout);                           \
    }                                                   \
    print_full_line(offset - LINE_SIZE, old_line);      \
    ina_dup = 0;                                        \
}

void hexdump_fd (int in_fd)
{
    int		 	nbytes;
    unsigned int        dup_start_offset;
    int                 ina_dup;
    unsigned int 	offset;
    unsigned char *	line;
    unsigned char  	old_line[LINE_SIZE];
    unsigned char	bigbuf[CHUNK * LINE_SIZE];

    offset  = 0;
    ina_dup = 0;
    dup_start_offset = 0;
    while ((nbytes = read(in_fd, bigbuf, CHUNK * LINE_SIZE)) > 0) {
	line = bigbuf;
	while (nbytes >= LINE_SIZE) {
            if (0 == offset) {
                /* special-case the very first line, by forcing a mismatch */
                old_line[0] = line[0] + 1;
            }
            if (0 != memcmp(old_line, line, LINE_SIZE)) {   /* different line */
                LEAVING_DUPLICATE_RUN
                print_full_line(offset, line);
                /* Remember line for next time */
                /* NB: next time *might* span across successive read() calls */
                memcpy(old_line, line, LINE_SIZE);             
            } else {                                        /* duplicate line */
                if (0 == ina_dup) {
                    /* Entering a run of duplicate lines */
                    dup_start_offset = offset;
                    ina_dup = 1;
                }
            }
            offset += LINE_SIZE;
	    line   += LINE_SIZE;
	    nbytes -= LINE_SIZE;
        }
        if (nbytes > 0) {
            LEAVING_DUPLICATE_RUN
            print_line(offset, line, nbytes);
        }
    }
    LEAVING_DUPLICATE_RUN
}


void hexdump_file (char * filename)
{
    int		        in_fd;
    int		        rval;
    struct stat         statbuf;
   
    in_fd = open(filename, O_RDONLY);
    if (in_fd < 0) {
        fprintf(stderr, "%s: %s\n", filename, strerror(errno));
        return;
    }
    rval = fstat(in_fd, &statbuf);
    if ((0 == rval) && (S_ISDIR(statbuf.st_mode))) {
        fprintf(stderr, "%s is a directory\n", filename);
        close(in_fd);
        return;
    }
    hexdump_fd(in_fd);
    close(in_fd);
}


int main (int argc, char *argv[])
{
    int		        ix;
    char *              filename;

    init_lookup_table();
    if (1 == argc) {                            /* dump standard input */
	hexdump_fd(0);
        exit(EXIT_SUCCESS);
    }

    for (ix = 1; ix < argc; ix++) {             /* dump one or more files */
        filename = argv[ix];
        if (argc > 2) {
            printf("::::::::::::::\n%s\n::::::::::::::\n", filename);
        }
        hexdump_file(filename);
    }
    exit(EXIT_SUCCESS);
}
