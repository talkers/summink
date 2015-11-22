/*
 * parse.c
 */

#include <ctype.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>

#include "config.h"
#include "player.h"
#include "fix.h"
#include "clist.h"

/* externs */

extern char    *end_string();
extern int      nhash_update[], update[];
extern file     load_file(), load_file_verbose(char *, int);
extern void     sync_notes();
extern void     log(char *, char *);
extern char    *full_name(player *);
extern void     tell_player(player *, char *);
extern void     tell_room(room *, char *);
extern void     save_player(player *);
extern void     do_inform(player *, char *);
extern void     destroy_player(player *);
extern void     do_prompt(player *, char *);
extern void     sync_to_file(char, int);
extern void     handle_error(char *);
extern char    *convert_time(time_t);
extern void     move_to(player *, char *, int);
extern player  *find_player_global(char *);
extern char    shutdown_reason[];

/* interns */

struct command *last_com;
char           *stack_check;
int             nsync = 0, synct = 0, sync_counter = 0, note_sync = NOTE_SYNC_TIME;
int     mem_use_log_count = 0;
int             account_wobble = 1;
int             performance_timer = 0;
struct command *help_list = 0;
file            help_file = {0, 0};
time_t          shutdown_count = -1;

/* returns the first char of the input buffer */

char           *first_char(player * p)
{
   char           *scan;
   scan = p->ibuffer;
   while (*scan && isspace(*scan))
      *scan++;
   return scan;
}

/* what happens if bad stack detected */

void            bad_stack()
{
   int             missing;
   missing = (int) stack - (int) stack_check;
   if (last_com)
      sprintf(stack_check, "Bad stack in function %s, missing %d bytes",
         last_com->text, missing);
   else
      sprintf(stack_check, "Bad stack somewhere, missing %d bytes", missing);
   stack = end_string(stack_check);
   log("stack", stack_check);
   stack = stack_check;
}


/* flag changing routines */


/* returns the value of a flag from the flag list */

int             get_flag(flag_list * list, char *str)
{
   for (; list->text; list++)
      if (!strcmp(list->text, str))
    return list->change;
   return 0;
}


/* routine to get the next part of an arg */

char           *next_space(char *str)
{
   while (*str && *str != ' ')
      str++;
   if (*str == ' ')
   {
      while (*str == ' ')
    str++;
      str--;
   }
   return str;
}


/* view command lists */

void view_commands(player * p, char *str)
{
   struct command *comlist;
   char *oldstack;
   int s;
   player *p2;

   oldstack = stack;
   if (*str && p->residency & ADMIN)
   {
      p2 = find_player_global(str);
      if (!p2)
      {
         return;
      }
      sprintf(stack, " Commands available to %s ...\n", p2->name);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
   } else
   {
      strcpy(stack, " Commands available to you ...\n");
      stack = strchr(stack, 0);
      p2 = p;
   }

   for (s = 0; s < 27; s++)
   {
      for (comlist = coms[s]; comlist->text; comlist++)
      {
         if ((!comlist->level || ((p2->residency) & comlist->level)) &&
	     (!comlist->andlevel || ((p2->residency) & comlist->andlevel)))
         {
            sprintf(stack, "%s, ", comlist->text);
            stack = strchr(stack, 0);
         }
      }
   }
   stack -= 2;
   *stack++ = '\n';
   *stack++ = 0;
   tell_player(p, oldstack);
   stack = oldstack;
}


void            view_sub_commands(player * p, struct command * comlist)
{
   char           *oldstack;
   oldstack = stack;

   strcpy(stack, " Available sub commands ...\n");
   stack = strchr(stack, 0);

   for (; comlist->text; comlist++)
      if (((!comlist->level) || ((p->residency) & (comlist->level))) &&
	  ((!comlist->andlevel) || ((p->residency) & (comlist->andlevel))))
      {
    sprintf(stack, "%s, ", comlist->text);
    stack = strchr(stack, 0);
      }
   stack -= 2;
   *stack++ = '\n';
   *stack++ = 0;
   tell_player(p, oldstack);
   stack = oldstack;
}

/* initialise the hash array */

