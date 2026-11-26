/*
 * #   #   ###   ####   ####   #   #  #       ####  #####
 * ## ##  #   #  #   #  #   #  #   #  #      #      #
 * # # #  #####  ####   ####   #   #  #       ###   ####
 * #   #  #   #  #      #      #   #  #          #  #
 * #   #  #   #  #      #       ###   #####  ####   #####
 * mappulse_live.inc | MapPulse v0.9.0
 * Created: 2026-11-26 | Author: 0xOgysha
 * Copyright (c) 2026 0xOgysha | MIT License
 * https://github.com/0xOgysha/MapPulse
 */

#if defined _mappulse_live_included
    #endinput
#endif
#define _mappulse_live_included
#include <a_samp>
#include <streamer>
#include <mappulse_settings>
#if MPL_USE_PAWNCMD
    #include <Pawn.CMD>
#endif
#define MPL_BUILD_CODE (9000)
new bool:MPL_Ready;
new MPL_Active[MAX_PLAYERS], MPL_ActiveIndex[MAX_PLAYERS], MPL_ActiveCount;
new MPL_SampleTotal, MPL_PollTotal, MPL_PollPeak, MPL_PollLimited;
new MPL_Stable[MAX_PLAYERS], MPL_Tier[MAX_PLAYERS];
new Float:MPL_SpeedSq[MAX_PLAYERS];
new bool:MPL_ReportOpen[MAX_PLAYERS];
new MPL_Timer, bool:MPL_Enabled = (MPL_START_ENABLED != 0), MPL_Cursor;

new bool:MPL_Known[MAX_PLAYERS], bool:MPL_Owned[MAX_PLAYERS], bool:MPL_Conflict[MAX_PLAYERS], bool:MPL_Position[MAX_PLAYERS], bool:MPL_Pressure[MAX_PLAYERS];
new Float:MPL_Base[MAX_PLAYERS], Float:MPL_Applied[MAX_PLAYERS];
new Float:MPL_X[MAX_PLAYERS], Float:MPL_Y[MAX_PLAYERS], Float:MPL_Z[MAX_PLAYERS], Float:MPL_Speed[MAX_PLAYERS];
new MPL_Last[MAX_PLAYERS], MPL_World[MAX_PLAYERS], MPL_Interior[MAX_PLAYERS], MPL_Adjust[MAX_PLAYERS];
new MPL_Test[MAX_PLAYERS]; 
new MPL_Samples[MAX_PLAYERS][2], MPL_Full[MAX_PLAYERS][2], MPL_Peak[MAX_PLAYERS][2], MPL_TestChanges[MAX_PLAYERS][2];
new MPL_TestJumps[MAX_PLAYERS][2], MPL_TestLimit[MAX_PLAYERS][2], bool:MPL_TestLimitChanged[MAX_PLAYERS][2];
new Float:MPL_Mean[MAX_PLAYERS][2], Float:MPL_Use[MAX_PLAYERS][2], Float:MPL_Route[MAX_PLAYERS][2], Float:MPL_MaxSpeed[MAX_PLAYERS][2];
new Float:MPL_TestStartX[MAX_PLAYERS][2], Float:MPL_TestStartY[MAX_PLAYERS][2], Float:MPL_TestEndX[MAX_PLAYERS][2], Float:MPL_TestEndY[MAX_PLAYERS][2];
new MPL_TestWorld[MAX_PLAYERS][2], MPL_TestInterior[MAX_PLAYERS][2];
forward MPL_Poll();

