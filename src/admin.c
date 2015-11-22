/*
 * admin.c
 */

#include <malloc.h>
#include <string.h>
#include <memory.h>
#include <time.h>

#include "config.h"
#include "player.h"

/* externs */
extern void         swap_list_names(char *, char *);
extern void         lower_case(char *);
extern char         *do_crypt(char *, player *);
extern char         *end_string(), *next_space(), *tag_string(), *bit_string();
extern player       *find_player_global(), *find_player_global_quiet(char *),
                    *create_player();
extern saved_player *find_saved_player();
extern int          remove_player_file(), set_update();
extern int          get_flag();
extern void         hard_load_one_file(), sync_to_file(), remove_entire_list(),
                    destroy_player();
extern player       *find_player_absolute_quiet(char *);
extern file         newban_msg, nonewbies_msg, connect_msg, motd_msg,
                    banned_msg, banish_file, banish_msg, full_msg, newbie_msg,
                    newpage1_msg, newpage2_msg, disclaimer_msg, splat_msg,
                    load_file(char *), load_file_verbose(char *, int);
extern int          match_banish();
extern void         soft_eject(player *, char *);
extern player       *find_player_absolute_quiet(char *);
extern char         *self_string(player *p);
extern void         all_players_out(saved_player *);
extern note         *find_note(int);
extern char          shutdown_reason[];
extern time_t        shutdown_count;
extern room         *comfy;
#ifdef TRACK
extern int addfunction(char *);
#endif

/* interns */

flag_list       permission_list[] = {
   {"residency", BASE | BUILD | LIST | ECHO_PRIV | MAIL | SESSION},
   {"nosync", NO_SYNC},
   {"base", BASE},
   {"echo", ECHO_PRIV},
   {"no_timeout", NO_TIMEOUT},
   {"banished", BANISHD},
   {"sysroom", SYSTEM_ROOM},
   {"mail", MAIL},
   {"list", LIST},
   {"build", BUILD},
   {"session", SESSION},
   {"su_channel", PSU},
   {"warn", WARN},
   {"frog", FROG},
   {"script", SCRIPT},
   {"trace", TRACE},
   {"house", HOUSE},
   {"hcadmin", HCADMIN},
   {"lower_admin", LOWER_ADMIN},
   {"su", SU | PSU | WARN},
   {"admin", ADMIN | SU | PSU | WARN},
{0, 0}};


#if !defined(linux)
/* malloc data */

void show_malloc(player * p, char *str)
{
   char *oldstack;
   struct mallinfo i;

#ifdef TRACK
   sprintf(functionin,"show_malloc(%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   i = mallinfo();

   sprintf(stack, "Total arena space\t%d\n"
                  "Ordinary blocks\t\t%d\n"
                  "Small blocks\t\t%d\n"
                  "Holding blocks\t\t%d\n"
                  "Space in headers\t\t%d\n"
                  "Small block use\t\t%d\n"
                  "Small blocks free\t%d\n"
                  "Ordinary block use\t%d\n"
                  "Ordinary block free\t%d\n"
#if defined( ULTRIX ) || defined( SOLARIS )
                  "Keep cost\t\t%d\n",
#else
                  "Keep cost\t\t%d\n"
                  "Small block size\t\t%d\n"
                  "Small blocks in holding\t%d\n"
                  "Rounding factor\t\t%d\n"
                  "Ordinary block space\t%d\n"
                  "Ordinary blocks alloc\t%d\n"
                  "Tree overhead\t%d\n",
#endif
                  i.arena, i.ordblks, i.smblks, i.hblks, i.hblkhd, i.usmblks,
#if defined( ULTRIX ) || defined ( SOLARIS )
                  i.fsmblks, i.uordblks, i.fordblks, i.keepcost);
#else
                  i.fsmblks, i.uordblks, i.fordblks, i.keepcost,
                  i.mxfast, i.nlblks, i.grain, i.uordbytes, i.allocated,
                  i.treeoverhead);
#endif
   stack = end_string(stack);
   tell_player(p, oldstack);
   stack = oldstack;
}

#else /* The LINUX version of this... */

void show_malloc(player *p, char *str)
{
   char *oldstack;
   struct mstats memstats;

   memstats = mstats();
   oldstack = stack;

   sprintf(stack, " Memory statistics:-\n"
                  "  Total Heap Size:                %8d\n"
                  "  Chunks Allocated:               %8d\n"
                  "  Byte Total of Chunks:           %8d\n"
                  "  Chunks in Free List:            %8d\n"
                  "  Byte Total of Free List Chunks: %8d\n",
           memstats.bytes_total, memstats.chunks_used, memstats.bytes_used,
           memstats.chunks_free, memstats.bytes_free);
   stack = end_string(stack);
   tell_player(p, oldstack);
   stack = oldstack;
}

#endif /* LINUX */

/* view logs */

void            vlog(player * p, char *str)
{
   char *oldstack;
   file logb;

#ifdef TRACK
   sprintf(functionin,"vlog(%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;

   switch (*str)
   {
      case 0:
      case '?':
         tell_player(p, " Log files you can view: angel, banish, blanks, boot,"
		     " bug, connection, drag, dump, duty, error, grant, help,"
		     " lag, mem, newconn, nuke, resident, rm_move, rm_shout, session, sigpipe, site, sneeze,"
		     " stack, sync, timeouts, validate_email, warn\n");
         return;
      case '.':
         tell_player(p, " Uh-uh, you can't do that !\n");
         return;
   }
   sprintf(stack, "logs/%s.log", str);
   stack = end_string(stack);
   logb = load_file_verbose(oldstack, 0);
   if (logb.where)
   {
      if (*(logb.where))
         pager(p, logb.where, 1);
      else
      {
         sprintf(oldstack, " Couldn't find logfile 'logs/%s.log'\n", str);
         stack = end_string(oldstack);
         tell_player(p, oldstack);
      }
      free(logb.where);
   }
   stack = oldstack;
}


/* net stats */

void netstat(player * p, char *str)
{
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"netstat(%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   sprintf(stack, "Total bytes:\t\t(I) %d\t(O) %d\n"
                  "Average bytes:\t\t(I) %d\t\t(O) %d\n"
                  "Bytes per second:\t(I) %d\t\t(O) %d\n"
                  "Total packets:\t\t(I) %d\t(O) %d\n"
                  "Average packets:\t(I) %d\t\t(O) %d\n"
                  "Packets per second:\t(I) %d\t\t(O) %d\n",
                  in_total, out_total, in_average, out_average, in_bps, out_bps,
                  in_pack_total, out_pack_total, in_pack_average,
                  out_pack_average, in_pps, out_pps);
   stack = end_string(stack);
   tell_player(p, oldstack);
   stack = oldstack;
}


/* warn someone */

void warn(player * p, char *str)
{
   char *oldstack, *msg, *pstring, *final;
   player **list, **step;
   int i,n, old_com, self = 0;

#ifdef TRACK
   sprintf(functionin,"show_malloc(%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif
   
   oldstack = stack;
   align(stack);
   command_type = PERSONAL | SEE_ERROR | WARNING;

   if (p->saved_flags & BLOCK_TELLS)
   {
      tell_player(p, " You are currently BLOCKING TELLS. It might be an idea to"
                     " unblock so they can reply, eh?\n");
   }
   msg = next_space(str);
   if (*msg)
      *msg++ = 0;
   if (!*msg)
   {
      tell_player(p, " Format: warn <player(s)> <message>\n");
      stack = oldstack;
      return;
   }

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," You cannot warn when off_duty\n");
       stack=oldstack;
       return;
     }

   /* no warns to groups */
   if (!strcasecmp(str, "everyone") || !strcasecmp(str, "friends")
       || !strcasecmp(str, "supers") || !strcasecmp(str, "sus")
       || strstr(str, "everyone"))
   {
      tell_player(p, " Now that would be a bit silly wouldn't it?\n");
      stack = oldstack;
      return;
   }
   /* should you require warning, the consequences are somewhat severe */
   if (!strcasecmp(str, "me"))
   {
      tell_player(p, " You Silly Sod\n\nBye!\n");
      stack = oldstack;
      quit(p, 0);
      return;
   }
   list = (player **) stack;
   n = global_tag(p, str);
   if (!n)
   {
      stack = oldstack;
      return;
   }
   final = stack;
   if (p->gender==PLURAL)
     sprintf(stack, "-=> %s warn you: %s\n\n", p->name, msg);
   else
     sprintf(stack, "-=> %s warns you: %s\n\n", p->name, msg);
   stack = end_string(stack);
   for (step = list, i = 0; i < n; i++, step++)
   {
      if (*step != p)
      {
         command_type |= HIGHLIGHT;
         tell_player(*step, "\a\n");
         tell_player(*step, final);
         command_type &= ~HIGHLIGHT;
      }
   }
   stack = final;

   pstring = tag_string(p, list, n);
   final = stack;
   if (p->gender==PLURAL)
     sprintf(stack, "-=> %s warn %s: %s", p->name, pstring, msg);
   else
     sprintf(stack, "-=> %s warns %s: %s", p->name, pstring, msg);
   stack = end_string(stack);
   log("warn", final);
   strcat(final, "\n");
   stack = end_string(final);
   command_type = 0;
   su_wall(final); 

   cleanup_tag(list, n);
   stack = oldstack;
}


/* trace someone and check against email */

void trace(player * p, char *str)
{
   char *oldstack;
   player *p2, dummy;

#ifdef TRACK
   sprintf(functionin,"trace (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: trace <person>\n");
      return;
   }
   p2 = find_player_absolute_quiet(str);
   if (!p2)
   {
      sprintf(stack, " \'%s\' not logged on, checking saved files...\n",
              str);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      strcpy(dummy.lower_name, str);
      lower_case(dummy.lower_name);
      dummy.fd = p->fd;
      if (!load_player(&dummy))
      {
         tell_player(p, " Not found.\n");
         return;
      }
      if (dummy.residency == BANISHD)
      {
         tell_player(p, " That is a banished name.\n");
          return;
      }
      if ( dummy.email[0] )
      {
         if ( dummy.email[0] == -1 )
         {
            sprintf(stack, " %s has declared no email address.\n", dummy.name);
            stack = strchr(stack, 0);
         } else if ( p->residency & ADMIN )
         {
            sprintf(stack, " %s [%s]\n", dummy.name, dummy.email);
            if (dummy.saved_flags & PRIVATE_EMAIL)
            {
               while (*stack != '\n')
                  stack++;
               strcpy(stack, " (private)\n");
            }
            stack = strchr(stack, 0);
         }
      }
      sprintf(stack, " %s last connected from %s\n   and disconnected at ",
               dummy.name, dummy.saved->last_host);
      stack = strchr(stack, 0);
      if (p->jetlag)
         sprintf(stack, "%s\n", convert_time(dummy.saved->last_on
                                                 + (p->jetlag * 3600)));
      else
         sprintf(stack, "%s\n", convert_time(dummy.saved->last_on));
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      return;
   }

   if (p2->residency == NON_RESIDENT)
   {
      sprintf(stack, " %s is non resident.\n", p2->name);
      stack = strchr(stack, 0);
   }
   else if (p2->email[0])
   {
      if (p2->email[0] == -1)
      {
         sprintf(stack, " %s has declared no email address.\n", p2->name);
         stack = strchr(stack, 0);
      } else if ( p->residency & ADMIN )
      {
         sprintf(stack, " %s [%s]\n", p2->name, p2->email);
         if (p2->saved_flags & PRIVATE_EMAIL)
         {
            while (*stack != '\n')
               stack++;
            strcpy(stack, " (private)\n");
         }
         stack = strchr(stack, 0);
      }
   } else
   {
      sprintf(stack, " %s has not set an email address.\n", p2->name);
      stack = strchr(stack, 0);
   }
   sprintf(stack, " %s is connected from %s.\n", p2->name, p2->inet_addr);
   stack =end_string(stack);
   tell_player(p, oldstack);
   stack = oldstack;
}


/* list people who are from the same site */