void            init_parser()
{
   int             i;
   struct command *scan;
   scan = complete_list;
   for (i = 0; i < 27; i++)
   {
      coms[i] = scan;
      while (scan->text)
    scan++;
      scan++;
   }
}

/* see if any commands fit the bill */

char           *do_match(char *str, struct command * com_entry)
{
   char           *t;
   for (t = com_entry->text; *t; t++, str++)
      if (tolower(*str) != *t)
    return 0;
   if ((com_entry->space) && (*str) && (!isspace(*str)))
      return 0;
   while (*str && isspace(*str))
      str++;
   return str;
}

/* execute a function from a sub command list */

void            sub_command(player * p, char *str, struct command * comlist)
{
   char           *oldstack, *rol;
   void            (*fn) ();
   oldstack = stack;

   while (comlist->text)
   {
      if (((!comlist->level) || ((p->residency) & (comlist->level))) &&
	  ((!comlist->andlevel) || ((p->residency) & (comlist->andlevel))))
      {
    rol = do_match(str, comlist);
    if (rol)
    {
       last_com = comlist;
       stack_check = stack;
       fn = comlist->function;
       (*fn) (p, rol);
       if (stack != stack_check)
          bad_stack();
       sys_flags &= ~(FAILED_COMMAND | PIPE | ROOM_TAG | FRIEND_TAG | EVERYONE_TAG);
       command_type = 0;
       return;
    }
      }
      comlist++;
   }
   rol = str;
   while (*rol && !isspace(*rol))
      rol++;
   *rol = 0;
   sprintf(oldstack, " Cannot find sub command '%s'\n", str);
   stack = end_string(oldstack);
   tell_player(p, oldstack);
   stack = oldstack;
}


/* match commands to the main command lists */

void            match_commands(player * p, char *str)
{
   struct command *comlist;
   char           *rol, *oldstack, *space;
   void            (*fn) ();
   oldstack = stack;

   while (*str && *str == ' ')
      str++;
   space = strchr(str, 0);
   space--;
   while (*space == ' ')
      *space-- = 0;
   if (!*str)
      return;
   if (isalpha(*str))
      comlist = coms[((int) (tolower(*str)) - (int) 'a' + 1)];
   else
      comlist = coms[0];

   while (comlist->text)
   {
      if (((!comlist->level) || ((p->residency) & (comlist->level))) &&
	  ((!comlist->andlevel) || ((p->residency) & (comlist->andlevel))))
      {
    rol = do_match(str, comlist);
    if (rol)
    {
       last_com = comlist;
       stack_check = stack;
       fn = comlist->function;
       (*fn) (p, rol);
       if (stack != stack_check)
          bad_stack();
       sys_flags &= ~(FAILED_COMMAND | PIPE | ROOM_TAG | FRIEND_TAG | EVERYONE_TAG);
       command_type = 0;
       return;
    }
      }
      comlist++;
   }
   p->antipipe++;

/*
   if (p->antipipe > 30)
   {
      quit(p, 0);
      return;
   }
*/
   rol = str;
   while (*rol && !isspace(*rol))
      rol++;
   *rol = 0;
   sprintf(oldstack, " Cannot find command '%s'\n", str);
   stack = end_string(oldstack);
   tell_player(p, oldstack);
   stack = oldstack;
}

/* handle input from one player */

void            input_for_one(player * p)
{
   char           *pick;
   void            (*fn) ();

   if (p->input_to_fn &&
			!(current_room == prison && !(p->residency & (ADMIN | SU))))
   {
      p->idle = 0;
      p->idle_msg[0] = 0;
      last_com = &input_to;
      stack_check = stack;
      fn = p->input_to_fn;
      (*fn) (p, p->ibuffer);
      if (stack != stack_check)
         bad_stack();
      sys_flags &= ~(FAILED_COMMAND | PIPE | ROOM_TAG | FRIEND_TAG
                     | EVERYONE_TAG);
      command_type = 0;
      return;
   }
   if (!p->ibuffer[0])
      return;
   p->idle = 0;
   p->idle_msg[0] = 0;
   action = "doing command";
   if (p->ibuffer[0] != '#')
   {
      if (p->saved_flags & CONVERSE)
      {
         pick = p->ibuffer;
         while (*pick && isspace(*pick))
            pick++;
         if (*pick)
            if (*pick == '/')
               if (current_room == prison &&
                   !(p->residency & (ADMIN | SU)))
                  sub_command(p, pick + 1, restricted_list);
               else
                  match_commands(p, pick + 1);
            else
               say(p, pick);
      } else if (current_room == prison && !(p->residency & (ADMIN | SU)))
         sub_command(p, p->ibuffer, restricted_list);
      else
         match_commands(p, p->ibuffer);
   }
}