stock bool:MPL_Eligible(p)
{
    new playerState=GetPlayerState(p);
    return (playerState==PLAYER_STATE_ONFOOT || playerState==PLAYER_STATE_DRIVER || playerState==PLAYER_STATE_PASSENGER);
}
stock MPL_Restore(p)
{
    if (!MPL_Owned[p]) return 1;
    new Float:current;
    if (!Streamer_GetRadiusMultiplier(STREAMER_TYPE_OBJECT,current,p)) return 0;
    if (floatabs(current-MPL_Applied[p])<MPL_EPS)
    {
        if (floatabs(current-MPL_Base[p])>=MPL_EPS && !Streamer_SetRadiusMultiplier(STREAMER_TYPE_OBJECT,MPL_Base[p],p))
        { printf("[MapPulse] Restore failed player=%d",p); return 0; }
    }
    else MPL_Conflict[p]=true;
    MPL_Owned[p]=false;
    return 1;
}
stock MPL_ResetTest(p,slot)
{
    MPL_Samples[p][slot]=0; MPL_Full[p][slot]=0; MPL_Peak[p][slot]=0;
    MPL_Mean[p][slot]=0.0; MPL_Use[p][slot]=0.0; MPL_Route[p][slot]=0.0; MPL_MaxSpeed[p][slot]=0.0;
    MPL_TestChanges[p][slot]=0; MPL_TestJumps[p][slot]=0;
    MPL_TestLimit[p][slot]=0; MPL_TestLimitChanged[p][slot]=false;
    return 1;
}
stock MPL_PlayerReset(p)
{
    MPL_ReportOpen[p]=false; MPL_Known[p]=false; MPL_Owned[p]=false; MPL_Conflict[p]=false; MPL_Position[p]=false; MPL_Pressure[p]=false;
    MPL_Stable[p]=0; MPL_Tier[p]=0; MPL_SpeedSq[p]=0.0; MPL_Base[p]=0.0; MPL_Applied[p]=0.0; MPL_Last[p]=0; MPL_Adjust[p]=0; MPL_Test[p]=0; MPL_Speed[p]=0.0;
    MPL_ResetTest(p,0); MPL_ResetTest(p,1);
    return 1;
}
stock MPL_Join(p)
{
    if (!MPL_Ready || MPL_ActiveIndex[p]) return 0;
    MPL_PlayerReset(p);
    MPL_Known[p]=true;
    MPL_Active[MPL_ActiveCount]=p;
    MPL_ActiveIndex[p]=++MPL_ActiveCount;
    return 1;
}
stock MPL_Leave(p)
{
    new index=MPL_ActiveIndex[p]-1;
    if (index<0) return 0;
    new last=--MPL_ActiveCount;
    if (index!=last)
    {
        new moved=MPL_Active[last];
        MPL_Active[index]=moved;
        MPL_ActiveIndex[moved]=index+1;
    }
    MPL_ActiveIndex[p]=0;
    if (index<MPL_Cursor) MPL_Cursor=index;
    if (MPL_Cursor>=MPL_ActiveCount) MPL_Cursor=0;
    MPL_PlayerReset(p);
    return 1;
}
stock bool:MPL_CheckDependencies()
{
    #if MPL_USE_PAWNCMD
        #if defined PAWNCMD_VERSION
            new pcVersion=PAWNCMD_VERSION;
        #elseif defined PAWNCMD_INCLUDE_VERSION
            new pcVersion=PAWNCMD_INCLUDE_VERSION;
        #else
            new pcVersion=0;
        #endif
        return MPD_CheckLive(pcVersion);
    #else
        return MPD_CheckLive(0);
    #endif
}
stock MPL_Init()
{
    if (MPL_Timer) return 0;
    MPV_Start("Filterscript",MPL_BUILD_CODE);
    #if !MPL_USE_PAWNCMD
        print("[MapPulse] Command interface: native Pawn");
    #else
        print("[MapPulse] Command interface: Pawn.CMD");
    #endif
    MPL_Ready=MPL_CheckDependencies();
    if (!MPL_Ready)
    {
        MPL_Enabled=false;
        print("[MapPulse][ERROR] Individual optimizer not started. Fix dependencies and reload.");
        return 0;
    }
    MPL_Enabled=(MPL_START_ENABLED!=0);
    for (new p=0;p<MAX_PLAYERS;p++) if (IsPlayerConnected(p)) MPL_Join(p);
    MPL_Timer=SetTimer("MPL_Poll",MPL_TICK,true);
    printf("[MapPulse] Individual AUTO: %d | static map policy: optional",MPL_Enabled);
    print("----------------------------------------------------------");
    return 1;
}
stock MPL_Shutdown()
{
    if (MPL_Timer) KillTimer(MPL_Timer);
    MPL_Timer=0;
    MPV_Stop();
    MPL_Ready=false;
    for (new i=0;i<MPL_ActiveCount;i++) MPL_Restore(MPL_Active[i]);
    print("[MapPulse] Individual settings restored where owned; global map policy stays until maps are recreated/restart.");
    return 1;
}
stock MPL_Sample(p,slot,visible,limit,Float:x,Float:y,Float:dist,bool:jump)
{
    new n=++MPL_Samples[p][slot];
    MPL_Mean[p][slot]+=(float(visible)-MPL_Mean[p][slot])/float(n);
    MPL_Use[p][slot]+=(100.0*float(visible)/float(limit)-MPL_Use[p][slot])/float(n);
    if (visible>=limit) MPL_Full[p][slot]++;
    if (visible>MPL_Peak[p][slot]) MPL_Peak[p][slot]=visible;
    if (n==1)
    {
        MPL_TestStartX[p][slot]=x; MPL_TestStartY[p][slot]=y;
        MPL_TestWorld[p][slot]=GetPlayerVirtualWorld(p); MPL_TestInterior[p][slot]=GetPlayerInterior(p);
        MPL_TestLimit[p][slot]=limit;
    }
    else if (MPL_TestLimit[p][slot]!=limit) MPL_TestLimitChanged[p][slot]=true;
    MPL_TestEndX[p][slot]=x; MPL_TestEndY[p][slot]=y;
    if (jump) MPL_TestJumps[p][slot]++;
    else if (n>1) MPL_Route[p][slot]+=dist;
    if (MPL_Speed[p]>MPL_MaxSpeed[p][slot]) MPL_MaxSpeed[p][slot]=MPL_Speed[p];
    return 1;
}
stock MPL_TickPlayer(p,now)
{
    new dt=now-MPL_Last[p];
    if (!MPL_Enabled && !MPL_Test[p] && !MPL_Owned[p]) return 0;
    new interval=MPL_PLAYER_MS;
    if (!MPL_Test[p] && MPL_Position[p] && !MPL_Owned[p] && MPL_SpeedSq[p]<=MPL_IDLE_SPEED*MPL_IDLE_SPEED) interval=MPL_IDLE_MS;
    if (MPL_Last[p] && dt>=0 && dt<interval) return 0;
    MPL_Last[p]=now;
    if (!MPL_Enabled && !MPL_Test[p])
    {
        MPL_Restore(p);
        MPL_Position[p]=false;
        return 1;
    }
    if (MPL_Conflict[p] && !MPL_Test[p]) return 1;
    if (IsPlayerNPC(p) || !MPL_Eligible(p) || Streamer_IsToggleCameraUpdate(p) || !Streamer_IsToggleItemUpdate(p,STREAMER_TYPE_OBJECT))
    {
        if (MPL_Test[p] && MPL_Position[p]) MPL_TestJumps[p][MPL_Test[p]-1]++;
        MPL_Position[p]=false; MPL_SpeedSq[p]=0.0; MPL_Speed[p]=0.0;
        MPL_Stable[p]=0; MPL_Tier[p]=0; MPL_Restore(p);
        return 1;
    }
    new Float:x, Float:y, Float:z;
    if (!GetPlayerPos(p,x,y,z)) return 1;
    new world=GetPlayerVirtualWorld(p), interior=GetPlayerInterior(p);
    new Float:distSq=0.0, bool:jump=false;
    if (MPL_Position[p])
    {
        new Float:dx=x-MPL_X[p], Float:dy=y-MPL_Y[p], Float:dz=z-MPL_Z[p];
        distSq=dx*dx+dy*dy+dz*dz;
        new Float:elapsed=float(dt);
        new Float:jumpLimit=MPL_JUMP_DISTANCE;
        if (dt>0 && elapsed*MPL_JUMP_METERS_PER_MS>jumpLimit) jumpLimit=elapsed*MPL_JUMP_METERS_PER_MS;
        jump=(world!=MPL_World[p] || interior!=MPL_Interior[p] || dt<=0 || dt>MPL_MAX_GAP_MS || distSq>jumpLimit*jumpLimit);
        if (!jump)
        {
            new Float:scale=3600.0/elapsed;
            MPL_SpeedSq[p]=MPL_SpeedSq[p]*(1.0-MPL_SPEED_WEIGHT)+distSq*scale*scale*MPL_SPEED_WEIGHT;
        }
        else MPL_SpeedSq[p]=0.0;
    }
    else MPL_SpeedSq[p]=0.0;
    MPL_X[p]=x; MPL_Y[p]=y; MPL_Z[p]=z;
    MPL_World[p]=world; MPL_Interior[p]=interior; MPL_Position[p]=true;
    if (MPL_Test[p]) MPL_Speed[p]=floatsqroot(MPL_SpeedSq[p]);
    if (!MPL_Test[p] && !MPL_Owned[p] && (interior!=0 || MPL_SpeedSq[p]<MPL_WALK_SPEED*MPL_WALK_SPEED || jump))
    {
        MPL_Stable[p]=0; MPL_Tier[p]=0;
        return 1;
    }
    new visible=Streamer_CountVisibleItems(p,STREAMER_TYPE_OBJECT,1), limit=Streamer_GetVisibleItems(STREAMER_TYPE_OBJECT,p);
    if (limit<=0 || visible<0) return 1;
    if (MPL_Test[p]) MPL_Sample(p,MPL_Test[p]-1,visible,limit,x,y,floatsqroot(distSq),jump);
    if (!MPL_Enabled || MPL_Test[p]==1 || interior!=0 || MPL_Conflict[p] || jump)
    {
        MPL_Stable[p]=0; MPL_Tier[p]=0; MPL_Restore(p);
        return 1;
    }
    new Float:current;
    if (!Streamer_GetRadiusMultiplier(STREAMER_TYPE_OBJECT,current,p) || current<=0.0) return 1;
    if (MPL_Owned[p] && floatabs(current-MPL_Applied[p])>=MPL_EPS)
    {
        MPL_Owned[p]=false; MPL_Conflict[p]=true;
        printf("[MapPulse] Player %d: other script changed radius; yielding until reconnect.",p);
        return 1;
    }
    if (!MPL_Owned[p]) { MPL_Base[p]=current; MPL_Applied[p]=current; }
    if (visible*100>=limit*MPL_PRESSURE_HIGH) MPL_Pressure[p]=true;
    else if (visible*100<=limit*MPL_PRESSURE_LOW) MPL_Pressure[p]=false;
    if (MPL_Pressure[p] || MPL_SpeedSq[p]<MPL_WALK_SPEED*MPL_WALK_SPEED)
    {
        MPL_Stable[p]=0; MPL_Tier[p]=0;
        if (MPL_Owned[p] && MPL_Restore(p))
        {
            MPL_Adjust[p]=now;
            if (MPL_Test[p]) MPL_TestChanges[p][MPL_Test[p]-1]++;
        }
        return 1;
    }
    new tier=1;
    if (MPL_SpeedSq[p]>=MPL_FAST_SPEED*MPL_FAST_SPEED) tier=3;
    else if (MPL_SpeedSq[p]>=MPL_DRIVE_SPEED*MPL_DRIVE_SPEED) tier=2;
    if (tier!=MPL_Tier[p]) { MPL_Tier[p]=tier; MPL_Stable[p]=1; }
    else if (MPL_Stable[p]<MPL_STABLE_SAMPLES) MPL_Stable[p]++;
    if (MPL_Stable[p]<MPL_STABLE_SAMPLES || (MPL_Adjust[p] && now-MPL_Adjust[p]<MPL_ADJUST_MS)) return 1;
    new Float:factor=MPL_WALK_FACTOR;
    if (tier==3) factor=MPL_FAST_FACTOR;
    else if (tier==2) factor=MPL_DRIVE_FACTOR;
    if (visible>0)
    {
        new Float:budget=float(limit*MPL_TARGET_OCCUPANCY)/float(visible*100);
        new Float:relative=current/MPL_Base[p];
        budget*=relative*relative;
        if (factor*factor>budget && factor>MPL_DRIVE_FACTOR) factor=MPL_DRIVE_FACTOR;
        if (factor*factor>budget) factor=1.0;
    }
    if (factor<1.0) factor=1.0;
    new Float:target=MPL_Base[p]*factor;
    if (target>MPL_MAX_MULTIPLIER && target>MPL_Base[p]) target=(MPL_Base[p]>MPL_MAX_MULTIPLIER)?MPL_Base[p]:MPL_MAX_MULTIPLIER;
    if (floatabs(current-target)>=MPL_MIN_CHANGE)
    {
        if (Streamer_SetRadiusMultiplier(STREAMER_TYPE_OBJECT,target,p))
        {
            MPL_Applied[p]=target; MPL_Owned[p]=(floatabs(target-MPL_Base[p])>=MPL_EPS); MPL_Adjust[p]=now;
            if (MPL_Test[p]) MPL_TestChanges[p][MPL_Test[p]-1]++;
        }
    }
    return 1;
}
public MPL_Poll()
{
    if (!MPL_ActiveCount) return 1;
    new started=GetTickCount(), visited, work, count=MPL_ActiveCount;
    while (visited<count && visited<MPL_WORK_PER_TICK && MPL_ActiveCount)
    {
        if (MPL_Cursor>=MPL_ActiveCount) MPL_Cursor=0;
        new p=MPL_Active[MPL_Cursor++];
        visited++;
        if (!IsPlayerConnected(p)) MPL_Leave(p);
        else work+=MPL_TickPlayer(p,started);
        if (GetTickCount()-started>=MPL_CPU_BUDGET_MS) { MPL_PollLimited++; break; }
    }
    new elapsed=GetTickCount()-started;
    MPL_PollTotal++; MPL_SampleTotal+=work;
    if (elapsed>MPL_PollPeak) MPL_PollPeak=elapsed;
    return 1;
}
stock MPL_Show(p,const title[],const body[])
{

    #if MPL_DIALOG_BRIDGE
        CallRemoteFunction("MapPulse_ShowReport","iss",p,title,body);
    #else
        MPL_ReportOpen[p]=bool:ShowPlayerDialog(p,MPL_DIALOG_ID,DIALOG_STYLE_TABLIST_HEADERS,title,body,"Zatvori","");
    #endif
    return 1;
}
stock MPL_Admin(p)
{
    if (!IsPlayerAdmin(p)) { SendClientMessage(p,MPL_COLOR,"[MP] Potreban je RCON admin."); return 0; }
    if (!MPL_Ready) { SendClientMessage(p,MPL_COLOR,"[MP] Dependency check failed. Check server console and reload."); return 0; }
    return 1;
}
stock MPL_ABCell(p,slot,metric,output[],size)
{
    new n=MPL_Samples[p][slot];
    if (!n) { format(output,size,"{B6C2D0}-"); return 1; }
    switch (metric)
    {
        case 0: format(output,size,"%d",n);
        case 1: format(output,size,"%.1f",MPL_Mean[p][slot]);
        case 2: format(output,size,"%d",MPL_Peak[p][slot]);
        case 3: format(output,size,"%d",MPL_TestLimit[p][slot]);
        case 4: format(output,size,"%.1f%%",MPL_Use[p][slot]);
        case 5: format(output,size,"%.1f%%",100.0*float(MPL_Full[p][slot])/float(n));
        case 6: format(output,size,"%.0f m",MPL_Route[p][slot]);
        case 7: format(output,size,"%.0f km/h",MPL_MaxSpeed[p][slot]);
        case 8: format(output,size,"%d",MPL_TestChanges[p][slot]);
        case 9: format(output,size,"%d",MPL_TestJumps[p][slot]);
    }
    return 1;
}
stock MPL_Compare(p,bool:save)
{
    new body[2200], line[240], left[40], right[40];
    new const metrics[][]={"Uzorci","Prosek objekata","Maks. objekata","Limit","Popunjenost","Pun kapacitet","Duzina rute","Maks. brzina","Promene radijusa","Skokovi"};
    format(body,sizeof body,"{73BFFF}Metrika\t{E8EDF4}A - Original\t{75D69C}B - MapPulse\n");
    for (new i=0;i<sizeof metrics;i++)
    {
        MPL_ABCell(p,0,i,left,sizeof left); MPL_ABCell(p,1,i,right,sizeof right);
        format(line,sizeof line,"{C0CCD9}%s\t{FFFFFF}%s\t{FFFFFF}%s\n",metrics[i],left,right);
        strcat(body,line,sizeof body);
    }
    new a=MPL_Samples[p][0], b=MPL_Samples[p][1];
    if (a && b)
    {
        new Float:delta=100.0*float(MPL_Full[p][1])/float(b)-100.0*float(MPL_Full[p][0])/float(a);
        new Float:dx=MPL_TestStartX[p][0]-MPL_TestStartX[p][1], Float:dy=MPL_TestStartY[p][0]-MPL_TestStartY[p][1];
        new Float:ex=MPL_TestEndX[p][0]-MPL_TestEndX[p][1], Float:ey=MPL_TestEndY[p][0]-MPL_TestEndY[p][1];
        new bool:comparable=(!MPL_Conflict[p] && a>=MPL_AB_MIN_SAMPLES && b>=MPL_AB_MIN_SAMPLES && floatabs(MPL_Route[p][1]-MPL_Route[p][0])<=MPL_Route[p][0]*MPL_AB_ROUTE_REL+MPL_AB_ROUTE_ABS && dx*dx+dy*dy<=MPL_AB_ENDPOINT_DISTANCE*MPL_AB_ENDPOINT_DISTANCE && ex*ex+ey*ey<=MPL_AB_ENDPOINT_DISTANCE*MPL_AB_ENDPOINT_DISTANCE && MPL_TestWorld[p][0]==MPL_TestWorld[p][1] && MPL_TestInterior[p][0]==MPL_TestInterior[p][1] && MPL_TestJumps[p][0]==0 && MPL_TestJumps[p][1]==0 && !MPL_TestLimitChanged[p][0] && !MPL_TestLimitChanged[p][1] && MPL_TestLimit[p][0]==MPL_TestLimit[p][1] && floatabs(MPL_MaxSpeed[p][0]-MPL_MaxSpeed[p][1])<=MPL_MaxSpeed[p][0]*MPL_AB_SPEED_REL+MPL_AB_SPEED_ABS);
        format(line,sizeof line,"{73BFFF}Razlika B - A\t{FFFFFF}%.1f p.p.\tPun kapacitet\n{73BFFF}Razlika B - A\t{FFFFFF}%.1f p.p.\tPopunjenost\n{73BFFF}Razlika B - A\t{FFFFFF}%.1f\tProsek objekata\n",delta,MPL_Use[p][1]-MPL_Use[p][0],MPL_Mean[p][1]-MPL_Mean[p][0]);
        strcat(body,line,sizeof body);
        if (!comparable) strcat(body,"{F6C85F}Uporedivost\tNe\tPonovi istu rutu\n",sizeof body);
        else if (delta < -MPL_AB_DELTA_PP) strcat(body,"{75D69C}Rezultat\tManje zasicenje\tUporedivo\n",sizeof body);
        else if (delta > MPL_AB_DELTA_PP) strcat(body,"{F08A8A}Rezultat\tVece zasicenje\tUporedivo\n",sizeof body);
        else strcat(body,"{F6C85F}Rezultat\tBez bitne promene\tUporedivo\n",sizeof body);
        if (!MPL_TestChanges[p][1]) strcat(body,"{F6C85F}Napomena\tB bez promena\tRadijus\n",sizeof body);
    }
    else strcat(body,"{F6C85F}Testovi\t/mpbase i /mpon\tZavrsi /mpoff\n",sizeof body);
    strcat(body,"{B6C2D0}Napomena\tZasicenje nije FPS\tIsta mapa\n",sizeof body);
    if (save && MPL_SAVE_RESULTS)
    {
        new File:f=fopen(MPL_RESULTS_FILE,io_append);
        if (f) { new stamp[90], y,m,d,h,mi,s; getdate(y,m,d); gettime(h,mi,s); format(stamp,sizeof stamp,"\r\n[%04d-%02d-%02d %02d:%02d:%02d] player=%d\r\n",y,m,d,h,mi,s,p); fwrite(f,stamp); fwrite(f,body); fwrite(f,"\r\n"); fclose(f); }
    }
    return MPL_Show(p,"{73BFFF}MapPulse {FFFFFF}| A/B Performance",body);
}
stock MPL_Begin(p,slot)
{
    if (!MPL_Admin(p)) return 1;
    if (!MPL_Enabled && slot==1) return SendClientMessage(p,MPL_COLOR,"[MP] LIVE je ugasen. Prvo /mpliveon.");
    if (MPL_Test[p]) return SendClientMessage(p,MPL_COLOR,"[MP] Prvo zavrsi prethodni test: /mpoff.");
    if (!MPL_Known[p]) { MPL_PlayerReset(p); MPL_Known[p]=true; }
    if (!MPL_Restore(p)) return SendClientMessage(p,MPL_COLOR,"[MP] Nije uspelo vracanje radijusa; test nije pokrenut.");
    MPL_Stable[p]=0; MPL_Tier[p]=0; MPL_SpeedSq[p]=0.0; MPL_Pressure[p]=false; MPL_ResetTest(p,slot); MPL_Test[p]=slot+1; MPL_Position[p]=false; MPL_Speed[p]=0.0; MPL_Last[p]=0;
    if (slot==0) SendClientMessage(p,MPL_COLOR,"[MP] A: bazni radijus samo za tebe; globalna politika ostaje ista.");
    else SendClientMessage(p,MPL_COLOR,"[MP] B: MapPulse LIVE. Ponovi rutu A.");
    return 1;
}
stock MPL_Command_base(playerid)
{
    return MPL_Begin(playerid,0);
}
stock MPL_Command_on(playerid)
{
    return MPL_Begin(playerid,1);
}
stock MPL_Command_off(playerid)
{
    if (!MPL_Admin(playerid)) return 1;
    MPL_Test[playerid]=0; MPL_Position[playerid]=false; MPL_Pressure[playerid]=false; MPL_Stable[playerid]=0; MPL_Tier[playerid]=0; MPL_SpeedSq[playerid]=0.0; MPL_Last[playerid]=0;
    SendClientMessage(playerid,MPL_COLOR,"[MP] Test zavrsen. Automatski LIVE rezim nastavlja ako je ukljucen.");
    return MPL_Compare(playerid,true);
}
stock MPL_Command_compare(playerid)
{
    if (!MPL_Admin(playerid)) return 1;
    return MPL_Compare(playerid,false);
}
stock MPL_Command_status(playerid)
{
    if (!MPL_Admin(playerid)) return 1;
    new body[1800], line[350], update[48], Float:radius, Float:base;
    Streamer_GetRadiusMultiplier(STREAMER_TYPE_OBJECT,radius,playerid);
    base=MPL_Owned[playerid]?MPL_Base[playerid]:radius;
    MPV_Status(update,sizeof update);
    format(body,sizeof body,"{73BFFF}Kategorija\t{73BFFF}Parametar\t{73BFFF}Vrednost\n{C0CCD9}Sistem\tVerzija\t{FFFFFF}%s\n{C0CCD9}Sistem\tAutor\t{FFFFFF}%s\n{C0CCD9}Sistem\tNadogradnja\t%s\n{C0CCD9}Sistem\tAutomatski rezim\t%s\n",MPV_VERSION,MPV_AUTHOR,update,MPL_Enabled?"{75D69C}AKTIVAN":"{F08A8A}ISKLJUCEN");
    format(line,sizeof line,"{C0CCD9}Streaming\tRadijus / bazni\t{FFFFFF}%.2f / %.2f\n{C0CCD9}Streaming\tBrzina\t{FFFFFF}%.0f km/h\n{C0CCD9}Streaming\tObjekti / limit\t{FFFFFF}%d / %d\n{C0CCD9}Streaming\tZasicenje\t%s\n{C0CCD9}Streaming\tKonflikt\t%s\n{C0CCD9}Streaming\tA/B test\t{FFFFFF}%d\n",radius,base,floatsqroot(MPL_SpeedSq[playerid]),Streamer_CountVisibleItems(playerid,STREAMER_TYPE_OBJECT,1),Streamer_GetVisibleItems(STREAMER_TYPE_OBJECT,playerid),MPL_Pressure[playerid]?"{F6C85F}VISOKO":"{75D69C}NORMALNO",MPL_Conflict[playerid]?"{F08A8A}DA":"{75D69C}NE",MPL_Test[playerid]);
    strcat(body,line,sizeof body);
    format(line,sizeof line,"{C0CCD9}Scheduler\tIgraci\t{FFFFFF}%d\n{C0CCD9}Scheduler\tObrade\t{FFFFFF}%d\n{C0CCD9}Scheduler\tProlazi\t{FFFFFF}%d\n{C0CCD9}Scheduler\tNajduzi prolaz\t{FFFFFF}%d ms\n{C0CCD9}Scheduler\tDostignut budzet\t{FFFFFF}%d\n",MPL_ActiveCount,MPL_SampleTotal,MPL_PollTotal,MPL_PollPeak,MPL_PollLimited);
    strcat(body,line,sizeof body);
    return MPL_Show(playerid,"{73BFFF}MapPulse {FFFFFF}| Live Dashboard",body);
}
stock MPL_Command_cache(playerid)
{
    if (!MPL_Admin(playerid)) return 1;
    if (CallRemoteFunction("MPM_CacheMetric","i",0)!=MPL_BUILD_CODE)
        return SendClientMessage(playerid,MPL_COLOR,"[MP] Map include nije dostupan ili nije iste verzije. Proveri gamemode.");
    new enabled=CallRemoteFunction("MPM_CacheMetric","i",1);
    new hits=CallRemoteFunction("MPM_CacheMetric","i",2);
    new misses=CallRemoteFunction("MPM_CacheMetric","i",3);
    new occupied=CallRemoteFunction("MPM_CacheMetric","i",4);
    new evictions=CallRemoteFunction("MPM_CacheMetric","i",5);
    new geometry=CallRemoteFunction("MPM_CacheMetric","i",6);
    new unknown=CallRemoteFunction("MPM_CacheMetric","i",7);
    new created=CallRemoteFunction("MPM_CacheMetric","i",8);
    new prioritized=CallRemoteFunction("MPM_CacheMetric","i",9);
    new trimmed=CallRemoteFunction("MPM_CacheMetric","i",10);
    new slots=CallRemoteFunction("MPM_CacheMetric","i",11);
    new invalid=CallRemoteFunction("MPM_CacheMetric","i",12);
    new Float:rate=0.0, body[1200], line[420];
    if (hits>0 || misses>0) rate=100.0*float(hits)/(float(hits)+float(misses));
    format(body,sizeof body,"{73BFFF}Parametar\t{73BFFF}Vrednost\n{C0CCD9}Status\t%s\n{C0CCD9}Cache hits\t{FFFFFF}%d\n{C0CCD9}Cache misses\t{FFFFFF}%d\n{C0CCD9}Hit rate\t{FFFFFF}%.2f%%\n{C0CCD9}Popunjeni slotovi\t{FFFFFF}%d / %d\n{C0CCD9}Zamene slotova\t{FFFFFF}%d\n",enabled?"{75D69C}AKTIVAN":"{F6C85F}ISKLJUCEN",hits,misses,rate,occupied,slots,evictions);
    format(line,sizeof line,"{C0CCD9}Memorija niza\t{FFFFFF}%d B\n{C0CCD9}Geometrijski proracuni\t{FFFFFF}%d\n{C0CCD9}Nepoznata geometrija\t{FFFFFF}%d\n{C0CCD9}Neispravan prag\t{FFFFFF}%d\n{C0CCD9}Kreirani objekti\t{FFFFFF}%d\n{C0CCD9}Prioriteti\t{FFFFFF}%d\n{C0CCD9}Skracena udaljenost\t{FFFFFF}%d\n",slots*8,geometry,unknown,invalid,created,prioritized,trimmed);
    strcat(body,line,sizeof body);
    strcat(body,"{B6C2D0}Izvor\tMap include / AMX\n{B6C2D0}Autor\t0xOgysha",sizeof body);
    return MPL_Show(playerid,"{73BFFF}MapPulse {FFFFFF}| Model Cache",body);
}
stock MPL_Command_liveoff(playerid)
{
    if (!MPL_Admin(playerid)) return 1;
    MPL_Enabled=false;
    for (new i=0;i<MPL_ActiveCount;i++) { new p=MPL_Active[i]; MPL_Test[p]=0; MPL_Position[p]=false; MPL_Last[p]=0; }
    return SendClientMessage(playerid,MPL_COLOR,"[MP] LIVE ugasen; radijusi se vracaju postepeno. Globalna politika ostaje do restarta.");
}
stock MPL_Command_liveon(playerid)
{
    if (!MPL_Admin(playerid)) return 1;
    MPL_Enabled=true;
    return SendClientMessage(playerid,MPL_COLOR,"[MP] AUTO ukljucen za sve.");
}

