/*
 * glue.c
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <memory.h>

#include "config.h"
#include "player.h"
#include "fix.h"

#define ABS(x) (((x)>0)?(x):-(x))

/* extern definitions */

extern void     raw_wall(char *);
extern void     lower_case(char *);
extern char    *sys_errlist[];
/* extern int strftime(char *,int,char *,struct tm*); */
extern void     alive_connect();

extern void     close_down_socket(), scan_sockets(), process_players(),
                init_parser(), init_rooms(), init_plist(), init_socket(),
                save_player(), sync_all(), actual_timer(), init_notes(),
                sync_notes(), init_help(), do_update(), timer_function(),
                fork_the_thing_and_sync_the_playerfiles();

extern int      total_processing;
extern int      do_backup();

void            close_down();

/* interns */

char           *tens_words[] = {"", "ten", "twenty", "thirty", "forty", "fifty",
                                "sixty", "seventy", "eighty", "ninety"};

char           *units_words[] = {"none", "one", "two", "three", "four", "five",
                                 "six", "seven", "eight", "nine"};

char           *teens[] = {"ten", "eleven", "twelve", "thirteen", "fourteen",
                           "fifteen", "sixteen", "seventeen", "eighteen",
                           "nineteen"};


char           *months[12] = {"January", "February", "March", "April", "May",
                              "June", "July", "August", "September", "October",
                              "November", "December"};

char shutdown_reason[256] = "";
extern time_t shutdown_count;


/* print up birthday */

char           *birthday_string(time_t bday)
{
   static char     bday_string[50];
   struct tm      *t;
   t = localtime(&bday);
   if ((t->tm_mday) > 10 && (t->tm_mday) < 20)
      sprintf(bday_string, "%dth of %s", t->tm_mday, months[t->tm_mon]);
   else
      switch ((t->tm_mday) % 10)
      {
    case 1:
       sprintf(bday_string, "%dst of %s", t->tm_mday, months[t->tm_mon]);
       break;
    case 2:
       sprintf(bday_string, "%dnd of %s", t->tm_mday, months[t->tm_mon]);
       break;
    case 3:
       sprintf(bday_string, "%drd of %s", t->tm_mday, months[t->tm_mon]);
       break;
    default:
       sprintf(bday_string, "%dth of %s", t->tm_mday, months[t->tm_mon]);
       break;
      }
   return bday_string;
}

/* return a string of the system time */

char           *sys_time()
{
   time_t          t;
   static char     time_string[25];
   t = time(0);
   strftime(time_string, 25, "%H:%M:%S - %d/%m/%y", localtime(&t));
   return time_string;
}

/* returns converted user time */

char           *convert_time(time_t t)
{
   static char     time_string[50];
   strftime(time_string, 49, "%I.%M:%S %p - %a, %d %B", localtime(&t));
   return time_string;
}

/* get local time for all those americans :) */

char           *time_diff(int diff)
{
   time_t          t;
   static char     time_string[50];

   t = time(0) + 3600 * diff;
   strftime(time_string, 49, "%I.%M:%S %p - %a, %d %B", localtime(&t));
   return time_string;
}

char           *time_diff_sec(time_t last_on, int diff)
{
   static char     time_string[50];
   time_t             sec_diff;

   sec_diff = (3600 * diff) + last_on;
   strftime(time_string, 49, "%I.%M:%S %p - %a, %d %B", localtime(&sec_diff));
   return time_string;
}



/* converts time into words */

char           *word_time(int t)
{
   static char     time_string[100], *fill;
   int             days, hrs, mins, secs;
   if (!t)
      return "no time at all";
   days = t / 86400;
   hrs = (t / 3600) % 24;
   mins = (t / 60) % 60;
   secs = t % 60;
   fill = time_string;
   if (days)
   {
      sprintf(fill, "%d day", days);
      while (*fill)
    fill++;
      if (days != 1)
    *fill++ = 's';
      if (hrs || mins || secs)
      {
    *fill++ = ',';
    *fill++ = ' ';
      }
   }
   if (hrs)
   {
      sprintf(fill, "%d hour", hrs);
      while (*fill)
    fill++;
      if (hrs != 1)
    *fill++ = 's';
      if (mins && secs)
      {
    *fill++ = ',';
    *fill++ = ' ';
      }
      if ((mins && !secs) || (!mins && secs))
      {
    strcpy(fill, " and ");
    while (*fill)
       fill++;
      }
   }
   if (mins)
   {
      sprintf(fill, "%d minute", mins);
      while (*fill)
    fill++;
      if (mins != 1)
    *fill++ = 's';
      if (secs)
      {
    strcpy(fill, " and ");
    while (*fill)
       fill++;
      }
   }
   if (secs)
   {
      sprintf(fill, "%d second", secs);
      while (*fill)
    fill++;
      if (secs != 1)
    *fill++ = 's';
   }
   *fill++ = 0;
   return time_string;
}

