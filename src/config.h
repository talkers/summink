/*
 * config.h
 */

/* define this for ULTRIX */

#undef  ULTRIX
#define ULTRIX_PLAYER_LIM 200

/*
#if !defined(linux)
 #define ULTRIX
#endif
*/

/* define this for Solaris 2.2 */

#undef SOLARIS

/* the system equivalent of the timelocal command */

#if !defined(linux)
 /* this for SunOS  */
 #define TIMELOCAL(x) timelocal(x)
#else
 /* This for Linux */
 #define TIMELOCAL(x) mktime(x)
#endif /* LINUX */


/* this for ULTRIX and Solaris */
#ifdef ULTRIX 
 #define TIMELOCAL(x) mktime(x)
#endif /* ULTIRX */
#ifdef SOLARIS
 #define TIMELOCAL(x) mktime(x)
#endif /* SOLARIS */

/* default port no */

#define DEFAULT_PORT 3456

/* Root directory */

/* NB: the trailing / *is* important!!! */
#define ROOT "/home/rob/code/gideon/portal/bases/summink/src/"

/* path for the test alive socket */

#define SOCKET_PATH "junk/alive_socket"

/*
 * this is the room where people get chucked to when they enter the program
 */

#define ENTRANCE_ROOM "summink.main"

/* this is the size of the stack for normal functions */

#define STACK_SIZE 200001

/* largest permitted log size */

#define MAX_LOG_SIZE 5000

/* saved hash table size */

#define HASH_SIZE 64

/* note hash table size */

#define NOTE_HASH_SIZE 40

/* speed of the prog (number of clicks a second) */

#define TIMER_CLICK 5

/* speed of the virtual timer */

#define VIRTUAL_CLICK 10000

/* time in seconds between every player file sync */

#define SYNC_TIME 60

/* time in seconds between every full note sync */

#define NOTE_SYNC_TIME 1800

/* defines how many lines EW-three thinks a terminal has */

#define TERM_LINES  18

/* enable or disable malloc debugging */

#undef MALLOC_DEBUG

/* timeout on news articles */

/* 7 Days */
#define NEWS_TIMEOUT (7 * 60 * 60 * 24)

/* timeout on a mail article */

/* 7 Days */
#define MAIL_TIMEOUT (7 * 60 * 60 * 24)

/* timeout on players */

/* 3 Months */
#define PLAYER_TIMEOUT (3 * 30 * 60 * 60 * 24)


/* how many names can be included in a pipe */

#define NAME_MAX_IN_PIPE 10

/* maximum number of you and yours and stuff in a pipe */

#define YOU_MAX_IN_PIPE 3


/* which malloc routines to use */

#define MALLOC malloc

#define FREE free

/* maximum resident memory size of the program */

#define MAX_RES 1048576




/* this stuff for testing on a PC */

#undef PC

#ifdef PC
#undef tolower
#define tolower(x) mytolower(x)
#ifndef PC_FILE
extern char     mytolower();
#endif
#endif
