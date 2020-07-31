/* Copyright (c) 1999, 2001, 2002, 2011, 2013 Thorsten Kukuk
   Author: Thorsten Kukuk <kukuk@suse.de>

   The YP Server is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

   The YP Server is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public
   License along with the YP Server; see the file COPYING. If
   not, write to the Free Software Foundation, Inc., 51 Franklin Street,
   Suite 500, Boston, MA 02110-1335, USA. */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <netdb.h>
#include <rpc/types.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif /* HAVE_ALLOCA_H */
#include <stdlib.h>
#if defined(HAVE_GETOPT_H)
#include <getopt.h>
#endif
#if defined(HAVE_LIBGDBM)
#include <gdbm.h>
#elif defined(HAVE_LIBQDBM)
#include <hovel.h>
#elif defined(HAVE_NDBM)
#include <ndbm.h>
#elif defined(HAVE_LIBTC)
#include <tcbdb.h>
#endif
#include "yp.h"
#include "compat.h"
#include <rpcsvc/ypclnt.h>
#if defined(HAVE_RPC_CLNT_SOC_H)
#include <rpc/clnt_soc.h> /* for clntudp_create() */
#endif /* HAVE_RPC_CLNT_SOC_H */
#include <arpa/nameser.h>
#ifdef HAVE_SHADOW_H
#include <shadow.h>

struct __sgrp {
  char *sg_name;       /* group name */
  char *sg_passwd;     /* group password */
};

#endif

#ifndef YPERR_SUCCESS
#define YPERR_SUCCESS   0
#endif

static struct timeval RPCTIMEOUT = {25, 0};
static struct timeval UDPTIMEOUT = {5, 0};

static int
_yp_maplist (const char *server, char *indomain,
	     struct ypmaplist **outmaplist)
{
  CLIENT *clnt;
  struct ypresp_maplist resp;
  enum clnt_stat result;
  int sock;
  struct sockaddr_in saddr;

  if (indomain == NULL || indomain[0] == '\0')
    return YPERR_BADARGS;

  memset (&resp, '\0', sizeof (resp));
  sock = RPC_ANYSOCK;
  memset (&saddr, 0, sizeof saddr);
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = inet_addr (server);
  if (saddr.sin_addr.s_addr == (in_addr_t) -1)
    {
      struct hostent *hent = gethostbyname (server);
      if (hent == NULL)
	exit (1);
      bcopy ((char *) hent->h_addr_list[0],
	     (char *) &saddr.sin_addr, sizeof saddr.sin_addr);
    }
  clnt = clntudp_create (&saddr, YPPROG, YPVERS, UDPTIMEOUT, &sock);
  if (clnt == NULL)
    exit (1);
  result = clnt_call (clnt, YPPROC_MAPLIST, (xdrproc_t) xdr_domainname,
		      (caddr_t) & indomain, (xdrproc_t) xdr_ypresp_maplist,
		      (caddr_t) & resp, RPCTIMEOUT);

  if (result != YPERR_SUCCESS)
    return result;
  if (resp.stat != YP_TRUE)
    return ypprot_err (resp.stat);

  *outmaplist = resp.maps;
  /* We give the list not free, this will be done by ypserv
     xdr_free((xdrproc_t)xdr_ypresp_maplist, (char *)&resp); */

  return YPERR_SUCCESS;
}

