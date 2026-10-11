#include "GameManager.hpp"
#include "AsciiManager.hpp"
#include "BulletManager.hpp"
#include "ChainPriorities.hpp"
#include "EclManager.hpp"
#include "EffectManager.hpp"
#include "EnemyManager.hpp"
#include "Global.hpp"
#include "Gui.hpp"
#include "Player.hpp"
#include "ReplayManager.hpp"
#include "ResultScreen.hpp"
#include "ScreenEffect.hpp"
#include "SoundPlayer.hpp"
#include "Stage.hpp"
#include "Supervisor.hpp"
#include "ZunTimer.hpp"

#include <d3d8types.h>
#include <d3dx8math.h>

namespace th06
{

u32 g_ExtraLivesScores[] = {10000000, 20000000, 40000000, 60000000, 1900000000};

struct DifficultyInfo
{
    u32 rank;
    u32 minRank;
    u32 maxRank;
};
ZUN_ASSERT_TYPE(DifficultyInfo, 0xc, 4);

BSS_SORT(H1) GameManager g_GameManager;
BSS_SORT(H2) ChainElem g_GameManagerCalcChain;
BSS_SORT(H3) ChainElem g_GameManagerDrawChain;

#define MAX_SCORE 999999999

#define DEMO_FADEOUT_FRAMES 3600
#define DEMO_FRAMES 3720

#define GUI_SCORE_STEP 78910

#define MAX_LIVES 8

ZunBool GameManager::IsInBounds(f32 x, f32 y, f32 width, f32 height)
{
    if (width / 2.0f + x < GAME_REGION_LEFT)
    {
        return false;
    }
    if ((x - width / 2.0f) > /*GAME_REGION_LEFT +*/ g_GameManager.gameRegionSize.x)
    {
        return false;
    }
    if (height / 2.0f + y < GAME_REGION_TOP)
    {
        return false;
    }
    if (y - height / 2.0f > /*GAME_REGION_TOP +*/ g_GameManager.gameRegionSize.y)
    {
        return false;
    }

    return true;
}

#pragma var_order(scoreIncrement, isInMenu)
ChainCallbackResult GameManager_OnUpdate(GameManager *gameManager)
{
    ZunBool isInMenu;
    u32 scoreIncrement;

    if (gameManager->demoMode)
    {
        if (WAS_PRESSED(TH_BUTTON_ANY))
        {
            g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU;
        }
        gameManager->demoFrames++;
        if (gameManager->demoFrames == DEMO_FADEOUT_FRAMES)
        {
            ScreenEffect_RegisterChain(SCREEN_EFFECT_FADE_OUT, 120, 0x000000, 0, 0);
        }
        if (gameManager->demoFrames >= DEMO_FRAMES)
        {
            g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU;
        }
    }
    if (!gameManager->isInRetryMenu && !gameManager->isInGameMenu && !gameManager->demoMode &&
        WAS_PRESSED(TH_BUTTON_MENU))
    {
        gameManager->isInGameMenu = 1;
        g_GameManager.gameRegionScreenPos.x = GAME_REGION_POS_X;
        g_GameManager.gameRegionScreenPos.y = GAME_REGION_POS_Y;
        g_GameManager.gameRegionSize.x = GAME_REGION_WIDTH;
        g_GameManager.gameRegionSize.y = GAME_REGION_HEIGHT;
#if !TRIALBUILD
        g_Supervisor.forceRedrawFrames = 3;
#endif
    }

    if (!gameManager->isInRetryMenu && !gameManager->isInGameMenu)
    {
        isInMenu = true;
    }
    else
    {
        isInMenu = false;
    }

    gameManager->isInMenu = isInMenu;

    g_Supervisor.viewport.X = gameManager->gameRegionScreenPos.x;
    g_Supervisor.viewport.Y = gameManager->gameRegionScreenPos.y;
    g_Supervisor.viewport.Width = gameManager->gameRegionSize.x;
    g_Supervisor.viewport.Height = gameManager->gameRegionSize.y;
    g_Supervisor.viewport.MinZ = 0.5f;
    g_Supervisor.viewport.MaxZ = 1.0f;

    GameManager_SetupCamera(0);

    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
    g_Supervisor.d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, g_Stage.skyFog.color, 1.0f, 0);

    // Seems like gameManager->isInGameMenu was supposed to have 3 states, but all the times it ends up checking both
    if (gameManager->isInGameMenu == 1 || gameManager->isInGameMenu == 2 || gameManager->isInRetryMenu)
    {
        return CHAIN_CALLBACK_RESULT_BREAK;
    }