/* scan through the players and see if anything needs doing */

void            process_players()
{
   player         *scan;
   char           *oldstack;

   for (scan = flatlist_start; scan; scan = scan->flat_next)
   {
     if (scan->flat_next)
       if (((player *)scan->flat_next)->flat_previous != scan)
	 {
	   raw_wall("\n\n   -=> Non-terminated flatlist <=-\n\n");
	   raw_wall("\n\n   -=> Dumping end off of list <=-\n\n");
	   scan->flat_next=NULL;
	 }

      if ((scan->fd < 0) || (scan->flags & PANIC) ||
          (scan->flags & CHUCKOUT))
      {

         oldstack = stack;
         current_player = scan;

         if (scan->location && scan->name[0] && !(scan->flags & RECONNECTION))
         {
            sprintf(stack, " %s suddenly dissolve%s into a squintillion dots"
                           " that quickly disperse.\n",
                    scan->name, single_s(scan));
            stack = end_string(stack);
            tell_room(scan->location, oldstack);
            stack = oldstack;
            save_player(scan);
         }
         if (!(scan->flags & RECONNECTION))
         {
            command_type = 0;
	    if (scan->gender==PLURAL)
	      do_inform(scan, "[%s have disconnected] %s");
	    else
	      do_inform(scan, "[%s has disconnected] %s");
            if (scan->saved && !(scan->flags & NO_SAVE_LAST_ON))
               scan->saved->last_on = time(0);
         }
         if (sys_flags & VERBOSE || scan->residency == 0)
         {
            if (scan->name[0])
               sprintf(oldstack, "%s has disconnected from %s", scan->name,
                       scan->inet_addr);
            else
               sprintf(oldstack, "Disconnect from login. [%s]",
                       scan->inet_addr);
            stack = end_string(oldstack);
            log("newconn", oldstack);
         }
         destroy_player(scan);
         current_player = 0;
         stack = oldstack;
      } else if (scan->flags & INPUT_READY)
      {
/* there used to be this here...
            if (!(scan->lagged) && !(scan->flags & PERM_LAG))
   for reference... */

         if (!(scan->lagged))
         {
            current_player = scan;
            current_room = scan->location;
            input_for_one(scan);
            action = "processing players";
            current_player = 0;
            current_room = 0;

#ifdef PC
            if (scan->flags & PROMPT && scan == input_player)
#else
            if (scan->flags & PROMPT)
#endif
            {
               if (scan->saved_flags & CONVERSE)
                  do_prompt(scan, scan->converse_prompt);
               else
                  do_prompt(scan, scan->prompt);
            }
         }
         memset(scan->ibuffer, 0, IBUFFER_LENGTH);
         scan->flags &= ~INPUT_READY;
      }
   }
}




/* timer things */


/* automessages */

void            do_automessage(room * r)
{
   int             count = 0, type;
   char           *scan, *oldstack;
   oldstack = stack;
   scan = r->automessage.where;
   if (!scan)
   {
      r->flags &= ~AUTO_MESSAGE;
      return;
   }
   for (; *scan; scan++)
      if (*scan == '\n')
    count++;
   if (!count)
   {
      FREE(r->automessage.where);
      r->automessage.where = 0;
      r->automessage.length = 0;
      r->flags &= ~AUTO_MESSAGE;
      stack = oldstack;
      return;
   }
   count = rand() % count;
   for (scan = r->automessage.where; count; count--, scan++)
      while (*scan != '\n')
    scan++;
   while (*scan != '\n')
      *stack++ = *scan++;
   *stack++ = '\n';
   *stack++ = 0;
   type = command_type;
   command_type = AUTO;
   tell_room(r, oldstack);
   command_type = type;
   r->auto_count = r->auto_base + (rand() & 63);
   stack = oldstack;
}