static int
_yp_master (const char *server, char *indomain, char *inmap, char **outname)
{
  CLIENT *clnt;
  int sock;
  struct sockaddr_in saddr;
  ypreq_nokey req;
  ypresp_master resp;
  enum clnt_stat result;

  if (indomain == NULL || indomain[0] == '\0' ||
      inmap == NULL || inmap[0] == '\0')
    return YPERR_BADARGS;

  req.domain = indomain;
  req.map = inmap;

  memset (&resp, '\0', sizeof (ypresp_master));
  memset (&resp, '\0', sizeof (resp));
  sock = RPC_ANYSOCK;
  memset (&saddr, 0, sizeof saddr);
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = inet_addr (server);
  if (saddr.sin_addr.s_addr == (in_addr_t) -1)
    {
      struct hostent *hent = gethostbyname (server);
      if (hent == NULL)
	exit (1);
      bcopy ((char *) hent->h_addr_list[0],
	     (char *) &saddr.sin_addr, sizeof saddr.sin_addr);
    }
  clnt = clntudp_create (&saddr, YPPROG, YPVERS, UDPTIMEOUT, &sock);
  if (clnt == NULL)
    exit (1);
  result = clnt_call (clnt, YPPROC_MASTER, (xdrproc_t) xdr_ypreq_nokey,
		      (caddr_t) & req, (xdrproc_t) xdr_ypresp_master,
		      (caddr_t) & resp, RPCTIMEOUT);

  if (result != YPERR_SUCCESS)
    return result;
  if (resp.stat != YP_TRUE)
    return ypprot_err (resp.stat);

  *outname = strdup (resp.peer);
  xdr_free ((xdrproc_t) xdr_ypresp_master, (char *) &resp);

  return *outname == NULL ? YPERR_YPERR : YPERR_SUCCESS;
}

static void
print_hostname (char *param)
{
  char hostname[MAXHOSTNAMELEN + 1];
#if USE_FQDN
  struct hostent *hp = NULL;
#endif

  if (param == NULL)
    gethostname (hostname, sizeof (hostname));
  else
    {
      strncpy (hostname, param, sizeof (hostname));
      hostname[sizeof (hostname) - 1] = '\0';
    }
#if !USE_FQDN
  fputs (hostname, stdout);
#else
  if (isdigit (hostname[0]))
    {
      char addr[INADDRSZ];
      if (inet_pton (AF_INET, hostname, &addr))
	hp = gethostbyaddr (addr, sizeof (addr), AF_INET);
    }
  else
    hp = gethostbyname (hostname);

  if (hp == NULL)
    fputs (hostname, stdout);
  else
    fputs (hp->h_name, stdout);
#endif
  fputs ("\n", stdout);

  exit (0);
}

/* Show the master for all maps */
static void
print_maps (char *server, char *domain)
{
#if USE_FQDN
  struct hostent *hp = NULL;
#endif
  char *master = NULL, *domainname;
  struct ypmaplist *ypmap = NULL, *y, *old;
  int ret;

  if (domain != NULL)
    domainname = domain;
  else
    if ((ret = yp_get_default_domain (&domainname)) != 0)
      {
	fprintf (stderr, "can't get local yp domain: %s\n",
		 yperr_string (ret));
	exit (1);
      }

#if USE_FQDN
  if (isdigit (server[0]))
    {
      char addr[INADDRSZ];
      if (inet_pton (AF_INET, server, &addr))
	hp = gethostbyaddr (addr, sizeof (addr), AF_INET);
    }
  else
    hp = gethostbyname (server);
  if (hp != NULL)
    {
      server = alloca (strlen (hp->h_name) + 1);
      strcpy (server, hp->h_name);
    }
#endif

  ret = _yp_maplist (server, domainname, &ypmap);
  switch (ret)
    {
    case YPERR_SUCCESS:
      for (y = ypmap; y;)
	{
	  ret = _yp_master (server, domainname, y->map, &master);
	  if (ret == YPERR_SUCCESS)
	    {
	      int is_same = 0;
#if USE_FQDN
	      hp = gethostbyname (master);
	      if (hp != NULL)
		{
		  if (strcasecmp (server, hp->h_name) == 0)
		    is_same = 1;
		}
	      else
#endif
		{
		  if (strcasecmp (server, master) == 0)
		    is_same = 1;
		}
	      if (is_same)
		{
		  fputs (y->map, stdout);
		  fputs ("\n", stdout);
		}
	      free (master);
	    }
	  old = y;
	  y = y->next;
	  free (old);
	}
      break;
    default:
      exit (1);
    }

  exit (0);
}