    if (gameManager->score >= MAX_SCORE + 1)
    {
        gameManager->score = MAX_SCORE - 9;
    }
    if (gameManager->guiScore != gameManager->score)
    {
        if (gameManager->score < gameManager->guiScore)
        {
            gameManager->score = gameManager->guiScore;
        }

        scoreIncrement = (gameManager->score - gameManager->guiScore) >> 5;
        if (scoreIncrement >= GUI_SCORE_STEP)
        {
            scoreIncrement = GUI_SCORE_STEP;
        }
        else if (scoreIncrement < 10)
        {
            scoreIncrement = 10;
        }
        scoreIncrement = scoreIncrement - scoreIncrement % 10;

        if (gameManager->nextScoreIncrement < scoreIncrement)
        {
            gameManager->nextScoreIncrement = scoreIncrement;
        }
        if (gameManager->guiScore + gameManager->nextScoreIncrement > gameManager->score)
        {
            gameManager->nextScoreIncrement = gameManager->score - gameManager->guiScore;
        }

        gameManager->guiScore += gameManager->nextScoreIncrement;
        if (gameManager->guiScore >= gameManager->score)
        {
            gameManager->nextScoreIncrement = 0;
            gameManager->guiScore = gameManager->score;
        }
        if (gameManager->extraLives >= 0 && g_ExtraLivesScores[gameManager->extraLives] <= gameManager->guiScore)
        {
            if (gameManager->livesRemaining < MAX_LIVES)
            {
                gameManager->livesRemaining++;
                g_SoundPlayer.PlaySoundByIdx(SOUND_1UP);
            }
            g_Gui.flags.flag0 = 2;
            gameManager->extraLives++;
            g_GameManager.IncreaseSubrank(200);
        }
        if (gameManager->highScore < gameManager->guiScore)
        {
            gameManager->highScore = gameManager->guiScore;
        }
    }
    gameManager->gameFrames++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult GameManager_OnDraw(GameManager *gameManager)
{
    if (gameManager->isInGameMenu)
    {
        gameManager->isInGameMenu = 2;
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(failedToLoadReplay, catk, i, catkCursor, scoredat, clrdIdx)
static ZunResult GameManager_AddedCallback(GameManager *mgr)
{
    static const char *g_EclFiles[] = {"dummy",
                                       "data/ecldata1.ecl",
                                       "data/ecldata2.ecl",
                                       "data/ecldata3.ecl",
                                       "data/ecldata4.ecl",
                                       "data/ecldata5.ecl",
                                       "data/ecldata6.ecl",
                                       "data/ecldata7.ecl"};
    static const char *g_AnmStageFiles[][2] = {
        {"dummy", "dummy"},
        {"data/stg1enm.anm", "data/stg1enm2.anm"},
        {"data/stg2enm.anm", "data/stg2enm2.anm"},
        {"data/stg3enm.anm", NULL},
        {"data/stg4enm.anm", NULL},
        {"data/stg5enm.anm", "data/stg5enm2.anm"},
        {"data/stg6enm.anm", "data/stg6enm2.anm"},
        {"data/stg7enm.anm", "data/stg7enm2.anm"},
    };
    static DifficultyInfo g_DifficultyInfoForReplay[] = {
        // rank, minRank, maxRank
        /* EASY    */ {16, 12, 20},
        /* NORMAL  */ {16, 10, 32},
        /* HARD    */ {16, 10, 32},
        /* LUNATIC */ {16, 10, 32},
        /* EXTRA   */ {16, 14, 18},
    };
    static DifficultyInfo g_DifficultyInfo[] = {
        // rank, minRank, maxRank
        /* EASY    */ {16, 12, 20},
        /* NORMAL  */ {16, 10, 32},
        /* HARD    */ {16, 10, 32},
        /* LUNATIC */ {16, 10, 32},
        /* EXTRA   */ {16, 14, 18},
    };

    ScoreDat *scoredat;
    u32 clrdIdx;
    u32 catkCursor;
    i32 i;
    Catk *catk;

    ZunBool failedToLoadReplay = false;
    g_Supervisor.d3dDevice->ResourceManagerDiscardBytes(0);
    if (g_Supervisor.curState != SUPERVISOR_STATE_NEXT_STAGE)
    {
#if BUILD_VERSION >= BUILD_VERSION_102h
        g_Supervisor.defaultConfig.bombCount = g_GameManager.bombsRemaining;
        g_Supervisor.defaultConfig.lifeCount = g_GameManager.livesRemaining;
#endif
        mgr->gameRegionScreenPos.x = GAME_REGION_POS_X;
        mgr->gameRegionScreenPos.y = GAME_REGION_POS_Y;
        mgr->gameRegionSize.x = GAME_REGION_WIDTH;
        mgr->gameRegionSize.y = GAME_REGION_HEIGHT;
        mgr->playerMovementAreaTopLeftPos.x = GAME_REGION_LEFT + 8.0f;
        mgr->playerMovementAreaTopLeftPos.y = GAME_REGION_TOP + 16.0f;
        mgr->playerMovementAreaSize.x = GAME_REGION_WIDTH - 8.0f * 2.0f;
        mgr->playerMovementAreaSize.y = GAME_REGION_HEIGHT - 16.0f * 2.0f;
        mgr->counat = 0;
        mgr->guiScore = 0;
        mgr->score = 0;
        mgr->nextScoreIncrement = 0;
        mgr->highScore = 100000;
        mgr->currentPower = 0;
        mgr->numRetries = 0;
        if (mgr->currentStage >= 6)
        {
            mgr->difficulty = EXTRA;
        }
        if (mgr->difficulty < EXTRA)
        {
            mgr->extraLives = 0;
        }
        else
        {
            mgr->extraLives = 4;
        }
        g_GameManager.powerItemCountForScore = 0;
        mgr->rank = 8;
        mgr->grazeInTotal = 0;
        mgr->pointItemsCollected = 0;
        for (catk = mgr->catk, i = 0; i < CATK_COUNT; i++, catk++)
        {
            // Randomize catk content.
            for (catkCursor = 0; catkCursor < sizeof(Catk) / sizeof(u16); catkCursor++)
            {
                ((u16 *)catk)[catkCursor] = g_Rng.GetRandomU16();
            }
            catk->base.magic = CATK_MAGIC;
            catk->base.unkLen = sizeof(Catk);
            catk->base.th6kLen = sizeof(Catk);
            catk->base.version = TH6K_VERSION;
            catk->idx = i;
            catk->numAttempts = 0;
            catk->numSuccess = 0;
        }
        scoredat = OpenScore("score.dat");
        g_GameManager.highScore =
            GetHighScore(scoredat, NULL, GameManager_CharacterShotType(), g_GameManager.difficulty);
        ParseCatk(scoredat, mgr->catk);
        ParseClrd(scoredat, mgr->clrd);
        ParsePscr(scoredat, (Pscr *)mgr->pscr);
        if (mgr->isInPracticeMode)
        {
            g_GameManager.highScore =
                mgr->pscr[GameManager_CharacterShotType()][g_GameManager.currentStage][g_GameManager.difficulty].score;
        }
        ReleaseScoreDat(scoredat);
        mgr->rank = g_DifficultyInfo[g_GameManager.difficulty].rank;
        mgr->minRank = g_DifficultyInfo[g_GameManager.difficulty].minRank;
        mgr->maxRank = g_DifficultyInfo[g_GameManager.difficulty].maxRank;
        mgr->deaths = 0;
        mgr->bombsUsed = 0;
        mgr->spellcardsCaptured = 0;
    }
    else
    {
        mgr->guiScore = mgr->score;
        mgr->nextScoreIncrement = 0;
    }
    mgr->subRank = 0;
    mgr->pointItemsCollectedInStage = 0;
    mgr->grazeInStage = 0;
    mgr->isInGameMenu = 0;
    mgr->currentStage++;
    if (!g_GameManager.isInReplay)
    {
        clrdIdx = GameManager_CharacterShotType();
        if (mgr->numRetries == 0 &&
            mgr->clrd[clrdIdx].stagesClearedWithoutContinues[g_GameManager.difficulty] < mgr->currentStage - 1)
        {
            mgr->clrd[clrdIdx].stagesClearedWithoutContinues[g_GameManager.difficulty] = mgr->currentStage - 1;
        }
        if (mgr->clrd[clrdIdx].stagesCleared[g_GameManager.difficulty] < mgr->currentStage - 1)
        {
            mgr->clrd[clrdIdx].stagesCleared[g_GameManager.difficulty] = mgr->currentStage - 1;
        }
    }
    if (mgr->isInPracticeMode)
    {
        switch (mgr->currentStage)
        {
        case 1:
            break;
        case 2:
            mgr->currentPower = MAX_POWER / 2;
            break;
        default:
            mgr->currentPower = MAX_POWER;
        }
    }
    g_Supervisor.LoadPbg3(CM_PBG3_INDEX, TH_CM_DAT_FILE);
    g_Supervisor.LoadPbg3(ST_PBG3_INDEX, TH_ST_DAT_FILE);
    if (g_GameManager.isInReplay == TRUE)
    {
        if (ReplayManager_RegisterChain(true, g_GameManager.replayFile) != ZUN_SUCCESS)
        {
            failedToLoadReplay = true;
        }
        while (g_ExtraLivesScores[mgr->extraLives] <= mgr->guiScore)
        {
            mgr->extraLives++;
        }
        mgr->minRank = g_DifficultyInfoForReplay[g_GameManager.difficulty].minRank;
        mgr->maxRank = g_DifficultyInfoForReplay[g_GameManager.difficulty].maxRank;
    }
    g_Rng.generationCount = 0;
    mgr->randomSeed = g_Rng.seed;
    if (Stage_RegisterChain(mgr->currentStage) != ZUN_SUCCESS)
    {
        g_GameErrorContext.Log(TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_STAGE);
        return ZUN_ERROR;
    }

    if (Player_RegisterChain(0) != ZUN_SUCCESS)
    {
        g_GameErrorContext.Log(TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_PLAYER);
        return ZUN_ERROR;
    }
    if (BulletManager_RegisterChain("data/etama.anm") != ZUN_SUCCESS)
    {
        g_GameErrorContext.Log(TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_BULLETMANAGER);
        return ZUN_ERROR;
    }
    if (EnemyManager_RegisterChain(g_AnmStageFiles[mgr->currentStage][0], g_AnmStageFiles[mgr->currentStage][1]) !=
        ZUN_SUCCESS)
    {
        g_GameErrorContext.Log(TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_ENEMYMANAGER);
        return ZUN_ERROR;
    }
    if (g_EclManager.Load(g_EclFiles[mgr->currentStage]) != ZUN_SUCCESS)
    {
        g_GameErrorContext.Log(TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_ECLMANAGER);
        return ZUN_ERROR;
    }
    if (EffectManager_RegisterChain() != ZUN_SUCCESS)
    {
        g_GameErrorContext.Log(TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_EFFECTMANAGER);
        return ZUN_ERROR;
    }
    if (Gui_RegisterChain() != ZUN_SUCCESS)
    {
        g_GameErrorContext.Log(TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_GUI);
        return ZUN_ERROR;
    }
    if (!g_GameManager.isInReplay)
    {
        ReplayManager_RegisterChain(false, "replay/th6_00.rpy");
    }
    if (!g_GameManager.demoMode)
    {
        // Read boss battle, and store it for use when boss is started.
        g_Supervisor.ReadMidiFile(1, g_Stage.stdData->songPaths[1]);
        // Immediately start playing this level's theme.
        g_Supervisor.PlayAudio(g_Stage.stdData->songPaths[0]);
    }
    mgr->isInRetryMenu = false;
    mgr->isInMenu = true;
    if (g_Supervisor.curState != SUPERVISOR_STATE_NEXT_STAGE)
    {
        g_Supervisor.unk1b4 = 0.0f;
        g_Supervisor.unk1b8 = 0.0f;
    }
    mgr->isTimeStopped = false;
    mgr->score = 0;
    mgr->isGameCompleted = false;
    g_AsciiManager.InitializeVms();
#if !TRIALBUILD
    if (failedToLoadReplay)
    {
        g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU;
    }
    g_Supervisor.forceRedrawFrames = 3;
#endif
    return ZUN_SUCCESS;
}

static ZunResult GameManager_DeletedCallback(GameManager *mgr)
{

    g_Supervisor.d3dDevice->ResourceManagerDiscardBytes(0);
    if (!g_GameManager.demoMode)
    {
        g_Supervisor.StopAudio();
    }
    Stage_CutChain();
    BulletManager_CutChain();
    Player_CutChain();
    EnemyManager_CutChain();
    g_EclManager.Unload();
    EffectManager_CutChain();
    Gui_CutChain();
    StopRecordingReplay();
    mgr->isInMenu = false;
    g_AsciiManager.InitializeVms();
    return ZUN_SUCCESS;
}

ZunResult GameManager_RegisterChain()
{
    GameManager *mgr = &g_GameManager;

    g_GameManagerCalcChain.SetCallback((ChainCallback)GameManager_OnUpdate);
    g_GameManagerCalcChain.addedCallback = (ChainAddedCallback)GameManager_AddedCallback;
    g_GameManagerCalcChain.deletedCallback = (ChainDeletedCallback)GameManager_DeletedCallback;
    g_GameManagerCalcChain.arg = mgr;

    mgr->gameFrames = 0;

    if (g_Chain.AddToCalcChain(&g_GameManagerCalcChain, TH_CHAIN_PRIO_CALC_GAMEMANAGER) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    g_GameManagerDrawChain.SetCallback((ChainCallback)GameManager_OnDraw);
    g_GameManagerDrawChain.arg = mgr;
    g_Chain.AddToDrawChain(&g_GameManagerDrawChain, TH_CHAIN_PRIO_DRAW_GAMEMANAGER);
    return ZUN_SUCCESS;
}

void GameManager_CutChain()
{
    g_Chain.Cut(&g_GameManagerCalcChain);
    g_Chain.Cut(&g_GameManagerDrawChain);
}

#pragma var_order(cameraDistance, viewportMiddleHeight, viewportMiddleWidth, aspectRatio, fov)
void GameManager_SetupCameraStageBackground(f32 extraRenderDistance)
{
    f32 fov;
    f32 aspectRatio;
    f32 viewportMiddleWidth;
    f32 viewportMiddleHeight;
    f32 cameraDistance;

    viewportMiddleWidth = g_Supervisor.viewport.Width / 2.0f;
    viewportMiddleHeight = g_Supervisor.viewport.Height / 2.0f;
    aspectRatio = (f32)g_Supervisor.viewport.Width / (f32)g_Supervisor.viewport.Height;
    fov = D3DXToRadian(30.0f);
    cameraDistance = viewportMiddleHeight / (f32)tan(fov / 2.0f);
    D3DXMatrixLookAtLH(&g_Supervisor.viewMatrix,
                       &D3DXVECTOR3(viewportMiddleWidth, -viewportMiddleHeight, -cameraDistance),
                       &D3DXVECTOR3(viewportMiddleWidth, -viewportMiddleHeight, 0.0f), &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
    g_GameManager.cameraDistance = fabsf(cameraDistance);
    D3DXMatrixPerspectiveFovLH(&g_Supervisor.projectionMatrix, fov, aspectRatio, 100.0f,
                               10000.0f + extraRenderDistance);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_VIEW, &g_Supervisor.viewMatrix);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_PROJECTION, &g_Supervisor.projectionMatrix);
}

#pragma var_order(cameraDistance, viewportMiddleHeight, viewportMiddleWidth, aspectRatio, fov)
void GameManager_SetupCamera(f32 extraRenderDistance)
{
    f32 fov;
    f32 aspectRatio;
    f32 viewportMiddleWidth;
    f32 viewportMiddleHeight;
    f32 cameraDistance;

    viewportMiddleWidth = g_Supervisor.viewport.Width / 2.0f;
    viewportMiddleHeight = g_Supervisor.viewport.Height / 2.0f;
    aspectRatio = (f32)g_Supervisor.viewport.Width / (f32)g_Supervisor.viewport.Height;
    fov = D3DXToRadian(30.0f);
    cameraDistance = viewportMiddleHeight / (f32)tan(fov / 2.0f);
    D3DXMatrixLookAtLH(&g_Supervisor.viewMatrix,
                       &D3DXVECTOR3(viewportMiddleWidth, -viewportMiddleHeight,
                                    -cameraDistance * (f32)g_GameManager.stageCameraFacingDir.z),
                       &D3DXVECTOR3(viewportMiddleWidth + (f32)g_GameManager.stageCameraFacingDir.x,
                                    -viewportMiddleHeight + (f32)g_GameManager.stageCameraFacingDir.y, 0.0f),
                       &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
    g_GameManager.cameraDistance = fabsf(cameraDistance);
    D3DXMatrixPerspectiveFovLH(&g_Supervisor.projectionMatrix, fov, aspectRatio, 100.0f,
                               10000.0f + extraRenderDistance);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_VIEW, &g_Supervisor.viewMatrix);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_PROJECTION, &g_Supervisor.projectionMatrix);
}

void GameManager::IncreaseSubrank(i32 amount)
{
    this->subRank += amount;
    while (this->subRank >= 100)
    {
        this->rank++;
        this->subRank -= 100;
    }
    if (this->rank > this->maxRank)
    {
        this->rank = this->maxRank;
    }
}

void GameManager::DecreaseSubrank(i32 amount)
{
    this->subRank -= amount;
    while (this->subRank < 0)
    {
        this->rank--;
        this->subRank += 100;
    }
    if (this->rank < this->minRank)
    {
        this->rank = this->minRank;
    }
}

GameManager::GameManager()
{
    memset(this, 0, sizeof(GameManager));

    this->gameRegionScreenPos.x = GAME_REGION_POS_X;
    this->gameRegionScreenPos.y = GAME_REGION_POS_Y;
    this->gameRegionSize.x = GAME_REGION_WIDTH;
    this->gameRegionSize.y = GAME_REGION_HEIGHT;
}
} // namespace th06
