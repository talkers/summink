/*
 * session.c
 */

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>


#include "config.h"
#include "player.h"
#include "fix.h"

extern char     session[];
extern int      session_reset;
extern player  *p_sess;
extern char     sess_name[];
extern char    *end_string(char *);

extern char    *word_time(int);

void            set_session(player * p, char *str)
{
   char           *oldstack;
   time_t          t;
   player         *scan;
   int             wait, yessu = 0;

   oldstack = stack;

   t = time(0);
   if (session_reset)
      wait = session_reset - (int) t;
   else
      wait = 0;
   if (wait < 0)
      wait = 0;
   if (strlen(session) == 0)
   {
      strncpy(session, "not set", MAX_SESSION - 2);
      strcpy(sess_name, "No-One");
   }
   if (!*str)
   {
      sprintf(stack, " The session is currently '%s'\n", session);
      stack = strchr(stack, 0);
      sprintf(stack, "  It was set by %s, and can be reset in %s.\n",
              sess_name, word_time(wait));
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      return;
   }
   if (wait > 0 && p != p_sess)
   {
      sprintf(stack, " Session can be reset in %s\n", word_time(wait));
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      return;
   }
   if (strlen(str) > 55)
   {
      tell_player(p, " Too long a session name ...\n");
      stack = oldstack;
      return;
   }
   strcpy(session, str);
   sprintf(stack, " You reset the session message to be '%s'\n", str);
   stack = end_string(stack);
   tell_player(p, oldstack);

   /* reset comments */
   for (scan = flatlist_start; scan; scan = scan->flat_next)
      strncpy(scan->comment, "", MAX_COMMENT - 2);

   stack = oldstack;
   sprintf(stack, "%s set%s the session to be '%s'\n", p->name,
           single_s(p), str);
   stack = end_string(stack);

   command_type |= EVERYONE;

   for (scan = flatlist_start; scan; scan = scan->flat_next)
      if (scan != p && !(scan->saved_flags & YES_SESSION))
	 tell_player(scan, oldstack);

   stack = oldstack;

   if (strcmp(sess_name, p->name) || wait <= 0)
      session_reset = t + (60 * 15);
   p_sess = p;
   strcpy(sess_name, p->name);

   sprintf(stack, "%s- %s", p->name, session);
   stack = end_string(stack);
   log("session", oldstack);
   stack = oldstack;
}



void            reset_session(player * p, char *str)
{
   char *oldstack;
   player *scan;
   time_t t;
   int wait, yessu = 0;

   oldstack = stack;
   if (*str)
   {
      strcpy(session, str);
      sprintf(stack, " You reset the session message to be '%s'\n", str);
      stack = end_string(stack);
      tell_player(p, oldstack);

      /* reset comments */
      for (scan = flatlist_start; scan; scan = scan->flat_next)
         strncpy(scan->comment, "", MAX_COMMENT - 2);
   
      stack = oldstack;
      sprintf(stack, "%s reset%s the session to be '%s'\n", p->name,
              single_s(p),str);
      stack = end_string(stack);
   
      command_type |= EVERYONE;

      for (scan = flatlist_start; scan; scan = scan->flat_next)
         if (scan != p && !(scan->saved_flags & YES_SESSION))
        	   tell_player(scan, oldstack);
   
      stack = oldstack;

      p_sess = p;
      strcpy(sess_name, p->name);
   
      sprintf(stack, "%s- %s", p->name, session);
      stack = end_string(stack);
      log("session", oldstack);
      stack = oldstack;
   }
   session_reset = 0;
   tell_player(p, " Session timer reset.\n");
}

void            set_comment(player * p, char *str)
{
   char           *oldstack;

   oldstack = stack;

   if (!*str)
   {
      tell_player(p, " You reset your session comment.\n");
      strncpy(p->comment, "", MAX_COMMENT - 2);
      return;
   }
   strncpy(p->comment, str, MAX_COMMENT - 2);
   sprintf(stack, " You set your session comment to be '%s'\n", p->comment);
   stack = end_string(stack);
   tell_player(p, oldstack);
   stack = oldstack;
}

/*
 * void view_session(player *p,char *str) { char *oldstack,middle[80]; player
 * *scan; int page,pages=0,count; oldstack=stack;
 * 
 * page=atoi(str); if (page<=0) page=1; page--;
 * 
 * comment out line        pages=(current_players-1)/(TERM_LINES-2);
 * 
 * for (scan=flatlist_start;scan;scan=scan->flat_next) if (scan->comment[0]!=0 ||
 * scan==p_sess) pages++;
 * 
 * if (page>pages) page=pages;
 * 
 * if (strlen(session)==0) strncpy(session,"not set",MAX_SESSION-2);
 * 
 * strcpy(middle,session);
 * 
 * pstack_mid(middle);
 * 
 * count=page*(TERM_LINES-2); for(scan=flatlist_start;count;scan=scan->flat_next)
 * if (!scan) { tell_player(p,"Bad who listing, abort.\n"); log("error","Bad
 * who list (session.c)"); stack=oldstack; return; } else if (scan->name[0])
 * count--; for(count=0;(count<(TERM_LINES-1) && scan);scan=scan->flat_next)
 * if ((scan->name[0] && scan->location) && (scan==p_sess ||
 * scan->comment[0]!=0)) { if (scan==p_sess) sprintf(stack,"%-20s*
 * %s\n",scan->name,scan->comment); else sprintf(stack,"%-20s-
 * %s\n",scan->name,scan->comment); stack=strchr(stack,0); count++; }
 * 
 * sprintf(middle,"Page %d of %d",page+1,pages+1); pstack_mid(middle); stack++=0;
 * if (count) tell_player(p,oldstack); else { stack=oldstack;
 * sprintf(stack,"Noone has commented on the session '%s'\n",session);
 * stack=end_string(stack); tell_player(p,oldstack); } stack=oldstack; }
 */