static void
merge_passwd (char *passwd, char *shadow)
{
  FILE *p_input, *s_input;
  struct passwd *pwd;
  struct spwd *spd;

  p_input = fopen (passwd, "r");
  if (p_input == NULL)
    {
      fprintf (stderr, "yphelper: Cannot open %s\n", passwd);
      exit (1);
    }

  s_input = fopen (shadow, "r");
  if (s_input == NULL)
    {
      fclose (p_input);
      fprintf (stderr, "yphelper: Cannot open %s\n", shadow);
      exit (1);
    }

  while ((pwd = fgetpwent (p_input)) != NULL)
    {
      char *pass;

      if (pwd->pw_name[0] == '-' || pwd->pw_name[0] == '+' ||
	  pwd->pw_name == NULL || pwd->pw_name[0] == '\0')
	continue;

      /* If we found an passwd entry which could have a shadow
	 password, we try the following:
	 At first, try the next entry in the shadow file. If we
	 have luck, the shadow file is sorted in the same order
	 then as the passwd file is. If not, try the whole shadow
	 file. */

      /* XXXXX Some systems and old programs uses '*' as marker
	 for shadow !!! */
      if (pwd->pw_passwd[1] == '\0' &&
	  (pwd->pw_passwd[0] == 'x' || pwd->pw_passwd[0] == '*'))
	{
	  pass = NULL;
#ifdef HAVE_GETSPNAM /* shadow password */
	  spd = fgetspent (s_input);
	  if (spd != NULL)
	    {
	      if (strcmp (pwd->pw_name, spd->sp_namp) == 0)
		pass = spd->sp_pwdp;
	    }
	  if (pass == NULL)
	    {
	      rewind (s_input);
	      while ((spd = fgetspent (s_input)) != NULL)
		{
		  if (strcmp (pwd->pw_name, spd->sp_namp) == 0)
		    {
		      pass = spd->sp_pwdp;
		      break;
		    }
		}
	    }
#endif /* HAVE_GETSPNAM */
	  if (pass == NULL)
	    pass = pwd->pw_passwd;
	}
      else
	pass = pwd->pw_passwd;

      fprintf (stdout, "%s:%s:%d:%d:%s:%s:%s\n",
	       pwd->pw_name, pass, pwd->pw_uid,
	       pwd->pw_gid, pwd->pw_gecos, pwd->pw_dir,
	       pwd->pw_shell);
    }
  fclose (p_input);
  fclose (s_input);

  exit (0);
}

#ifdef HAVE_GETSPNAM /* shadow password */
static struct __sgrp *
fgetsgent (FILE *fp)
{
  static struct __sgrp sgroup;
  static char sgrbuf[BUFSIZ*4];
  char *cp;

  if (! fp)
    return 0;

  if (fgets (sgrbuf, sizeof (sgrbuf), fp) != (char *) 0)
    {
      if ((cp = strchr (sgrbuf, '\n')))
	*cp = '\0';

      sgroup.sg_name = sgrbuf;
      if ((cp = strchr (sgrbuf, ':')))
	*cp++ = '\0';

      if (cp == NULL)
	return 0;

      sgroup.sg_passwd = cp;
      if ((cp = strchr (cp, ':')))
	*cp++ = '\0';

      return &sgroup;
    }
  return 0;
}
#endif /* HAVE_GETSPNAM */