/* returns a number in words */

char           *number2string(int n)
{
   int             hundreds, tens, units;
   static char     words[50];
   char           *fill;
   if (n >= 1000)
   {
      sprintf(words, "%d", n);
      return words;
   }
   if (!n)
      return "none";
   hundreds = n / 100;
   tens = (n / 10) % 10;
   units = n % 10;
   fill = words;
   if (hundreds)
   {
      sprintf(fill, "%s hundred", units_words[hundreds]);
      while (*fill)
    fill++;
   }
   if (hundreds && (units || tens))
   {
      strcpy(fill, " and ");
      while (*fill)
    fill++;
   }
   if (tens && tens != 1)
   {
      strcpy(fill, tens_words[tens]);
      while (*fill)
    fill++;
   }
   if (tens != 1 && tens && units)
      *fill++ = ' ';
   if (units && tens != 1)
   {
      strcpy(fill, units_words[units]);
      while (*fill)
    fill++;
   }
   if (tens == 1)
   {
      strcpy(fill, teens[(n % 100) - 10]);
      while (*fill)
    fill++;
   }
   *fill++ = 0;
   return words;
}

/* point to after a string */

char           *end_string(char *str)
{
   str = strchr(str, 0);
   str++;
   return str;
}

/* get gender string function */

char           *get_gender_string(player * p)
{
   switch (p->gender)
   {
    case MALE:
    return "him";
    break;
      case FEMALE:
    return "her";
    break;
      case PLURAL:
    return "them";
    break;
      case OTHER:
    return "it";
    break;
      case VOID_GENDER:
    return "it";
    break;
   }
   return "(this is frogged)";
}

/* get gender string for possessives */

char           *gstring_possessive(player * p)
{
   switch (p->gender)
   {
    case MALE:
    return "his";
    break;
      case FEMALE:
    return "her";
    break;
      case PLURAL:
    return "their";
    break;
      case OTHER:
    return "its";
    break;
      case VOID_GENDER:
    return "its";
    break;
   }
   return "(this is frogged)";
}


/* more gender strings */

char           *gstring(player * p)
{
   switch (p->gender)
   {
    case MALE:
    return "he";
    break;
      case FEMALE:
    return "she";
    break;
  case PLURAL:
    return "they";
    break;
      case OTHER:
    return "it";
    break;
      case VOID_GENDER:
    return "it";
    break;
   }
   return "(this is frogged)";
}

char *havehas(player *p)
{
  switch (p->gender)
    {
    case PLURAL:
      return "have";
      break;
    default:
      return "has";
      break;
    }
  return "has";
}
char *isare(player *p)
{
  switch (p->gender)
    {
    case PLURAL:
      return "are";
      break;
    default:
      return "is";
      break;
    }
  return "is";
}

char *waswere(player *p)
{
  switch (p->gender)
    {
    case PLURAL:
      return "were";
      break;
    default:
      return "was";
      break;
    }
  return "was";
}

char *single_s(player *p)
{
    /* for use when you want an s returns for a SINGULAR player */
    switch (p->gender)
    {
      case PLURAL:
	return "";
	break;
      default:
	return "s";
	break;
    }
    return "";
}

/* returns the 'full' name of someone, that is their pretitle and name */

char           *full_name(player * p)
{
   static char     fname[MAX_PRETITLE + MAX_NAME];
   if ((!(sys_flags & NO_PRETITLES)) && (p->residency & BASE) && p->pretitle[0])
   {
      sprintf(fname, "%s %s", p->pretitle, p->name);
      return fname;
   }
   return p->name;
}



/* log errors and things to file */

