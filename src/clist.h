/*
 * clist.h
 */

/* The stuff for the dynamic rooms */
extern void dynamic_defrag_rooms();
extern void dynamic_test_func_blocks(), dynamic_test_func_keys();
extern void dynamic_dfstats();
extern void dynamic_validate_rooms();
/**/


extern char     *check_legal_entry(player *, char *, int);
extern char     *list_lines(list_ent *);
extern void     to_room1(), to_room2(), room_link();
extern void     say(), quit(), pulldown(), change_password(), change_email(),
                do_save(), wall(), tell(), grant(), remove(), nuke_player(),
                view_saved_lists(), banish_player(), unbanish(), sync_files(),
                sync_all_by_user(), restore_files(), check(),abort_shutdown(),
                make_new_character(), room_command(), change_room_limit(),
                test_fn(), unlink_from_room(), look(), trans_to(),
                validate_email(), go_room(), remote(), recho(), whisper(),
                emote(), echo(), gender(), shout(), exclude(),comments(),
                change_exit_limit(), change_auto_limit(), remove_shout(),
                set_title(), set_description(), set_plan(), set_prompt(),
                set_converse_prompt(), set_term_width(), set_word_wrap(),
                blank_password(), new_blankpass(), pemote(), premote(),
                converse_mode_on(), converse_mode_off(), swho(), set_pretitle(),
                clear_list(), change_list_absolute(), change_list_limit(),
                set_list(), reset_list(), toggle_list(), noisy(), ignore(),
                inform(), grab(), friend(), bar(), invite(), key(), listfind(),
                set_enter_msg(), set_ignore_msg(), view_commands(), qwho(),
                beep(), blocktells(), earmuffs(), move_to(),
                change_room_entermsg(), hide(), check_idle(), set_idle_msg(),
                view_ip(), view_player_email(), public_com(), do_grab(),
                examine(), go_quiet(), view_time(), toggle_tags(), echoall(),
                straight_home(), close_to_newbies(), su(), suemote(),
                su_hilited(), emergency(), finger(), see_echo(), dump_com(),
                news_command(), mail_command(), list_notes(), list_all_notes(),
                change_mail_limit(), toggle_news_inform(), toggle_mail_inform(),
                recap(), soft_eject(), show_malloc(), help(), reload(),
                hitells(), block(), privs(), boot_out(), wake(), view_note(),
                dest_note(), sneeze(), resident(), join(), crash(), port(),
                motd(), trace(), trans_fn(), remove_move(), relink_note(),
                add_lag(), toggle_iacga(), recount_news(), banish_edit(),
                same_site(), set_age(), set_birthday(), warn(), netstat(),
                nopager(), bounce(), vlog(), reset_sneeze(), splat_player(),
                block_su(), unsplat(), invites_list(), rename_player(),
                quiet_rename(), mindseye(), think(), set_session(),
                set_comment(), newthink(), view_session(), super_help(),
                set_yes_session(), reply(), ereply(), prison_player(), twho(),
                reset_session(), set_login_room(), room_entry(),
                assist_player(), on_duty(), barge(), report_error(),
                clear_screen(), confirm_password(), inform_room_enter(),
                show_exits(), blank_email(), hang(), frog(), unfrog(),
                unconverse(), unjail(), go_colony(), suthink(), script(),
                player_stats(), go_comfy(), mode(), hilltop(), yoyo(),
                tell_friends(), remote_friends(), remote_think(),
                blank_prefix();

extern void     edit_quit(), edit_end(), edit_wipe(), edit_view(),
                edit_view_line(), edit_back_line(), edit_forward_line(),
                edit_view_commands(), edit_goto_top(), edit_goto_bottom(),
                edit_stats(), edit_delete_line(), edit_goto_line(),
                toggle_quiet_edit(), change_auto_base();

extern void     check_wrap(), check_updates(), check_email(), check_rooms(),
                check_exits(), view_others_list(), do_list(), view_list(),
                view_check_commands(), check_info(), check_room(),
                check_entry(), check_alist();