static void
merge_group (char *group, char *gshadow)
{
  FILE *g_input, *s_input;
  struct group *grp;
  struct __sgrp *spd;
  int i;

  g_input = fopen (group, "r");
  if (g_input == NULL)
    {
      fprintf (stderr, "yphelper: Cannot open %s\n", group);
      exit (1);
    }

  s_input = fopen (gshadow, "r");
  if (s_input == NULL)
    {
      fclose (g_input);
      fprintf (stderr, "yphelper: Cannot open %s\n", gshadow);
      exit (1);
    }

  while ((grp = fgetgrent (g_input)) != NULL)
    {
      char *pass;

      if (grp->gr_name[0] == '-' || grp->gr_name[0] == '+' ||
	  grp->gr_name == NULL || grp->gr_name[0] == '\0')
	continue;

      /* If we found an group entry which could have a shadow
	 password, we try the following:
	 At first, try the next entry in the gshadow file. If we
	 have luck, the gshadow file is sorted in the same order
	 then as the group file is. If not, try the whole gshadow
	 file. */
      /* XXXXX Some systems and old programs uses '*' as marker
	 for shadow !!! */
      if (grp->gr_passwd[1] == '\0' &&
	  (grp->gr_passwd[0] == 'x' || grp->gr_passwd[0] == '*'))
	{
	  pass = NULL;
#ifdef HAVE_GETSPNAM /* shadow password */
	  spd = fgetsgent (s_input);
	  if (spd != NULL)
	    {
	      if (strcmp (grp->gr_name, spd->sg_name) == 0)
		pass = spd->sg_passwd;
	    }
	  if (pass == NULL)
	    {
	      rewind (s_input);
	      while ((spd = fgetsgent (s_input)) != NULL)
		{
		  if (strcmp (grp->gr_name, spd->sg_name) == 0)
		    {
		      pass = spd->sg_passwd;
		      break;
		    }
		}
	    }
#endif /* HAVE_GETSPNAM */
	  if (pass == NULL)
	    pass = grp->gr_passwd;
	}
      else
	pass = grp->gr_passwd;

      fprintf (stdout, "%s:%s:%d:", grp->gr_name, pass, grp->gr_gid);
      i =  0;
      while (grp->gr_mem[i] != NULL)
        {
          if (i != 0)
            fprintf (stdout, ",");
          fprintf (stdout, "%s", grp->gr_mem[i]);
          ++i;
        }
      printf ("\n");
    }
  fclose (g_input);
  fclose (s_input);

  exit (0);
}

static char *
get_dbm_entry (char *key, char *map, char *domainname)
{
  static char mappath[MAXPATHLEN + 2];
  char *val;
  datum dkey, dval;
#if defined(HAVE_COMPAT_LIBGDBM)
  GDBM_FILE dbm;
#elif defined (HAVE_NDBM)
  DBM *dbm;
#elif defined (HAVE_LIBTC)
  TCBDB *dbm;
#endif

  if (strlen (YPMAPDIR) + strlen (domainname) + strlen (map) + 3 < MAXPATHLEN)
    sprintf (mappath, "%s/%s/%s", YPMAPDIR, domainname, map);
  else
    {
      fprintf (stderr, "yphelper: path to long: %s/%s/%s\n", YPMAPDIR, domainname, map);
      exit (1);
    }

#if defined(HAVE_COMPAT_LIBGDBM)
  dbm = gdbm_open (mappath, 0, GDBM_READER, 0600, NULL);
#elif defined(HAVE_NDBM)
  dbm = dbm_open (mappath, O_RDONLY, 0600);
#elif defined(HAVE_LIBTC)
  dbm = tcbdbnew();
  if (!tcbdbopen(dbm, mappath, BDBOREADER | BDBONOLCK))
    {
      tcbdbdel(dbm);
      dbm = NULL;
    }
#endif
  if (dbm == NULL)
    {
      fprintf (stderr, "yphelper: cannot open %s\n", mappath);
      fprintf (stderr, "yphelper: consider rebuilding maps using ypinit\n");
      exit (1);
    }

  dkey.dptr = key;
  dkey.dsize = strlen (dkey.dptr);
#if defined(HAVE_COMPAT_LIBGDBM)
  dval = gdbm_fetch (dbm, dkey);
#elif defined(HAVE_NDBM)
  dval = dbm_fetch (dbm, dkey);
#elif defined(HAVE_LIBTC)
  dval.dptr = tcbdbget (dbm, dkey.dptr, dkey.dsize, &dval.dsize);
#endif
  if (dval.dptr == NULL)
    val = NULL;
  else
    {
      val = malloc (dval.dsize + 1);
      strncpy (val, dval.dptr, dval.dsize);
      val[dval.dsize] = 0;
    }
#if defined(HAVE_COMPAT_LIBGDBM)
  gdbm_close (dbm);
#elif defined(HAVE_NDBM)
  dbm_close (dbm);
#elif defined(HAVE_LIBTC)
  tcbdbclose (dbm);
  tcbdbdel (dbm);
#endif
  return val;
}