void same_site(player * p, char *str)
{
   char *oldstack, *text;
   player *p2;

#ifdef TRACK
   sprintf(functionin,"same_site (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (isalpha(*str))
   {
      if (!strcasecmp(str, "me"))
      {
         p2 = p;
      } else
      {
         p2 = find_player_global(str);
      }
      if (!p2)
      {
         stack = oldstack;
         return;
      }
      str = stack;
      text = p2->num_addr;
      while (isdigit(*text))
         *stack++ = *text++;
      *stack++ = '.';
      *text++;
      while (isdigit(*text))
         *stack++ = *text++;
      *stack++ = '.';
      *stack++ = '*';
      *stack++ = '.';
      *stack++ = '*';
      *stack++ = 0;
   }
   if (!isdigit(*str))
   {
      tell_player(p, " Format: site <inet_number> or site <person>\n");
      stack = oldstack;
      return;
   }
   text = stack;
   sprintf(stack, "People from .. %s\n", str);
   stack = strchr(stack, 0);
   for (p2 = flatlist_start; p2; p2 = p2->flat_next)
   {
      if (match_banish(p2, str))
      {
         sprintf(stack, "(%s) %s : %s ", p2->num_addr, p2->inet_addr, p2->name);
         stack = strchr(stack, 0);
         if (p2->residency == NON_RESIDENT)
         {
            strcpy(stack, "non resident.\n");
            stack = strchr(stack, 0);
         } else if (p2->email[0])
         {
            if (p2->email[0] == -1)
               strcpy(stack, "No email address.");
            else
            {
               if (((p2->saved_flags & PRIVATE_EMAIL) &&
                                        (p->residency & ADMIN))
                    || !(p2->saved_flags & PRIVATE_EMAIL))
               {
                 
                  sprintf(stack, "[%s]", p2->email);
                  stack = strchr(stack, 0);
               }
               if (p2->saved_flags & PRIVATE_EMAIL)
               {
                  strcpy(stack, " (private)");
                  stack = strchr(stack, 0);
               }
            }
            *stack++ = '\n';
         } else
         {
            strcpy(stack, "Email not set\n");
            stack = strchr(stack, 0);
         }
      }
   }
   *stack++ = 0;
   tell_player(p, text);
   stack = oldstack;
}


/* crash ! */

void crash(player * p, char *str)
{
   char *flop = 0;
   char *oldstack;

   oldstack=stack;

#ifdef TRACK
   sprintf(functionin,"crash (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," Go back on duty first.\n");
       return;
     }

   sprintf(oldstack,"Crash used by %s",p->name);
   stack=end_string(oldstack);
   log("shutdown",oldstack);
   stack=oldstack;

   *flop = -1;
}


/* reload everything */

void reload(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"reload (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   tell_player(p, " Loading help\n");
   init_help();
   tell_player(p, " Loading messages\n");
   if (newban_msg.where)
      FREE(newban_msg.where);
   if (nonewbies_msg.where)
      FREE(nonewbies_msg.where);
   if (connect_msg.where)
      FREE(connect_msg.where);
   if (motd_msg.where)
      FREE(motd_msg.where);
   if (banned_msg.where)
      FREE(banned_msg.where);
   if (banish_file.where)
      FREE(banish_file.where);
   if (banish_msg.where)
      FREE(banish_msg.where);
   if (full_msg.where)
      FREE(full_msg.where);
   if (newbie_msg.where)
      FREE(newbie_msg.where);
   if (newpage1_msg.where)
      FREE(newpage1_msg.where);
   if (newpage2_msg.where)
      FREE(newpage2_msg.where);
   if (disclaimer_msg.where)
      FREE(disclaimer_msg.where);
   if (splat_msg.where)
      FREE(splat_msg.where);
#ifdef PC
   newban_msg = load_file("files\\newban.msg");
   nonewbies_msg = load_file("files\\nonew.msg");
   connect_msg = load_file("files\\connect.msg");
   motd_msg = load_file("files\\motd.msg");
   banned_msg = load_file("files\\banned.msg");
#else
   newban_msg = load_file("files/newban.msg");
   nonewbies_msg = load_file("files/nonew.msg");
   connect_msg = load_file("files/connect.msg");
   motd_msg = load_file("files/motd.msg");
   banned_msg = load_file("files/banned.msg");
#endif
   banish_file = load_file("files/banish");
   banish_msg = load_file("files/banish.msg");
   full_msg = load_file("files/full.msg");
   newbie_msg = load_file("files/newbie.msg");
   newpage1_msg = load_file("files/newpage1.msg");
   newpage2_msg = load_file("files/newpage2.msg");
   disclaimer_msg = load_file("files/disclaimer.msg");
   splat_msg = load_file("files/splat.msg");
   tell_player(p, " Done\n");
}


/* edit the banish file from the program */

void quit_banish_edit(player * p)
{
#ifdef TRACK
   sprintf(functionin,"quit_banish_edit (%s)",p->name);
   addfunction(functionin);
#endif

   tell_player(p, " Leaving without changes.\n");
}

void end_banish_edit(player * p)
{
#ifdef TRACK
   sprintf(functionin,"end_banish_edit (%s)",p->name);
   addfunction(functionin);
#endif

   if (banish_file.where)
      FREE(banish_file.where);
   banish_file.length = p->edit_info->size;
   banish_file.where = (char *) MALLOC(banish_file.length);
   memcpy(banish_file.where, p->edit_info->buffer, banish_file.length);
   tell_player(p, " Banish file changed.\n");
	if (save_file(&banish_file, "files/banish"))
	{
		tell_player(p, " ALSO saved to disk *8-)\n");
	}
}

void            banish_edit(player * p, char *str)
{
   start_edit(p, 10000, end_banish_edit, quit_banish_edit, banish_file.where);
}


/* the eject command , muhahahahaa */

void sneeze(player * p, char *str)
{
   time_t t;
   int nologin = 0;
   char *oldstack, *text, *num;
   player*e;

#ifdef TRACK
   sprintf(functionin,"sneeze (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: sneeze <person/s> [<time in minutes>]\n");
      return;
   }

   if (p->flags & BLOCK_SU)
     {
       tell_player (p," You need to be on_duty to eject people\n");
       return;
     }

   t = time(0);
   if (num = strrchr(str, ' '))
      nologin = atoi(num) * 60;
   if (nologin > (60 * 10) && !(p->residency & ADMIN))
   {
      tell_player(p, " That amount of time is too harsh, set to 10 mins now "
                     "...\n");
      nologin = (60 * 10);
   }
   if (!nologin)
      nologin = 60;
   else
      *num = 0;
   while (*str)
   {
      while (*str && *str != ',')
         *stack++ = *str++;
      if (*str)
         str++;
      *stack++ = 0;
      if (*oldstack)
      {
         e = find_player_global(oldstack);
         if (e)
         {
            text = stack;
            if (e->residency >= p->residency)
            {
               tell_player(p, " No way pal !!!\n");
               sprintf(stack, " -=> %s tried to sneeze over you.\n", p->name);
               stack = end_string(stack);
               tell_player(e, text);
               stack = text;
               sprintf(stack, "%s failed to sneeze all over %s", p->name,
                       e->name);
               stack = end_string(stack);
               log("sneeze", text);
               stack = text;
            } else
            {
               strcpy(stack, "\n\n One of the Super Users unclogs their nose in"
                             " your direction. You die a horrible green death."
                             "\n\n");
               stack = end_string(stack);
               tell_player(e, text);
               e->sneezed = t + nologin;
               stack = text;
               quit(e, 0);
               sprintf(stack, " -=> %s %s sneezed upon, and not at all "
                              "happy about it.\n", e->name, isare(e));
               stack = end_string(stack);
               tell_room(e->location, text);
               stack = text;
	       if (p->gender==PLURAL)
		 sprintf(stack, " -=> All of the %s group together, "
			 "handkerchieves at the ready, and as one they all "
			 "sneeze on %s.\n -=> %s was from "
			 "%s\n",p->name, e->name, e->name, e->inet_addr);
	       else
		 sprintf(stack, " -=> %s sneezes on %s.\n -=> %s was from "
			 "%s\n",p->name, e->name, e->name, e->inet_addr);
		 
               stack = end_string(stack);
               su_wall(text);
               stack = text;
               sprintf(text, "%s sneezes on %s [%s]", p->name, e->name,
                       e->inet_addr);
               stack = end_string(text);
               log("sneeze", text);
               stack = text;
               sync_to_file(*(e->lower_name), 0);
            }
         }
      }
      stack = oldstack;
   }
}


/*
 * reset person (in case the su over does it (which wouldn't be like an su at
 * all.. nope no no))
 */

void reset_sneeze(player * p, char *str)
{
   char *oldstack, *newtime;
   time_t t, nologin;
   player dummy;

#ifdef TRACK
   sprintf(functionin,"reset_sneeze (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: reset_sneeze <person>"
                     " [<new time in minutes>]\n");
      return;
   }

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," Go back on duty first.\n");
       return;
     }

   newtime = next_space(str);
   if (*newtime)
   {
      t = time(0);
      *newtime++ = 0;
      nologin = atoi(newtime) * 60;
      if (nologin > (60 * 10) && !( p->residency & ADMIN))
      {
         tell_player(p, " Now that isn't a very nice amount of time is it?\n"
                        " Reset to 10 minutes...\n");
         nologin = 60 * 10;
      }
      nologin += t;
   } else
   {
      nologin = 0;
   }
   memset(&dummy, 0, sizeof(player));
   strcpy(dummy.lower_name, str);
   lower_case(dummy.lower_name);
   dummy.fd = p->fd;
   if (!load_player(&dummy))
   {
      tell_player(p, " No such person in saved files.\n");
      return;
   }
   switch (dummy.residency)
   {
      case SYSTEM_ROOM:
         tell_player(p, " That's a system room.\n");
         return;
      default:
         if (dummy.residency & BANISHD)
         {
            if (dummy.residency == BANISHD)
               tell_player(p, " That Name is banished.\n");
            else
               tell_player(p, " That Player is banished.\n");
            return;
         }
         break;
   }
   dummy.sneezed = nologin;
   dummy.location = (room *) - 1;
   save_player(&dummy);
   if (!nologin)
   {
      sprintf(stack, " Reset the sneeze time on %s ...\n", dummy.name);
   } else
   {
      sprintf(stack, " Changed the Sneeze time on %s to %d seconds.\n",
              dummy.name, nologin - t);
   }
   stack = end_string(stack);
   tell_player(p, oldstack);
   stack = oldstack;

   /* tell the SUs, too */
   if (!nologin)
   {
       sprintf(stack, " -=> %s reset%s the sneeze time on %s ...\n", 
	       p->name,single_s(p), dummy.name);
   } else
   {
       sprintf(stack, " -=> %s change%s the Sneeze time on %s to %d "
	       "seconds.\n", p->name, single_s(p), dummy.name, nologin - t);
   }
   stack = end_string(stack);
   su_wall_but(p, oldstack);
   log("sneeze",oldstack);

   stack = oldstack;
}


/* SPLAT!!!! Wibble plink, if I do say so myself */

void soft_splat(player * p, char *str)
{
   char *oldstack, *reason;
   player *dummy;
   int no1, no2, no3, no4;

#ifdef TRACK
   sprintf(functionin,"soft_splat (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (!(reason = strchr(str, ' ')))
   {
      tell_player(p, " Format: splat <person> <reason>\n");
      return;
   }

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," You need to be on_duty to splat anyone.\n");
       return;
     }

   *reason++ = 0;
   dummy = find_player_global(str);
   if (!dummy)
      return;
   sprintf(stack, "%s SPLAT: %s", str, reason);
   stack = end_string(stack);
   soft_eject(p, oldstack);
   *reason = ' ';
   stack = oldstack;
   if (!(dummy->flags & CHUCKOUT))
      return;
   soft_timeout = time(0) + (5 * 60);
   sscanf(dummy->num_addr, "%d.%d.%d.%d", &no1, &no2, &no3, &no4);
   soft_splat1 = no1;
   soft_splat2 = no2;
   sprintf(stack, "-=> Site %d.%d.*.* banned to newbies for 5 minutes.\n",
           no1, no2);
   stack = end_string(stack);
   su_wall(oldstack);
   stack = oldstack;
}


void splat_player(player * p, char *str)
{
   time_t t;
   char *oldstack, *space;
   player *dummy;
   int no1, no2, no3, no4, tme = 0;

   tme=0;

   oldstack = stack;
   if (!(p->residency & (SU | ADMIN)))
   {
      soft_splat(p, str);
      return;
   }
   if (!*str)
   {
       tell_player(p, " Format: splat <person> <time>\n");
       return;
   }
   if (space = strchr(str, ' '))
   {
       *space++ = 0;
       tme = atoi(space);
   }
   if (((p->residency & SU && !(p->residency & ADMIN)) && (tme < 0 || tme > 10)) ||
       (p->residency & ADMIN && (tme < 0)))
     {
       tell_player(p, " That's not a very nice amount of time.  Set to 10 "
		   "minutes ...\n");
       tme = 10;
     }
   else
     {
       /* when no time specified */
       if (!tme)
	 {
	   tell_player(p, "Time set to 5 minutes.\n");
	   tme = 5;
	 }
     }

   dummy = find_player_global(str);
   if (!dummy)
       return;
   sneeze(p, dummy->lower_name);
   if (!(dummy->flags & CHUCKOUT))
       return;
   t = time(0);
   splat_timeout = t + (tme * 60);
   sscanf(dummy->num_addr, "%d.%d.%d.%d", &no1, &no2, &no3, &no4);
   splat1 = no1;
   splat2 = no2;
   sprintf(stack, "-=> Site %d.%d.*.* banned for %d minutes because of %s\n",
           no1, no2, tme, dummy->name);
   stack = end_string(stack);
   su_wall(oldstack);
   stack = oldstack;
}

void unsplat(player * p, char *str)
{
   char *oldstack, *spc;
   time_t t;
   int number = -1;

#ifdef TRACK
   sprintf(functionin,"unsplat (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player (p," You need to be on_duty to do that.\n");
       return;
     }

   oldstack = stack;
   t = time(0);
   if (*str)
   {
      spc = strchr(str, ' ');
      if (spc)
      {
         *spc++;
         number = atoi(spc);
      } else
      {
         number = 0;
      }
   }
   if (!*str || number < 0)
   {
      number = splat_timeout - (int) t;
      if (number <= 0)
      {
         tell_player(p, " No site banned atm.\n");
         return;
      }
      sprintf(stack, " Site %d.%d.*.* is banned for %d more seconds.\n",
              splat1, splat2, number);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      return;
   }
   if (splat1 == 0 && splat2 == 0)
   {
      tell_player(p, " No site banned atm.\n");
      return;
   }
   if (number == 0)
   {
      sprintf(stack, "-=> %s unbans site %d.%d.*.*\n", p->name, splat1, splat2);
      splat_timeout = (int) t;
      stack = end_string(stack);
      su_wall(oldstack);
      stack = oldstack;
      return;
   }
   if (number > 600)
   {
      tell_player(p, " That's not a very nice time,"
                     " resetting to 10 minutes...\n");
      number = 600;
   }
   sprintf(stack, "-=> %s changes the ban on site %d.%d.*.*"
                  " to a further %d seconds.\n",
           p->name, splat1, splat2, number);
   splat_timeout = (int) t + number;
   stack = end_string(stack);
   su_wall(oldstack);
   stack = oldstack;
}


/* the eject command (too) , muhahahahaa */

void soft_eject(player * p, char *str)
{
   char *oldstack, *text, *reason;
   player *e;

#ifdef TRACK
   sprintf(functionin,"tell_player (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   reason = next_space(str);
   if (*reason)
      *reason++ = 0;
   if (!*reason)
   {
      tell_player(p, " Format: drag <person> <reason>\n");
      return;
   }

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," Its best to be on_duty to do that kind of thing.\n");
       return;
     }

   e = find_player_global(str);
   if (e)
   {
      text = stack;
      if (e->residency >= p->residency)
      {
         tell_player(p, " Sorry, you can't...\n");
         sprintf(stack, "%s tried to drag %s", p->name, e->name);
         stack = end_string(stack);
         log("drag", text);
         stack = text;
      } else
      {
         strcpy(stack, "\n\n A large wave drags you back into the sea and "
		"dumps you.\n   Divine punishment...\n\n");
         stack = end_string(stack);
         tell_player(e, text);
         stack = text;
         quit(e, 0);
         sprintf(stack, "-=> %s has been dragged into the sea by a large"
                        " wave.!!\n",
                 e->name);
         stack = end_string(stack);
         tell_room(e->location, text);
         stack = text;
	 if (p->gender==PLURAL)
	   sprintf(stack, " -=> %s drag %s\n -=> %s was from %s\n",
		   p->name, e->name, e->name, e->inet_addr);
	 else
	   sprintf(stack, " -=> %s drags %s\n -=> %s was from %s\n",
		   p->name, e->name, e->name, e->inet_addr);
         stack = end_string(stack);
         su_wall(text);
         stack = text;
         sprintf(stack, "%s - %s : %s", p->name, e->name, reason);
         stack = end_string(stack);
         log("drag", text);
         stack = text;
      }
   }
   stack = oldstack;
}


/* similar to shout but only goes to super users (eject and higher) */

void su(player * p, char *str)
{
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"su (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   command_type = 0;

   if (!*str)
   {
      tell_player(p, " Format: su <message>\n");
      return;
   }
   if (p->flags & BLOCK_SU)
   {
      tell_player(p, " You can't do sus when you're ignoring them.\n");
      return;
   }
   if (*str == ';')
   {
      str++;
      while (*str == ' ')
         str++;
      if ( p->flags & FROGGED )
         sprintf(stack, "<%s croakily %s>\n", p->name, str);
      else
         sprintf(stack, "<%s %s>\n", p->name, str);
   } else
   {
      if ( p->flags & FROGGED )
         sprintf(stack, "<%s> %s Ribbet!\n", p->name, str);
      else
         sprintf(stack, "<%s> %s\n", p->name, str);
   }
   stack = end_string(stack);
   su_wall_but(p, oldstack);
   tell_player(p, oldstack);
   stack = oldstack;
}


/* su-emote.. it's spannerish, I know, but what the hell */

void suemote(player * p, char *str)
{
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"suemote (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   command_type = 0;

   if (!*str)
   {
      tell_player(p, " Format: se <message>\n");
      return;
   }
   if (p->flags & BLOCK_SU)
   {
      tell_player(p, " You can't do su emotes when you're ignoring them.\n");
      return;
   }
   if ( p->flags & FROGGED )
      sprintf(stack, "<%s croakily %s>\n", p->name, str);
   else
      sprintf(stack, "<%s %s>\n", p->name, str);
   stack = end_string(stack);
   su_wall_but(p, oldstack);
   tell_player(p, oldstack);
   stack = oldstack;
}

/* Su think */

void suthink(player * p, char *str)
{
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"suthink (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   command_type = 0;

   if (!*str)
   {
      tell_player(p, " Format: st <message>\n");
      return;
   }
   if (p->flags & BLOCK_SU)
   {
      tell_player(p, " You can't do su thinks when you're ignoring them.\n");
      return;
   }
   if ( p->flags & FROGGED )
      sprintf(stack, "<%s thinks in a green fashion . o O ( %s )>\n", p->name, str);
   else
      sprintf(stack, "<%s thinks . o O ( %s )>\n", p->name, str);
   stack = end_string(stack);
   su_wall_but(p, oldstack);
   tell_player(p, oldstack);
   stack = oldstack;
}


/* toggle whether the su channel is highlighted or not */

void su_hilited(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"su_hilited (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->saved_flags & SU_HILITED)
   {
      tell_player(p, " You will not get the su channel hilited.\n");
      p->saved_flags &= ~SU_HILITED;
   } else
   {
      tell_player(p, " You will get the su channel hilited.\n");
      p->saved_flags |= SU_HILITED;
   }
}


/* Sync all player files */

void sync_all_by_user(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"sync_all_by_user (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   tell_player(p, " Starting to sync ALL players...");
   sync_all();
   tell_player(p, " Completed\n\r");
}


/* toggle whether the program is globally closed to newbies */

