/*
 * editor.c
 */

#include <memory.h>
#include <stdlib.h>

#include "config.h"
#include "player.h"
#include "fix.h"

/* externs */

extern struct command editor_list[];
extern char    *end_string();

/* interns */

void            editor_main();

/* print out some stats */

void            edit_stats(player * p, char *str)
{
   int             words = 0, lines = 0, blip = 0;
   char           *scan, *oldstack;
   oldstack = stack;
   for (scan = p->edit_info->buffer; *scan; scan++)
   {
      switch (*scan)
      {
    case ' ':
       if (blip)
          words++;
       blip = 0;
       break;
    case '\n':
       if (blip)
          words++;
       blip = 0;
       lines++;
       break;
    default:
       blip = 1;
      }
   }
   sprintf(oldstack, " Used %d bytes out of %d, in %d lines and %d words.\n",
           p->edit_info->size, p->edit_info->max_size, lines, words);
   stack = end_string(stack);
   tell_player(p, oldstack);
   stack = oldstack;
}

/* view the entire buffer */

void            edit_view(player * p, char *str)
{
   tell_player(p, p->edit_info->buffer);
}

/* insert a line into the buffer */

void            insert_line(player * p, char *str)
{
   ed_info        *e;
   int             len;
   char           *from, *to, *oldstack;
   e = p->edit_info;

   oldstack = stack;
   sprintf(stack, "%s\n", str);

   len = strlen(oldstack);
   if ((len + e->size) >= (e->max_size))
   {
      tell_player(p, " Line too long to fit into buffer.\n");
      return;
   }
   from = e->buffer + e->size;
   to = from + len;
   while (from != e->current)
      *(--to) = *(--from);
   *(--to) = *(--from);
   e->size += len;
   for (; len; len--)
      *(e->current)++ = *stack++;
   *stack++ = 0;
   tell_player(p, oldstack);
   stack = oldstack;
}


/* save and restore important flags that the editor changes */

void            save_flags(player * p)
{
   if (!p->edit_info)
      return;
   p->edit_info->flag_copy = p->flags;
   p->edit_info->sflag_copy = p->saved_flags;
   p->edit_info->input_copy = p->input_to_fn;
   p->flags &= ~PROMPT;
   if (p->saved_flags & QUIET_EDIT)
      p->saved_flags |= BLOCK_TELLS | BLOCK_SHOUT;
   p->input_to_fn = editor_main;
}

void            restore_flags(player * p)
{
   if (!p->edit_info)
      return;
   p->flags = p->edit_info->flag_copy;
   p->saved_flags = p->edit_info->sflag_copy;
   p->input_to_fn = p->edit_info->input_copy;
}


/* the main editor function */

void            editor_main(player * p, char *str)
{
   if (!p->edit_info)
   {
      log("error", "Editor called with no edit_info");
      return;
   }
   if (*str == '/')
   {
      restore_flags(p);
      match_commands(p, str + 1);
      save_flags(p);
      return;
   }
   if (*str == '.')
   {
      sub_command(p, str + 1, editor_list);
      if (p->edit_info)
    do_prompt(p, "+");
      return;
   }
   insert_line(p, str);
   do_prompt(p, "+");
}


/* fire editor up */

void            start_edit(player * p, int max_size, void *finish_func, void *quit_func,
                            char *current)
{
   ed_info        *e;
   int             old_length;
   if (p->edit_info)
      return;
   e = (ed_info *) MALLOC(sizeof(ed_info));
   memset(e, 0, sizeof(ed_info));
   p->edit_info = e;
   e->buffer = (char *) MALLOC(max_size);
   memset(e->buffer, 0, max_size);
   e->max_size = max_size;
   e->finish_func = finish_func;
   e->quit_func = quit_func;
   if (current)
   {
      old_length = strlen(current);
      memcpy(e->buffer, current, old_length);
      e->size = old_length + 1;
   } else
      e->size = 1;
   e->current = e->buffer + e->size - 1;
   p->flags |= DONT_CHECK_PIPE;
   save_flags(p);
   tell_player(p, " Entering editor ... Use /help editor for help.\n"
                  " /<command> for general commands, .<command> for editor "
                  "commands\n"
                  " Use '.end' to finish and keep edit\n");
   if (p->saved_flags & QUIET_EDIT)
      tell_player(p, " Blocking shouts and tells whilst in editor.\n");
   edit_view(p, 0);
   edit_stats(p, 0);
   do_prompt(p, "+");
}