/* Show the master for all maps */
static void
is_master (char *map, char *domain, char *host)
{
#if USE_FQDN
  struct hostent *hp = NULL;
#endif
  char *hostname, *domainname, *val;
  int ret;

  if (domain != NULL)
    domainname = domain;
  else if ((ret = yp_get_default_domain (&domainname)) != 0)
    {
      fprintf (stderr, "can't get local yp domain: %s\n",
	       yperr_string (ret));
      exit (1);
    }

  if (host)
    hostname = host;
  else
    {
      char h_tmp[MAXHOSTNAMELEN+1];

      if (gethostname (h_tmp, sizeof (h_tmp)) != 0)
	{
	  perror ("gethostname");
	  exit (1);
	}
      hostname = strdup (h_tmp);
    }

#if USE_FQDN
  if (isdigit (hostname[0]))
    {
      char addr[INADDRSZ];
      if (inet_pton (AF_INET, hostname, &addr))
	hp = gethostbyaddr (addr, sizeof (addr), AF_INET);
    }
  else
    hp = gethostbyname (hostname);

  if (hp != NULL)
    hostname = strdup (hp->h_name);
#endif

  if (strcasecmp (hostname,
	  (val = get_dbm_entry ("YP_MASTER_NAME", map, domainname))) == 0)
    ret = 0;
  else
    ret = 1;

  free(hostname);
  exit (ret);
}

static void
Warning (void)
{
  fprintf (stderr, "yphelper: This program is for internal use from some\n");
  fprintf (stderr, "          ypserv scripts and should never be called\n");
  fprintf (stderr, "          from a terminal\n");
  exit (1);
}

int
main (int argc, char *argv[])
{
  int hostname = 0;
  char *master = NULL;
  char *domainname = NULL;
  char *map = NULL;
  int merge_pwd = 0;
  int merge_grp = 0;

  while (1)
    {
      int c;
      int option_index = 0;
      static struct option long_options[] =
	{
	  {"hostname", no_argument, NULL, 'h'},
	  {"version", no_argument, NULL, 'v'},
	  {"maps", required_argument, NULL, 'm'},
	  {"merge_passwd", no_argument, NULL, 'p'},
	  {"merge-passwd", no_argument, NULL, 'p'},
	  {"merge_group", no_argument, NULL, 'g'},
	  {"merge-group", no_argument, NULL, 'g'},
	  {"domainname", required_argument, NULL, 'd'},
	  {"is_master", required_argument, NULL, 'i'},
	  {"is-master", required_argument, NULL, 'i'},
	  {NULL, 0, NULL, '\0'}
	};

      c = getopt_long (argc, argv, "d:hvm:pgi:", long_options, &option_index);
      if (c == EOF)
        break;
      switch (c)
	{
	case 'd':
	  domainname = optarg;
	  break;
	case 'h':
	  ++hostname;
	  break;
	case 'm':
	  master = optarg;
	  break;
	case 'p':
	  merge_pwd = 1;
	  break;
	case 'g':
	  merge_grp = 1;
	  break;
	case 'v':
	  printf ("yphelper (%s) %s", PACKAGE, VERSION);
	  exit (0);
	case 'i':
	  map = optarg;
	  break;
	default:
	  Warning ();
	  return 1;
	}
    }

  argc -= optind;
  argv += optind;

  if (hostname)
    {
      if (argc == 0)
	print_hostname (NULL);
      else
	print_hostname (argv[0]);
    }

  if (master != NULL)
    print_maps (master, domainname);

  if (merge_pwd && argc == 2)
    merge_passwd (argv[0], argv[1]);

  if (merge_grp && argc == 2)
    merge_group (argv[0], argv[1]);

  if (map)
    is_master (map, domainname, NULL);

  Warning ();
  return 1;
}