void close_to_newbies(player * p, char *str)
{
   char *oldstack;
   int wall = 0;

#ifdef TRACK
   sprintf(functionin,"close_to_newbies (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;

   if (p->flags & BLOCK_SU )
     {
       tell_player(p," You cant do THAT when off_duty.\n");
       return;
     }

   if ((!strcasecmp("on", str)||!strcasecmp("open",str))
       && sys_flags & CLOSED_TO_NEWBIES)
   {
      sys_flags &= ~CLOSED_TO_NEWBIES;

      /*log the open*/
      sprintf(oldstack,"Program opened to newbies by %s",p->name);
      stack=end_string(oldstack);
      log("newbies",oldstack);
      stack=oldstack;

      wall = 1;
   } else if ((!strcasecmp("off", str)||!strcasecmp("close",str))
	      && !(sys_flags & CLOSED_TO_NEWBIES))
   {
      sys_flags |= CLOSED_TO_NEWBIES;

      /*log the close*/
      sprintf(oldstack,"Program closed to newbies by %s",p->name);
      stack=end_string(oldstack);
      log("newbies",oldstack);
      stack=oldstack;

      wall = 1;
   } else
      wall = 0;

   if (sys_flags & CLOSED_TO_NEWBIES)
   {
      if (!wall)
         tell_player(p, " Program is closed to all newbies.\n");
      sprintf(oldstack, "\n   <%s closes the prog to newbies>\n\n", p->name);
   } else
   {
      if (!wall)
         tell_player(p, " Program is open to newbies.\n");
      sprintf(oldstack, "\n   <%s opens the prog to newbies>\n\n", p->name);
   }
   stack = end_string(oldstack);
   if (wall)
      su_wall(oldstack);
   stack = oldstack;
}


/* command to list lots of info about a person */

void check_info(player * p, char *str)
{
   player dummy, *p2;
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"check_info (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: check info <player>\n");
      return;
   }
   memset(&dummy, 0, sizeof(player));

   p2 = find_player_absolute_quiet(str);
   if (p2)
      memcpy(&dummy, p2, sizeof(player));
   else
   {
      strcpy(dummy.lower_name, str);
      lower_case(dummy.lower_name);
      dummy.fd = p->fd;
      if (!load_player(&dummy))
      {
         tell_player(p, " No such person in saved files.\n");
         return;
      }
   }

   switch (dummy.residency)
   {
      case SYSTEM_ROOM:
         tell_player(p, " Standard rooms file\n");
         return;
      default:
         if (dummy.residency & BANISHD)
         {
            if (dummy.residency == BANISHD)
               tell_player(p, "BANISHED (Name only).\n");
            else
               tell_player(p, "BANISHED.\n");
         }
         sprintf(stack, "            <   Res  >                <SU >\n"
                        "            B ETbsMLBS    OW    F ST  LH SA\n"
                        "Residency   %s\n", bit_string(dummy.residency));
         break;
   }
   stack = strchr(stack, 0);

   sprintf(stack, "%s %s %s\n%s\n%s\n%s %s\nEMAIL:%s\n",
           dummy.pretitle, dummy.name, dummy.title, dummy.description,
           dummy.plan, dummy.name, dummy.enter_msg, dummy.email);
   stack = strchr(stack, 0);
   switch (dummy.gender)
   {
      case MALE:
         strcpy(stack, "Gender set to male.\n");
         break;
      case FEMALE:
         strcpy(stack, "Gender set to female.\n");
         break;
      case PLURAL:
         strcpy(stack, "Gender set to plural.\n");
         break;
      case OTHER:
         strcpy(stack, "Gender set to something.\n");
         break;
      case VOID_GENDER:
         strcpy(stack, "Gender not set.\n");
         break;
   }
   stack = strchr(stack, 0);
   if ((dummy.password[0]) <= 0)
   {
      strcpy(stack, "NO PASSWORD SET\n");
      stack = strchr(stack, 0);
   }
   sprintf(stack, "            CHTSHPQEPRSHENAMNALICNDPSFSJRES-\n"
                  "Saved flags %s\n", bit_string(dummy.saved_flags));
   stack = strchr(stack, 0);
   sprintf(stack, "            PRNREPCPTLCEISDRUBSWAFS---------\n"
                  "flags       %s\n", bit_string(dummy.flags));
   stack = strchr(stack, 0);
   sprintf(stack, "Max: rooms %d, exits %d, autos %d, list %d, mails %d\n",
           dummy.max_rooms, dummy.max_exits, dummy.max_autos,
           dummy.max_list, dummy.max_mail);
   stack = strchr(stack, 0);
   sprintf(stack, "Term: width %d, wrap %d\n",
           dummy.term_width, dummy.word_wrap);
   stack = strchr(stack, 0);
   if (dummy.script)
   {
      sprintf(stack, "Scripting on for another %s.\n",
              word_time(dummy.script));
      stack = strchr(stack, 0);
   }
   *stack++ = 0;
   tell_player(p, oldstack);
   stack = oldstack;
}


/* command to check IP addresses */

void view_ip(player * p, char *str)
{
   player *scan;
   char *oldstack, middle[80];
   int page, pages, count;

#ifdef TRACK
   sprintf(functionin,"view_ip (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (isalpha(*str))
   {
      scan = find_player_global(str);
      stack = oldstack;
      if (!scan)
         return;
      if (scan->gender==PLURAL)
	sprintf(stack, "%s are logged in from %s.\n", scan->name,
		scan->inet_addr);
      else
	sprintf(stack, "%s is logged in from %s.\n", scan->name,
		scan->inet_addr);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      return;
   }
   page = atoi(str);
   if (page <= 0)
      page = 1;
   page--;

   pages = (current_players - 1) / (TERM_LINES - 2);
   if (page > pages)
      page = pages;

   if (current_players == 1)
      strcpy(middle, "There is only you on the program at the moment");
   else
      sprintf(middle, "There are %s people on the program",
              number2string(current_players));
   pstack_mid(middle);

   count = page * (TERM_LINES - 2);
   for (scan = flatlist_start; count; scan = scan->flat_next)
   {
      if (!scan)
      {
         tell_player(p, " Bad where listing, abort.\n");
         log("error", "Bad where list");
         stack = oldstack;
         return;
      } else if (scan->name[0])
         count--;
   }

   for (count = 0; (count < (TERM_LINES - 1) && scan); scan = scan->flat_next)
   {
      if (scan->name[0] && scan->location)
      {
         if (scan->flags & SITE_LOG)
            *stack++ = '*';
         else
            *stack++ = ' ';
	 if (scan->gender==PLURAL)
	   sprintf(stack, "%s are logged in from %s.\n", scan->name,
		   scan->inet_addr);
	 else
	   sprintf(stack, "%s is logged in from %s.\n", scan->name,
		   scan->inet_addr);
         stack = strchr(stack, 0);
         count++;
      }
   }
   sprintf(middle, "Page %d of %d", page + 1, pages + 1);
   pstack_mid(middle);

   *stack++ = 0;
   tell_player(p, oldstack);
   stack = oldstack;
}


/* command to view email status about people on the prog */

void view_player_email(player * p, char *str)
{
   player *scan;
   char *oldstack, middle[80];
   int page, pages, count;

#ifdef TRACK
   sprintf(functionin,"view_player_email (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   page = atoi(str);
   if (page <= 0)
      page = 1;
   page--;

   pages = (current_players - 1) / (TERM_LINES - 2);
   if (page > pages)
      page = pages;

   if (current_players == 1)
      strcpy(middle, "There is only you on the program at the moment");
   else
      sprintf(middle, "There are %s people on the program",
              number2string(current_players));
   pstack_mid(middle);

   count = page * (TERM_LINES - 2);
   for (scan = flatlist_start; count; scan = scan->flat_next)
   {
      if (!scan)
      {
         tell_player(p, " Bad where listing, abort.\n");
         log("error", "Bad where list");
         stack = oldstack;
         return;
      } else if (scan->name[0])
         count--;
   }

   for (count = 0; (count < (TERM_LINES - 1) && scan); scan = scan->flat_next)
   {
      if (scan->name[0] && scan->location)
      {
         if (scan->residency == NON_RESIDENT)
            sprintf(stack, "%s is non resident.\n", scan->name);
         else if (scan->email[0])
         {
            if (scan->email[0] == -1)
               sprintf(stack, "%s has declared no email address.\n",
                       scan->name);
            else if (scan->email[0] == -2)
            {
               sprintf(stack, "%s has not yet set an email address.\n",
                       scan->name);
            } else
            {
               sprintf(stack, "%s [%s]\n", scan->name, scan->email);
               if (scan->saved_flags & PRIVATE_EMAIL)
               {
                  while (*stack != '\n')
                     stack++;
                  strcpy(stack, " (private)\n");
               }
            }
         } else
         sprintf(stack, "%s has not set an email address.\n", scan->name);
         stack = strchr(stack, 0);
         count++;
      }
   }
   sprintf(middle, "Page %d of %d", page + 1, pages + 1);
   pstack_mid(middle);

   *stack++ = 0;
   tell_player(p, oldstack);
   stack = oldstack;
}


/* command to validate lack of email */

void validate_email(player * p, char *str)
{
   player *p2;
   char *oldstack;

   oldstack=stack;

#ifdef TRACK
   sprintf(functionin,"validate_email (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," You cant validate emails when off_duty.\n");
       return;
     }

   p2 = find_player_global(str);
   if (!p2)
      return;
   p2->email[0] = ' ';
   p2->email[1] = 0;
   tell_player(p, " Set player as having no email address.\n");

   sprintf(stack,"%s validated email for %s",p->name,p2->name);
   stack=end_string(oldstack);
   log("validate_email",oldstack);
   stack=oldstack;
   
}


/* New version of blankpass */

void new_blankpass(player *p, char *str)
{
   char *oldstack;
   char *pass,*size;
   player *p2, dummy;
   saved_player *sp;

#ifdef TRACK
   sprintf(functionin,"new_blankpass (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if(!*str)
     {
       tell_player(p, " Format: blankpass <player> [new password]\n");
       return;
   } else
     {
       

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," You cant blankpass when off_duty.\n");
       return;
     }

   oldstack = stack;
   pass = 0;
   pass = strchr(str, ' ');
   if (pass)
   {
      *pass++ = 0;
      if (strlen(pass) > 9 || strlen(pass) < 3)
      {
         tell_player(p, " Try a reasonable length password.\n");
         return;
      }
   }
   lower_case(str);
   p2 = find_player_absolute_quiet(str);
   if (p2)
   {
/* if player is logged in */
      if ( (p2->residency >= p->residency) && !(p->residency & HCADMIN) )
      {
         tell_player(p, " You can't blankpass THAT person!\n");
         sprintf(stack, " -=> %s TRIED to blankpass %s!\n", p->name, p2->name);
         stack = end_string(stack);
         su_wall_but(p, oldstack);
         stack = oldstack;
         return;
      }
      if (!pass)
      {
         sprintf(stack, " -=> %s has just blanked your password.\n", p->name);
         stack = end_string(stack);
         tell_player(p2, oldstack);
         stack = oldstack;
         p2->password[0] = 0;
         tell_player(p, "Password blanked.\n");
	 sprintf(stack, "%s blanked %s's password (logged in)", p->name,
		 p2->name);
	 stack = end_string(stack);
	 log("blanks", oldstack);
	 stack = oldstack;
    } else
      {
         sprintf(stack, " -=> %s has just changed your password.\n", p->name);
         stack = end_string(stack);
         tell_player(p2, oldstack);
         stack = oldstack;
         strcpy(p2->password, do_crypt(pass, p2));
         tell_player(p, " Password changed. They have NOT been informed of"
                        " what it is.\n");
	 sprintf(stack, "%s changed %s's password (logged in)", p->name,
		 p2->name);
	 stack = end_string(stack);
	 log("blanks", oldstack);
	 stack = oldstack;
      }
      set_update(*str);
      return;
    }
   else
     {
       strcpy(dummy.lower_name, str);
       dummy.fd = p->fd;
       if (load_player(&dummy))
       {
	   if (dummy.residency & BANISHD)
	   {
	       tell_player(p, " By the way, this player is currently BANISHD.");
	       if (dummy.residency == BANISHD)
	       {
		   tell_player(p, " (Name Only)\n");
	       } else
	       {
		   tell_player(p, "\n");
	       }
	   }
      if ( (dummy.residency >= p->residency) && !(p->residency & HCADMIN) )
      {
         tell_player(p, " You can't blankpass THAT person!\n");
         sprintf(stack, " -=> %s TRIED to blankpass %s!\n", p->name, dummy.name);
         stack = end_string(stack);
         su_wall_but(p, oldstack);
         stack = oldstack;
         return;
      }
	   if (pass)
	   {
	       strcpy(dummy.password, do_crypt(pass, &dummy));
	       tell_player(p, " Password changed in saved files.\n");
	       sprintf(stack, "%s changed %s's password (logged out)", p->name,
		 dummy.name);
	       stack = end_string(stack);
	       log("blanks", oldstack);
	       stack = oldstack;
	   } else
	   {
	       dummy.password[0] = 0;
	       tell_player(p, " Password blanked in saved files.\n");
	       sprintf(stack, "%s changed %s's password (logged out)", p->name,
		       dummy.name);
	       stack = end_string(stack);
	       log("blanks", oldstack);
	       stack = oldstack;
	   }
	   dummy.script = 0;
	   dummy.script_file[0] = 0;
	   dummy.flags &= ~SCRIPTING;
	   dummy.location = (room *) -1;
	   save_player(&dummy);
       } else
	 tell_player(p, " Can't find that player in saved files.\n");
     }   
     }

}
/* a test fn to test things */

void test_fn(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"test_fn (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   do_birthdays();
}


/* give someone lag ... B-) */

void            add_lag(player * p, char *str)
{
   char           *size;
   int             new_size;
   char           *oldstack;
   player         *p2;

#ifdef TRACK
   sprintf(functionin,"add_lag (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," Lagging isnt nice at the best of times, the least "
		   "you can do is go on_duty before you torture the poor "
		   "victim {:-)\n");
       return;
     }

   oldstack = stack;
   size = next_space(str);
   *size++ = 0;
   new_size = atoi(size);

   /*change minutes to seconds*/
   new_size*=60;

   /* can't check with new_size == 0 as we need that for unlagging */
   /* check for duff command syntax */
   if (strlen(size) == 0)
   {
      tell_player(p, " Format: lag <player> <time in minutes>\n");
      return;
   }
   /* find them and return if they're not on */
   p2 = find_player_global(str);
   if (!p2)
      return;
   /* thou shalt not lag those above you */
   if (p2->residency >= p->residency)
   {
       tell_player(p, " You can't do that !!\n");
       sprintf(oldstack, " -=> %s tried to lag you.\n", p->name);
       stack = end_string(oldstack);
       tell_player(p2, oldstack);
       stack = oldstack;
       return;
   }
   
   /* check for silly or nasty amounts of lag */
   if (new_size < 0)
   {
	 tell_player(p, " That's not nice, and anyway you can't lag anyone "
		     "permanently any more. Set to 10 minutes.\n");
	 new_size = 600;
   }
   if (new_size > 600 && !(p->residency & ADMIN))
   {
       tell_player(p, "That's kinda excessive, set to 10 minutes.\n");
       new_size = 600;
   }
   /* lag 'em */
   p2->lagged = new_size;

   /* report success */
   if (new_size == 0)
   {
       sprintf(oldstack, " %s has been unlagged.\n", p2->name);
       stack = end_string(oldstack);
       tell_player(p, oldstack);
       stack = oldstack;
       sprintf(oldstack," -=> %s unlags %s.\n",p->name,p2->name);
       stack=end_string(oldstack);
       su_wall_but(p,oldstack);
       stack=oldstack;
       sprintf(oldstack,"%s unlags %s",p->name,p2->name);
       stack=end_string(oldstack);
       log("lag",oldstack);
       stack=oldstack;
   }
   else
   {
       tell_player(p, " Tis Done ..\n");
       stack = oldstack;
       sprintf(oldstack," -=> %s lags %s for %d minutes.\n",p->name,p2->name,
	       new_size/60);
       stack=end_string(oldstack);
       su_wall(oldstack);
       stack=oldstack;
       sprintf(oldstack,"%s lags %s for %d minutes",p->name,
	       p2->name,new_size/60);
       stack=end_string(oldstack);
       log("lag",oldstack);
       stack=oldstack;
   }
}