#if MPL_USE_PAWNCMD
CMD:mpbase(playerid,params[])
{
    #pragma unused params
    return MPL_Command_base(playerid);
}
CMD:mpon(playerid,params[])
{
    #pragma unused params
    return MPL_Command_on(playerid);
}
CMD:mpoff(playerid,params[])
{
    #pragma unused params
    return MPL_Command_off(playerid);
}
CMD:mpcompare(playerid,params[])
{
    #pragma unused params
    return MPL_Command_compare(playerid);
}
CMD:mpstatus(playerid,params[])
{
    #pragma unused params
    return MPL_Command_status(playerid);
}
CMD:mpcache(playerid,params[])
{
    #pragma unused params
    return MPL_Command_cache(playerid);
}
CMD:mpliveoff(playerid,params[])
{
    #pragma unused params
    return MPL_Command_liveoff(playerid);
}
CMD:mpliveon(playerid,params[])
{
    #pragma unused params
    return MPL_Command_liveon(playerid);
}
#endif

stock MPL_HandleCommand(playerid,const cmdtext[])
{
    if (!strcmp(cmdtext,"/mpbase",true)) return MPL_Command_base(playerid);
    if (!strcmp(cmdtext,"/mpon",true)) return MPL_Command_on(playerid);
    if (!strcmp(cmdtext,"/mpoff",true)) return MPL_Command_off(playerid);
    if (!strcmp(cmdtext,"/mpcompare",true)) return MPL_Command_compare(playerid);
    if (!strcmp(cmdtext,"/mpstatus",true)) return MPL_Command_status(playerid);
    if (!strcmp(cmdtext,"/mpcache",true)) return MPL_Command_cache(playerid);
    if (!strcmp(cmdtext,"/mpliveoff",true)) return MPL_Command_liveoff(playerid);
    if (!strcmp(cmdtext,"/mpliveon",true)) return MPL_Command_liveon(playerid);
    return 0;
}