void            log(char *file, char *string)
{
   int             fd, length;

#ifdef PC
   sprintf(stack, "logs\\%s.log", file);
   fd = open(stack, O_CREAT | O_WRONLY);
#else
   sprintf(stack, "logs/%s.log", file);
   fd = open(stack, O_CREAT | O_WRONLY | O_SYNC, S_IRUSR | S_IWUSR);
#endif
   length = lseek(fd, 0, SEEK_END);
   if (length > MAX_LOG_SIZE)
   {
      close(fd);
#ifdef PC
      fd = open(stack, O_CREAT | O_WRONLY | O_TRUNC);
#else
      fd = open(stack, O_CREAT | O_WRONLY | O_SYNC | O_TRUNC, S_IRUSR | S_IWUSR);
#endif
   }
   sprintf(stack, "%s - %s\n", sys_time(), string);
   if (!(sys_flags & NO_PRINT_LOG))
      printf(stack);
   write(fd, stack, strlen(stack));
   close(fd);
}

/* what happens when *shriek* an error occurs */

void            handle_error(char *error_msg)
{
   char            dump[80];

   /*
    * if (errno==EINTR) { log("error","EINTR trap"); log("error",error_msg);
    * return; }
    */
   if (sys_flags & PANIC)
   {
      stack = stack_start;
      log("error", "Immediate PANIC shutdown.");
      exit(-1);
   }
   sys_flags |= PANIC;

   /*
    * sprintf(dump,"gcore %d",getpid()); system(dump);
    */

   stack = stack_start;

   if ( sys_flags & UPDATE )
      sys_flags &= ~NO_PRINT_LOG;

   log("error", error_msg);
   log("boot", "Abnormal exit from error handler");

   /* dump possible useful info */

   log("dump", "------------ Starting dump");

   sprintf(stack_start, "Errno set to %d, %s", errno, sys_errlist[errno]);
   stack = end_string(stack_start);
   log("dump", stack_start);

   if (current_player)
   {
      log("dump", current_player->name);
      if (current_player->location)
      {
    sprintf(stack_start, "player %s.%s",
       current_player->location->owner->lower_name,
       current_player->location->id);
    stack = end_string(stack_start);
    log("dump", stack_start);
      } else
    log("dump", "No room of current player");

      sprintf(stack_start, "flags %d saved %d residency %d", current_player->flags,
         current_player->saved_flags, current_player->residency);
      stack = end_string(stack_start);
      log("dump", stack_start);
      log("dump", current_player->ibuffer);
   } else
      log("dump", "No current player !");
   if (current_room)
   {
      sprintf(stack_start, "current %s.%s", current_room->owner->lower_name,
         current_room->id);
      stack = end_string(stack_start);
      log("dump", stack_start);
   } else
      log("dump", "No current room");

   sprintf(stack_start, "global flags %d, players %d", sys_flags, current_players);
   stack = end_string(stack_start);
   log("dump", stack_start);

   sprintf(stack_start, "action %s", action);
   stack = end_string(stack_start);
   log("dump", stack_start);

   log("dump", "---------- End of dump info");

   raw_wall("\n\n"
"      -=> *WIBBLE* Something bad has happened. Trying to save files <=-\007\n\n\n");

   close_down();
   exit(-1);
}


/* function to convert seamlessly to caps (ish) */

char           *caps(char *str)
{
   static char     buff[500];
   strncpy(buff, str, 498);
   buff[0] = toupper(buff[0]);
   return buff;
}


/* load a file into memory */

file            load_file_verbose(char *filename, int verbose)
{
   file            f;
   int             d;
   char           *oldstack;

   oldstack = stack;

   d = open(filename, O_RDONLY);
   if (d < 0)
   {
      sprintf(oldstack, "Can't find file:%s", filename);
      stack = end_string(oldstack);
      if (verbose)
    log("error", oldstack);
      f.where = (char *) MALLOC(1);
      *(char *) f.where = 0;
      f.length = 0;
      stack = oldstack;
      return f;
   }
   f.length = lseek(d, 0, SEEK_END);
   lseek(d, 0, SEEK_SET);
   f.where = (char *) MALLOC(f.length + 1);
   memset(f.where, 0, f.length + 1);
   if (read(d, f.where, f.length) < 0)
   {
      sprintf(oldstack, "Error reading file:%s", filename);
      stack = end_string(oldstack);
      log("error", oldstack);
      f.where = (char *) MALLOC(1);
      *(char *) f.where = 0;
      f.length = 0;
      stack = oldstack;
      return f;
   }
   close(d);
   if (sys_flags & VERBOSE)
   {
      sprintf(oldstack, "Loaded file:%s", filename);
      stack = end_string(oldstack);
      log("boot", oldstack);
      stack = oldstack;
   }
   stack = oldstack;
   *(f.where + f.length) = 0;
   return f;
}