/* remove shout from someone for a period */

void remove_shout(player * p, char *str)
{
   char *oldstack, *size = 0;
   int new_size = 5;
   player *p2;
   
#ifdef TRACK
   sprintf(functionin,"remove_shout (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: rm_shout <player> [<for how long>]\n");
      return;
   }

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," You need to be on_duty for that\n");
       return;
     }

   oldstack = stack;
   size = strchr(str, ' ');
   if (size)
   {
      *size++ = 0;
      new_size = atoi(size);
   }
   p2 = find_player_global(str);
   if (!p2)
      return;
   if (p2->residency >= p->residency)
   {
      tell_player(p, " You can't do that !!\n");
      sprintf(oldstack, " -=> %s tried to remove shout from you.\n", p->name);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      stack = oldstack;
      return;
   }
   p2->saved_flags &= ~SAVENOSHOUT;
   if (new_size)
      tell_player(p2, " -=> You suddenly find yourself with a sore throat.\n");
   else
      tell_player(p2, " -=> Someone hands you a cough sweet.\n");
   if (new_size > 30)
     if (!(p->residency & ADMIN))
       new_size = 5;
   switch (new_size)
   {
      case -1:
         sprintf(stack, " -=> %s just remove shouted %s. (permanently!)\n",
                 p->name, p2->name);
         p2->saved_flags |= SAVENOSHOUT;
         p2->no_shout = -1;
         break;
      case 0:
         sprintf(stack, " -=> %s just allowed %s to shout again.\n", p->name,
                 p2->name);
         break;
      case 1:
         sprintf(stack, " -=> %s just remove shouted %s for 1 minute.\n",
                 p->name, p2->name);;
         break;
      default:
         sprintf(stack, " -=> %s just remove shouted %s for %d minutes.\n",
                 p->name, p2->name, new_size);
         break;
   }
   new_size *= 60;
   if (new_size >= 0)
      p2->no_shout = new_size;
   stack = end_string(stack);
   su_wall(oldstack);
   stack = oldstack;

   if (new_size != 0)
     sprintf(stack,"%s removed %s's shout for %d.",p->name,p2->name,
	     new_size);
   else
     sprintf(stack,"%s regranted shouts to %s.\n",p->name,p2->name);
   stack = end_string(stack);
   log("rm_shout",oldstack);

   stack=oldstack;
}


/* remove trans movement from someone for a period */

void remove_move(player * p, char *str)
{
   char *size;
   int new_size = 5;
   char *oldstack;
   player         *p2;

#ifdef TRACK
   sprintf(functionin,"remove_move (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: rm_move <player> [<for how long>]\n");
      return;
   }

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," You need to be on_duty for that.\n");
       return;
     }

   size = strchr(str, ' ');
   if (size)
   {
      *size++ = 0;
      new_size = atoi(size);
   } else
      new_size = 1;
   p2 = find_player_global(str);
   if (!p2)
      return;
   if (p2->residency > p->residency)
   {
      tell_player(p, " You can't do that !!\n");
      sprintf(oldstack, " -=> %s tried to remove move from you.\n", p->name);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      stack = oldstack;
      return;
   }
   if (new_size)
      tell_player(p2, " -=> You step on some chewing-gum, and you suddenly "
                      "find it very hard to move ...\n");
   else
      tell_player(p2, " -=> Someone hands you a new pair of shoes ...\n");
   if (new_size > 30)
      new_size = 5;
   new_size *= 60;
   if (new_size >= 0)
      p2->no_move = new_size;
   if ((new_size/60) == 1)
      sprintf(stack, " -=> %s remove moves %s for 1 minute.\n", p->name,
              p2->name);
   else if (new_size == 0)
      sprintf(stack, " -=> %s allows %s to move again.\n", p->name,
              p2->name);
   else if (new_size <0 )
      sprintf(stack, " -=> %s remove moves %s. Permanently!\n", p->name,
              p2->name);
   else
      sprintf(stack, " -=> %s remove moves %s for %d minutes.\n", p->name,
              p2->name, new_size/60);
   stack = end_string(stack);
   su_wall(oldstack);
   log("rm_move",oldstack);
   stack = oldstack;
}


/* change someones max mail limit */

