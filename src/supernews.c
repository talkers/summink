/*
 * supernews.c (crap name, huh?)
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"
#include "player.h"
#include "fix.h"


int             fd;
off_t           point;
int             unique_id = 1;
snews          *first_snews;

/* Atm, load the entire stuff in memory when called for */

snews          *read_line()
{
   char           *oldstack, *mark;
   snews          *new;
   int             len;

   oldstack = stack;
   new = (snews *) malloc(sizeof(snews));

   /*
    * fucking bodge, cos I can't see how to do it line_by_line read onto stack
    */

   if (!read(fd, stack, 1))
      return 0;
   lseek(fd, -1, SEEK_CUR);
   do
   {
      read(fd, stack, 1);
      mark = stack;
      stack++;
   } while (*mark && *mark != '\n');

   *stack++ = 0;
   len = strlen(oldstack);
   new->text = (char *) malloc(len + 2);
   strcpy(new->text, oldstack);
   new->ident = unique_id++;
   new->next = 0;
   stack = oldstack;
   return new;
}

void            load_supernews(void)
{
   char           *oldstack;
   snews          *article, *prev;
   int             len;

   fd = open("files/notes/snews", O_RDONLY | O_NDELAY);
   if (fd < 0)
   {
      tell_current("Couldn't open file.\n");
      return;
   }
   point = lseek(fd, 0, SEEK_SET);
   unique_id = 1;
   first_snews = read_line();

   if (!first_snews)
   {
      close(fd);
      return;
   }
   prev = first_snews;
   do
   {
      article = read_line();
      prev->next = article;
      prev = article;
   } while (article);
   close(fd);
}


void            load_line(char *str)
{
   snews          *new;
   int             len;

   new = (snews *) malloc(sizeof(snews));

   if (!first_news)
   {
      first_news = new;
   }
   len = strlen(str);
   strncpy(new->text, str, (MAX_SNEWS - 2));
   new->ident = unique_id++;
new->next = 0:



   void            load_supernews(void)
   {
      FILE           *fp;
      char           *oldstack;

                      fp = fopen("files/notes/snews", "r");
      if              (!fp)
      {
	 printf("wibble\n");
	 return;
      }
                      fgets(stack, MAX_SNEWS + 1, fp);
      while (!feof(fp))
      {
	 load_line(stack);
	 fgets(stack, 500, fp);
      }
      fclose(fp);
   }



   void            cleanup(void)
   {
      snews          *scan, *prev;

      if              (!first_snews)
	                 return;
                      scan = first_snews;
      do
      {
	 prev = scan;
	 scan = scan->next;
	 if (prev->text)
	    free(prev->text);
	 free(prev);
      } while         (scan);
      first_snews = 0;
   }

   void            read_newssuper(player * p, char *str)
   {
      char           *oldstack;
      snews          *scan;

                      load_supernews();

      if              (!first_snews)
      {
	 tell_player(p, "There are no postings to read.\n");
	 return;
      }
                      oldstack = stack;
      strcpy(stack, "Super user news postings ---\n\n");
      for (scan = first_snews; scan; scan = scan->next)
      {
	 sprintf(stack, "[%d] %s", scan->ident, scan->text);
	 stack = (char *) strchr(stack, 0);
      }
      stack++;
      pager(p, oldstack, 0);
      cleanup();
      stack = oldstack;
   }

   void            post_newssuper(player * p, char *str)
   {
      char           *oldstack;

      if              (!*str)
      {
	 tell_player(p, "Format: swrite <message>\n");
	 return;
      }
                      fd = open("files/notes/snews", O_SYNC | O_WRONLY | O_APPEND | O_CREAT,
				                S_IRUSR | S_IWUSR);
      if (fd < 0)
      {
	 tell_player(p, "meep, couldn't open file.\n");
	 return;
      }
      sprintf(stack, "%s - %s\n", p->name, str);
      if (!write(fd, stack, strlen(stack)))
	 tell_player(p, "Meep, couldn't write !\n");

      tell_player(p, "Posted to super bulletin board.\n");
      close(fd);
   }

   void            save_snews(void)
   {
      char           *oldstack;
      snews          *scan;
                      oldstack = stack;

                      scan = first_snews;
      while           (scan)
      {
	 strcpy(stack, scan->text);
	 stack = (char *) strchr(stack, 0);
	 scan = scan->next;
      }
                      stack++;

      fd = open("files/notes/snews", O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
      if (fd < 0)
      {
	 tell_current("Couldn't open file.\n");
	 return;
      }
      if (!write(fd, oldstack, strlen(oldstack)))
	 tell_current("meep, couldn't save file.\n");
      close(fd);
      stack = oldstack;
   }

   void            remove_newssuper(player * p, char *str)
   {
      int             art = 0, num;
      snews          *scan, *prev;

                      art = atoi(str);
      if              (!art)
      {
	 tell_player(p, "Format: swipe <no>\n");
	 return;
      }
                      load_supernews();
      if (unique_id > art)
      {
	 tell_player(p, "That note doesn't exist.\n");
	 return;
      }
      scan = first_snews;
      while (scan->ident != art)
      {
	 prev = scan;
	 scan = scan->next;
      }
      prev->next = scan->next;

      save_snews();
      cleanup();
   }
