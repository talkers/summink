/*
 * fix all those nasty warnings
 */

#ifndef SOLARIS
struct rlimit;
struct timval;
struct itimerval;
extern int      sigpause(int);
#if !defined(linux)
extern int      setrlimit(int, struct rlimit *);
#endif /* LINUX */
extern int      getitimer(int, struct itimerval *);
extern int      getrlimit(int, struct rlimit *);
#if !defined(linux)
extern int      setitimer(int, struct itimerval *, struct itimerval *);
#endif /* LINUX */
/*
 * extern char toupper(char); extern char tolower(char);
 */
extern void     lower_case(char *);
/*
 * extern int system(char *); extern int printf(char  *,...); extern char
 * *strncpy(char *,char *,int);
 */
#if !defined(linux)
extern int      strcasecmp(char *, char *);
#endif /* LINUX */
/*
 * extern int sscanf(char *,char *,...);
 */
#endif
