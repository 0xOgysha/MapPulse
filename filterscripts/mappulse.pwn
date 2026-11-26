#define FILTERSCRIPT

#pragma dynamic 16384
#include "mappulse_live.inc"
public OnFilterScriptInit() { MPL_Init(); return 1; }
public OnFilterScriptExit() { MPL_Shutdown(); return 1; }
public OnPlayerConnect(playerid) { MPL_Join(playerid); return 1; }
public OnPlayerDisconnect(playerid,reason)
{
    #pragma unused reason
    MPL_Leave(playerid); return 1;
}
public OnDialogResponse(playerid,dialogid,response,listitem,inputtext[])
{
    #pragma unused response, listitem, inputtext
    if (dialogid!=MPL_DIALOG_ID || !MPL_ReportOpen[playerid]) return 0;
    MPL_ReportOpen[playerid]=false;
    return 1;
}

#if !MPL_USE_PAWNCMD
public OnPlayerCommandText(playerid,cmdtext[])
{
    return MPL_HandleCommand(playerid,cmdtext);
}
#endif
