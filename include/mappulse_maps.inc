#if defined _mappulse_maps_included
    #endinput
#endif
#define _mappulse_maps_included
#include <streamer>
#if !defined STREAMER_TAG_OBJECT
    #define STREAMER_TAG_OBJECT _
    #define STREAMER_TAG_AREA _
#endif
#include <mappulse_settings>
new MPM_Created, MPM_Prioritized, MPM_Trimmed;
stock MPM_Apply(modelid, &priority, &Float:sd, Float:dd)
{
    if (!MPM_ENABLE || priority != 0 || sd <= 0.0) return 0;
    #if MPM_ENABLE_PRIORITY
    switch (modelid)
    {
        case MPM_HIGH_MODELS: priority=MPM_PRIORITY_HIGH;
        case MPM_MEDIUM_MODELS: priority=MPM_PRIORITY_MEDIUM;
    }
    #endif
    if (priority) return 1;
    #if MPM_TRIM_DECOR

        if (modelid == MPM_DECOR_MODEL && sd >= MPM_DECOR_MIN_SD && sd <= MPM_DECOR_MAX_SD)
        {
            new Float:target=sd*MPM_DECOR_FACTOR;
            if (target<MPM_DECOR_MIN_SD) target=MPM_DECOR_MIN_SD;
            if (dd>target) target=dd;
            if (target<sd-MPM_DECOR_MIN_CHANGE) { sd=target; return 2; }
        }
    #endif
    #pragma unused dd
    return 0;
}
stock MPM_Record(STREAMER_TAG_OBJECT:id, change)
{
    if (id == STREAMER_TAG_OBJECT:INVALID_STREAMER_ID) return 0;
    MPM_Created++;
    if (change==1) MPM_Prioritized++;
    else if (change==2) MPM_Trimmed++;
    return 1;
}
stock STREAMER_TAG_OBJECT:MPM_Create(modelid, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz, worldid=-1, interiorid=-1, playerid=-1, Float:streamdistance=STREAMER_OBJECT_SD, Float:drawdistance=STREAMER_OBJECT_DD, STREAMER_TAG_AREA:areaid=STREAMER_TAG_AREA:-1, priority=0)
{
    new change;
    new bool:allowPolicy=MPD_AllowMapPolicy();
    if (allowPolicy && (worldid==0 || worldid==-1) && (interiorid==0 || interiorid==-1) && playerid==-1 && areaid==STREAMER_TAG_AREA:-1)
        change=MPM_Apply(modelid,priority,streamdistance,drawdistance);
    new STREAMER_TAG_OBJECT:id=CreateDynamicObject(modelid,x,y,z,rx,ry,rz,worldid,interiorid,playerid,streamdistance,drawdistance,areaid,priority);
    MPM_Record(id,change);
    return id;
}
stock STREAMER_TAG_OBJECT:MPM_CreateEx(modelid, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz, Float:streamdistance=STREAMER_OBJECT_SD, Float:drawdistance=STREAMER_OBJECT_DD, const worlds[]={-1}, const interiors[]={-1}, const players[]={-1}, const STREAMER_TAG_AREA:areas[]={STREAMER_TAG_AREA:-1}, priority=0, maxworlds=sizeof worlds, maxinteriors=sizeof interiors, maxplayers=sizeof players, maxareas=sizeof areas)
{
    new change;
    new bool:allowPolicy=MPD_AllowMapPolicy();

    if (allowPolicy && maxworlds==1 && maxinteriors==1 && maxplayers==1 && maxareas==1)
    {
        if ((worlds[0]==0 || worlds[0]==-1) && (interiors[0]==0 || interiors[0]==-1) && players[0]==-1 && areas[0]==STREAMER_TAG_AREA:-1)
            change=MPM_Apply(modelid,priority,streamdistance,drawdistance);
    }
    new STREAMER_TAG_OBJECT:id=CreateDynamicObjectEx(modelid,x,y,z,rx,ry,rz,streamdistance,drawdistance,worlds,interiors,players,areas,priority,maxworlds,maxinteriors,maxplayers,maxareas);
    MPM_Record(id,change);
    return id;
}
stock MPM_PrintStats()
{
    MPD_CheckStreamer();
    #if MPL_PRINT_BANNER
    print("----------------------------------------------------------");
    print("#   #   ###   ####   ####   #   #  #       ####  #####");
    print("## ##  #   #  #   #  #   #  #   #  #      #      #");
    print("# # #  #####  ####   ####   #   #  #       ###   ####");
    print("#   #  #   #  #      #      #   #  #          #  #");
    print("#   #  #   #  #      #       ###   #####  ####   #####");
    print("----------------------------------------------------------");
    #endif
    printf("[MapPulse v0.7.3 | Map include] Created=%d priority=%d decor=%d",MPM_Created,MPM_Prioritized,MPM_Trimmed);
    return 1;
}

#define CreateDynamicObject MPM_Create
#define CreateDynamicObjectEx MPM_CreateEx