void            view_session(player * p, char *str)
{
   char           *oldstack, middle[80];
   player         *scan;
   int             page, pages, count;
   oldstack = stack;

   page = atoi(str);
   if (page <= 0)
      page = 1;
   page--;

   pages = (current_players - 1) / (TERM_LINES - 2);
   if (page > pages)
      page = pages;

   if (strlen(session) == 0)
      strncpy(session, "not set", MAX_SESSION - 2);

   strcpy(middle, session);

   pstack_mid(middle);

   count = page * (TERM_LINES - 2);
   for (scan = flatlist_start; count; scan = scan->flat_next)
      if (!scan)
      {
	 tell_player(p, " Bad who listing, abort.\n");
	 log("error", "Bad who list (session.c)");
	 stack = oldstack;
	 return;
      } else if (scan->name[0])
	 count--;
   for (count = 0; (count < (TERM_LINES - 1) && scan); scan = scan->flat_next)
      if (scan->name[0] && scan->location)
      {
/*
	 if (scan == p_sess)
*/
    if (!strcasecmp(sess_name, scan->lower_name))
	    sprintf(stack, "%-20s* ", scan->name);
	 else
	    sprintf(stack, "%-20s- ", scan->name);
	 stack = strchr(stack, 0);
	 strcpy(stack, scan->comment);
	 stack = strchr(stack, 0);
	 *stack++ = '\n';
	 count++;
      }
   sprintf(middle, "Page %d of %d", page + 1, pages + 1);
   pstack_mid(middle);
   *stack++ = 0;
   tell_player(p, oldstack);
   stack = oldstack;
}

/* new version of view_session, to try and show only those who have commented
   Hope this works... */

void comments(player * p, char *str)
{

  player *scan,*start;
  int pages=1,page,line;
  char *oldstack,middle[80];
  
  oldstack=stack;
  
  if (strlen(str)<1)
    page=1;
  else
    page=atoi(str);
  
  if (page<1)
    {
      tell_player (p," Usage : comments [<pagenumber>]\n");
      return;
    }
  
  scan=flatlist_start;
  start=NULL;
  
  line=0;
  
  for (scan = flatlist_start; scan; scan = scan->flat_next)
    {
      if (pages<=page && line==0)
	start=scan;
      
      if (scan->comment[0] != 0)
	line++;
      
      if (line>TERM_LINES-2)
	{
	  line=0;
	  pages++;
	}
    }
  
  if (page>pages)
    page=pages;

  if (strlen(session)<1)
    strncpy(session, "not set", MAX_SESSION - 2);
  
  pstack_mid(session);
  
  line=0;
  for (; start; start = start->flat_next)
    {
      if (line>TERM_LINES)
	break;
      
      if (start->comment[0]!=0)
	{
	  if (!strcasecmp(sess_name, start->lower_name))
	    sprintf(stack, "%-20s* ", start->name);
	  else
	    sprintf(stack, "%-20s- ", start->name);
	  
	  strcat(stack, start->comment);
	  
	  stack = strchr(stack, 0);
	  *stack++ = '\n';
	  
	  line++;
	}
    }

  sprintf(middle, "Page %d of %d", page,pages);
  pstack_mid(middle);
  *stack++ = 0;
  tell_player(p, oldstack);
  stack = oldstack;
  return;
  
  
#ifdef 0
  
    char           *oldstack, middle[80];
    player         *scan, *start;
    int             page, pages, count, count2;
    oldstack = stack;
    
    /* work out where to start the page on...
       Basically count down n multiples of TERM_LINES-2 and then have a
       pointer left to there */

    /* set up a count and the initial value of the pointer to start
       listing from */
    count = 0;
    start = flatlist_start;
    /* count down the list, setting start to be the top of the page each
       time, ie the (n * (TERMLINES -2))th person to have set a comment */

    ccount = 0; /* the count of people who have commented */
    scan = flatlist_start; /* current person being checked */

    /* count the number of peeps who have commented */
    /* ASSUMPTION: that the flatlist_start list is NULL terminated */
    /* the original view_session had a check for a damaged who list
       by checking that current_players matched the number of people
       processed. This is not implemented here */
    for (scan = flatlist_start; scan; scan = scan->flat_next)
	if (scan->comment[0] != 0) ccount++;

    /* work out number of pages */
    pages = (ccount / (TERM_LINES -2)) + 1;

    /* find out which page we want */
    page = atoi(str);
    if (page <= 0)
	page = 1;

    /* check for page > pages and correct if so */
    if (page > pages)
	page = pages;
    
    /* now look down until we find the first comment for the page */
    count = 0;
    for (start = flatlist_start; (page * (TERM_LINES -2) - count); count++)
	start = start->flatlist_next;
    
    /* fix session if it is not set */
    if (strlen(session) == 0)
	strncpy(session, "not set", MAX_SESSION - 2);

    /* build the start of the text on the stack */
    strcpy(middle, session);
    pstack_mid(middle);
    
    /* build each comment line */


    /* build footer */

#endif
}