file            load_file(char *filename)
{
   return load_file_verbose(filename, 1);
}

int save_file(file *f, char *filename)
{
	char *oldstack;
	FILE *fp;

	if (NULL == (fp = fopen(filename, "w")))
	{
		return 0;
	}

	if (-1 == fwrite(f->where, f->length, 1, fp))
	{
		oldstack = stack;
		sprintf(stack, "Failed to save %s\n", filename);
		log("files", stack);
		stack = oldstack;
	}
	fclose(fp);
}

/* convert a string to lower case */

void            lower_case(char *str)
{
   while (*str)
      *str++ = tolower(*str);
}

/* fns to block signals */

void            sigpipe()
{
   if (c_player)
   {
      log("sigpipe", "Closing connection due to sigpipe");
      shutdown(c_player->fd, 0);
      close(c_player->fd);
   } else
   {
      log("sigpipe", "Eeek! sigpipe but no current_player");
   }
#if !defined(hpux) && !defined(linux)
   signal(SIGPIPE, sigpipe);
#endif /* !hpux && !linux */
   return;
}
void            sighup()
{
   log("boot", "Terminated by hangup signal");
   close_down();
   exit(0);
}
void            sigquit()
{
   handle_error("Quit signal received.");
}
void            sigill()
{
   handle_error("Illegal instruction.");
}
void            sigfpe()
{
   handle_error("Floating Point Error.");
}
void            sigbus()
{
   handle_error("Bus Error.");
}
void            sigsegv()
{
   handle_error("Segmentation Violation.");
}
#if !defined(linux)
void            sigsys()
{
   handle_error("Bad system call.");
}
#endif
void            sigterm()
{
   handle_error("Terminate signal received.");
}
void            sigxfsz()
{
   handle_error("File descriptor limit exceeded.");
}
void            sigusr1()
{
/* dyathink he could have made this a bit longer? */
   fork_the_thing_and_sync_the_playerfiles();
#if !defined(hpux) && !defined(linux)
   signal(SIGUSR1, sigusr1);
#endif /* hpux && linux*/
}

/* Get ALL the files sunc to disk for a backup */
void sigusr2()
{
   backup=1;
#if !defined(hpux) && !defined(linux)
   signal(SIGUSR2, sigusr2);
#endif /* hpux && linux */
}

void            sigchld()
{
/*
   log("error", "WIbble, server's child died");
*/
#if !defined(hpux) && !defined(linux)
   signal(SIGCHLD, sigchld);
#endif /* hpux && linux */
   return;
}

/* close down sequence */

void            close_down()
{
   player         *scan, *old_current;

#ifndef PC
   struct itimerval new, old;
#endif

   raw_wall("\007\n\n");
   command_type |= HIGHLIGHT;
   if (shutdown_count == 0)
   {
      raw_wall(shutdown_reason);
      log("shutdown",shutdown_reason);
   }
   raw_wall("\n\n\n          ---====>>>> Program shutting down NOW <<<<====---"
       "\n\n\n");
   command_type &= ~HIGHLIGHT;

#ifndef PC
   new.it_interval.tv_sec = 0;
   new.it_interval.tv_usec = 0;
   new.it_value.tv_sec = 0;
   new.it_value.tv_usec = new.it_interval.tv_usec;
   if (setitimer(ITIMER_REAL, &new, &old) < 0)
      handle_error("Can't set timer.");
   if (sys_flags & VERBOSE || sys_flags & PANIC)
      log("boot", "Timer Stopped");
#endif

   if (sys_flags & VERBOSE || sys_flags & PANIC)
      log("boot", "Saving all players.");
   for (scan = flatlist_start; scan; scan = scan->flat_next)
      save_player(scan);
   if (sys_flags & VERBOSE || sys_flags & PANIC)
      log("boot", "Syncing to disk.");
   sync_all();

   old_current = current_player;
   current_player = 0;
   sync_notes(0);
   current_player = old_current;

   if (sys_flags & PANIC)
      raw_wall("\n\n              ---====>>>> Files sunc (phew !) <<<<====---"
          "\007\n\n\n");
   for (scan = flatlist_start; scan; scan = scan->flat_next)
      close(scan->fd);

   close_down_socket();

#ifdef PC
   chdir("src");
#endif

   if (!(sys_flags & PANIC))
   {
      unlink("junk/PID");
      log("boot", "Program exited normally.");
      exit(0);
   }
}


/* the boot sequence */

