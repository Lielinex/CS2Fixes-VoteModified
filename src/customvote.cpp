/**
 * =============================================================================
 * CS2Fixes
 * Copyright (C) 2023-2026 Source2ZE
 * =============================================================================
 *
 * Custom vote commands using official localization keys.
 */

#include "customvote.h"
#include "commands.h"
#include "common.h"
#include "panoramavote.h"
#include "convar.h"
#include "entity/ccsplayercontroller.h"
#include "recipientfilters.h"
#include "tier0/memdbgon.h"

// ConVars for custom vote configuration
CConVar<float> g_cvarVoteCustomRatio("cs2f_vote_custom_ratio", FCVAR_NONE, "Ratio of yes votes needed to pass a custom vote", 0.5f, true, 0.0f, true, 1.0f);
CConVar<float> g_cvarVoteCustomDuration("cs2f_vote_custom_duration", FCVAR_NONE, "Duration of custom vote in seconds", 30.0f, true, 5.0f, false, 0.0f);

static void EmptyVoteHandler(YesNoVoteAction action, int param1, int param2) {}

static int GetCallerSlot(CCSPlayerController* pPlayer)
{
    return pPlayer ? pPlayer->GetPlayerSlot() : VOTE_CALLER_SERVER;
}

/**
 * !vote_for <content>
 * Uses "#SFUI_Vote_loadbackup" – displays "读取回合备份\n{content}？"
 */
CON_COMMAND_CHAT_FLAGS(vote_for, "<content> - Start a vote with custom content", ADMFLAG_GENERIC)
{
    if (args.ArgC() < 2)
    {
        if (player)
            ClientPrint(player, HUD_PRINTTALK, CHAT_PREFIX "Usage: !vote_for <content>");
        else
            ConMsg("Usage: c_vote_for <content>\n");
        return;
    }

    g_pPanoramaVoteHandler->Init();

    int callerSlot = GetCallerSlot(player);
    std::string content = args[1]; // 直接取第一个参数作为具体内容

    float duration = g_cvarVoteCustomDuration.Get();
    float ratio = g_cvarVoteCustomRatio.Get();

    const char* voteTitleKey = "#SFUI_Vote_loadbackup";

    auto resultCallback = [ratio](YesNoVoteInfo info) -> bool {
        int yes = info.yes_votes;
        int no = info.no_votes;
        int total = yes + no;

        if (total == 0)
        {
            ClientPrintAll(HUD_PRINTTALK, CHAT_PREFIX "Vote ended with no votes.");
            return false;
        }

        float yesRatio = (float)yes / total;
        bool passed = yesRatio >= ratio;

        if (passed)
            ClientPrintAll(HUD_PRINTTALK, CHAT_PREFIX "Vote passed (%.1f%% yes).", yesRatio * 100.0f);
        else
            ClientPrintAll(HUD_PRINTTALK, CHAT_PREFIX "Vote failed (%.1f%% yes, need %.0f%%).", yesRatio * 100.0f, ratio * 100.0f);
        return passed;
    };

    // 详情直接使用 content
    if (!g_pPanoramaVoteHandler->SendYesNoVoteToAll(duration, callerSlot, voteTitleKey, content.c_str(), resultCallback, EmptyVoteHandler))
    {
        if (player)
            ClientPrint(player, HUD_PRINTTALK, CHAT_PREFIX "Could not start vote (another vote may be in progress).");
        else
            ConMsg("Could not start vote (another vote may be in progress).\n");
    }
}

/**
 * !vote_restart - Vote to restart the match
 * Uses "#SFUI_vote_restart_game" – displays "重新开始比赛？"
 */
CON_COMMAND_CHAT_FLAGS(vote_restart, "- Vote to restart the match", ADMFLAG_GENERIC)
{
    g_pPanoramaVoteHandler->Init();

    int callerSlot = GetCallerSlot(player);
    float duration = g_cvarVoteCustomDuration.Get();
    float ratio = g_cvarVoteCustomRatio.Get();

    const char* voteTitleKey = "#SFUI_vote_restart_game";

    auto resultCallback = [ratio](YesNoVoteInfo info) -> bool {
        int yes = info.yes_votes;
        int no = info.no_votes;
        int total = yes + no;

        if (total == 0)
        {
            ClientPrintAll(HUD_PRINTTALK, CHAT_PREFIX "Vote ended with no votes.");
            return false;
        }

        float yesRatio = (float)yes / total;
        bool passed = yesRatio >= ratio;

        if (passed)
        {
            ClientPrintAll(HUD_PRINTTALK, CHAT_PREFIX "Vote passed (%.1f%%). Restarting match...", yesRatio * 100.0f);
            g_pEngineServer2->ServerCommand("mp_restartgame 1");
        }
        else
        {
            ClientPrintAll(HUD_PRINTTALK, CHAT_PREFIX "Vote failed (%.1f%% yes, need %.0f%%).", yesRatio * 100.0f, ratio * 100.0f);
        }
        return passed;
    };

    if (!g_pPanoramaVoteHandler->SendYesNoVoteToAll(duration, callerSlot, voteTitleKey, "", resultCallback, EmptyVoteHandler))
    {
        if (player)
            ClientPrint(player, HUD_PRINTTALK, CHAT_PREFIX "Could not start vote (another vote may be in progress).");
        else
            ConMsg("Could not start vote (another vote may be in progress).\n");
    }
}

void RegisterCustomVoteCommands()
{
    Message("Custom vote commands registered.\n");
}