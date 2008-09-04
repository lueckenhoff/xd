/*
 * hd.c -- enhanced hex dump
 * bruce lueckenhoff, 10/28/98
 * $Id$
 */

#include <stdio.h>
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

char the_prog_name[80];
void
setprogname (char *nam)
{
    bzero (the_prog_name, sizeof (the_prog_name));
    strncpy (the_prog_name, nam, sizeof(the_prog_name) - 2);
}

const char *
progname (void)
{
    return the_prog_name;
}

int
min (int x, int y)
{
    if (x < y)
	return x;
    else
	return y;
}

void
dump_line (unsigned char *old_buf, unsigned char *buf, int n_r, int *offset,
    int *ina_dup)
{
    int		 	 i;

    if ((*offset > 0) && (0 == bcmp (old_buf, buf, LINE_SIZE)))
    {
	if (0 == *ina_dup)
	{
	    *ina_dup = 1;
	    printf ("*\n");
	}
    }
    else 
    {
	if (1 == *ina_dup)
	{
		*ina_dup = 0;
	}

	{
	    printf ("%06x ", *offset);
	    for (i = 0; i < LINE_SIZE; i++)
	    {
		if (i < n_r)
		{
		    printf ("%02x", buf[i]);
		}
		else
		{
		    printf ("  ");
		}
		if (i % 2)
		{
		    printf (" ");
		}
	    }
	    printf ("   ");
	    for (i = 0; i < n_r; i++)
	    {
		if (isprint (buf[i]))
		{
		    printf ("%c", buf[i]);
		}
		else
		{
		    printf (".");
		}
	    }
	    printf ("\n");
	}
    }
    *offset += n_r;
}

void
hexdump_fd (int in_fd)
{
    int		 	 n_r;
    unsigned char	*buf;
    unsigned char	 old_buf[LINE_SIZE];
    unsigned char	 bigbuf[CHUNK * LINE_SIZE];
    int		 	 offset;
    int		 	 ina_dup;

    offset = 0;
    ina_dup = 0;

    while ((n_r = read (in_fd, bigbuf, CHUNK * LINE_SIZE)) > 0)
    {
	buf = bigbuf;
	while (n_r > 0)
	{
	    dump_line (old_buf, buf, min (LINE_SIZE, n_r), &offset, &ina_dup);
	    bcopy (buf, old_buf, LINE_SIZE);
	    buf += LINE_SIZE;
	    n_r -= LINE_SIZE;
	}
    }
}

int
hexdump_file (char *filename)
{
    int		in_fd;

    in_fd = open (filename, O_RDONLY);
    if (-1 == in_fd)
    {
	char tmpstr[80];
	sprintf (tmpstr, "%s: cannot open %s", progname (), filename);
	perror (tmpstr);
	return -1;
    }
    hexdump_fd (in_fd);
    close (in_fd);
    return 0;
}

int
main (int argc, char *argv[])
{
    int		 i;

    if (1 == argc)
    {
	hexdump_fd (0);
	return 0;
    }

    setprogname ("hd");
    for (i = 1; i < argc; i++)
    {
	char	*filename = argv[i];

	if (argc > 2)
	{
	    printf ("::::::::::::::\n%s\n::::::::::::::\n", filename);
	}
	if (0 != hexdump_file (filename))
	{
	    return -1;
	}
    }
    return 0;
}