void change_mail_limit(player * p, char *str)
{
   char *size;
   int new_size;
   char *oldstack;
   player *p2;
   player dummy;

#ifdef TRACK
   sprintf(functionin,"change_mail_limit (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   size = next_space(str);
   *size++ = 0;
   new_size = atoi(size);

   if (new_size < 0)
   {
      tell_player(p, " Now try a _positive_ limit...\n");
      return;
   }
   p2 = find_player_absolute_quiet(str);
   if (!p2)
   {
     /* load them if they're not logged in */
      strcpy(dummy.lower_name, str);
      lower_case(dummy.lower_name);
      dummy.fd = p->fd;
      /* if they don't exist, say so and return */
      if (!load_player(&dummy))
      {
         tell_player(p, " That player does not exist.\n");
         return;
      }
      p2 = &dummy;
   }

   if (p2->residency > p->residency)
   {
     /* now now, no messing with your superiors */
      tell_player(p, " You can't do that !!\n");
      sprintf(oldstack, " -=> %s tried to change your mail limit.\n", p->name);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      stack = oldstack;
      return;
   }
   /* otherwise change the limit */
   p2->max_mail = new_size;
   /* and if they are logged in, tell them */
   if (p2 != &dummy)
   {
      sprintf(oldstack, " -=> %s has changed your mail limit to %d.\n",
	      p->name,new_size);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
   } else
   {
      save_player(&dummy);
   }
   tell_player(p, " Tis Done ..\n");
   stack = oldstack;
}


/* change someones max list limit */

void change_list_limit(player * p, char *str)
{
   char *size;
   int new_size;
   char *oldstack;
   player *p2;
   player dummy;

#ifdef TRACK
   sprintf(functionin,"change_list_limit (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   size = next_space(str);
   *size++ = 0;
   new_size = atoi(size);
   /* syntax check */
   if (!new_size)
   {
      tell_player(p, " Format: change_list_limit <player> <new size>\n");
      return;
   }

   /* negative limit trap */
   if (new_size < 0)
     {
       tell_player(p, " Now try a _positive_ list limit...\n");
       return;
     }

   p2 = find_player_absolute_quiet(str);
   if (!p2)
   {
      strcpy(dummy.lower_name, str);
      lower_case(dummy.lower_name);
      dummy.fd = p->fd;
      if (!load_player(&dummy))
      {
         tell_player(p, " That player doesn't exist.\n");
         return;
      }
      p2 = &dummy;
   }
   if (p2->residency > p->residency)
   {
      tell_player(p, " You can't do that !!\n");
      sprintf(oldstack, " -=> %s tried to change your list limit.\n", p->name);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      stack = oldstack;
      return;
   }
   p2->max_list = new_size;
   if (p2 != &dummy)
   {
      sprintf(oldstack, " -=> %s has changed your list limit to %d.\n", p->name,
              new_size);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
   } else
   {
      save_player(&dummy);
   }
   tell_player(p, " Tis Done ..\n");
   stack = oldstack;
}


/* change someones max room limit */

void change_room_limit(player * p, char *str)
{
   char *size;
   int new_size;
   char *oldstack;
   player *p2;
   player dummy;

#ifdef TRACK
   sprintf(functionin,"change_room_limit (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   size = next_space(str);
   *size++ = 0;
   new_size = atoi(size);
   if (!new_size)
   {
      tell_player(p, " Format: change_room_limit <player> <new size>\n");
      return;
   }

   /* sponge trap :-) */
   if (new_size < 0)
     {
       tell_player(p, " Now try a _positive_ room limit...\n");
       return;
     }

   p2 = find_player_absolute_quiet(str);
   if (!p2)
   {
      strcpy(dummy.lower_name, str);
      lower_case(dummy.lower_name);
      dummy.fd = p->fd;
      if (!load_player(&dummy))
      {
         tell_player(p, " That player doesn't exist.\n");
         return;
      }
      p2 = &dummy;
   }
   if (p2->residency > p->residency)
   {
     /* no messing with those above you */
      tell_player(p, " You can't do that !!\n");
      sprintf(oldstack, " -=> %s tried to change your room limit.\n", p->name);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      stack = oldstack;
      return;
   }
   p2->max_rooms = new_size;
   if (p2 != &dummy)
   {
      sprintf(oldstack, " -=> %s has changed your room limit to %d.\n", p->name,
              new_size);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
   } else
   {
      save_player(&dummy);
   }
   tell_player(p, " Tis Done ..\n");
   stack = oldstack;
}


/* change someones max exit limit */

void change_exit_limit(player * p, char *str)
{
   char *size;
   int new_size;
   char *oldstack;
   player *p2;
   player dummy;

#ifdef TRACK
   sprintf(functionin,"change_exit_limit (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   size = next_space(str);
   *size++ = 0;
   new_size = atoi(size);
   if (!new_size)
   {
      tell_player(p, " Format: change_exit_limit <player> <new size>\n");
      return;
   }

   if (new_size < 0)
     {
       tell_player(p, " Now try a _positive_ exit limit...\n");
       return;
     }

   p2 = find_player_absolute_quiet(str);
   if (!p2)
   {
      strcpy(dummy.lower_name, str);
      lower_case(dummy.lower_name);
      dummy.fd = p->fd;
      if (!load_player(&dummy))
      {
         tell_player(p, " That player does not exist.\n");
         return;
      }
      p2 = &dummy;
   }
   if (p2->residency > p->residency)
   {
      tell_player(p, " You can't do that !!\n");
      sprintf(oldstack, " -=> %s tried to change your exit limit.\n", p->name);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      stack = oldstack;
      return;
   }
   p2->max_exits = new_size;
   if (p2 != &dummy)
   {
      sprintf(oldstack, " %s has changed your exit limit to %d.\n", p->name,
              new_size);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
   } else
   {
      save_player(&dummy);
   }
   tell_player(p, " Tis Done ..\n");
   stack = oldstack;
}


/* change someones max autos limit */

void change_auto_limit(player * p, char *str)
{
   char *size;
   int             new_size;
   char           *oldstack;
   player         *p2;
   player dummy;

#ifdef TRACK
   sprintf(functionin,"change_auto_limit (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   size = next_space(str);
   *size++ = 0;
   new_size = atoi(size);
   if (!new_size)
   {
      tell_player(p, " Format: change_auto_limit <player> <new size>\n");
      return;
   }
   if (new_size < 0)
     {
       tell_player(p, " Now try a _positive_ auto limit...\n");
       return;
     }
       
   p2 = find_player_absolute_quiet(str);
   if (!p2)
   {
      strcpy(dummy.lower_name, str);
      lower_case(dummy.lower_name);
      dummy.fd = p->fd;
      if (!load_player(&dummy))
      {
         tell_player(p, " That player does not exist.\n");
         return;
      }
      p2 = &dummy;
   }
   if (p2->residency > p->residency)
   {
      tell_player(p, " You can't do that !!\n");
      sprintf(oldstack, " -=> %s tried to change your automessage limit.\n",
         p->name);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      stack = oldstack;
      return;
   }
   p2->max_autos = new_size;
   if (p2 != &dummy)
   {
      sprintf(oldstack, " -=> %s has changed your automessage limit to %d.\n",
              p->name, new_size);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
   } else
   {
      save_player(&dummy);
   }
   tell_player(p, " Tis Done ..\n");
   stack = oldstack;
}


/* manual command to sync files to disk */

void sync_files(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"sync_files (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!isalpha(*str))
   {
      tell_player(p, " Argument must be a letter.\n");
      return;
   }
   sync_to_file(tolower(*str), 1);
   tell_player(p, " Sync succesful.\n");
}


/* manual retrieve from disk */

void restore_files(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"restore_files (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!isalpha(*str))
   {
      tell_player(p, " Argument must be a letter.\n");
      return;
   }
   remove_entire_list(tolower(*str));
   hard_load_one_file(tolower(*str));
   tell_player(p, " Restore succesful.\n");
}


/* shut down the program */

void pulldown(player * p, char *str)
{
   char *oldstack, *reason, *i;

#ifdef TRACK
   sprintf(functionin,"pulldown (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif
   
   if (p->flags & BLOCK_SU)
   {
       tell_player(p," You need to be on_duty for that\n");
       return;
   }
   oldstack = stack;
   command_type &= ~HIGHLIGHT;
   
   if (!(p->residency & (LOWER_ADMIN|ADMIN)))
   {
       /* SUs can see a shutdown but not start one */
       if (*str)
       {
	   /* lest they try... */
	   tell_player(p, " NOT bloody likely.\n");
	   return;
       }
       if (shutdown_count > 1)
	   /* if a shutdown is in progress */
       {
	   /* contruct the message to tell them and send it to them */
	   sprintf(stack, "\n %s, in %d seconds.\n",
		   shutdown_reason, shutdown_count);
	   stack = end_string(stack);
	   tell_player(p, oldstack);
	   /* clean up stack and exit */
	   stack = oldstack;
	   return;
       }
       else
       {
	   /* tell them no joy */
	   tell_player(p, " No shutdown in progress.\n");
	   return;
       }
       
   }
   if (!*str)
   {
       if (shutdown_count > -1)
       {
	   sprintf(stack, "\n %s, in %d seconds.\n  \'shutdown -1\' to abort.\n\n"
		   , shutdown_reason, shutdown_count);
	   stack = end_string(stack);
	   tell_player(p, oldstack);
	   stack = oldstack;
	   return;
       } else
       {
	   tell_player(p, " Format: shutdown <countdown> [<reason>]\n");
	   return;
       }
   }
   reason = strchr(str, ' ');
   if (!reason)
   {
      sprintf(shutdown_reason, "%s is shutting the program down - it is "
	      "probably for a good reason too\n",p->name);
   } else
   {
      *reason++ = 0;
      sprintf(shutdown_reason, "%s is shutting the program down - %s",
              p->name, reason);
   }
   if (!strcmp(str, "-1"))
   {
      shutdown_reason[0] = '\0';
      if (shutdown_count < 300)
      {
         raw_wall("\n\nShutdown aborted "
                  "(If you ever knew one was in progress...)\n\n");
      } else
      {
         tell_player(p, " Shutdown Aborted.\n");
      }
      shutdown_count = -1;
      return;
   }
   i = str;
   while (*i != 0)
   {
      if (!isdigit(*i))
      {
         tell_player(p, " Format: shutdown <countdown> [<reason>]\n");
         return;
      }
      *i++;
   }
   shutdown_count = atoi(str);
   sprintf(stack, " -=> Program set to shutdown in %d seconds...\n",
           shutdown_count);
   stack = end_string(stack);
   tell_player(p, oldstack);
   stack = oldstack;
   command_type &= ~HIGHLIGHT;
}


/* wall to everyone, non blockable */

void wall(player * p, char *str)
{
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"wall (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if(p->flags & BLOCK_SU)
     {
       tell_player (p,"Permissions changed...\nOnly kidding {:-) \n"
		    "No, seriously, you cant use wall when off_duty.\n");
       return;
     }

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: wall <arg>\n");
      return;
   }
   sprintf(oldstack, " %s screams -=> %s <=-\007\n", p->name, str);
   stack = end_string(oldstack);
   command_type |= HIGHLIGHT;
   raw_wall(oldstack);
   command_type &= ~HIGHLIGHT;
   stack = oldstack;
}


/* permission changes routines */

/* the resident command */

void resident(player * p, char *str)
{
   player *p2;
   char *oldstack;
   int ressie = 0;

#ifdef TRACK
   sprintf(functionin,"resident (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: resident <whoever>\n");
      return;
   }

   if (p->flags &BLOCK_SU)
     {
       tell_player(p," Nope, not whilst you are off_duty you wont.\n");
       return;
     }
   
   if (!strcasecmp(str, "me"))
      p2 = p;
   else
      p2 = find_player_global(str);
   if (!p2)
   {
      stack = oldstack;
      return;
   }
   if (!strcasecmp(p2->name, "guest"))
   {
      tell_player(p, "\n The name 'Guest' is reserved because people may use "
                     "that when first logging in before using the name they "
                     "REALLY want to use. So get this person to choose another "
                     "name, THEN make them resident.\n\n");
      stack = oldstack;
      return;
   }
   if ((p2->residency != NON_RESIDENT) && p2 != p)
   {
      if (p2->saved)
      {
         if (p2->saved->last_host)
         {
            if (p2->saved->last_host[0] != 0)
            {
               tell_player(p, " That player is already resident, and has "
                              "re-logged in\n");
               stack = oldstack;
               return;
            }
         }
      }
      ressie = 1;
   }
   if (ressie)
   {
      sprintf(oldstack, "\n\n -=> You are now a resident.\n");
   } else
   {
     if (p->gender==PLURAL)
       sprintf(oldstack, "\n\n -=> %s have made you a resident.\n", p->name);
     else
       sprintf(oldstack, "\n\n -=> %s has made you a resident.\n", p->name);
   }
   stack = strchr(oldstack, 0);

   sprintf(stack, " For this to take effect, you MUST set an email address"
     " and password NOW.\n"
     " If you don't you will still not be able to save, and next time you"
     " log in, you will be no longer resident.\n"
     " To set an email address, simply type 'email <whatever>' as a command. "
     "(without the quotes or <>'s)\n"
     " You must use your proper system email address for this.\n"
     " To set your password, simply type 'password' as a command, and follow"
     " the prompts.\n"
     " IF you get stuck, read the help, using the command 'help',  ask %s, "
     "or any other Super User (type lsu for a list), for help...\n\n",
	   p->name);
   stack = end_string(stack);
   tell_player(p2, oldstack);
   if (ressie)
   {
      stack = oldstack;
      sprintf(stack, " You repeat the message about setting email and "
                     "password to %s\n", p2->name);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      return;
   }
   if (p2 != p)
   {
      p2->residency |= get_flag(permission_list, "residency");
      p2->residency |= NO_SYNC;
      p2->email[0] = 2;
      p2->email[1] = 0;
      p2->flags &= ~SCRIPTING;
      p2->script = 0;
      p2->script_file[0] = 0;
      strcpy(p2->script_file, "dummy");
      tell_player(p, " Residency granted ...\n");
      stack = oldstack;

      if (p->gender==PLURAL)
	sprintf(oldstack, " -=> All the %s gang up and grant residency to %s\n", p->name,
		p2->name);
      else
	sprintf(oldstack, " -=> %s grants residency to %s\n", p->name,
		p2->name);
      stack = end_string(oldstack);
      su_wall(oldstack);
      stack = oldstack;
      p2->saved_residency = p2->residency;
      p2->saved = 0;
      sprintf(stack, "%s made %s a resident.", p->name, p2->name);
      stack = end_string(stack);
      log("resident", oldstack);
   }
   stack = oldstack;
}


/* the grant command */

void grant(player * p, char *str)
{
   char *permission;
   player *p2;
   saved_player *sp;
   int change;
   char *oldstack;
   int count;

#ifdef TRACK
   sprintf(functionin,"grant (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags &BLOCK_SU)
     {
       tell_player(p," No, Im sorry, Im not granting that till you go back "
		   "on_duty\n Luv - Summink\n");
       return;
     }
   
   oldstack = stack;
   permission = next_space(str);
   if (!*permission)
   {
      tell_player(p, " Format: grant <whoever> <whatever>\n");
      tell_player(p, " Grantable privs are: ");
      for (count=0;permission_list[count].text!=0;count++)
      {
         sprintf(stack, "%s, ", permission_list[count].text);
         stack = strchr(stack, 0);
      }
      while (*stack != ',')
         *stack--;
      *stack++ = '.';
      *stack++ = '\n';
      *stack++ = 0;
      tell_player(p, oldstack);
      stack = oldstack;
      return;
   }
   *permission++ = 0;

   change = get_flag(permission_list, permission);
   if (!change)
   {
      tell_player(p, " Can't find that permission.\n");
      return;
   }
   if (!(p->residency & change) )
   {
     if ( !(p->residency & HCADMIN))
      {
         tell_player(p, " You can't give out permissions you haven't got "
           "yourself.\n");
         return;
      }
   }
   p2 = find_player_global(str);
   if (!p2)
   {
      lower_case(str);
      sp = find_saved_player(str);
      if (!sp)
      {
         tell_player(p, " Couldn't find player.\n");
         stack = oldstack;
         return;
      }
      if (sp->residency == BANISHD || sp->residency == SYSTEM_ROOM)
      {
         tell_player(p, " That is a banished NAME, or System Room.\n");
         stack = oldstack;
         return;
      }
      if ((change == SYSTEM_ROOM) && !(sp->residency == 0))
      {
         tell_player(p, " You can't grant sysroom to anything but a blank"
                        "playerfile.\n");
         stack = oldstack;
         return;
      }
      if (sp->residency > p->residency)
      {
         tell_player(p, " You can't alter that save file\n");
         sprintf(oldstack, "%s failed to grant %s to %s\n", p->name,
                 permission, str);
         stack = end_string(oldstack);
         log("grant", oldstack);
         stack = oldstack;
         return;
      }
      tell_player(p, " Permission changed in player files.\n");
      stack = oldstack;
      sprintf(stack, "%s granted %s to %s", p->name, permission, 
	      sp->lower_name);
      stack = end_string(stack);
      log("grant",oldstack);
      sp->residency |= change;
      set_update(*str);
      stack = oldstack;
      return;
   } else
   {
      if (p2->residency == NON_RESIDENT)
      {
         tell_player(p, " That player is non-resident!\n");
         stack = oldstack;
         return;
      }
      if (p2->residency == BANISHD || p2->residency == SYSTEM_ROOM)
      {
         tell_player(p, " That is a banished NAME, or System Room.\n");
         stack = oldstack;
         return;
      }
      if (p2->residency > p->residency)
      {
         tell_player(p, " No Way Pal !!\n");
         sprintf(oldstack, " -=> %s tried to grant your permissions.\n"
                 ,p->name);
         stack = end_string(oldstack);
         tell_player(p2, oldstack);
         stack = oldstack;
         return;
      }
      sprintf(oldstack, "\n%s has changed your permissions.\n", p->name);
      p2->saved_residency |= change;
      p2->residency = p2->saved_residency;
      stack = strchr(stack, 0);
      if (p2->residency & SU)
      {
         strcpy(stack, "Read the appropriate files please ( shelp "
                       "basic and shelp advanced )\n\n");
      }
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      stack = oldstack;
      sprintf(stack, "%s granted %s to %s", p->name, permission, p2->name);
      stack = end_string(stack);
      log("grant",oldstack);
      save_player(p2);
      tell_player(p, " Permissions changed ...\n");
   }
   stack = oldstack;
}


/* the remove command */

void remove(player * p, char *str)
{
   char *permission;
   player *p2;
   saved_player *sp;
   int change, count;
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"remove (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags &BLOCK_SU)
     {
       tell_player(p," Go back on duty first.\n");
       return;
     }

   oldstack = stack;
   permission = next_space(str);
   if (!*permission)
   {
      tell_player(p, " Format: remove <whoever> <whatever>\n");
      tell_player(p, " Remove-able privs are: ");
      for (count=0;permission_list[count].text!=0;count++)
      {
         sprintf(stack, "%s, ", permission_list[count].text);
         stack = strchr(stack, 0);
      }
      while (*stack != ',')
         *stack--;
      *stack++ = '.';
      *stack++ = '\n';
      *stack++ = 0;
      tell_player(p, oldstack);
      stack = oldstack;
      return;
   }
   *permission++ = 0;

   if (!(strcasecmp("everyone", str))
         && !(strcasecmp("everything", permission))
         && (p->residency & (1 << 27)))
   {
      tell_player(p, "\n You can sod off and die if you think I'm going to "
        "let you do that ...\n\n");
      su_wall("\n -=>Someone just tried to rm -rf * my entire sodding "
         "directory!\n\n");
      return;
   }
   change = get_flag(permission_list, permission);
   if (!change)
   {
      tell_player(p, " Can't find that permission.\n");
      return;
   }
   if (!(p->residency & change))
   {
      if ( !(p->residency & HCADMIN) )
      {
         tell_player(p, " You can't remove permissions you haven't got "
                        "yourself.\n");
         return;
      }
   }

   p2 = find_player_global(str);
   if (!p2)
   {
      sp = find_saved_player(str);
      if (!sp)
      {
         tell_player(p, " Couldn't find player.\n");
         return;
      }
      if (sp->residency > p->residency)
      {
         tell_player(p, " You cant change that save file !!!\n");
         sprintf(oldstack, "%s failed to remove %s from %s", p->name,
                 permission, str);
         stack = end_string(oldstack);
         log("grant", oldstack);
         stack = oldstack;
         return;
      }
      sp->residency &= ~change;
      if (sp->residency == NON_RESIDENT)
         remove_player_file(sp->lower_name);
      set_update(*str);
      tell_player(p, " Permissions changed in save files.\n");
      stack = oldstack;
      sprintf(oldstack, "%s removes %s from %s", p->name,
           permission, str);
      stack = end_string(oldstack);
      log("grant", oldstack);
      stack=oldstack;
      return;
   } else
   {
      if (p2->residency > p->residency)
      {
         tell_player(p, " No Way Pal !!\n");
         sprintf(oldstack, " -=> %s tried to remove your permissions.\n", p->name);
         stack = end_string(oldstack);
         tell_player(p2, oldstack);
         stack = oldstack;
         return;
      }
      p2->residency &= ~change;
      p2->saved_residency = p2->residency;
      sprintf(oldstack, " -=> %s has changed your permissions.\n", p->name);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      if (p2->residency != NON_RESIDENT)
         save_player(p2);
      else
         remove_player_file(p2->lower_name);
      tell_player(p, " Permissions changed ...\n");
   }
   stack = oldstack;
}


/* remove player completely from the player files */

void nuke_player(player * p, char *str)
{
   char *oldstack;
   player *p2, dummy;
   saved_player *sp;
   char nuked[MAX_NAME] = "";
   char nukee[MAX_NAME] = "";
   char naddr[MAX_INET_ADDR] = "";
   int mesg_done = 0;
   int *scan, *scan_count, mcount = 0, sscan;
   note *smail, *snext;

#ifdef TRACK
   sprintf(functionin,"nuke_player (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;

   if (p->flags &BLOCK_SU)
     {
       tell_player(p," I'm sorry Dave, I cannot allow you to do that\n"
		   " Luv - Hal\n"
		   " OK, not really, it was cos you were off_duty. {:-)\n");
       return;
     }

   p2 = find_player_absolute_quiet(str);
   if (!p2)
      tell_player(p, "No such person on the program.\n");
   if (p2)
   {
      if (p->residency <= p2->residency)
      {
         tell_player(p, " You can't nuke them !\n");
         sprintf(oldstack, " -=> %s tried to nuke you.\n", p->name);
         stack = end_string(oldstack);
         tell_player(p2, oldstack);
         stack = oldstack;
         return;
      }
      if (p2->saved)
         all_players_out(p2->saved);
      strcpy(oldstack, 
             "\n"
             " -=> There are times that you wish you'd developed      <=-\n"
             " -=> some way of stopping stray nukes from dropping on  <=-\n"
             " -=> your head and killing you.                         <=-\n"
             " -=>                                                    <=-\n"
             " -=> This is one of those times, mostly because that's  <=-\n"
             " -=> what just happened.                                <=-\n"
             " -=>                                                    <=-\n"
             " -=> See you in hell :-)                                <=-\n"
             " -=>                                                    <=-\n"
             " -=> (If you hadn't guessed, you've just been nuked...) <=-\n");
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      stack = oldstack;
      p2->saved = 0;
      p2->residency = 0;
      quit(p2, 0);
      strcpy(nuked, p2->name);
      strcpy(naddr, p2->inet_addr);
      if (p->gender==PLURAL)
	sprintf(stack, " -=> %s nuke %s to a crisp, toast time!\n -=> %s "
		"was from %s\n",p->name, nuked, nuked, naddr);
      else
	sprintf(stack, " -=> %s nukes %s to a crisp, toast time!\n -=> %s "
		"was from %s\n",p->name, nuked, nuked, naddr);
      stack = end_string(stack);
      su_wall(oldstack);
      stack = oldstack;
      mesg_done = 1;
   }
   strcpy(nukee, str);
   lower_case(nukee);
   sp = find_saved_player(nukee);
   if (!sp)
   {
      sprintf(stack, " Couldn't find saved player '%s'.\n", str);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      return;
   }
   if (sp->residency >= p->residency)
   {
      tell_player(p, " You can't nuke that save file !\n");
      stack = oldstack;
      return;
   }
/* TRY to clean up notes */
   strcpy(dummy.lower_name, sp->lower_name);
   dummy.fd = p->fd;
   load_player(&dummy);
   if (!*nuked)
   {
      strcpy(nuked, dummy.name);
      strcpy(naddr, sp->last_host);
   }
   scan = dummy.saved->mail_received;
   if (scan)
   {
      for (scan_count = scan; *scan_count; scan_count++)
      {
         mcount++;
      }
      for (;mcount;mcount--)
      {
         delete_received(&dummy, "1");
      }
   }
   mcount = 1;
   sscan = dummy.saved->mail_sent;
   smail = find_note(sscan);
   if (smail)
   {
      while (smail)
      {
         mcount++;
         sscan = smail->next_sent;
         snext = find_note(sscan);
         if (!snext && sscan)
         {
            smail->next_sent = 0;
            smail = 0;
         } else
         {
            smail = snext;
         }
      }
      for(;mcount;mcount--)
      {
         delete_sent(&dummy, "1");
      }
   }
   save_player(&dummy);
/* END clean up notes */
   all_players_out(sp);
   tell_player(p, " Files succesfully nuked.\n");
   if(!mesg_done)
   {
     if (p->gender==PLURAL)
       sprintf(stack, " -=> %s nuke \'%s\' to a crisp, toast time!\n",
	       p->name,sp->lower_name);
     else
       sprintf(stack, " -=> %s nukes \'%s\' to a crisp, toast time!\n",
	       p->name,sp->lower_name);
      stack = end_string(stack);
      su_wall(oldstack);
      stack = oldstack;
   }
   sprintf(stack, "%s nuked %s [%s]", p->name, nuked, naddr);
   stack = end_string(stack);
   log("nuke", oldstack);
   stack = oldstack;
   remove_player_file(nukee);
}


/* banish a player from the program */

void banish_player(player * p, char *str)
{
   char *oldstack, *i, ban_name[MAX_NAME + 1] = "";
   player *p2;
   saved_player *sp;
   int newbie=0;

#ifdef TRACK
   sprintf(functionin,"banish_player (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags &BLOCK_SU)
     {
       tell_player(p," Nope, go back on duty, doing it when off_duty is "
		   "cheating.\n");
       return;
     }

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: banish <player>\n");
      return;
   }
   sprintf(oldstack, "%s %s trying to banish %s.", p->name, isare(p), str);
   stack = end_string(oldstack);
   log("banish", oldstack);
   lower_case(str);
   p2 = find_player_absolute_quiet(str);
   if (!p2)
      tell_player(p, " No such person on the program.\n");
   if (p2)
   {
      if (p->residency <= p2->residency)
      {
         tell_player(p, " You can't banish them !\n");
         sprintf(oldstack, " -=> %s tried to banish you.\n", p->name);
         stack = end_string(oldstack);
         tell_player(p2, oldstack);
         stack = oldstack;
         return;
      }
      if ( p2->residency == 0 )
         newbie=1;
      sprintf(oldstack, "\n\n -=> You have been banished !!!.\n\n\n");
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      p2->saved_residency |= BANISHD;
      p2->residency = p2->saved_residency;
      quit(p2, 0);
      strcpy(ban_name, p2->name);
   }
   if (!newbie)
   {
      strcpy(oldstack, str);
      lower_case(oldstack);
      stack = end_string(oldstack);
      sp = find_saved_player(oldstack);
      if (sp)
      {
         if (sp->residency & BANISHD)
         {
            tell_player(p," Already banished!\n");
            stack = oldstack;
            return;
         }
         if ( sp->residency >= p->residency )
         {
            tell_player(p, " You can't banish that save file !\n");
            stack = oldstack;
            return;
         }
         sp->residency |= BANISHD;
         set_update(*str);
         tell_player(p, " Player successfully banished.\n");
      } else
      {
      /* Create a new file with the BANISHD flag set */
         i = str;
         while (*i)
         {
            if (!isalpha(*i++))
            {
               tell_player(p, " Banished names must only contain letters!\n");
               return;
            }
         }
         create_banish_file(str);
         tell_player(p, " Name successfully banished.\n");
      }
      if (ban_name[0] == '\0')
      {
         sprintf(ban_name, "\'%s\'", str);
      }
   }
   if (ban_name[0] != '\0')
   {
      stack = oldstack;
      if (p->gender==PLURAL)
	sprintf(stack, " -=> %s banish %s.\n", p->name, ban_name);
      else
	sprintf(stack, " -=> %s banishes %s.\n", p->name, ban_name);
      stack = end_string(stack);
      su_wall(oldstack);
   }
   stack = oldstack;
}


/* Unbanish a player or name */

void unbanish(player *p, char *str)
{
   saved_player *sp;
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"unbanish (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags &BLOCK_SU)
     {
       tell_player(p," Ahem. Look at your flags. You are off_duty. Get back "
		   "on_duty if you wanna do that.\n");
       return;
     }

   oldstack = stack;
   lower_case(str);
   sp = find_saved_player(str);
   if (!sp)
   {
      tell_player(p, " Can't find saved player file for that name.\n");
      return;
   }
   if ( !(sp->residency & BANISHD) )
   {
      tell_player(p, " That player isn't banished!\n");
      return;
   }
   if ( sp->residency == BANISHD || sp->residency == BANISHED )
   {
      remove_player_file(str);
      if (p->gender==PLURAL)
	sprintf(stack, " -=> %s unbanish the Name \'%s\'\n", p->name, str);
      else
	sprintf(stack, " -=> %s unbanishes the Name \'%s\'\n", p->name, str);
      stack = end_string(stack);
      su_wall(oldstack);
      stack = oldstack;
      return;
   }
   sp->residency &= ~BANISHD;
   set_update(*str);
   sync_to_file(str[0], 0);
   sprintf(stack, " -=> %s unbanishes the Player \'%s\'\n", p->name, str);
   stack = end_string(stack);
   su_wall(oldstack);
   log("banish",oldstack);
   stack = oldstack;
}


/* create a new character */

void make_new_character(player * p, char *str)
{
   char *oldstack, *cpy, *email, *password=0;
   player *np;
   int length = 0;

#ifdef TRACK
   sprintf(functionin,"make_new_character (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags &BLOCK_SU)
     {
       tell_player(p," on_duty first please.\n");
       return;
     }

   oldstack = stack;
   email = next_space(str);

   if (!*str || !*email)
   {
      tell_player(p, " Format: make <character name> <email addr> "
                     "<password>\n");
      return;
   }

/* chop the argument into "name\000email\000password\000" with ptrs as
   appropriate */
   *email++ = 0;

   password = end_string(email);
   while (*password != ' ')
      *password--;
   *password++ = 0;

   for (cpy = str; *cpy; cpy++)
   {
      if (isalpha(*cpy))
      {
         *stack++ = *cpy;
         length++;
      }
   }
   *stack++ = 0;
   if (length > (MAX_NAME - 2))
   {
      tell_player(p, " Name too long.\n");
      stack = oldstack;
      return;
   }
   if (find_saved_player(oldstack))
   {
      tell_player(p, " That player already exists.\n");
      stack = oldstack;
      return;
   }
   np = create_player();
   np->flags &= ~SCRIPTING;
   strcpy(np->script_file, "dummy");
   np->fd = p->fd;
   np->location = (room *) -1;

   restore_player(np, oldstack);
   np->flags &= ~SCRIPTING;
   strcpy(np->script_file, "dummy");
   strcpy (np->inet_addr, "NOT YET LOGGED ON");
   np->residency = get_flag(permission_list, "residency");
   np->saved_residency = np->residency;

   /* Crypt that password, why don't you */

   strcpy(np->password, do_crypt(password, np));

   /* strncpy(np->password,oldstack,(MAX_PASSWORD-2)); */

   strncpy(np->email, email, (MAX_EMAIL - 2));
   save_player(np);
   np->fd = 0;
   np->location = 0;
   destroy_player(np);
   cpy = stack;
   sprintf(cpy, "%s creates %s.", p->name, oldstack);
   stack = end_string(cpy);
   log("make", cpy);
   tell_player(p, " Player created.\n");
   stack = oldstack;
   return;
}


/* port from EW dump file */

void port(player * p, char *str)
{
   char *oldstack, *scan;
   player *np;
   file old;

#ifdef TRACK
   sprintf(functionin,"port (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   old = load_file("files/old.players");
   scan = old.where;

   while (old.length > 0)
   {
      while (*scan != ' ')
      {
         *stack++ = *scan++;
         old.length--;
      }
      scan++;
      *stack++ = 0;
      strcpy(stack, oldstack);
      lower_case(stack);
      if (!find_saved_player(stack))
      {
         np = create_player();
         np->fd = p->fd;
         restore_player(np, oldstack);
         np->residency = get_flag(permission_list, "residency");
         stack = oldstack;
         while (*scan != ' ')
         {
            *stack++ = *scan++;
            old.length--;
         }
         *stack++ = 0;
         scan++;
         strncpy(np->password, oldstack, MAX_PASSWORD - 2);
         stack = oldstack;
         while (*scan != '\n')
         {
            *stack++ = *scan++;
            old.length--;
         }
         *stack++ = 0;
         scan++;
         strncpy(np->email, oldstack, MAX_EMAIL - 2);
         sprintf(oldstack, "%s [%s] %s\n", np->name, np->password, np->email);
         stack = end_string(oldstack);
         tell_player(p, oldstack);
         stack = oldstack;
         save_player(np);
         np->fd = 0;
         destroy_player(np);
      } else
      {
         while (*scan != '\n')
         {
            scan++;
            old.length--;
         }
         scan++;
      }
   }
   if (old.where)
      FREE(old.where);
   stack = oldstack;
}


/* List the Super Users who're on */

void lsu(player * p, char *str)
{
   int count = 0;
   char *oldstack, *prestack;
   player *scan;

#ifdef TRACK
   sprintf(functionin,"lsu (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   strcpy(stack, "---------------------------------- Supers on "
                 "----------------------------------\n");
   stack = strchr(stack, 0);
   for (scan = flatlist_start; scan; scan = scan->flat_next)
   {
      prestack = stack;
      if (scan->residency & PSU && scan->location)
      {
         if ( (scan->flags & BLOCK_SU) && !(p->residency & PSU) )
            continue;
         count++;
         *stack = ' ';
         stack++;
         sprintf(stack, "%-20s", scan->name);
         stack = strchr(stack, 0);
         if (scan->saved_residency & ADMIN)
            strcpy(stack, "< Admin >       ");
         else if (scan->saved_residency & LOWER_ADMIN)
            strcpy(stack, "< Lower Admin > ");
         else if (scan->saved_residency & SU)
            strcpy(stack, "< Super User >  ");
/* It's a horrible kludge I know... */
         else if (!(p->residency & PSU))
         {
            count--;
            stack = prestack;
            continue;
         } else if (scan->saved_residency & PSU)
            strcpy(stack, "< Pseudo SU >   ");
         stack = strchr(stack, 0);
   
         if (scan->flags & BLOCK_SU)
         {
            strcpy(stack, "      Off Duty atm.");
            stack = strchr(stack, 0);
         }
         *stack++ = '\n';
      }
   }
   if (count > 1)
      sprintf(stack, "--------------------- There are %2d Super Users connected"
                     " ----------------------\n", count);
   else if (count == 1)
      sprintf(stack, "---------------------- There is one Super User connected "
                     "----------------------\n", count);
   else
      sprintf(stack, "--------------------- There are no Super Users connected "
                     "----------------------\n");
   stack = end_string(stack);
   tell_player(p, oldstack);
   stack = oldstack;
}


/* List the Newbies that're on */

void lnew(player * p, char *str)
{
   char *oldstack;
   int count = 0;
   player *scan;

#ifdef TRACK
   sprintf(functionin,"lnew (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   command_type = EVERYONE;
   strcpy(stack, "---------------------------------- Newbies on "
                 "---------------------------------\n");
   stack = strchr(stack, 0);
   for (scan = flatlist_start; scan; scan = scan->flat_next)
   {
      if (scan->residency == NON_RESIDENT && scan->location)
      {
         count++;
         sprintf(stack, "%-20s ", scan->name);
         stack = strchr(stack, 0);
         sprintf(stack, "%-40s ", scan->inet_addr);
         stack = strchr(stack, 0);
         if (scan->assisted_by[0] != '\0')
         {
            sprintf(stack, "[%s]", scan->assisted_by);
            stack = strchr(stack, 0);
         }
         *stack++ = '\n';
      }  
   }

   if (count > 1)
      sprintf(stack, "------------------------ There are %2d Newbies connected "
                     "-----------------------\n", count);
   else if (count == 1)
      sprintf(stack, "------------------------ There is one Newbie connected "
                     "------------------------\n", count);
   else
      sprintf(stack, "----------------------- There are no Newbies connected "
                     "------------------------\n");
   stack = end_string(stack);

   if (count == 0)
      tell_player(p, " No newbies on at the moment.\n");
   else
      tell_player(p, oldstack);
   stack = oldstack;
}


/*
 * rename a person (yeah, right, like this is going to work .... )
 * 
 */

void do_rename(player * p, char *str, int verbose)
{
   char *oldstack, *firspace, name[MAX_NAME + 2], *letter, *oldlist;
   char oldname[MAX_NAME+2];
   int *oldmail;
   int hash;
   player *oldp, *scan, *previous;
   saved_player *sp,*oldsp;
   room *oldroom,*rscan;

#ifdef TRACK
   sprintf(functionin,"do_rename (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," It would be in the best interests of humanity in "
		   "general, if you were to go on_duty to this mightily "
		   "important task.\n");
	 return;
     }

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: rename <person> <new-name>\n");
      return;
   }
   if (!(firspace = strchr(str, ' ')))
      return;
   *firspace = 0;
   firspace++;
   letter = firspace;
   if (!(oldp = find_player_global(str)))
      return;
   if (oldp->residency & BASE)
   {
      sprintf(stack, " But you cannot rename %s. They are a resident.\n"
	      , oldp->name);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      return;
   }
/*   if (oldp->residency > p->residency)
   {
      sprintf(stack, " But you cannot rename %s. They have more privs than "
	      "you.\n", oldp->name);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      return;
   }*/
   strcpy(oldname,oldp->lower_name);
   scan = find_player_global_quiet(firspace);
   if (scan)
   {
      sprintf(stack, " There is already a person with the name '%s' "
                     "logged on.\n", scan->name);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      return;
   }
   strcpy(name, firspace);
   lower_case(name);
   sp = find_saved_player(name);
   if (sp)
   {
      sprintf(stack, " There is already a person with the name '%s' "
                     "in the player files.\n", sp->lower_name);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      return;
   }
   /* Test for a nice inputted name */

   if (strlen(letter) > MAX_NAME - 2 || strlen(letter) < 2)
   {
      tell_player(p, " Try picking a name of a decent length.\n");
      stack = oldstack;
      return;
   }
   while (*letter)
   {
      if (!isalpha(*letter))
      {
         tell_player(p, " Letters in names only, please ...\n");
         stack = oldstack;
         return;
      }
      *letter++;
   }

   /* right, newname doesn't exist then, safe to make a change (I hope) */
   /* Remove oldp from hash list */

   scan = hashlist[oldp->hash_top];
   previous = 0;
   while (scan && scan != oldp)
   {
      previous = scan;
      scan = scan->hash_next;
   }
   if (!scan)
      log("error", "Bad hash list (rename)");
   else if (!previous)
      hashlist[oldp->hash_top] = oldp->hash_next;
   else
      previous->hash_next = oldp->hash_next;

   strcpy(name, oldp->lower_name);
   strncpy(oldp->name, firspace, MAX_NAME - 2);
   lower_case(firspace);
   strncpy(oldp->lower_name, firspace, MAX_NAME - 2);

   /* now place oldp back into named hashed lists */

   hash = ((int) (oldp->lower_name[0]) - (int) 'a' + 1);
   oldp->hash_next = hashlist[hash];
   hashlist[hash] = oldp;
   oldp->hash_top = hash;

   /*This section ONLY if they are a resident*/
   if (oldp->residency & BASE)
     {
       /*Change the lower_name in the saved player area*/
       strcpy(oldp->saved->lower_name,oldp->lower_name);
       
       /*Find all the rooms, and then transfer them*/

       /*Get the rooms info from the OLD playerfile*/
       /*And reset the owner of them*/
       if (oldsp)
	 for (rscan=oldsp->rooms;rscan;rscan=rscan->next)
	   rscan->owner=oldp->saved;
     }

   if (oldp->saved)
     save_player(oldp);
   stack = oldstack;
   if (verbose)
   {
      sprintf(stack, " %s dissolves in front of your eyes, and "
                     "rematerialises as %s ...\n", name, oldp->name);
      stack = end_string(stack);

      /* tell room */
      scan = oldp->location->players_top;
      while (scan)
      {
        if (scan != oldp && scan != p)
           tell_player(scan, oldstack);
        scan = scan->room_next;
      }
      stack = oldstack;
      sprintf(stack, "\n -=> %s %s just changed your name to be '%s' ...\n\n",
         p->name, havehas(p), oldp->name);
      stack = end_string(stack);
      tell_player(oldp, oldstack);
   }
   tell_player(p, " Tis done ...\n");
   stack = oldstack;

   /* log it */
   sprintf(stack, "Rename by %s - %s to %s", p->name, name, oldp->name);
   stack = end_string(stack);
   log("rename", oldstack);
   stack = oldstack;
   if (p->gender==PLURAL)
     sprintf(stack, " -=> %s rename %s to %s.\n", p->name, name, oldp->name);
   else
     sprintf(stack, " -=> %s renames %s to %s.\n", p->name, name, oldp->name);
   stack = end_string(stack);
   su_wall(oldstack);
   stack = oldstack;
}


/* User interface to renaming a newbie */

void rename_player(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"rename_player (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   do_rename(p, str, 1);
}

void quiet_rename(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"quiet_rename (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," It would be in the best interests of humanity in "
		   "general, if you were to go on_duty to this mightily "
		   "important task.\n");
	 return;
     }

   do_rename(p, str, 0);
}


/* For an SU to go back on duty */

void on_duty(player * p, char *str)
{
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"on_duty (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if ((p->flags & BLOCK_SU) != 0)
   {
      p->flags &= ~BLOCK_SU;
      tell_player(p, " You return to duty.\n");
      p->residency = p->saved_residency;
      oldstack = stack;
      if (p->gender==PLURAL)
	sprintf(stack, " -=> %s return to duty.", p->name);
      else
	sprintf(stack, " -=> %s returns to duty.", p->name);
      stack = end_string(stack);
      log("duty", oldstack);
      strcat(oldstack, "\n");
      *stack++;
      su_wall_but(p, oldstack);
      stack = oldstack;
   } else
   {
      tell_player(p, " Are you asleep or something? You are ALREADY On Duty!"
                     " <smirk>\n");
   }
}


/* For an SU to go off duty */

void block_su(player * p, char *str)
{
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"block_su (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if ((p->flags & BLOCK_SU) == 0)
   {
      p->flags |= BLOCK_SU;
      tell_player(p, " You're now off duty ... The rest of the sarcastic "
		  "message is removed, cos it was written by chris, and we "
		  "dont wanna take chances with chris code {:-)\n");
      /*I mean, why though,"
	" what the hell is the point of being a superuser if"
	" you're going to go off duty the whole time?\n");*/

      p->saved_residency = p->residency;
      if (p->gender==PLURAL)
	sprintf(stack, " -=> The %s all go off duty.", p->name);
      else
	sprintf(stack, " -=> %s goes off duty.", p->name);
      stack = end_string(stack);
      log("duty", oldstack);
      strcat(oldstack, "\n");
      *stack++;
      su_wall_but(p, oldstack);
      stack = oldstack;
   } else
   {
      tell_player(p, " But you are ALREADY Off Duty! <boggle>\n");
   }
}


/* help for superusers */

void super_help(player * p, char *str)
{
   char *oldstack;
   file help;

#ifdef TRACK
   sprintf(functionin,"super_help (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (!*str || (!strcasecmp(str, "admin") && !(p->residency & ADMIN)))
   {
      tell_player(p, " SuperUser help files that you can read are: basic, "
                     "advanced.\n");
      return;
   }
   if (*str == '.')
   {
      tell_player(p, " Uh-uh, cant do that ...\n");
      return;
   }
   sprintf(stack, "doc/%s.doc", str);
   stack = end_string(stack);
   help = load_file_verbose(oldstack, 0);
   if (help.where)
   {
      if (*(help.where))
      {
         if (p->saved_flags & NO_PAGER)
            tell_player(p, help.where);
         else
            pager(p, help.where, 1);
      } else
      {
         tell_player(p, " Couldn't find that help file ...\n");
      }
      FREE(help.where);
   }
   stack = oldstack;
}


/* assist command */

void assist_player(player * p, char *str)
{
   char *oldstack, *comment;
   player *p2, *p3;

#ifdef TRACK
   sprintf(functionin,"assist_player (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," Go on_duty first.\n");
       return;
     }

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: assist <person>\n");
      return;
   }
   if (!strcasecmp(str, "me"))
   {
      p2 = p;
   } else
   {
      p2 = find_player_global(str);
      if (!p2)
         return;
   }
   if (p != p2)
   {
      if (p2->residency != NON_RESIDENT)
      {
         tell_player(p, " That person isn't a newbie though ...\n");
         return;
      }
   }
   if (p2->flags & ASSISTED)
   {
      p3 = find_player_absolute_quiet(p2->assisted_by);
      if (p3)
      {
         if (p != p3)
         {
            sprintf(stack, " That person is already assisted by %s.\n",
                    p2->assisted_by);
         } else
         {
            sprintf(stack, " That person has already been assisted by %s."
                           " Oh! That's you that is! *smirk*\n", p2->assisted_by);
         }
         stack = end_string(stack);
         tell_player(p, oldstack);
         stack = oldstack;
         return;
      }
   }
   if (p!=p2)
   {
      p2->flags |= ASSISTED;
      strcpy(p2->assisted_by, p->name);
   }
   oldstack = stack;
   if (p->gender == PLURAL)
     sprintf(stack, "\n -=> %s are superusers, and would be more than "
	     "happy to assist you in any problems you may have (including "
	     "gaining residency, type 'help residency' to find out more "
	     "about that).  To talk to %s, type 'tell %s <whatever>\', "
	     "short forms of names usually work as well.\n\n", p->name,
	     get_gender_string(p), p->lower_name);
   else
     sprintf(stack, "\n -=> %s is a superuser, and would be more than "
	     "happy to assist you in any problems you may have (including "
	     "gaining residency, type 'help residency' to find out more "
	     "about that).  To talk to %s, type 'tell %s <whatever>\', "
	     "short forms of names usually work as well.\n\n", p->name,
	     get_gender_string(p), p->lower_name);
   
   stack = end_string(stack);
   tell_player(p2, oldstack);
   stack = oldstack;
   if (p!=p2)
   {
      sprintf(stack, " -=> %s assists %s.\n", p->name, p2->name);
      stack = end_string(stack);
      p->flags |= NO_SU_WALL;
      su_wall(oldstack);
      stack = oldstack;
      sprintf(stack, " You assist %s.\n", p2->name);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      sprintf(stack, "%s assists %s", p->name, p2->name);
      stack = end_string(stack);
      log("resident", oldstack);
      stack = oldstack;
   }
}


/* Confirm if password and email are set on a resident */

void confirm_password(player * p, char *str)
{
   char *oldstack;
   player *p2;

#ifdef TRACK
   sprintf(functionin,"confirm_password (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: confirm <name>\n");
      return;
   }
   p2 = find_player_global(str);
   if (!p2)
      return;

   if (p2->residency == NON_RESIDENT)
   {
      tell_player(p, " That person is not a resident.\n");
      return;
   }
   oldstack = stack;

   p2->residency |= NO_SYNC;
   /* check email */
   if (p2->email[0] == 2)
   {
      strcpy(stack, " Email has not yet been set.");
   } else if (p2->email[0] == ' ')
   {
      strcpy(stack, " Email validated set.");
      p2->residency &= ~NO_SYNC;
   } else if (!strstr(p2->email, "@") || !strstr(p2->email, "."))
   {
      strcpy(stack, " Probably not a correct email.");
      p2->residency &= ~ NO_SYNC;
   } else
   {
      strcpy(stack, " Email set.");
      p2->residency &= ~ NO_SYNC;
   }
   stack = strchr(stack, 0);

   if (p2->email[0] != 2 && p2->email[0] != ' ')
   {
      if (p->residency & ADMIN || !(p2->saved_flags & PRIVATE_EMAIL))
      {
         sprintf(stack, " - %s", p2->email);
         stack = strchr(stack, 0);
         if (p2->saved_flags & PRIVATE_EMAIL)
         {
            strcpy(stack, " (private)\n");
         } else
         {
            strcpy(stack, "\n");
         }
      } else 
      {
         strcpy(stack, "\n");
      }
   } else
   {
      strcpy(stack, "\n");
   }
   stack = strchr(stack, 0);

   /* password */
   if (p2->password[0] && p2->password[0] != -1)
   {
      strcpy(stack, " Password set.\n");
   } else
   {
      strcpy(stack, " Password NOT-set.\n");
      p2->residency |= NO_SYNC;
   }
   stack = strchr(stack, 0);

   if (p2->residency & NO_SYNC)
      sprintf(stack, " Character '%s' won't be saved.\n", p2->name);
   else
      sprintf(stack, " Character '%s' will be saved.\n", p2->name);
   stack = end_string(stack);
   tell_player(p, oldstack);
   stack = oldstack;
}

/* reset email of a player */
/* this version even manages to check if they are logged in at the time :-/ */
/* leave the old one in a little while until we are sure this works */

void blank_email(player * p, char *str)
{
   player dummy;
   player *p2;
   char *space, *oldstack;

#ifdef TRACK
   sprintf(functionin,"blank_email (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," Go on_duty first.\n");
       return;
     }

   /* we need the stack for printing some stuff */
   oldstack = stack;

   /* spot incorrect syntax */
   if (!*str)
   {
      tell_player(p, " Format: blank_email <player> [<email>]\n");
      return;
   }

   /* spot lack of sensible email address */
   space = 0;
   space = strchr(str, ' ');
   if (space != NULL)
   {
      *space++ = 0;
      if (strlen(space) < 7)
      {
         tell_player(p, " Try a reasonable email address.\n");
         return;
      }
   }

   /* look for them on the prog */
   lower_case(str);
   p2 = find_player_absolute_quiet(str);

   /* if player logged in */
   if (p2)
   {
       /* no blanking the emails of superiors... */
       if ((p2->residency >= p->residency) && !(p->residency & HCADMIN))
	   /* naughty, naughty, so tell the person, the target, and the
	      su channel */
       {
	   tell_player(p, " You cannot blank that person's email address.\n");
	   sprintf(stack, " -=> %s tried to blank your email address, but "
		   "failed.\n", p->name);
	   stack = end_string(stack);
	   tell_player(p2, oldstack);
	   stack = oldstack;
	   sprintf(stack, " -=> %s failed in an attempt to blank the email "
		   "address of %s.\n", p->name, p2->name);
	   stack = end_string(stack);
	   su_wall_but(p, oldstack);
	   stack = oldstack;
	   return;
       }
       else
	   /* p is allowed to do things to p2 */
       {
	   /* tell the target and the SUs all about it */
	   if (space == NULL)
	       sprintf(stack, " -=> Your email address has been blanked "
		       "by %s.\n", p->name);
	   else
	       sprintf(stack, " -=> Your email address has been changed "
		       "by %s.\n", p->name);	       
	   stack = end_string(stack);
	   tell_player(p2, oldstack);
	   stack = oldstack;
	   if (space == NULL)
	       sprintf(stack, " -=> %s %s their email blanked by %s.\n",
		       p2->name, havehas(p2), p->name);
	   else
	       sprintf(stack, " -=> %s %s their email changed by %s.\n",
		       p2->name, havehas(p2), p->name);
	   stack = end_string(stack);
	   su_wall_but(p, oldstack);
	   stack = oldstack;

	   /* actually blank it, and flag the player for update */
	   /* and avoid strcpy from NULL since it's very dodgy */
	   if (space != NULL)
	       strcpy(p2->email, space);
	   else
	       p2->email[0] = 0;
	   set_update(*str);

	   /* report success to the player */
	   if (space == NULL)
	       sprintf(stack, " -=> You successfully blank %s's email.\n", 
		       p2->name);
	   else
	       sprintf(stack, " -=> You successfully change %s's email.\n", 
		       p2->name);
	   stack = end_string(stack);
	   tell_player(p, oldstack);
	   stack = oldstack;
	   /* log the change */
	   if (space == NULL)
	       sprintf(stack, "%s blanked %s's email address (logged in)",
		       p->name, p2->name);
	   else
	       sprintf(stack, "%s changed %s's email address (logged in)",
		       p->name, p2->name);
	   stack = end_string(stack);
	   log("blanks", oldstack);
	   return;
       }
   }
   else
       /* they are not logged in, so load them */
       /* set up the name and port first */
   {
       strcpy(dummy.lower_name, str);
       dummy.fd = p->fd;
       if (load_player(&dummy))
       {
	   /* might as well point this out if it is so */
 	   if (dummy.residency & BANISHD)
	   {
	       tell_player(p, " By the way, this player is currently BANISHD.");
	       if (dummy.residency == BANISHD)
	       {
		   tell_player(p, " (Name Only)\n");
	       } else
	       {
		   tell_player(p, "\n");
	       }
	   }
	   /* announce to the SU channel */
	   if (space == NULL)
	       sprintf(stack, " -=> %s blanks the email of %s, who is "
		       "logged out at the moment.\n", p->name, dummy.name);
	   else
	       sprintf(stack, " -=> %s changes the email of %s, who is "
		       "logged out at the moment.\n", p->name, dummy.name);
	   stack = end_string(stack);
	   su_wall_but(p, oldstack);
	   stack = oldstack;
	   /* change or blank the email address */
	   if (space == NULL)
	       dummy.email[0] = 0;
	   else
	       strcpy(dummy.email, space);

	   /* report success to player */
	   if (space == NULL)
	       sprintf(stack, " -=> Successfully blanked the email of %s, "
		       "not logged in atm.\n", dummy.name);
	   else
	       sprintf(stack, " -=> Successfully changed the email of %s, "
		       "not logged in atm.\n", dummy.name);
	   stack = end_string(stack);
	   tell_player(p, oldstack);
	   stack = oldstack;

	   /* and log it */
	   if (space == NULL)
	       sprintf(stack, "%s blanked %s's email address (logged out)",
		       p->name, dummy.name);
	   else
	       sprintf(stack, "%s changed %s's email address (logged out)",
		       p->name, dummy.name);
	   stack = end_string(stack);
	   log("blanks", oldstack);
	   stack = oldstack;

	   /* save char LAST thing so maybe we won't blancmange the files */
	   dummy.script = 0;
	   dummy.script_file[0] = 0;
	   dummy.flags &= ~SCRIPTING;
	   dummy.location = (room *) -1;
	   save_player(&dummy);
	   return;
       }
       else
	   /* name does not exist, tell the person so and return */
       {
	   sprintf(stack, " -=> The name '%s' was not found in saved files.\n",
		   dummy.name);
	   stack = end_string(stack);
	   tell_player(p, oldstack);
	   stack = oldstack;
	   return;
       }
   }
}


/* Now this is just plain silly, probly worst than 'crash' */

void hang(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"hang (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   while (1)
      sleep(1);
}


/* The almighty frog command!!!! */

void frog(player *p, char *str)
{
   char *oldstack;
   player *d;

#ifdef TRACK
   sprintf(functionin,"frog (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," Go on_duty first. Next time you do this when off_duty "
		   "it will affect YOU instead!!\n");
       return;
     }

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: frog <player>\n");
      return;
   }
   if (!strcasecmp(str, "me"))
   {
      tell_player(p, " Do you REALLY want to frog yourself?\n");
      return;
   }
   d = find_player_global(str);
   if (d)
   {
      if (d == p)
      {
         tell_player(p, " Do you REALLY want to frog yourself?\n");
         return;
      }
      if (d->flags & FROGGED)
      {
         tell_player(p, " That player is ALREADY frogged!\n");
         return;
      }

      if (d->residency >= p->residency)
      {
         tell_player(p, " You can't do that!\n");
         sprintf(stack, " -=> %s tried to frog you!\n", p->name);
         stack = end_string(oldstack);
         tell_player(d, oldstack);
         stack = oldstack;
         return;
      }
      d->flags |= FROGGED;
      d->saved_flags |= SAVEDFROGGED;
      sprintf(stack, " You frog %s!\n", d->name);
      stack = end_string(oldstack);
      tell_player(p, oldstack);
      stack = oldstack;
      sprintf(stack, " -=> %s turn%s you into a frog!\n", p->name,single_s(p));
      stack = end_string(stack);
      tell_player(d, oldstack);
      stack = oldstack;
      sprintf(oldstack, " -=> %s turn%s %s into a frog!\n", p->name,
	      single_s(p), d->name);
      stack = end_string(oldstack);
      su_wall_but(p, oldstack);
      log("frog",oldstack);
      stack = oldstack;
   }
}


/* Well, I s'pose we'd better have this too */

void unfrog(player *p, char *str)
{
   char *oldstack;
   player *d;
   saved_player *sp;

#ifdef TRACK
   sprintf(functionin,"unfrog (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," Go on_duty first. {:-P\n");
       return;
     }

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: unfrog <player>\n");
      return;
   }
   d = find_player_global(str);
   if (d)
   {
      if (d == p)
      {
         if (p->flags & FROGGED)
         {
            tell_player(p, " You can't, you spoon!\n");
	    if (p->gender==PLURAL)
	      sprintf(stack, " -=> %s try to unfrog %s...\n", p->name,
		      self_string(p));
	    else
	      sprintf(stack, " -=> %s tries to unfrog %s...\n", p->name,
		      self_string(p));
            stack = end_string(oldstack);
            su_wall_but(p, oldstack);
            stack = oldstack;
         } else
            tell_player(p, " But you're not frogged...\n");
         return;
      }
      if (!(d->flags & FROGGED))
      {
          tell_player(p, " That person isn't a frog...\n");
          return;
      }
      d->flags &= ~FROGGED;
      d->saved_flags &= ~SAVEDFROGGED;
      if (p->gender==PLURAL)
	sprintf(stack, " -=> The %s all kiss you and you are no longer a "
		"frog.\n",p->name);
      else
	sprintf(stack, " -=> %s kisses you and you are no longer a frog.\n",
		p->name);
      stack = end_string(oldstack);
      tell_player(d, oldstack);
      stack = oldstack;
      sprintf(stack, " You kiss %s and %s %s no longer a frog.\n", d->name,
              gstring(d),isare(d));
      stack = end_string(oldstack);
      tell_player(p, oldstack);
      stack = oldstack;
      if (p->gender==PLURAL)
	sprintf(stack, " -=> The %s all kiss %s, and %s %s no longer a "
		"frog.\n",p->name, d->name, gstring(d), isare(d));
      else
	sprintf(stack, " -=> %s kisses %s, and %s %s no longer a frog.\n",
		p->name, d->name, gstring(d), isare(d));
      stack = end_string(oldstack);
      su_wall_but(p, oldstack);
      log("frog",oldstack);
      stack = oldstack;
   } else
   {
      tell_player(p, " Checking saved files...\n");
      sp = find_saved_player(str);
      if (!sp)
      {
         tell_player(p, " Not found.\n");
         return;
      }
      if (!(sp->saved_flags & SAVEDFROGGED))
      {
         tell_player(p, " But that person isn't a frog...\n");
         return;
      }
      sp->saved_flags &= ~SAVEDFROGGED;
      sprintf(stack, " Ok, %s is no longer a frog.\n", sp->lower_name);
      stack = end_string(stack);
      tell_player(p, oldstack);
      sprintf(oldstack, " -=> %s unfrog%s %s.\n", p->name, 
	      single_s(p), sp->lower_name);
      stack = end_string(oldstack);
      su_wall_but(p, oldstack);
      stack = oldstack;
   }
}


/* unconverse, get idiots out of converse mode */

void unconverse(player *p, char *str)
{
   player *p2;
   saved_player *sp;
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"unconverse (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," Go on_duty first... Please..... Thanx.\n");
       return;
     }

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: unconverse <player>\n");
      return;
   }
   lower_case(str);
   p2 = find_player_global_quiet(str);
   if (!p2)
   {
      tell_player(p, " Player not logged on, checking saved player files...\n");
      sp = find_saved_player(str);
      if (!sp)
      {
         tell_player(p, " Can't find saved player file.\n");
         return;
      }
      if (!(sp->residency & SU) && !(sp->residency & ADMIN))
      {
         if (!(sp->saved_flags & CONVERSE))
         {
            tell_player(p, " They aren't IN converse mode!!!\n");
            return;
         }
         sp->saved_flags &= ~CONVERSE;
         set_update(*str);
         sprintf(stack, " You take \'%s' out of converse mode.\n",
                 sp->lower_name);
         stack = end_string(stack);
         tell_player(p, oldstack);
         stack = oldstack;
      } else
      {
         tell_player(p, " You can't do that to them!\n");
      }
      return;
   }
   if (!(p2->saved_flags & CONVERSE))
   {
      tell_player(p, " But they're not in converse mode!!!\n");
      return;
   }
   if (!(p2->residency & SU) && !(p2->residency & ADMIN))
   {
      p2->saved_flags &= ~CONVERSE;
      p2->mode &= ~CONV;
      if (p->gender == PLURAL)
	sprintf(stack, " -=> %s have taken you out of converse mode.\n",
		p->name);
      else
	sprintf(stack, " -=> %s has taken you out of converse mode.\n",
		p->name);
      stack = end_string(stack);
      tell_player(p2, oldstack);
      stack = oldstack;
      do_prompt(p2, p2->prompt);
      sprintf(stack, " You take %s out of converse mode.\n", p2->name);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
   } else
   {
      tell_player(p, " You can't do that to them!\n");
      sprintf(stack, " -=> %s tried to unconverse you!\n", p->name);
      stack = end_string(stack);
      tell_player(p2, oldstack);
      stack = oldstack;
   }
}