void boot(int port)
{
   char *oldstack;
   int i;
#ifndef PC
   struct rlimit   rlp;
   struct itimerval new, old;
#endif
#if defined(hpux) | defined(linux)
   struct sigaction sa;
#endif /* hpux | linux */

   oldstack = stack;
   log("boot", "-=> EW-three <=- Boot Started");

   up_date = time(0);

#ifndef PC
#ifndef ULTRIX

#if !defined(linux)
   getrlimit(RLIMIT_NOFILE, &rlp);
   rlp.rlim_cur = rlp.rlim_max;
   setrlimit(RLIMIT_NOFILE, &rlp);
#endif /* LINUX */
/*
   max_players = (rlp.rlim_cur) - 20;
*/
   max_players = 210;

   if (sys_flags & VERBOSE)
   {
      sprintf(oldstack, "Got %d file descriptors, Allocated %d for players",
         rlp.rlim_cur, max_players);
      stack = end_string(oldstack);
      log("boot", oldstack);
      stack = oldstack;
   }
#else
   max_players = ULTRIX_PLAYER_LIM;

   if (sys_flags & VERBOSE)
   {
      sprintf(oldstack, "Set max players to %d.", max_players);
      stack = end_string(oldstack);
      log("boot", oldstack);
      stack = oldstack;
   }
#endif /* ULTRIX */

#ifndef SOLARIS
   getrlimit(RLIMIT_RSS, &rlp);
   rlp.rlim_cur = MAX_RES;
   setrlimit(RLIMIT_RSS, &rlp);
#endif

#else
   max_players = 10;
#endif

   flatlist_start = 0;
   for (i = 0; i < 27; i++)
      hashlist[i] = 0;

   stdout_player = (player *) MALLOC(sizeof(player));
   memset(stdout_player, 0, sizeof(player));

   srand(time(0));

   init_plist();
   init_parser();
   init_rooms();
   init_notes();
   init_help();

#ifndef PC
   if (!(sys_flags & SHUTDOWN))
   {
      new.it_interval.tv_sec = 0;
      new.it_interval.tv_usec = (1000000 / TIMER_CLICK);
      new.it_value.tv_sec = 0;
      new.it_value.tv_usec = new.it_interval.tv_usec;
#if defined(hpux) | defined(linux)
      sa.sa_handler = actual_timer;
      sa.sa_mask = 0;
      sa.sa_flags = 0;
      if ((int) sigaction(SIGALRM, &sa, 0) < 0)
#else
      if ((int) signal(SIGALRM, actual_timer) < 0)
#endif /* hpux | linux */
         handle_error("Can't set timer signal.");
      if (setitimer(ITIMER_REAL, &new, &old) < 0)
         handle_error("Can't set timer.");
      if (sys_flags & VERBOSE)
    log("boot", "Timer started.");
   }

#if defined(hpux) | defined(linux)
   sa.sa_handler = sigpipe;
   sa.sa_mask = 0;
   sa.sa_flags = 0;
   sigaction(SIGPIPE, &sa, 0);
   sa.sa_handler = sighup;
   sigaction(SIGHUP, &sa,0);
   sa.sa_handler = sigquit;
   sigaction(SIGQUIT, &sa, 0);
   sa.sa_handler = sigill;
   sigaction(SIGILL, &sa, 0);
   sa.sa_handler = sigfpe;
   sigaction(SIGFPE, &sa, 0);
   sa.sa_handler = sigbus;
   sigaction(SIGBUS, &sa, 0);
   sa.sa_handler = sigsegv;
   sigaction(SIGSEGV, &sa, 0);
#if !defined(linux)
   sa.sa_handler = sigsys;
   sigaction(SIGSYS, &sa, 0);
#endif /* linux */
   sa.sa_handler = sigterm;
   sigaction(SIGTERM, &sa, 0);
   sa.sa_handler = sigxfsz;
   sigaction(SIGXFSZ, &sa, 0);
   sa.sa_handler = sigusr1;
   sigaction(SIGUSR1, &sa, 0);
   sa.sa_handler = sigusr2;
   sigaction(SIGUSR2, &sa, 0);
   sa.sa_handler = sigchld;
   sigaction(SIGCHLD, &sa, 0);
#else
   signal(SIGPIPE, sigpipe);
   signal(SIGHUP, sighup);
   signal(SIGQUIT, sigquit);
   signal(SIGILL, sigill);
   signal(SIGFPE, sigfpe);
   signal(SIGBUS, sigbus);
   signal(SIGSEGV, sigsegv);
   signal(SIGSYS, sigsys);
   signal(SIGTERM, sigterm);
   signal(SIGXFSZ, sigxfsz);
   signal(SIGUSR1, sigusr1);
   signal(SIGUSR2, sigusr2);
   signal(SIGCHLD, sigchld);
#endif /* hpux | linux */
#endif /* PC */

   if (!(sys_flags & SHUTDOWN))
   {
      init_socket(port);
      alive_connect();
   }
   current_players = 0;

   stack = oldstack;
}


