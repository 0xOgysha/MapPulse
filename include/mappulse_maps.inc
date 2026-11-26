/*
 * #   #   ###   ####   ####   #   #  #       ####  #####
 * ## ##  #   #  #   #  #   #  #   #  #      #      #
 * # # #  #####  ####   ####   #   #  #       ###   ####
 * #   #  #   #  #      #      #   #  #          #  #
 * #   #  #   #  #      #       ###   #####  ####   #####
 * mappulse_maps.inc | MapPulse v0.9.0
 * Created: 2026-11-26 | Author: 0xOgysha
 * Copyright (c) 2026 0xOgysha | MIT License
 * https://github.com/0xOgysha/MapPulse
 */

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
#define MPM_BUILD_CODE (9000)
new MPM_Created, MPM_Prioritized, MPM_Trimmed;
new bool:MPM_Started;
stock MPM_Init()
{
    if (!MPM_Started)
    {
        MPM_Started=true;
        MPV_Start("Map include",MPM_BUILD_CODE);
        MPD_CheckStreamer();
    }
    return 1;
}
new MPM_CacheHits, MPM_CacheMisses, MPM_CacheUsed, MPM_CacheEvictions;
new MPM_GeometryEvaluations, MPM_GeometryUnknown, MPM_GeometryInvalid;
#if MPM_MODEL_CACHE
new MPM_CacheKeys[MPM_CACHE_MAX_MODELS], MPM_CachePriorities[MPM_CACHE_MAX_MODELS];
#endif

stock bool:MPM_GetModelGeometry(modelid, &Float:x, &Float:y, &Float:z)
{
    switch (modelid)
    {
        case 18762: { x=1.0; y=1.0; z=5.0; }
        case 19353: { x=0.1787764; y=3.21515; z=3.50048; }
        case 19377,19378,19379: { x=0.1787764; y=9.63762; z=10.50146; }
        case 19454,19455,19456: { x=0.1787764; y=9.63762; z=3.50048; }
        default: return false;
    }
    return true;
}

stock MPM_ClassifyModel(modelid)
{
    #if MPM_ENABLE_PRIORITY
        switch (modelid)
        {
            case MPM_HIGH_MODELS: return MPM_PRIORITY_HIGH;
            case MPM_MEDIUM_MODELS: return MPM_PRIORITY_MEDIUM;
        }
        #if MPM_AUTO_CLASSIFY
            if (MPM_GEOMETRY_MEDIUM <= 0.0 || MPM_GEOMETRY_HIGH < MPM_GEOMETRY_MEDIUM)
            {
                MPM_GeometryInvalid++;
                return 0;
            }
            new Float:x, Float:y, Float:z;
            if (!MPM_GetModelGeometry(modelid,x,y,z))
            {
                MPM_GeometryUnknown++;
                return 0;
            }
            MPM_GeometryEvaluations++;
            new Float:score=x*y+y*z+z*x;
            if (score >= MPM_GEOMETRY_HIGH) return MPM_PRIORITY_HIGH;
            if (score >= MPM_GEOMETRY_MEDIUM) return MPM_PRIORITY_MEDIUM;
        #endif
    #else
        #pragma unused modelid
    #endif
    return 0;
}

stock MPM_GetModelPriority(modelid)
{
    #if MPM_MODEL_CACHE
        if (modelid >= 0 && modelid < 2147483647)
        {
            new key=modelid+1, slot=modelid%MPM_CACHE_MAX_MODELS;
            if (MPM_CacheKeys[slot]==key)
            {
                MPM_CacheHits++;
                return MPM_CachePriorities[slot];
            }
            MPM_CacheMisses++;
            if (MPM_CacheKeys[slot]) MPM_CacheEvictions++;
            else MPM_CacheUsed++;
            new priority=MPM_ClassifyModel(modelid);
            MPM_CacheKeys[slot]=key;
            MPM_CachePriorities[slot]=priority;
            return priority;
        }
    #endif
    MPM_CacheMisses++;
    return MPM_ClassifyModel(modelid);
}

forward MPM_CacheMetric(metric);
public MPM_CacheMetric(metric)
{
    switch (metric)
    {
        case 0: return MPM_BUILD_CODE;
        case 1: return MPM_MODEL_CACHE;
        case 2: return MPM_CacheHits;
        case 3: return MPM_CacheMisses;
        case 4: return MPM_CacheUsed;
        case 5: return MPM_CacheEvictions;
        case 6: return MPM_GeometryEvaluations;
        case 7: return MPM_GeometryUnknown;
        case 8: return MPM_Created;
        case 9: return MPM_Prioritized;
        case 10: return MPM_Trimmed;
        case 11: return MPM_MODEL_CACHE ? MPM_CACHE_MAX_MODELS : 0;
        case 12: return MPM_GeometryInvalid;
    }
    return 0;
}

stock MPM_Apply(modelid, &priority, &Float:sd, Float:dd)
{
    if (!MPM_ENABLE || priority != 0 || sd <= 0.0) return 0;
    #if MPM_ENABLE_PRIORITY
    priority=MPM_GetModelPriority(modelid);
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
    if (!MPM_Started) MPM_Init();
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
    if (!MPM_Started) MPM_Init();
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
    MPM_Init();
    printf("[MapPulse | Map include] Author: %s | Created=%d priority=%d decor=%d",MPV_AUTHOR,MPM_Created,MPM_Prioritized,MPM_Trimmed);
    printf("[MapPulse | Cache] Hits=%d misses=%d occupied=%d evictions=%d geometry=%d unknown=%d",MPM_CacheHits,MPM_CacheMisses,MPM_CacheUsed,MPM_CacheEvictions,MPM_GeometryEvaluations,MPM_GeometryUnknown);
    return 1;
}

#define CreateDynamicObject MPM_Create
#define CreateDynamicObjectEx MPM_CreateEx