void unjail(player *p, char *str)
{
   char *oldstack;
   player *p2, dummy;

#ifdef TRACK
   sprintf(functionin,"unjail (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," Go on_duty first.\n");
       return;
     }

   if (!*str)
   {
      tell_player(p, " Format: unjail <player>\n");
      return;
   }

   if (!strcasecmp(str, "me"))
      p2 = p;
   else
      p2 = find_player_global(str);
   if (!p2)
   {
      tell_player(p, " Checking saved files... ");
      strcpy(dummy.lower_name, str);
      lower_case(dummy.lower_name);
      dummy.fd = p->fd;
      if (!load_player(&dummy))
      {
         tell_player(p, " Not found.\n");
         return;
      } else
      {
         tell_player(p, "\n");
         p2 = &dummy;
         p2->location = (room *) -1;
      }
   }
   if (p2 == p)
   {
      if (p->location == prison)
      {
         tell_player(p, " You struggle to open the door, but to no avail.\n");
	 if (p->gender == PLURAL)
	   sprintf(stack, "-=> %s try to unjail %s. *grin*\n", p->name,
		   self_string(p));
	 else
	   sprintf(stack, "-=> %s tries to unjail %s. *grin*\n", p->name,
		   self_string(p));
         stack = end_string(stack);
         su_wall_but(p, oldstack);
         stack = oldstack;
      } else
      {
         tell_player(p, " But you're not in jail!\n");
      }
      return;
   }

   if (p2 == &dummy)
   {
      if (!(p2->saved_flags & SAVEDJAIL))
      {
         tell_player(p, " Erm, how can I say this? They're not in jail...\n");
         return;
      }
   } else if (p2->jail_timeout == 0 || p2->location != prison)
   {
      tell_player(p, " Erm, how can I say this? They're not in jail...\n");
      return;
   }

   p2->jail_timeout = 0;
   p2->saved_flags &= ~SAVEDJAIL;
   if (p2 != &dummy)
   {
     if (p->gender== PLURAL)
       sprintf(stack, " -=> The %s release you from prison.\n", p->name);
     else
       sprintf(stack, " -=> %s releases you from prison.\n", p->name);
      stack = end_string(stack);
      tell_player(p2, oldstack);
      stack = oldstack;
      move_to(p2, ENTRANCE_ROOM, 0);
   }
   
   if (p->gender== PLURAL)
     sprintf(stack, " -=> The %s release %s from jail.\n", p->name, p2->name);
   else
     sprintf(stack, " -=> %s releases %s from jail.\n", p->name, p2->name);
   stack = end_string(stack);
   su_wall(oldstack);
   log("jail",oldstack);
   stack = oldstack;
   if (p2 == &dummy)
   {
      save_player(&dummy);
   }
}