extern void     exit_room_mode(), create_new_room(), change_room_id(),
                change_room_name(), delete_room(), add_exit(), remove_exit(),
                add_auto(), remove_auto(), check_autos(), autos_com(),
                view_room_commands(), view_room_key_commands(), who(), where(),
                room_lock(), room_bolt(), room_lockable(), room_open(),
                set_home(), go_home(), visit(), go_main(), room_edit(), with(),
                grabable(), transfer_room(), room_linkable();

extern void     view_news_commands(), exit_news_mode(), list_news(),
                post_news(), followup(), remove_article(), read_article(),
                reply_article();

extern void     view_mail_commands(), exit_mail_mode(), view_sent(),
                view_received(), send_letter(), read_letter(), reply_letter(),
                delete_received(), delete_sent(), toggle_anonymous(), lsu(),
                lnew(), ignoreprefix(), ignoreemoteprefix(), set_time_delay();

extern void     player_flags_verbose(),blank_list();

#ifdef PC
extern void     psuedo_person(), switch_person();
#endif

/* dummy commands for stack checks */

struct command  input_to = {"input_to fn", 0, 0, 0, 0, 0};
struct command  timer = {"timer fn", 0, 0, 0, 0, 0};

/* command list for editor */

struct command  editor_list[] = {
   {"del", edit_delete_line, 0, 0, 1, 0},
   {"-", edit_back_line, 0, 0, 1, 0},
   {"+", edit_forward_line, 0, 0, 1, 0},
   {"view", edit_view, 0, 0, 1, 0},
   {"l", edit_view_line, 0, 0, 1, 0},
   {"g", edit_goto_line, 0, 0, 1, 0},
   {"top", edit_goto_top, 0, 0, 1, 0},
   {"bot", edit_goto_bottom, 0, 0, 1, 0},
   {"end", edit_end, 0, 0, 1, 0},
   {"quit", edit_quit, 0, 0, 1, 0},
   {"wipe", edit_wipe, 0, 0, 1, 0},
   {"stats", edit_stats, 0, 0, 1, 0},
   {"quiet", toggle_quiet_edit, 0, 0, 1, 0},
   {"commands", edit_view_commands, 0, 0, 1, 0},
   {"?", help, 0, 0, 0, 0},
   {"help", help, 0, 0, 1, 0},
   {"man", help, 0, 0, 1, 0},
   {0, 0, 0, 0, 0, 0}
};

/* command list for the room function */

struct command  keyroom_list[] = {
   {"check", check_rooms, BUILD, 0, 1, 0},
   {"chekc", check_rooms, BUILD, 0, 1, 0},
   {"commands", view_room_key_commands, BUILD, 0, 1, 0},
   {"end", exit_room_mode, BUILD, 0, 1, 0},
   {"entermsg", change_room_entermsg, BUILD, 0, 1, 0},
   {"exits", check_exits, BUILD, 0, 1, 0},
   {"+exit", add_exit, BUILD, 0, 1, 0},
   {"-exit", remove_exit, BUILD, 0, 1, 0},
   {"go", go_room, BUILD, 0, 1, 0},
   {"?", help, 0, 0, 0, 0},
   {"help", help, 0, 0, 0, 0},
   {"info", check_room, BUILD, 0, 1, 0},
   {"lock", room_lock, BUILD, 0, 1, 0},
   {"lockable", room_lockable, BUILD, 0, 1, 0},
   {"look", look, BUILD, 0, 1, 0},
   {"linkable", room_linkable, BUILD, 0, 1, 0},
   {"man", help, 0, 0, 0, 0},
   {"name", change_room_name, BUILD, 0, 1, 0},
   {"open", room_open, BUILD, 0, 1, 0},
   {"trans", trans_fn, BUILD, 0, 1, 0},
   {0, 0, 0, 0, 0, 0}
};


