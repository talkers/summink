
/*
 * KCL TRAP
 * to be placed in socket.c
 */
if ( (no1==137) && (no2==73) && (no3!=16) && (no4!=2) )  
{				/* all kcl machines except 137.73.16.2  */
    				/* no time restrictions - 24hr, 7dy     */
    destroy_player(p);		/* unceremonious, I know, but necessary */
    /* that should do the job, failing that, uncomment the next line    */
    /* out_pack_current++; */
    return;
}