/* continuous scripting of a connection */

void script(player *p, char *str)
{
   char *oldstack;
   time_t t;
   char time_string[16];

#ifdef TRACK
   sprintf(functionin,"script (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (p->flags & SCRIPTING)
   {
      if (!*str)
      {
         tell_player(p, " You are ALREADY scripting! ('script off' to turn"
                        " current scripting off)\n");
      }
      if (!strcasecmp(str, "off"))
      {
         p->flags &= ~SCRIPTING;
         sprintf(stack, " -=> Scripting stopped at %s\n",
                 convert_time(time(0)));
         stack = end_string(stack);
         tell_player(p, oldstack);
         *(p->script_file)=0;
         stack = oldstack;
         sprintf(stack, " -=> %s has stopped continuous scripting.\n", p->name);
         stack = end_string(stack);
         su_wall(oldstack);
      }
      stack = oldstack;
      return;
   }

   if (!*str)
   {
      tell_player(p, " You must give a reason for starting scripting.\n");
      return;
   }
   p->flags |= SCRIPTING;
   sprintf(stack, " -=> Scripting started at %s, for reason \'%s\'\n",
           convert_time(time(0)), str);
   stack = end_string(stack);
   tell_player(p, oldstack);
   stack = oldstack;
   sprintf(stack, " -=> %s has started continuous scripting with reason "
                  "\'%s\'\n"
           , p->name, str);
   stack = end_string(stack);
   su_wall(oldstack);
   stack = oldstack;
   t = time(0);
   strftime(stack, 16, "%d%m%y%H%M%S", localtime(&t));
   stack = end_string(stack);
   sprintf(p->script_file, "%s%s", p->name, oldstack);
   stack = oldstack;
   sprintf(stack, "logs/scripts/%s.log", p->script_file);
   stack = end_string(stack);
   unlink(oldstack);
   stack = oldstack;
}

/* cut down version of lsu() to just return number of SUs on */
int count_su()
{
  int count=0;
  player *scan;
 
  for (scan=flatlist_start;scan;scan=scan->flat_next)
    if (scan->residency&PSU && scan->location)
      count++;
 
  return count;
}
 
/* cut down version of lnew() to just return number of newbies on */
int count_newbies()
{
  int count=0;
  player *scan;

  for (scan=flatlist_start;scan;scan=scan->flat_next)
    if (scan->residency==NON_RESIDENT && scan->location)
      count++;
 
  return count;
}
 
/*
   Now that we know:
   the number of SUs,
   the number of newbies,
   and the number of ppl on the prog (current_players),
   we can output some stats
   */
 
void player_stats(player *p, char *str)
{
  char *oldstack;

#ifdef TRACK
   sprintf(functionin,"player_stats (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

  oldstack=stack;
  tell_player(p,"Current Program/Player stats:\n");
  sprintf(oldstack," Players on program: %3d\n"
          "      Newbies on   : %3d\n"
          "      Supers on    : %3d\n"
          "      Normal res.  : %3d\n\n",
          current_players,
          count_newbies(),
          count_su(),
          (current_players-(count_su()+count_newbies())));
  stack=strchr(stack,0);
  *stack++=0;
  tell_player(p,oldstack);
  stack=oldstack;
}

/* Go to the SUs study */

void go_comfy(player *p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"go_comfy (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   command_type |= ADMIN_BARGE;
   if (p->location == comfy)
   {
      tell_player(p, " You're already in the study!\n");
      return;
   }
   if (p->no_move)
   {
      tell_player(p, " You seem to be stuck to the ground.\n");
      return;
   }
   move_to(p, "summink.comfy", 0);
}

/* Tell you what mode someone is in */

void mode(player *p, char *str)
{
   player *p2;
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"mode(%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: mode <player>\n");
      return;
   }

   p2 = find_player_global(str);
   if (!p2)
      return;

   if (p2->mode == NONE)
   {
      sprintf(stack, " %s is in no particular mode.\n", p2->name);
   } else if (p2->mode & PASSWORD)
   {
      sprintf(stack, " %s is in Password Mode.\n", p2->name);
   } else if (p2->mode & ROOMEDIT)
   {
      sprintf(stack, " %s is in Room Mode.\n", p2->name);
   } else if (p2->mode & MAILEDIT)
   {
      sprintf(stack, " %s is in Mail Mode.\n", p2->name);
   } else if (p2->mode & NEWSEDIT)
   {
      sprintf(stack, " %s is in News Mode.\n", p2->name);
   } else if (p2->mode & CONV)
   {
      sprintf(stack, " %s is in Converse Mode.\n", p2->name);
   } else
   {
      sprintf(stack, " Ermmm, %s doesn't appear to be in any mode at all.\n",
                 p2->name);
   }
   stack = end_string(stack);
   tell_player(p, oldstack);
   stack = oldstack;
}