/* tie up any loose ends */

void            finish_edit(player * p)
{
   if (!(p->edit_info))
      return;
   restore_flags(p);
   if (p->edit_info->buffer)
      FREE(p->edit_info->buffer);
   FREE(p->edit_info);
   p->edit_info = 0;
   p->flags &= ~DONT_CHECK_PIPE;
}

/* editor functions */

/* finish editing without changes */

void            edit_quit(player * p, char *str)
{
   void            (*fn) ();
   fn = p->edit_info->quit_func;
   (*fn) (p);
   finish_edit(p);
}

/* finish editing with changes */

void            edit_end(player * p, char *str)
{
   void            (*fn) ();
   fn = p->edit_info->finish_func;
   (*fn) (p);
   finish_edit(p);
}

/* clean the buffer completely */

void            edit_wipe(player * p, char *str)
{
   ed_info        *e;
   e = p->edit_info;
   memset(e->buffer, 0, e->size);
   e->size = 1;
   e->current = e->buffer;
   tell_player(p, " Edit buffer completely wiped ...\n");
}

/* view the current line */

void            edit_view_line(player * p, char *str)
{
   char           *scan, *oldstack;
   oldstack = stack;
   scan = p->edit_info->current;
   while (*scan && *scan != '\n')
      *stack++ = *scan++;
   *stack++ = '\n';
   *stack++ = 0;
   tell_player(p, oldstack);
   stack = oldstack;
}

/* move back up a line */

void            edit_back_line(player * p, char *str)
{
   char           *c;
   ed_info        *e;
   e = p->edit_info;
   c = e->current;
   if (c == e->buffer)
   {
      tell_player(p, " Can't go back any more, top of buffer.\n");
      return;
   }
/*   c -= 2;*/
	c--;
   while (c >= e->buffer && *c != '\n')
      c--;
   if (c == e->buffer)
      tell_player(p, " Reached top of buffer.\n");
   else
      c++;
   e->current = c;
   edit_view_line(p, 0);
}

/* move forward a line */

void            edit_forward_line(player * p, char *str)
{
   char           *c;
   ed_info        *e;
   e = p->edit_info;
   c = e->current;
   if (!*c)
   {
      tell_player(p, " Can't go forward, bottom of buffer.\n");
      return;
   }
   while (*c && *c != '\n')
      c++;
   if (*c)
      c++;
   e->current = c;
   if (!*c)
      tell_player(p, " Reached bottom of buffer.\n");
   else
      edit_view_line(p, 0);
}

/* print out available commands */

void            edit_view_commands(player * p, char *str)
{
   view_sub_commands(p, editor_list);
}

/* move to bottom of buffer */

void            edit_goto_top(player * p, char *str)
{
   p->edit_info->current = p->edit_info->buffer;
   tell_player(p, " Top of buffer.\n");
}

/* move to top of buffer */

void            edit_goto_bottom(player * p, char *str)
{
   p->edit_info->current = p->edit_info->buffer + p->edit_info->size - 1;
   tell_player(p, " Bottom of buffer.\n");
}

/* delete the current line */

void            edit_delete_line(player * p, char *str)
{
   ed_info        *e;
   char           *sl, *el;
   e = p->edit_info;
   sl = e->current;
   if (!*sl)
   {
      tell_player(p, " End of buffer, no line to delete.\n");
      return;
   }
   for (el = sl; (*el && *el != '\n'); el++)
      e->size--;
   if (*el)
   {
      el++;
      e->size--;
   }
   while (*el)
      *sl++ = *el++;
   while (sl != el)
      *sl++ = 0;
   tell_player(p, " Line deleted.\n");
}


/* go to a specific line */

void            edit_goto_line(player * p, char *str)
{
   char           *scan;
   int             line = 0;
   line = atoi(str);
   if (line < 1)
   {
      tell_player(p, " Argument is a number greater than zero.\n");
      return;
   }
   for (line--, scan = p->edit_info->buffer; (*scan && line); scan++)
      if (*scan == '\n')
    line--;
   if (!*scan)
   {
      tell_player(p, " Not that many lines.\n");
      return;
   }
   p->edit_info->current = scan;
   edit_view_line(p, 0);
}