/* file syncing */

void            do_sync()
{
   int             origin;
   action = "doing sync";
   sync_counter = SYNC_TIME;
   origin = synct;
   while (!update[synct])
   {
      synct = (synct + 1) % 26;
      if (synct == origin)
    break;
   }
   if (update[synct])
   {
      sync_to_file(synct + 'a', 0);
      synct = (synct + 1) % 26;
   }
}

/* this is the actual timer pling */

void            actual_timer()
{
   static int      pling = TIMER_CLICK;
   player         *scan, *wibble;
   time_t t;

   if (sys_flags & PANIC)
      return;

#if !defined(hpux) && !defined(linux)
   if ((int) signal(SIGALRM, actual_timer) < 0)
      handle_error("Can't set timer signal.");
#endif /* hpux */

   t = time(0);
   if ((splat_timeout - t) <= 0)
      splat1 = splat2 = 0;
   pling--;
   if (pling)
      return;

   pling = TIMER_CLICK;

   sys_flags |= DO_TIMER;

   if (mem_use_log_count > 0)
      mem_use_log_count--;
   if (shutdown_count > 0)
      shutdown_count--;

   for (scan = flatlist_start; scan; scan = scan->flat_next)
      if (!(scan->flags & PANIC))
      {
    scan->idle++;
    scan->total_login++;
    if (scan->script && scan->script > 1)
       scan->script--;
    if (scan->timer_fn && scan->timer_count > 0)
       scan->timer_count--;
    if (scan->no_shout > 0)
       scan->no_shout--;
    if (scan->no_move > 0)
       scan->no_move--;
    if (scan->lagged > 0)
       scan->lagged--;
    if (scan->shout_index > 0)
       scan->shout_index--;

    if (scan->jail_timeout > 0)
       scan->jail_timeout--;
      }
   net_count--;
   if (!net_count)
   {
      net_count = 10;
      in_total += in_current;
      out_total += out_current;
      in_pack_total += in_pack_current;
      out_pack_total += out_pack_current;
      in_bps = in_current / 10;
      out_bps = out_current / 10;
      in_pps = in_pack_current / 10;
      out_pps = out_pack_current / 10;
      in_average = (in_average + in_bps) >> 1;
      out_average = (out_average + out_bps) >> 1;
      in_pack_average = (in_pack_average + in_pps) >> 1;
      out_pack_average = (out_pack_average + out_pps) >> 1;
      in_current = 0;
      out_current = 0;
      in_pack_current = 0;
      out_pack_current = 0;
   }
}


/* the timer function */