struct command  room_list[] = {
   {"bolt", room_bolt, BUILD, 0, 1, 0},
   {"edit", room_edit, BUILD, 0, 1, 0},
   {"sethome", set_home, BUILD, 0, 1, 0},
   {"lock", room_lock, BUILD, 0, 1, 0},
   {"lockable", room_lockable, BUILD, 0, 1, 0},
   {"linkable", room_linkable, BUILD, 0, 1, 0},
   {"open", room_open, BUILD, 0, 1, 0},
   {"entrance", room_entry, BUILD, 0, 1, 0},
   {"entermsg", change_room_entermsg, BUILD, 0, 1, 0},
   {"+exit", add_exit, BUILD, 0, 1, 0},
   {"-exit", remove_exit, BUILD, 0, 1, 0},
   {"link", room_link, BUILD, 0, 1, 0},
   {"exits", check_exits, BUILD, 0, 1, 0},
   {"id", change_room_id, BUILD, 0, 1, 0},
   {"name", change_room_name, BUILD, 0, 1, 0},
   {"notify", inform_room_enter, BUILD, 0, 1, 0},
   {"end", exit_room_mode, BUILD, 0, 1, 0},
   {"info", check_room, BUILD, 0, 1, 0},
   {"check", check_rooms, BUILD, 0, 1, 0},
   {"chekc", check_rooms, BUILD, 0, 1, 0},
   {"+auto", add_auto, BUILD, 0, 1, 0},
   {"-auto", remove_auto, BUILD, 0, 1, 0},
   {"speed", change_auto_base, BUILD, 0, 1, 0},
   {"autos", autos_com, BUILD, 0, 1, 0},
   {"delete", delete_room, BUILD, 0, 1, 0},
   {"create", create_new_room, BUILD, 0, 1, 0},
   {"go", go_room, BUILD, 0, 1, 0},
   {"look", look, BUILD, 0, 1, 0},
   {"trans", trans_fn, BUILD, 0, 1, 0},
   {"home", go_home, BUILD, 0, 1, 0},
   {"commands", view_room_commands, BUILD, 0, 1, 0},
   {"?", help, 0, 0, 0, 0},
   {"help", help, 0, 0, 1, 0},
   {"man", help, 0, 0, 1, 0},
   {"transfer", transfer_room, ADMIN, 0, 1, 0},
   {0, 0, 0, 0, 0, 0}
};


/* command list for the check function */

struct command  check_list[] = {
   {"flags", player_flags_verbose, 0, 0, 1, 0},
   {"mail", view_received, MAIL, 0, 1, 0},
   {"sent", view_sent, MAIL, 0, 1, 0},
   {"news", list_news, 0, 0, 1, 0},
   {"exits", check_exits, 0, 0, 1, 0},
   {"entry", check_entry, 0, 0, 1, 0},
   {"list", check_alist, LIST, 0, 1, 0},
   {"autos", check_autos, BUILD, 0, 1, 0},
   {"room", check_room, 0, 0, 1, 0},
   {"rooms", check_rooms, BUILD, 0, 1, 0},
   {"email", check_email, 0, 0, 1, 0},
   {"wrap", check_wrap, 0, 0, 1, 0},
   {"res_list", view_saved_lists, ADMIN, 0, 1, 0},
   {"updates", check_updates, (LOWER_ADMIN | ADMIN), 0, 1, 0},
   {"info", check_info, ADMIN, 0, 1, 0},
   {"commands", view_check_commands, 0, 0, 1, 0},
   {"ip", view_ip, (SU | ADMIN), 0, 1, 0},
   {"mails", view_player_email, ADMIN, 0, 1, 0},
   {"?", help, 0, 0, 0, 0},
   {"help", help, 0, 0, 1, 0},
   {"man", help, 0, 0, 1, 0},
{0, 0, 0, 0, 0, 0}};

/* command list for the news sub command */