/* toggle whether someone goes into quiet mode on edit */

void            toggle_quiet_edit(player * p, char *str)
{
   restore_flags(p);
   if (!strcasecmp("off", str))
      p->saved_flags &= ~QUIET_EDIT;
   else if (!strcasecmp("on", str))
      p->saved_flags |= QUIET_EDIT;
   else
      p->saved_flags ^= QUIET_EDIT;

   if (p->saved_flags & QUIET_EDIT)
      tell_player(p, " You will block tells and shouts upon editing.\n");
   else
      tell_player(p, " You won't block shouts and tells on editing.\n");
   save_flags(p);
}


/* the dreaded pager B-)  */

int             draw_page(player * p, char *text)
{
   int             end_line = 0, n;
   ed_info        *e;
   char           *oldstack;
   oldstack = stack;

   for (n = TERM_LINES + 1; n; n--, end_line++)
   {
      while (*text && *text != '\n')
         *stack++ = *text++;
      if (!*text)
         break;
      *stack++ = *text++;
   }
   *stack++ = 0;
   tell_player(p, oldstack);
   if (*text && p->edit_info)
   {
      e = p->edit_info;
      end_line += e->size;
      sprintf(oldstack, 
              "(Pager: lines %d to %d, from a total %d) Type n for next page: "
	      "\n",e->size, end_line, e->max_size);
      stack = end_string(oldstack);
      do_prompt(p, oldstack);
   }
   stack = oldstack;
   return *text;
}

void            quit_pager(player * p, ed_info * e)
{
   p->input_to_fn = e->input_copy;
   p->flags = e->flag_copy;
   if (e->buffer)
      FREE(e->buffer);
   FREE(e);
   p->edit_info = 0;
}

void            back_page(player * p, ed_info * e)
{
   char           *scan;
   int             n;
   scan = e->current;
   for (n = TERM_LINES + 1; n; n--)
   {
      while (scan != e->buffer && *scan != '\n')
    scan--;
      if (scan == e->buffer)
    break;
      e->size--;
      scan--;
   }
   e->current = scan;
}

void            forward_page(player * p, ed_info * e)
{
   char           *scan;
   int             n;
   scan = e->current;
   for (n = TERM_LINES + 1; n; n--)
   {
      while (*scan && *scan != '\n')
    scan++;
      if (!*scan)
    break;
      e->size++;
      scan++;
   }
   e->current = scan;
}

void            pager_fn(player * p, char *str)
{
   ed_info        *e;
   e = p->edit_info;
   switch (tolower(*str))
   {
      case 'b':
    back_page(p, e);
    break;
      case 'p':
    back_page(p, e);
    break;
      case 0:
    forward_page(p, e);
    break;
      case 'f':
    forward_page(p, e);
    break;
      case 'n':
    forward_page(p, e);
    break;
      case 't':
    e->current = e->buffer;
    e->size = 0;
    draw_page(p, e->current);
    break;
      case 'q':
    quit_pager(p, e);
    return;
   }
   if (!draw_page(p, e->current))
      quit_pager(p, e);
}


void            pager(player * p, char *text, int page)
{
   ed_info        *e;
   int             length = 0, lines = 0;
   char           *scan;

   if (p->saved_flags & NO_PAGER && !page)
   {
      tell_player(p, text);
      return;
   }
   if (p->edit_info)
   {
      tell_player(p, " Eeek, can't enter pager right now.\n");
      return;
   }
   for (scan = text; *scan; scan++, length++)
   {
      if (*scan == '\n')
      {
         lines++;
      }
      length++;
   }
   if (lines > (TERM_LINES + 1))
   {
      e = (ed_info *) MALLOC(sizeof(ed_info));
      memset(e, 0, sizeof(ed_info));
      p->edit_info = e;
      e->buffer = (char *) MALLOC(length);
      memcpy(e->buffer, text, length);
      e->current = e->buffer;
      e->max_size = lines;
      e->size = 0;
      e->input_copy = p->input_to_fn;
      e->flag_copy = p->flags;
      p->input_to_fn = pager_fn;
      p->flags &= ~PROMPT;
   }
   draw_page(p, text);
}