/* Log the Process ID of this process to the file junk/PID */

void log_pid(void)
{
   FILE *f;

   f = fopen("junk/PID", "w");
   if (!f)
   {
      fprintf(stderr, "Log_Pid: Couldn't open junk/PID for writing!!!\n");
      exit(-1);
   }
   fprintf(f, "%d", getpid());
   fflush(f);
   fclose(f);
}


/* got to have a main to control everything */

void main(int argc, char *argv[])
{
   int port = 0;

   action = "boot";
   /*
    * if (mallopt(M_MXFAST,1024)) { perror("spoon:"); exit(0); }
    */


#ifdef MALLOC_DEBUG
   malloc_debug(2);
#endif

   backup=0;
   stack_start = (char *) MALLOC(STACK_SIZE);
   memset(stack_start, 0, STACK_SIZE);
   stack = stack_start;

#ifdef TRACK
   funcposition=0;
#endif

   if (argc == 3)
   {
      if (!strcasecmp("update", argv[1]))
      {
         if (!strcasecmp("rooms", argv[2]))
         {
            log("boot", "Program booted for file rooms update.");
            sys_flags |= SHUTDOWN | UPDATEROOMS;
         } else if (!strcasecmp("flags", argv[2]))
         {
            log("boot", "Program booted for flags update");
            sys_flags |= SHUTDOWN | UPDATEFLAGS;
         } else
         {
            log("boot", "Program booted for file players update.");
            sys_flags |= SHUTDOWN | UPDATE;
         }
      }
   }
   if (argc == 2)
      port = atoi(argv[1]);

   if (!port)
      port = DEFAULT_PORT;

   if (chdir(ROOT))
   {
      printf("Can't change to root directory.\n");
      exit(1);
   }
   boot(port);

#ifdef PC
   accept_new_connection();
#endif

   if (sys_flags & UPDATE)
      do_update(0);
   else if (sys_flags & UPDATEFLAGS)
      do_update(0);
   else if (sys_flags & UPDATEROOMS)
      do_update(1);
   sys_flags |= NO_PRINT_LOG;

   if ( !(sys_flags & UPDATE) )
   {
      /*
      fclose(stdout);
      fclose(stderr);
      */
   }

   /* This logs the Process ID of this version of the talker. Used by the 
    * crontab script to send it signals to trigger the backups
    */
   log_pid();

   while (!(sys_flags & SHUTDOWN))
   {
      errno = 0;

      if (backup)
	do_backup();

      if (stack != stack_start)
      {
         sprintf(stack_start, "Lost stack reclaimed %d bytes\n",
                 (int) stack - (int) stack_start);
         stack = end_string(stack_start);
         log("stack", stack_start);
         stack = stack_start;
      }
      action = "scan sockets";
      scan_sockets();
      action = "processing players";
      process_players();
      action = "";

      timer_function();
      sigpause(0);

      do_alive_ping();

   }

   close_down();
}

do_backup()
{
  char fname[256];

  backup=0;

  raw_wall("\n\n -=> Syncing all the files for the daily backups.... <=-"
           "\n -=> Program pausing to save files <=-\n\n");

  dynamic_validate_rooms((player *)0, (char *)0);
  raw_wall(" -=> Part one complete <=-\n");
  dynamic_defrag_rooms((player *)0,(char *)0);
  raw_wall(" -=> Part two complete <=-\n");
  sync_notes(0);
  raw_wall(" -=> Part three complete <=-\n");
  sync_all();
  raw_wall(" -=> Part four complete <=-\n");
  strcpy(fname,ROOT);
  strcat(fname,"backup/daily.backup.pt.1");
  system(fname);
  raw_wall(" -=> Backup complete <=-\n\n\n");
  switch(fork())
    {
    case 0:
      fname[strlen(fname)-1]='2';
      system(fname);
      exit(0);
    default:
      break;
    }
}