/* REPLYS */

/* save a list of who sent you the last list of names, for reply  */

void            make_reply_list(player * p, player ** list, int matches, int friend)
{
   char           *oldstack, *send, *mark, *scan;
   player        **step;
   time_t          t;
   int             i, count, timeout;

   oldstack = stack;

   t = time(0);
   timeout = t + (2 * 60);

   if (matches < 2)
      return;

   sprintf(stack, "%s.,", p->lower_name);
   count = strlen(stack);
   stack = strchr(stack, 0);
   for (step = list, i = 0; i < matches; i++, step++)
      if (*step != p)
      {
	 count += (strlen((*step)->lower_name) + 1);
	 if (count < (MAX_REPLY - 2))
	 {
	    sprintf(stack, "%s.,", (*step)->lower_name);
	    stack = strchr(stack, 0);
	 } else
	    log("reply", "Too longer reply string !!!");
      }
   stack = end_string(stack);

   /* should have string in oldstack */

   send = stack;
   for (step = list, i = 0; i < matches; i++, step++, mark = 0)
   {
     char buffer[50];

     sprintf(buffer,".,%s.,",(*step)->lower_name);
     mark = strstr(oldstack, buffer);
     if (mark)
       mark +=2;
     if (!mark)
       {
	 sprintf(buffer,"%s.,",(*step)->lower_name);
	 mark=strstr(oldstack, buffer);
	 if (mark!=oldstack)
	   mark=0;
       }
     
     if (!mark)
       {
	 log("reply", "Can't find player in reply string!!");
	 return;
       }
     for (scan = oldstack; scan != mark;)
       *stack++ = *scan++;
     while (*scan != ',')
       scan++;
     scan++;
     while (*scan)
       *stack++ = *scan++;
     *stack = 0;
     strcpy((*step)->reply, send);
     (*step)->reply_time = timeout;
     stack = send;
   }
}

/* Reply command itself */

void            reply(player * p, char *str)
{
   char           *oldstack;

   oldstack = stack;

   if (!*str)
   {
      tell_player(p, " Format: reply <msg>\n");
      return;
   }
   if (!*(p->reply) || (p->reply_time < (int) time(0)))
   {
      tell_player(p, " You don't have anyone to reply to!\n");
      return;
   }
   sprintf(stack, "%s ", p->reply);
   stack = strchr(stack, 0);
   strcpy(stack, str);
   stack = end_string(stack);
   sys_flags |= REPLY_TAG;
   tell(p, oldstack);
   stack = oldstack;
   sys_flags &= ~REPLY_TAG;
}

/* And the emote reply command itself */
 
void            ereply(player * p, char *str)
{
   char           *oldstack;
 
   oldstack = stack;
 
   if (!*str)
   {
      tell_player(p, " Format: ereply <msg>\n");
      return;
   }
   if (!*(p->reply) || (p->reply_time < (int) time(0)))
   {
      tell_player(p, " You don't have anyone to reply to!\n");
      return;
   }
   sprintf(stack, "%s ", p->reply);
   stack = strchr(stack, 0);
   strcpy(stack, str);
   stack = end_string(stack);
   sys_flags |= REPLY_TAG;
   remote(p, oldstack);
   stack = oldstack;
   sys_flags &= ~REPLY_TAG;
}


/*
 * Well, hopefully this WILL remove the player from all reply strings they are
 * in
 */
void            reply_remove_me(player * p, char *str)
{
   /*
    * We need to remove p's name from all the lists they are in, which should
    * be all the ppl they have in their reply string. So cycle through those
    * and remove this player
    */
}


void report_error(player * p, char *str)
{
   char *oldstack;
   if (!*str)
   {
      tell_player(p, " Format: bug <whatever the sodding bug is>\n");
      return;
   }
   if (strlen(str) > 240)
   {
      tell_player(p, " Make it a little smaller.\n");
      return;
   }
   tell_player(p, " Bug logged, thankyou.\n");
   oldstack = stack;
   sprintf(stack, "%s: %s", p->name, str);
   stack = end_string(stack);
   log("bug", oldstack);
   stack = oldstack;
}

void            show_exits(player * p, char *str)
{
   if (p->saved_flags & SHOW_EXITS)
   {
      tell_player(p, " You won't see exits when you enter a room now.\n");
      p->saved_flags &= ~SHOW_EXITS;
   } else
   {
      tell_player(p, " When you enter a room you will now see the exits.\n");
      p->saved_flags |= SHOW_EXITS;
   }
}