struct command  news_list[] = {
   {"check", list_news, 0, 0, 1, 0},
   {"chekc", list_news, 0, 0, 1, 0},
   {"read", read_article, 0, 0, 1, 0},
   {"view", list_news, 0, 0, 1, 0},
   {"reply", reply_article, MAIL, 0, 1, 0},
   {"areply", reply_article, MAIL, 0, 1, 0},
   {"post", post_news, MAIL, 0, 1, 0},
   {"apost", post_news, MAIL, 0, 1, 0},
   {"followup", followup, MAIL, 0, 1, 0},
   {"afollowup", followup, MAIL, 0, 1, 0},
   {"remove", remove_article, MAIL, 0, 1, 0},
   {"commands", view_news_commands, 0, 0, 1, 0},
   {"inform", toggle_news_inform, 0, 0, 1, 0},
   {"end", exit_news_mode, 0, 0, 1, 0},
   {"?", help, 0, 0, 0, 0},
   {"help", help, 0, 0, 1, 0},
   {"man", help, 0, 0, 1, 0},
{0, 0, 0, 0, 0, 0}};

/* command list for the mail sub command */

struct command  mail_list[] = {
   {"check", view_received, 0, 0, 1, 0},
   {"chekc", view_received, 0, 0, 1, 0},
   {"read", read_letter, MAIL, 0, 1, 0},
   {"post", send_letter, MAIL, 0, 1, 0},
   {"apost", send_letter, MAIL, 0, 1, 0},
   {"reply", reply_letter, MAIL, 0, 1, 0},
   {"areply", reply_letter, MAIL, 0, 1, 0},
   {"delete", delete_received, MAIL, 0, 1, 0},
   {"remove", delete_sent, MAIL, 0, 1, 0},
   {"commands", view_mail_commands, MAIL, 0, 1, 0},
   {"end", exit_mail_mode, MAIL, 0, 1, 0},
   {"sent", view_sent, MAIL, 0, 1, 0},
   {"view", view_received, MAIL, 0, 1, 0},
   {"inform", toggle_mail_inform, 0, 0, 1, 0},
   {"noanon", toggle_anonymous, 0, 0, 1, 0},
   {"?", help, 0, 0, 0, 0},
   {"help", help, 0, 0, 1, 0},
   {"man", help, 0, 0, 1, 0},
{0, 0, 0, 0, 0, 0}};


/* restricted command list for naughty peoples */

struct command  restricted_list[] = {
   {"'", say, 0, 0, 0, 0},
   {"`", say, 0, 0, 0, 0},
   {"\"", say, 0, 0, 0, 0},
   {"::", pemote, 0, 0, 0, 0},
   {":;", pemote, 0, 0, 0, 0},
   {";;", pemote, 0, 0, 0, 0},
   {";:", pemote, 0, 0, 0, 0},
   {";", emote, 0, 0, 0, 0},
   {":", emote, 0, 0, 0, 0},
   {"=", whisper, 0, 0, 0, 0},
   {"emote", emote, 0, 0, 1, 0},
   {"say", say, 0, 0, 1, 0},
   {"pemote", pemote, 0, 0, 1, 0},
   {"whisper", whisper, 0, 0, 1, 0},
   {"look", look, 0, 0, 1, 0},
   {"l", look, 0, 0, 1, 0},
   {"?", help, 0, 0, 1, 0},
   {"help", help, 0, 0, 1, 0},
   {"man", help, 0, 0, 1, 0},
{0, 0, 0, 0, 0, 0}};

/* this is the main command list */