/* the yoyo command! *grin* How'd I let myself get talked into this? */

void yoyo(player *p, char *str)
{
   char *oldstack;
   player *p2;

#ifdef TRACK
   sprintf(functionin,"yoyo (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," You attempt to use your yoyo, but the string gets "
		   "badly tangled and you accomplish nothing. Try again when "
		   "you are on_duty.\n");
       return;
     }

   if (!*str)
   {
      tell_player(p, " Format: yoyo <player>\n");
      return;
   }

   oldstack = stack;
   p2 = find_player_global(str);
   if (!p2)
   {
      sprintf(stack, " No-one of the name '%s' on at the moment.\n", str);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      return;
   }
   if (p2->residency >= p->residency)
   {
      sprintf(stack, " You try your best but end up nearly breaking your"
                     " finger trying to play yoyo with %s!\n", p2->name);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      sprintf(stack, " -=> %s tried to play yoyo with you!\n", p->name);
      stack = end_string(stack);
      tell_player(p2, oldstack);
      stack = oldstack;
      return;
   }
   if (p->gender==PLURAL)
     sprintf(stack, " -=> The %s all gang up and play yoyo with %s.\n", p->name, p2->name);
   else
     sprintf(stack, " -=> %s plays yoyo with %s.\n", p->name, p2->name);
   stack = end_string(stack);
   su_wall(oldstack);
   stack = oldstack;
   sprintf(stack, "%s played yoyo with %s.", p->name, p2->name);
   stack = end_string(stack);
   log("yoyo", oldstack);
   stack = oldstack;
   command_type |= ADMIN_BARGE;
   sprintf(stack, " %s %s swung out of the room on a yoyo string,"
                  " by some superuser!\n",
           p2->name, isare(p2));
   stack = end_string(stack);
   tell_room(p2->location, oldstack);
   stack = oldstack;
   trans_to(p2, "system.void");
   if (p->gender==PLURAL)
     sprintf(stack, " The %s all swing through the room on a yoyo string!\n",
	     p2->name);
   else
     sprintf(stack, " %s swings through the room on a yoyo string!\n",
	     p2->name);
   stack = end_string(stack);
   tell_room(p2->location, oldstack);
   stack = oldstack;
   trans_to(p2, "system.prison");
   if (p->gender==PLURAL)
     sprintf(stack, " The %s all swing through the room on a yoyo string!\n",
	     p2->name);
   else
     sprintf(stack, " %s swings through the room on a yoyo string!\n",
	     p2->name);
   stack = end_string(stack);
   tell_room(p2->location, oldstack);
   stack = oldstack;
   trans_to(p2, "summink.main");
   if (p->gender==PLURAL)
   sprintf(stack, " The %s all land back on the sand with a <THUMP> after an"
	   " exhausting trip.\n",p2->name);
   else
   sprintf(stack, " %s lands back on the sand with a <THUMP> after an"
	   " exhausting trip.\n",p2->name);
   stack = end_string(stack);
   tell_room(p2->location, oldstack);
   if (p->location==p2->location)
     tell_player(p,oldstack);
   stack = oldstack;
   command_type |= HIGHLIGHT;
   tell_player(p2, "  You just found out what it feels like to be a yoyo!\n");
   tell_player(p2, " If you'd rather not have it happen again, take a look"
                   " at the rules and consider following them...\n");
   command_type &= ~HIGHLIGHT;
}

void blank_prefix(player *p, char *str)
{
   char           *oldstack;
   player         *p2, dummy;

#ifdef TRACK
   sprintf(functionin,"blank_prefix (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," Go on_duty first.\n");
       return;
     }

   oldstack = stack;
   if (*str)
     {
       p2 = find_player_absolute_quiet(str);
       if (p2)
	 {
           if (p2->residency >= p->residency)
	     {
               tell_player(p, " You can't do that to THAT person.\n");
               sprintf(stack, " -=> %s tried to blank %s\'s prefix!\n",
                       p->name, p2->name);
               stack = end_string(stack);
               su_wall_but(p, oldstack);
               stack = oldstack;
               return;
	     }
           p2->pretitle[0] = 0;
           sprintf(stack, " -=> %s has blanked your prefix.\n", p->name);
           stack = end_string(stack);
           tell_player(p2, oldstack);
           stack = oldstack;
           sprintf(stack, "%s blanked %s's prefix.", p->name, p2->name);
           stack = end_string(stack);
           log("blanks", oldstack);
           tell_player(p, " Blanked.\n");
           stack = oldstack;
           return;
	 }
       strcpy(dummy.lower_name, str);
       dummy.fd = p->fd;
       if (load_player(&dummy))
	 {
           if (dummy.residency >= p->residency)
	     {
               tell_player(p, " You can't do that to THAT person.\n");
               sprintf(stack, " -=> %s tried to blank %s\'s prefix!\n",
                       p->name, dummy.name);
               stack = end_string(stack);
               su_wall_but(p, oldstack);
              stack = oldstack;
               return;
	     }
           dummy.pretitle[0] = 0;
           sprintf(stack, "%s blanked %s's prefix.", p->name, dummy.name);
           stack = end_string(stack);
           log("blanks", oldstack);
           stack = oldstack;
           dummy.location = (room *) -1;
           save_player(&dummy);
           tell_player(p, " Blanked in saved file.\n");
           return;
	 }
       else
           tell_player(p, " Can't find that person on the program or in the "
                       "files.\n");
     }
   else
       tell_player(p, " Format: blank_prefix <player>\n");
 }

void abort_shutdown(player *p,char *str)
{
  pulldown(p,"-1");

  return;
}

void echoall(player *p,char *str)
{
  char *oldstack;

  oldstack=stack;

  if (strlen(str)<1)
    {
      tell_player(p,"Usage: echoall <message>\n");
      return;
    }

  sprintf(oldstack,"%s\n",str);
  stack=end_string(oldstack);

  raw_wall(oldstack);

  stack=oldstack;

  return;
}
