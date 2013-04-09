/*
 * hd.c -- enhanced hex dump
 * bruce lueckenhoff, 10/28/98
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>


#define LINE_SIZE	16
#define CHUNK		256

struct context {
    unsigned int 	offset;
    int                 ina_dup;
    unsigned char *	buf;
    unsigned char *	old_buf;
};

void print_full_line (unsigned int offset, unsigned char * buf)
{
    printf("%08x"
           " %02x%02x %02x%02x %02x%02x %02x%02x"
           " %02x%02x %02x%02x %02x%02x %02x%02x"
           "   %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n"
           , offset
           , buf[0],  buf[1],  buf[2],  buf[3]
           , buf[4],  buf[5],  buf[6],  buf[7]
           , buf[8],  buf[9],  buf[10], buf[11]
           , buf[12], buf[13], buf[14], buf[15]
           , isprint(buf[0]) ? buf[0] : '.'
           , isprint(buf[1]) ? buf[1] : '.'
           , isprint(buf[2]) ? buf[2] : '.'
           , isprint(buf[3]) ? buf[3] : '.'
           , isprint(buf[4]) ? buf[4] : '.'
           , isprint(buf[5]) ? buf[5] : '.'
           , isprint(buf[6]) ? buf[6] : '.'
           , isprint(buf[7]) ? buf[7] : '.'
           , isprint(buf[8]) ? buf[8] : '.'
           , isprint(buf[9]) ? buf[9] : '.'
           , isprint(buf[10]) ? buf[10] : '.'
           , isprint(buf[11]) ? buf[11] : '.'
           , isprint(buf[12]) ? buf[12] : '.'
           , isprint(buf[13]) ? buf[13] : '.'
           , isprint(buf[14]) ? buf[14] : '.'
           , isprint(buf[15]) ? buf[15] : '.'
          );
}


void print_partial_line (unsigned int offset, unsigned char * buf, int n_r)
{
    int                 i;

    printf("%08x ", offset);
    for (i = 0; i < LINE_SIZE; i++) {
        if (i < n_r) {
            printf("%02x", buf[i]);
        } else {
            fputs("  ", stdout);
        }
        if (1 == (i & 1)) {
            putchar(' ');
        }
    }
    fputs("  ", stdout);
    for (i = 0; i < n_r; i++) {
        if (isprint(buf[i])) {
            putchar(buf[i]);
        } else {
            putchar('.');
        }
    }
    putchar('\n');
}


void dump_full_line (struct context * ctx)
{
    if ((NULL == ctx->old_buf) || memcmp(ctx->old_buf, ctx->buf, LINE_SIZE)) {
	if (1 == ctx->ina_dup) {
//            print_full_line(ctx->offset - LINE_SIZE, ctx->old_buf);
	    ctx->ina_dup = 0;
	}
        print_full_line(ctx->offset, ctx->buf);
    } else {
	if (0 == ctx->ina_dup) {
	    ctx->ina_dup = 1;
            fputs("*\n", stdout);
	}
    }
}


void hexdump_fd (int in_fd)
{
    int		 	n_r;
    int		 	nbytes;
    struct context      ctx;
    unsigned char	bigbuf[CHUNK * LINE_SIZE];

    ctx.offset  = 0;
    ctx.ina_dup = 0;
    while ((n_r = read(in_fd, bigbuf, CHUNK * LINE_SIZE)) > 0) {
        nbytes      = n_r;
	ctx.buf     = bigbuf;
        ctx.old_buf = NULL;
	while (nbytes >= LINE_SIZE) {
	    dump_full_line(&ctx);
            ctx.old_buf = ctx.buf;
            ctx.offset += LINE_SIZE;
	    ctx.buf    += LINE_SIZE;
	    nbytes     -= LINE_SIZE;
        }
    }
    if (nbytes > 0) {
        print_partial_line(ctx.offset, ctx.buf, nbytes);
    }
}


void hexdump_file (char * filename)
{
    int		in_fd;
    
    in_fd = open(filename, O_RDONLY);
    if (in_fd < 0) {
        printf("open failed %s!\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    hexdump_fd(in_fd);
    close(in_fd);
}


int main (int argc, char *argv[])
{
    int		        ix;

    if (2 == argc) {                            /* dump a single file */
        hexdump_file(argv[1]); 
    } else if (argc > 2) {                      /* dump multiple files */
        for (ix = 1; ix < argc; ix++) {
            char *  filename = argv[ix];
            printf("::::::::::::::\n%s\n::::::::::::::\n", filename);
            hexdump_file(filename);
        }
    } else {                                    /* just dump standard input */
	hexdump_fd(0);
    }
    exit(EXIT_SUCCESS);
}