void            timer_function()
{
   player         *scan, *old_current;
   void            (*fn) ();
   room           *r, **list;
   char           *oldstack, *text;
   int             count = 0, pcount = 0;
   char           *action_cpy;
   struct tm      *ts;
   time_t          t;
#if !defined(linux)
   struct mallinfo minfo;
#else
   struct mstats memstats;
#endif /* LINUX */

   if (!(sys_flags & DO_TIMER))
      return;
   sys_flags &= ~DO_TIMER;

   waitpid((pid_t) - 1, (int *) 0, WNOHANG);
   /* wait3(0,WNOHANG,0); */

   old_current = current_player;
   action_cpy = action;

   oldstack = stack;

   if (mem_use_log_count == 0)
   {
#if !defined(linux)
      minfo = mallinfo();
      sprintf(stack, "Total arena space - %d", minfo.arena);
#else
      memstats = mstats();
      sprintf(stack, "Total heap size - %d", memstats.bytes_total);
#endif /* LINUX */
      stack = end_string(stack);
      log("mem", oldstack);
      stack = oldstack;
      mem_use_log_count = 60;
   }

   if (shutdown_count > -1)
   {
      command_type |= HIGHLIGHT;
      switch (shutdown_count)
      {
      case 31536000:
	raw_wall("\n\n-=> We'll be shutting down this time next year <=-\n"
		 "-=> Anyone who is still on at this time wins a "
		 "prize <=-\n\n");
	break;
      case 86400:
	raw_wall("\n\n-=> We'll be shutting down this time tomorrow - "
		 "Helpful arent we! <=-\n\n");
	break;
      case 3600:
	 raw_wall("\n\n-=> We'll be rebooting in 1 hour, "
		  "pity the admin that sets it to such a "
		  "stupid time. <=-\n\n");
	break;
      case 900:
	raw_wall("\n\n-=> We'll be rebooting in 15 mins, "
		 "You can 5 eggs in that time! <=-\n\n");
	break;
      case 300:
	raw_wall("\n\n-=> We'll be rebooting in 5 mins, "
		 "go put the kettle on now <=-\n\n");
	break;
      case 180:
	raw_wall("\n\n-=> 3 minutes to go until the reboot, "
		 "time to make the tea <=-\n\n");
	break;
      case 60:
	raw_wall("\n\n-=> 1 minute left to the reboot, "
		 "time to say bye to people <=-\n\n");
	break;
      case 30:
	raw_wall("\n\n-=> 30 seconds until the reboot, "
		 "hold on to your drinks <=-\n\n");
	break;
      case 15:
	raw_wall("\n\n-=> 15 seconds to go to reboot, "
		 "see you in a little while! <=-\n\n");
	break;
      case 10:
	raw_wall("\n\n-=> 10 seconds to go to reboot, "
		 "The end is nigh! <=-\n\n");
	break;
      case 5:
	raw_wall("\n\n-=> 5 seconds to go to reboot, "
		 "The end is nigh(er)! <=-\n\n");
	break;
      case 1:
	raw_wall("\n\n-=> 1 second to go to reboot, "
		 "The end is REALLY nigh! <=-\n\n");
	break;
      case 0:
	log("shutdown", shutdown_reason);
	sys_flags |= SHUTDOWN;
	stack = oldstack;
	return;
      }
      command_type &= ~HIGHLIGHT;
    }

   if (sync_counter)
      sync_counter--;
   else
      do_sync();

   if (note_sync)
      note_sync--;
   else
   {
      note_sync = NOTE_SYNC_TIME;
      sync_notes(1);
   }

   align(stack);
   list = (room **) stack;

   for (scan = flatlist_start; scan; scan = scan->flat_next)
   {
      if (!(scan->flags & PANIC))
      {
         if (scan->script && scan->script == 1)
         {
            text = stack;
            sprintf(text, " Time is now %s.\n"
                          " Scripting stopped ...\n", convert_time(time(0)));
            stack = end_string(text);
            tell_player(scan, text);
            stack = text;
            scan->script = 0;
         }
         if (scan->timer_fn && !scan->timer_count)
         {
            current_player = scan;
            fn = scan->timer_fn;
            (*fn) (scan);
            scan->timer_fn = 0;
            scan->timer_count = -1;
         }
         current_player = old_current;
         action = "processing autos";
         r = scan->location;
         if (r)
         {
            pcount++;
            if (r->flags & AUTO_MESSAGE && !(r->flags & AUTOS_TAG))
            {
               if (!r->auto_count)
                  do_automessage(r);
               else
                  r->auto_count--;
               *(room **) stack = r;
               stack += sizeof(room *);
               count++;
               r->flags |= AUTOS_TAG;
            }
         }
/* Jail timeout thang */
         if (scan->jail_timeout == 0 && scan->location == prison)
         {
            command_type |= HIGHLIGHT;
            tell_player(scan, " After serving your sentence you are flung out"
                              " to society again.\n");
            command_type &= ~HIGHLIGHT;
            move_to(scan, ENTRANCE_ROOM, 0);
         }
      }
   }
   for (; count; count--, list++)
      (*list)->flags &= ~AUTOS_TAG;
   stack = oldstack;
   action = action_cpy;
   current_players = pcount;

   t = time(0);
   ts = localtime(&t);

   /*
    * if (!account_wobble && (ts->tm_hour)==0) { account_wobble=1;
    * do_birthdays(); }
    */
   if (account_wobble == 1 && ((ts->tm_hour) == 3))
   {
      account_wobble = 2;
      /* system("bin/account &"); */
   }
   if (account_wobble == 2 && ((ts->tm_hour) > 3))
      account_wobble = 1;
}

/* the help system (aargh argh argh) */


/* look through all possible places to find a bit of help */

