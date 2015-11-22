  /* remote command */

void remote(player *p,char *str)
{
    char *msg,*pstring,*final;
    char *oldstack;
    player **list,**step;
    int i,n;

    command_type=PERSONAL|SEE_ERROR;

    if (p->saved_flags&BLOCK_TELLS) {
	tell_player(p,"You can't remote to other people when you yourself are blocking tells.\n");
	return;
    }  

    oldstack=stack;
    align(stack);
    list=(player **)stack;

    msg=next_space(str);
    if (!*msg) { 
	tell_player(p,"Format : remote <player(s)> <msg>\n");
	stack=oldstack;
	return;
    }
    *msg++=0;
    command_type|=SORE;
    n=global_tag(p,str);
    if (!n) {
	stack=oldstack;
	return;
    }
    for(step=list,i=0;i<n;i++,step++) 
	if (*step!=p) {
	    final=stack;
	    if (*msg=='\'')
		sprintf(stack,"%s%s\n",p->name,msg);
	    else
		sprintf(stack,"%s %s\n",p->name,msg);
	    stack=end_string(stack);
	    tell_player(*step,final);
	    stack=final;
	}

    if (sys_flags&EVERYONE_TAG || !(sys_flags&FAILED_COMMAND)) {
	pstring=tag_string(p,list,n);
	final=stack;
	if (*msg==39)
	    sprintf(stack,"You emote '%s%s' to %s.\n",p->name,msg,pstring);
	else
	    sprintf(stack,"You emote '%s %s' to %s.\n",p->name,msg,pstring);
	stack=strchr(stack,0);
	if (idlingstring(p,list,n))
	    strcpy(stack,idlingstring(p,list,n));
	stack=end_string(stack);
	tell_player(p,final);
    }
    cleanup_tag(list,n);
    stack=oldstack;
}