struct command  complete_list[] = {	/* non alphabetic */
{"'", say, 0, 0, 0, 0},
{"`", say, 0, 0, 0, 0},
{"\"", say, 0, 0, 0, 0},
{"::", pemote, 0, 0, 0, 0},
{":;", pemote, 0, 0, 0, 0},
{";;", pemote, 0, 0, 0, 0},
{";:", pemote, 0, 0, 0, 0},
{";", emote, 0, 0, 0, 0},
{":", emote, 0, 0, 0, 0},
{">", tell, 0, 0, 0, 0},
{".", tell, 0, 0, 0, 0},
{"<:", premote, 0, 0, 0, 0},
{",:", premote, 0, 0, 0, 0},
{"<;", premote, 0, 0, 0, 0},
{",;", premote, 0, 0, 0, 0},
{"<", remote, 0, 0, 0, 0},
{",", remote, 0, 0, 0, 0},
{"=", whisper, 0, 0, 0, 0},
{"!", shout, 0, 0, 0, 0},
{"?", help, 0, 0, 0, 0},
{"+", echo, ECHO_PRIV, 0, 0, 0},
{"-", recho, ECHO_PRIV, 0, 0, 0},
{"]", reply, 0, 0, 0, 0},
{"[", ereply, 0, 0, 0, 0},
{"~", newthink, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0},

{"abort", abort_shutdown, ADMIN, 0, 1, 0},
{"age", set_age, 0, 0, 1, 0},	/* A */
{"assist", assist_player, SU, 0, 1, 0},
{0, 0, 0, 0, 0, 0},

{"banish", banish_player, (SU | ADMIN), 0, 1, 0},
{"bar", bar, LIST, 0, 1, 0},	/* B */
{"barge", barge, ADMIN, 0, 1, 0},
{"bedit", banish_edit,(LOWER_ADMIN | ADMIN), 0, 1, 0},
{"beep", beep, LIST, 0, 1, 0},
{"birthday", set_birthday, 0, 0, 1, 0},
{"blank_email", blank_email, (LOWER_ADMIN | ADMIN), 0, 1, 0},
{"blank_own_list",blank_list,LIST,1,0},
{"blank_prefix", blank_prefix, (SU | ADMIN), 0, 1, 0},
{"blankpass", new_blankpass, (SU | ADMIN), 0, 1, 0},
{"block", block, LIST, 0, 1, 0},
{"blocktells", blocktells, 0, 0, 1, 0},
{"boot", boot_out, BUILD, 0, 1, 0},
{"bounce", bounce, 0, 0, 1, 0},
{"bug", report_error, BASE, 0, 1, 0},
{0, 0, 0, 0, 0, 0},

{"check", check, 0, 0, 1, 0},	/* C */
{"chekc", check, 0, 0, 1, 0},
{"chlim_auto", change_auto_limit, ADMIN, 0, 1, 0},
{"chlim_exit", change_exit_limit, ADMIN, 0, 1, 0},
{"chlim_list", change_list_limit, (LOWER_ADMIN|ADMIN), 0, 1, 0},
{"chlim_mail", change_mail_limit, ADMIN, 0, 1, 0},
{"chlim_room", change_room_limit, (LOWER_ADMIN|ADMIN), 0, 1, 0},
{"clist", clear_list, LIST, 0, 1, 0},
{"cls", clear_screen, 0, 0, 1, 0},
{"colony", go_colony, 0, 0, 1, 0},
{"comfy", go_comfy, SU, 0, 1, 0},
{"commands", view_commands, 0, 0, 1, 0},
{"comment", set_comment, 0, 0, 1, 0},
{"comments",comments,0,1,0},
{"confirm", confirm_password, PSU, 0, 1, 0},
{"connect_room", set_login_room, BASE, 0, 1, 0},
{"converse", converse_mode_on, 0, 0, 1, 0},
{"cprompt", set_converse_prompt, 0, 0, 1, 0},
{0, 0, 0, 0, 0, 0},

/* Stuff for dynamic rooms */
/* These seems to crash it by putting too much in the pager.
   {"dtb", dynamic_test_func_blocks, ADMIN, 0, 1, 0},
   {"dtk", dynamic_test_func_keys, ADMIN, 0, 1, 0},
   */
{"defrag", dynamic_defrag_rooms, ADMIN, 0, 1, 0},
{"description", set_description, 0, 0, 1, 0},	/* D */
{"dfcheck", dynamic_validate_rooms, ADMIN, 0, 1, 0},
{"dfstats", dynamic_dfstats, ADMIN, 0, 1, 0},
{"drag", soft_eject, SU, 0, 1, 0},
{"dump", dump_com, (LOWER_ADMIN | ADMIN), 0, 1, 0},
/**/
{0, 0, 0, 0, 0, 0},

{"earmuffs", earmuffs, 0, 0, 1, 0},
{"echo", echo, ECHO_PRIV, 0, 1, 0},
{"echoall", echoall,ADMIN,0,1,0},
{"emergency", emergency, 0, 0, 1, 0},
{"emote", emote, 0, 0, 1, 0},	/* E */
{"end", converse_mode_off, 0, 0, 1, 0},
{"entermsg", set_enter_msg, 0, 0, 1, 0},
{"ereply", ereply, 0, 0, 1, 0},
{"email", change_email, 0, 0, 1, 0},
{"evict", sneeze, (SU | ADMIN), HOUSE, 1, 0},
{"examine", examine, 0, 0, 1, 0},
{"exclude", exclude, 0, 0, 1, 0},
{"exits", check_exits, 0, 0, 1, 0},
{0, 0, 0, 0, 0, 0},

{"find", listfind, LIST, 0, 1, 0},
{"finger", finger, 0, 0, 1, 0},
{"flist", change_list_absolute, LIST, 0, 1, 0},
{"friend", friend, LIST, 0, 1, 0},	/* F */
{"frog", frog,(FROG|SU), 0, 1, 0},
{"fwho", qwho, 0, 0, 1, 0},
{0, 0, 0, 0, 0, 0},

{"gender", gender, 0, 0, 1, 0},
{"ghome", straight_home, BUILD, 0, 1, 0},
{"go", go_room, 0, 0, 1, 0},	/* G */
{"grab", do_grab, 0, 0, 1, 0},
{"grabable", grabable, 0, 0, 1, 0},
{"grabme", grab, LIST, 0, 1, 0},
{"grant", grant, (ADMIN | HCADMIN), 0, 1, 0},
{0, 0, 0, 0, 0, 0},

{"help", help, 0, 0, 1, 0},	/* H */
{"hide", hide, 0, 0, 1, 0},
{"hilltop", hilltop, 0, 0, 1, 0},
{"hitells", hitells, 0, 0, 1, 0},
{"home", go_home, BUILD, 0, 1, 0},
{"homeview", mindseye, BUILD, 0, 1, 0},
{0, 0, 0, 0, 0, 0},

{"iacga", toggle_iacga, 0, 0, 1, 0},
{"idle", check_idle, 0, 0, 1, 0},	/* I */
{"idlemsg", set_idle_msg, 0, 0, 1, 0},
{"ignore", ignore, LIST, 0, 1, 0},
{"ignoremsg", set_ignore_msg, LIST, 0, 1, 0},
{"inform", inform, LIST, 0, 1, 0},
{"invite", invite, LIST, 0, 1, 0},
{"invites", invites_list, 0, 0, 1, 0},
{0, 0, 0, 0, 0, 0},

{"jail", prison_player, (SU | ADMIN), 0, 1, 0},
{"jetlag", set_time_delay, 0, 0, 1, 0},
{"join", join, 0, 0, 1, 0},	/* J */
{0, 0, 0, 0, 0, 0},

{"key", key, LIST, 0, 1, 0},
{0, 0, 0, 0, 0, 0},		/* K */

{"l", look, 0, 0, 1, 0},
{"lag", add_lag, ADMIN, 0, 1, 0},
{"leave", go_main, 0, 0, 1, 0},
{"linewrap", set_term_width, 0, 0, 1, 0},
{"list", view_list, LIST, 0, 1, 0},
{"list_new", lnew, PSU, 0, 1, 0},
{"list_notes", list_notes, ADMIN, 0, 1, 0},
{"list_res", view_saved_lists, (SU | ADMIN), 0, 1, 0},
{"list_su", lsu, 0, 0, 1, 0},
/*{"list_all_notes", list_all_notes, ADMIN, 0, 1, 0},*/
{"lock", room_lock, 0, 0, 1, 0},
{"look", look, 0, 0, 1, 0},	/* L */
{"lsn", lnew, PSU, 0, 1, 0},
{"lsu", lsu, 0, 0, 1, 0},
{0, 0, 0, 0, 0, 0},

{"mail", mail_command, MAIL, 0, 1, 0},	/* M */
{"main", go_main, 0, 0, 1, 0},
{"make", make_new_character, ADMIN, 0, 1, 0},
{"malloc", show_malloc, ( LOWER_ADMIN | ADMIN), 0, 1, 0},
{"man", help, 0, 0, 1, 0},
{"mindscape", go_home, BUILD, HOUSE, 1, 0},
{"mindseye", mindseye, BUILD, HOUSE, 1, 0},
{"mode", mode, ( PSU | SU | ADMIN ), 0, 1, 0},
{"motd", motd, 0, 0, 1, 0},
{0, 0, 0, 0, 0, 0},

{"netstat", netstat, ADMIN, 0, 1, 0},	/* N */
{"newbies", close_to_newbies, SU, 0, 1, 0},
{"news", news_command, 0, 0, 1, 0},
{"noisy", noisy, LIST, 0, 1, 0},
{"noeprefix", ignoreemoteprefix, 0, 0, 1, 0},
{"nopager", nopager, 0, 0, 1, 0},
{"noprefix", ignoreprefix, 0, 0, 1, 0},
{"nuke", nuke_player, (SU | ADMIN), 0, 1, 0},
{0, 0, 0, 0, 0, 0},

{"off_duty", block_su, PSU, 0, 1, 0},
{"on_duty", on_duty, PSU, 0, 1, 0},
{0, 0, 0, 0, 0, 0},		/* O */

{"password", change_password, 0, 0, 1, 0},	/* P */
{"pemote", pemote, 0, 0, 1, 0},
{"plan", set_plan, BASE, 0, 1, 0},
{"port", port, ADMIN, 0, 1, 0},
{"prefix", set_pretitle, BASE, 0, 1, 0},
{"premote", premote, 0, 0, 1, 0},
{"prompt", set_prompt, 0, 0, 1, 0},
{"privs", privs, BASE, 0, 1, 0},
#ifdef PC
{"pseudo", psuedo_person, 0, 0, 1, 0},
#endif
{"pstats", player_stats, PSU, 0, 1, 0},
{"public", public_com, 0, 0, 1, 0},
{0, 0, 0, 0, 0, 0},

{"quiet", go_quiet, 0, 0, 1, 0},
{"quit", quit, 0, 0, 1, 0},	/* Q */
{"qwho", qwho, 0, 0, 1, 0},
{0, 0, 0, 0, 0, 0},

{"recap", recap, 0, 0, 1, 0},	/* R */
{"recho", recho, ECHO_PRIV, 0, 1, 0},
{"recount", recount_news, ADMIN, 0, 1, 0},
{"relink", relink_note, ADMIN, 0, 1, 0},
{"remote", remote, 0, 0, 1, 0},
{"remove", remove, (ADMIN|HCADMIN), 0, 1, 0},
{"rename", rename_player, (SU | ADMIN), 0, 1, 0},
{"reply", reply, 0, 0, 1, 0},
{"reset_session", reset_session, SU, 0, 1, 0},
{"reset_sneeze", reset_sneeze, (SU | ADMIN), 0, 1, 0},
{"resident", resident, SU, 0, 1, 0},
{"restore", restore_files, ADMIN, 0, 1, 0},
{"rf", remote_friends, 0, 0, 1, 0},
{"rlist", reset_list, LIST, 0, 1, 0},
{"reload", reload, ( LOWER_ADMIN | ADMIN), 0, 1, 0},
{"rm_move", remove_move, ADMIN, 0, 1, 0},
{"rm_note", dest_note, ADMIN, 0, 1, 0},
{"rm_shout", remove_shout, (SU | ADMIN), 0, 1, 0},
{"room", room_command, BUILD, 0, 1, 0},
{"rt", remote_think, 0, 0, 1, 0},
{0, 0, 0, 0, 0, 0},

{"save", do_save, BASE, 0, 1, 0},
{"say", say, 0, 0, 1, 0},	/* S */
{"script", script, (SCRIPT), 0, 1, 0},
{"se", suemote, PSU, 0, 1, 0},
{"seeecho", see_echo, 0, 0, 1, 0},
{"seesess", view_session, 0, 0, 1, 0},
{"seetitle", set_yes_session, 0, 0, 1, 0},
{"session", set_session, SESSION, 0, 1, 0},
{"shelp", super_help, PSU, 0, 1, 0},
{"shout", shout, 0, 0, 1, 0},
{"show", toggle_tags, 0, 0, 1, 0},
{"showexits", show_exits, BASE, 0, 1, 0},
{"shutdown", pulldown, SU, 0, 1, 0},
{"site", same_site, (TRACE | SU | PSU), 0, 1, 0},
{"slist", set_list, LIST, 0, 1, 0},
{"sneeze", sneeze, (SU | ADMIN), 0, 1, 0},
{"splat", splat_player, SU, 0, 1, 0},
{"st", suthink, PSU, 0, 1, 0},
{"su", su, PSU, 0, 1, 0},
{"su_hi", su_hilited, PSU, 0, 1, 0},
{"su:", suemote, PSU, 0, 1, 0},
{"swho", swho, 0, 0, 1, 0},
#ifdef PC
{"switch", switch_person, 0, 0, 1, 0},
#endif
{"sync", sync_files, (LOWER_ADMIN | ADMIN), 0, 1, 0},
{"syncall", sync_all_by_user, (LOWER_ADMIN | ADMIN), 0, 1, 0},
{0, 0, 0, 0, 0, 0},

{"tell", tell, 0, 0, 1, 0},	/* T */
{"tf", tell_friends, 0, 0, 1, 0},
{"think", newthink, 0, 0, 1, 0},
{"tlist", toggle_list, LIST, 0, 1, 0},
{"time", view_time, 0, 0, 1, 0},
{"title", set_title, 0, 0, 1, 0},
{"trace", trace, (TRACE | SU | ADMIN), 0, 1, 0},
{"trans", trans_fn, 0, 0, 1, 0},
{"twho", twho, 0, 0, 1, 0},
{0, 0, 0, 0, 0, 0},

/* {"unlink",unlink_from_room,ADMIN,1,0},            U     */
{"unbanish", unbanish, (SU | ADMIN), 0, 1, 0},
{"unconverse", unconverse, (SU | ADMIN), 0, 1, 0},
{"unfrog", unfrog, (FROG | SU | ADMIN), 1 , 0},
{"unjail", unjail, (SU | ADMIN), 0, 1, 0},
{"unsplat", unsplat, (SU | ADMIN), 0, 1, 0},
{0, 0, 0, 0, 0, 0},

{"validate_email", validate_email, (SU | ADMIN), 0, 1, 0},
{"view_note", view_note, ADMIN, 0, 1, 0},
{"visit", visit, 0, 0, 1, 0},	/* V */
{"vlist", view_others_list, (LOWER_ADMIN | ADMIN), 0, 1, 0},
{"vlog", vlog, (LOWER_ADMIN|ADMIN), 0, 1, 0},
{0, 0, 0, 0, 0, 0},

{"wake", wake, 0, 0, 1, 0},
{"wall", wall, (LOWER_ADMIN | ADMIN), 0, 1, 0},
{"warn", warn, (WARN | SU ), 0, 1, 0},
{"where", where, 0, 0, 1, 0},
{"whisper", whisper, 0, 0, 1, 0},
{"who", who, 0, 0, 1, 0},	/* W */
{"with", with, 0, 0, 1, 0},
{"wordwrap", set_word_wrap, 0, 0, 1, 0},
{0, 0, 0, 0, 0, 0},

{"x", examine, 0, 0, 1, 0},	/* X */
{0, 0, 0, 0, 0, 0},

{"yoyo", yoyo, (SU | ADMIN), 0, 1, 0}, /* Y */
{0, 0, 0, 0, 0, 0},

{0, 0, 0, 0, 0, 0}		/* Z */
};

struct command *coms[27];
