/* hpprint.c 
 *
 * send stdin to printer connected via HP JetDirect using TCP socket
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Sat Jun 04 1994
 */
 
static char *rcsid =
"$Id: hpprint.c,v 1.1 1994/06/04 16:02:50 rommel Exp $";
static char *rcsrev = "$Revision: 1.1 $";

/* $Log: hpprint.c,v $
/* Revision 1.1  1994/06/04 16:02:50  rommel
/* Initial revision
/* */

#define DEFAULTPORT 9100

#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#define ioctl emx_ioctl
#define select emx_select
#include <io.h>
#undef ioctl
#undef select

#include <types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

void usage(void)
{
  printf("\nUsage: hpprint [-h hostname] [-p port]\n"
	 "\nAll standard input is sent to the printer.\n");
  exit(1);
}


int main(int argc, char **argv)
{
  unsigned long host = 0;
  unsigned short port = 0;
  struct hostent *hostnm;
  struct servent *portnm;
  struct sockaddr_in server;
  struct linger linger;
  int print_socket, opt, bytes, sent;
  char buffer[BUFSIZ], *ptr;

  if (argc == 1 && isatty(0))
    usage();

  while ((opt = getopt(argc, argv, "h:p:")) != EOF)
    switch (opt)
    {
    case 'h':
      if ((hostnm = gethostbyname(optarg)) != NULL)
	host = *((unsigned long *)hostnm -> h_addr);
      else if ((host = inet_addr(optarg)) == -1)
	usage();
      break;
    case 'p':
      if ((portnm = getservbyname(optarg, "tcp")) != NULL)
	port = portnm -> s_port;
      else if ((port = atoi(optarg)) == 0)
	usage();
      break;
    default:
      usage();
    }

  if (host == 0)
    usage();

  if (port == 0)
    port = DEFAULTPORT;

  sock_init();

  if ((print_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return psock_errno("socket()"), 2;

  server.sin_family      = AF_INET;
  server.sin_port        = htons(port);
  server.sin_addr.s_addr = host;
  
  if (connect(print_socket, (struct sockaddr *)&server, sizeof(server)) < 0)
    return psock_errno("connect()"), 2;

  linger.l_onoff = 1;
  linger.l_linger = 10;
  
  if (setsockopt(print_socket, SOL_SOCKET, SO_LINGER,
		 (char *) &linger, sizeof(linger)) < 0)
    return psock_errno("setsockopt()"), 2;

  setmode(0, O_BINARY);
  
  while ((bytes = read(0, buffer, sizeof(buffer))) > 0)
    for (ptr = buffer; bytes > 0; bytes -= sent, ptr += sent)
      if ((sent = send(print_socket, buffer, bytes, 0)) < 0)
	return psock_errno("send()"), 2;

  if (soclose(print_socket) < 0)
    return psock_errno("soclose()"), 2;

  return 0;
}

/* end of hpprint.c */
