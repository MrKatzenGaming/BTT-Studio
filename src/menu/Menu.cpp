#include "Menu.h"

#include "helpers/getHelper.h"

#include "al/Library/LiveActor/ActorPoseUtil.h"
#include "al/Library/LiveActor/ActorPoseKeeper.h"
#include "game/System/GameSystem.h"
#include "al/Library/Controller/InputFunction.h"
#include <cstddef>
#include <heap/seadHeapMgr.h>
#include "game/Sequence/ChangeStageInfo.h"
#include "game/Scene/StageScene.h"
#include "game/System/GameDataHolder.h"

namespace btt {

const char* stageNames[] = {"CapWorldHomeStage", "WaterfallWorldHomeStage", "SandWorldHomeStage", "LakeWorldHomeStage", "ForestWorldHomeStage", "CloudWorldHomeStage", "ClashWorldHomeStage", "CityWorldHomeStage","SnowWorldHomeStage", "SeaWorldHomeStage", "LavaWorldHomeStage", "BossRaidWorldHomeStage", "SkyWorldHomeStage", "MoonWorldHomeStage", "PeachWorldHomeStage", "Special1WorldHomeStage", "Special2WorldHomeStage", "MoonWorldBasementStage", "MoonWorldKoopa1Stage", "MoonWorldKoopa2Stage", "AnimalChaseExStage", "BikeSteelExStage", "BikeSteelNoCapExStage", "BullRunExStage", "ByugoPuzzleExStage", "CapAppearExStage", "CapAppearLavaLiftExStage", "CapRotatePackunExStage", "CapWorldTowerStage", "CityPeopleRoadStage", "CityWorldFactoryStage", "CityWorldMainTowerStage", "CityWorldSandSlotStage", "CityWorldShop01Stage", "ClashWorldShopStage", "CloudExStage", "Cube2DExStage", "DemoBossRaidAttackStage", "DemoChangeWorldBossRaidAttackStage", "DemoChangeWorldFindKoopaShipStage", "DemoChangeWorldStage", "DemoCrashHomeFallStage", "DemoCrashHomeStage", "DemoEndingStage", "DemoHackFirstStage", "DemoHackKoopaStage", "DemoLavaWorldScenario1EndStage", "DemoMeetCapNpcSubStage", "DemoOpeningStage", "DemoStartWorldWaterfallStage", "DemoTakeOffKoopaForMoonStage", "DemoWorldMoveBackwardArriveStage", "DemoWorldMoveBackwardStage", "DemoWorldMoveForwardArriveStage", "DemoWorldMoveForwardFirstStage", "DemoWorldMoveForwardStage", "DemoWorldMoveMoonBackwardStage", "DemoWorldMoveMoonForwardFirstStage", "DemoWorldMoveMoonForwardStage", "DemoWorldWarpHoleStage", "DonsukeExStage", "DotHardExStage", "DotTowerExStage", "ElectricWireExStage", "FastenerExStage", "FogMountainExStage", "ForestWorldBonusStage", "ForestWorldBossStage", "ForestWorldCloudBonusExStage", "ForestWorldTowerStage", "ForestWorldWaterExStage", "ForestWorldWoodsCostumeStage", "ForestWorldWoodsStage", "ForestWorldWoodsTreasureStage", "ForkExStage", "FrogPoisonExStage", "FrogSearchExStage", "FukuwaraiKuriboStage", "FukuwaraiMarioStage", "GabuzouClockExStage", "Galaxy2DExStage", "GotogotonExStage", "HomeShipInsideStage", "IceWalkerExStage", "IceWaterBlockExStage", "IceWaterDashExStage", "ImomuPoisonExStage", "JangoExStage", "JizoSwitchExStage", "KaronWingTowerStage", "KillerRailCollisionExStage", "KillerRoadExStage", "KillerRoadNoCapExStage", "LakeWorldShopStage", "LavaWorldBubbleLaneExStage", "LavaWorldClockExStage", "LavaWorldCostumeStage", "LavaWorldExcavationExStage", "LavaWorldFenceLiftExStage", "LavaWorldShopStage", "LavaWorldTreasureStage", "LavaWorldUpDownExStage", "LavaWorldUpDownYoshiExStage", "Lift2DExStage", "MeganeLiftExStage", "MoonAthleticExStage", "MoonWorldCaptureParadeStage", "MoonWorldShopRoom", "MoonWorldSphinxRoom", "MoonWorldWeddingRoom2Stage", "MoonWorldWeddingRoomStage", "Note2D3DRoomExStage", "PackunPoisonExStage", "PackunPoisonNoCapExStage", "PeachWorldCastleStage", "PeachWorldCostumeStage", "PeachWorldPictureBossForestStage", "PeachWorldPictureBossKnuckleStage", "PeachWorldPictureBossMagmaStage", "PeachWorldPictureBossRaidStage", "PeachWorldPictureGiantWanderBossStage", "PeachWorldPictureMofumofuStage", "PeachWorldShopStage", "PoisonWaveExStage", "PoleGrabCeilExStage", "PoleKillerExStage", "PushBlockExStage", "RadioControlExStage", "RailCollisionExStage", "ReflectBombExStage", "RevengeBossKnuckleStage", "RevengeBossMagmaStage", "RevengeBossRaidStage", "RevengeForestBossStage", "RevengeGiantWanderBossStage", "RevengeMofumofuStage", "RocketFlowerExStage", "RollingExStage", "SandWorldCostumeStage", "SandWorldKillerExStage", "SandWorldMeganeExStage", "SandWorldPressExStage", "SandWorldPyramid000Stage", "SandWorldPyramid001Stage", "SandWorldRotateExStage", "SandWorldSecretStage", "SandWorldShopStage", "SandWorldSlotStage", "SandWorldSphinxExStage", "SandWorldUnderground000Stage", "SandWorldUnderground001Stage", "SandWorldVibrationStage", "SeaWorldCostumeStage", "SeaWorldSecretStage", "SeaWorldSneakingManStage", "SeaWorldUtsuboCaveStage", "SeaWorldVibrationStage", "SenobiTowerExStage", "SenobiTowerYoshiExStage", "ShootingCityExStage", "ShootingCityYoshiExStage", "ShootingElevatorExStage", "SkyWorldCloudBonusExStage", "SkyWorldCostumeStage", "SkyWorldShopStage", "SkyWorldTreasureStage", "SnowWorldCloudBonusExStage", "SnowWorldCostumeStage", "SnowWorldLobby000Stage", "SnowWorldLobby001Stage", "SnowWorldLobbyExStage", "SnowWorldRace000Stage", "SnowWorldRace001Stage", "SnowWorldRaceExStage", "SnowWorldRaceHardExStage", "SnowWorldRaceTutorialStage", "SnowWorldShopStage", "SnowWorldTownStage", "Special1WorldTowerBombTailStage", "Special1WorldTowerCapThrowerStage", "Special1WorldTowerFireBlowerStage", "Special1WorldTowerStackerStage", "Special2WorldCloudStage", "Special2WorldKoopaStage", "Special2WorldLavaStage", "StaffRollMoonRockDemo", "SwingSteelExStage", "Theater2DExStage", "TogezoRotateExStage", "TrampolineWallCatchExStage", "TrexBikeExStage", "TrexPoppunExStage", "TsukkunClimbExStage", "TsukkunRotateExStage", "WanwanClashExStage", "WaterTubeExStage", "WaterValleyExStage", "WindBlowExStage", "WorldStage", "YoshiCloudExStage"};
#define NUM_STAGES 200

SEAD_SINGLETON_DISPOSER_IMPL(Menu);

void Menu::init(sead::Heap* heap) {
    mHeap = heap;
    sead::ScopedCurrentHeapSetter heapSetter(mHeap);
}

void Menu::draw(sead::DrawContext* drawContext) {




    if (mIsEnabledMenu) {

        switch (mCurrentPage) {
            case Main:
                drawMain(nullptr);
                break;

            case Options:
                drawOptions(nullptr);
                break;

            case Info:
                drawInfo(nullptr);
                break;
            
                case Misc:
                drawMisc(nullptr);
                break;

            default:
                break;
        }
    }

    if (al::isPadTriggerDown(-1) && mIsEnabledMenu && mIsEnabledInput) {
        mCurrentLine++;
    } else if (al::isPadTriggerUp(-1) && mIsEnabledMenu && mIsEnabledInput) {
        mCurrentLine--;
    }
    if (al::isPadTriggerPressLeftStick(-1)) {
        mIsEnabledMenu = !mIsEnabledMenu;
    }

    if (al::isPadHoldRight(-1) || al::isPadHoldLeft(-1)) {
        heldDirFrames++;
    } else {
        heldDirFrames = 0;
    }

}

void Menu::drawMain(void* renderer) {
    // TITLE("BTT Studio");
    // MAX_LINE(3);
    // CHANGE_PAGE("Misc", Misc, 0)
    // CHANGE_PAGE("Options", Options, 1);
    // CHANGE_PAGE("Info", Info, 2);


}

void Menu::drawOptions(void* renderer) {
    // TITLE("Options");
    // MAX_LINE(2);
    // BACK_PAGE(Main, 0);

    // TOGGLE("Moon Jump", SettingsMgr::instance()->mSettings.mIsEnableMoonJump, 1);
}

void Menu::drawInfo(void* renderer) {
    // TITLE("Info");
    // MAX_LINE(1);
    // BACK_PAGE(Main, 0);


    // al::LiveActor* player = helpers::tryGetPlayer();
    // if (player) {
    //     const sead::Vector3f& trans = al::getTrans(player);
    //     const sead::Vector3f& vel = player->getPoseKeeper()->getVelocity();
        
    //     TEXT(1,"Pos: %.2f %.2f %.2f\n", trans.x, trans.y, trans.z);
    //     TEXT(2,"Vel: %.2f %.2f %.2f\n", vel.x, vel.y, vel.z);
    // } else {
    //     TEXT(1,"No player\n");
    // }
}

void Menu::drawMisc(void* renderer) {
    // TITLE("Misc");
    // MAX_LINE(4);
    // BACK_PAGE(Main, 0);

    // INDEXRL(currentStage, 0, NUM_STAGES - 1, 1);
    // TEXT(1,"%sStage: %s\n", mCharCursor, stageNames[currentStage]);
    // INDEXRL(currentScenario, 0, 15, 2);

    // if (currentScenario != 0) {TEXT(2,"%sScenario: %d\n", mCharCursor, currentScenario);}
    // else TEXT(2,"%sScenario: Don't change\n", mCharCursor);

    // TRIGGER("Go", 3, {
    //     ChangeStageInfo info = ChangeStageInfo(helpers::tryGetStageScene()->mHolder->mData, "start", stageNames[currentStage], false, currentScenario ?: -1, ChangeStageInfo::NO_SUB_SCENARIO);
    //     helpers::tryGetStageScene()->mHolder->mData->changeNextStage(&info, 0);
    //     mCurrentLine = 0;
    // });


}
    

} // namespace btt