struct command *find_help(char *str)
{
   struct command *comlist;
   if (isalpha(*str))
      comlist = coms[((int) (tolower(*str)) - (int) 'a' + 1)];
   else
      comlist = coms[0];

   for (; comlist->text; comlist++)
      if (do_match(str, comlist))
    return comlist;
   comlist = help_list;
   if (!comlist)
      return 0;
   for (; comlist->text; comlist++)
      if (do_match(str, comlist))
    return comlist;

   return 0;
}


void            next_line(file * hf)
{
   while (hf->length > 0 && *(hf->where) != '\n')
   {
      hf->where++;
      hf->length--;
   }
   if (hf->length > 0)
   {
      hf->where++;
      hf->length--;
   }
}

void            init_help()
{
   file            hf;
   struct command *found, *hstart;
   char           *oldstack, *start, *scan;
   int             length;
   oldstack = stack;


   if (sys_flags & VERBOSE)
      log("boot", "Loading help pages");


   if (help_list)
      FREE(help_list);
   help_list = 0;

   if (help_file.where)
      FREE(help_file.where);
   help_file = load_file("doc/help");
   hf = help_file;

   align(stack);
   hstart = (struct command *) stack;

   while (hf.length > 0)
   {
      while (hf.length > 0 && *(hf.where) != ':')
    next_line(&hf);
      if (hf.length > 0)
      {
    scan = hf.where;
    next_line(&hf);
    *scan++ = 0;
    while (scan != hf.where)
    {
       start = scan;
       while (*scan != ',' && *scan != '\n')
          scan++;
       *scan++ = 0;
       found = find_help(start);
       if (!found)
       {
          found = (struct command *) stack;
          stack += sizeof(struct command);
          found->text = start;
          found->function = 0;
          found->level = 0;
          found->andlevel = 0;
          found->space = 1;
       }
       found->help = hf.where;
    }
      }
   }
   *(hf.where - 1) = 0;
   found = (struct command *) stack;
   stack += sizeof(struct command);
   found->text = 0;
   found->function = 0;
   found->level = 0;
   found->andlevel = 0;
   found->space = 0;
   found->help = 0;
   length = (int) stack - (int) hstart;
   help_list = (struct command *) MALLOC(length);
   memcpy(help_list, hstart, length);
   stack = oldstack;
}


/* load that help file in  */

int             get_help(player * p, char *str)
{
   int             fail = 0;
   file            text;
   char           *oldstack;

   oldstack = stack;
   if (*str == '.')
      return 0;

   sprintf(stack, "doc/%s.help", str);
   stack = end_string(stack);
   text = load_file_verbose(oldstack, 0);
   if (text.where)
   {
      if (*(text.where))
      {
    stack = oldstack;
    sprintf(stack, "-- Help ----------------------------------------"
       "---------------------\n%s-----------------------------"
       "----------------------------------------\n", text.where);
    stack = end_string(stack);
    pager(p, oldstack, 1);
    fail = 1;
      } else
    fail = 0;
      free(text.where);
   }
   stack = oldstack;
   return fail;
}



int             get_victim(player * p, char *text)
{
   return 0;
}


/* the help command */

void help(player * p, char *str)
{
   char *oldstack;
   struct command *fn;
   oldstack = stack;

   if (!*str)
   {
      if (p->residency)
         str = "general";
      else
         str = "newbie";
   }
   fn = find_help(str);
   if (!fn || !(fn->help))
   {
      if (get_help(p, str))
         return;
      sprintf(stack, " Cannot find any help on the subject of '%s'\n", str);
      stack = end_string(stack);
      if (p->saved_flags & NO_PAGER)
         tell_player(p, oldstack);
      else
         pager(p, oldstack, 0);
      stack = oldstack;
      return;
   }
   if (!strcasecmp(str, "newbie"))
      if (get_victim(p, fn->help))
      {
         stack = oldstack;
         return;
      }
   sprintf(stack, "-- Help ---------------------------------------------"
           "----------------\n%s---------------------------------------"
           "------------------------------\n", fn->help);
   stack = end_string(stack);
   if (p->saved_flags & NO_PAGER)
      tell_player(p, oldstack);
   else
      pager(p, oldstack, 0);
   stack = oldstack;
}
