/* printmon.c
 *
 * printer redirection monitor
 *
 * Author:  Kai Uwe Rommel <rommel@jonas>
 * Created: Sat Dec 24 1993
 *
 * Compile with MS C 6.00 (cl -G2s -O printmon.c -Lp -F 8000 -link /pm:vio)
 * or with emx+gcc using the Makefile.
 */
 
static char *rcsid =
"$Id: printmon.c,v 1.4 1994/02/25 16:16:47 rommel Exp $";
static char *rcsrev = "$Revision: 1.4 $";

/* $Log: printmon.c,v $
 * Revision 1.4  1994/02/25 16:16:47  rommel
 * changes for emx 0.8h fix 5
 *
 * Revision 1.3  1993/12/29 11:22:01  rommel
 * corrected error messages
 *
 * Revision 1.2  1993/12/25  12:27:53  rommel
 * made emx compatible
 *
 * Revision 1.1  1993/12/24  19:23:23  rommel
 * Initial revision
 *
 * */

#define   INCL_DOSFILEMGR
#define   INCL_DOSMONITORS
#define   INCL_DOSPROCESS
#include <os2.h>

#include <stdio.h>
#include <string.h>
#include <process.h>
#include <malloc.h>

#ifdef M_I86
#define popen _popen
#define pclose _pclose
#endif

#ifdef DEBUG
#define dprintf(f, p) printf(f, p)
#else
#define dprintf(f, p)
#endif

#pragma pack(1)
 
struct buffer
{
  short length;
  char data[158];
};

struct packet
{
  unsigned char mflag;
  unsigned char dflag;
  unsigned short sysfilenum;
  char wdata[128];
};

#pragma pack()


void main(int argc, char **argv)
{
  USHORT usRC, cbData;
  ULONG cbWritten;
  HMONITOR mon;
  struct buffer ibuffer, obuffer;
  struct packet wbuffer;
  char port[16], command[BUFSIZ]; 
  FILE *pipe;

  printf("\nPRINTMON: printer queue redirection device monitor\n");

  if (argc < 3)
  {
    printf("\nUsage:\tPRINTMON <port> <command>\n\n"
	   "\t<port>    = port set up as PostScript (with NO printer attached)\n"
	   "\t<command> = command to pipe the jobs into\n");
    exit(1);
  }

  argc--; argv++;
  strcpy(port, *argv);
  argc--; argv++;

  command[0] = 0;
  while (argc--)
    strcat(strcat(command, *argv++), " ");

  usRC = DosMonOpen(port, &mon);

  if (usRC != 0)
  {
    printf("DosMonOpen error code: %u\n", usRC);
    exit(1);
  }
 
  ibuffer.length = obuffer.length = sizeof(struct buffer);
  usRC = DosMonReg(mon, (PBYTE) &ibuffer, (PBYTE) &obuffer, MONITOR_END, 1);
 
  if (usRC)
  {
    printf("DosMonReg error code: %u\n", usRC);
    exit(1);
  }
 
  while (usRC == 0)
  {
    cbData = sizeof(struct packet);
    usRC = DosMonRead((PBYTE) &ibuffer, 0, (PBYTE) &wbuffer, &cbData);

    if (usRC)
    {
      printf("DosMonRead error code: %u\n", usRC);
      break;
    }

    if (wbuffer.mflag & 1)
    {
      dprintf("monitor: open\n", 0);

      pipe = popen(command, "wb");

      if (pipe == NULL)
      {
	printf("cannot open output pipe\n");
	break;
      }

      cbWritten = 0;
      continue;
    }

    if (wbuffer.mflag & 2)
    {
      dprintf("\nmonitor: close\n", 0);

      pclose(pipe);

      continue;
    }

    if (wbuffer.mflag & 4)
    {
      dprintf("monitor: flush\n", 0);

      usRC = fwrite(wbuffer.wdata, cbData - 4, 1, pipe);

      if (usRC < 1)
      {
	printf("cannot write to output queue\n");
	break;
      }

      continue;
    }

    if (wbuffer.dflag == 32)
    {
      dprintf("monitor: job id '%s'\n", wbuffer.wdata + sizeof(short));
      continue;
    }

    if (wbuffer.dflag == 0)
    {
      cbWritten += cbData - 4;
      dprintf("\rmonitor: %lu data bytes", cbWritten);

      usRC = (fwrite(wbuffer.wdata, cbData - 4, 1, pipe) < 1);

      if (usRC)
      {
	printf("cannot write to output queue\n");
	break;
      }

      continue;
    }
  }
 
  dprintf("monitor: closing\n", 0);

  DosMonClose(mon);
 
  exit(0);
}

/* end of printmon.c */
