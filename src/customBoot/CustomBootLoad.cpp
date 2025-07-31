#include "customBoot/CustomBootLoad.hpp"

#include "al/Library/Layout/LayoutActionFunction.h"
#include "al/Library/Layout/LayoutActorUtil.h"
#include "al/Library/Nerve/NerveUtil.h"
#include "custom/al/Library/Play/Layout/WipeSimple.h"

#include "sead/math/seadVectorFwd.h"

#include "game/Sequence/HakoniwaSequence.h"

#include "gfx/seadColor.h"
#include "Library/Layout/IUseLayout.h"
#include "Library/Layout/LayoutKeeper.h"
#include "menu/Menu.h"

#ifdef BTTDEBUG
# include "sead/prim/seadSafeString.h"
#endif

namespace customboot {
CustomBootLoad::CustomBootLoad(WorldResourceLoader* resourceLoader, const al::LayoutInitInfo& initInfo, float autoCloseAfter) :
    al::LayoutActor("SpeedbootLoad"), worldResourceLoader(resourceLoader), mAutoCloseAfter(autoCloseAfter) {
    al::initLayoutActor(this, initInfo, "SpeedbootLoad", nullptr);
    initNerve(&NrvCustomBootLoad.Appear, 0);
    appear();

    // if (!al::isEqualString(hk::ro::getMainModule()->getVersionName(), "130")) {
    //     wipe = new al::WipeSimple("黒フェード", "FadeBlack", initInfo, 0);
    // } else {
    //     wipe = new al::WipeSimple("黒フェードシーン情報", "FadeBlack", initInfo, 0);
    // }

    wipe = new al::WipeSimple("スキップワイプ", "WipeSkip", initInfo, 0);
}

void CustomBootLoad::exeAppear() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Appear", nullptr);
        btt::Menu::instance()->hideMenu();
    }

    if (al::isActionEnd(this, nullptr)) {
        al::setNerve(this, &NrvCustomBootLoad.Wait);
    }
}

void CustomBootLoad::exeWait() {
    if (al::isActionEnd(this, nullptr)) {
        al::setNerve(this, &NrvCustomBootLoad.Decrease);
    }
}

// Reimplementation and edit of al::setPaneVtxColor bc it doesn't exist on 1.3 (most variable names made up by Github Copilot)
void SetVtxColor(const al::IUseLayout* layoutUser, const char* paneName, const sead::Color4u8& color) {
    // Get the layout pointer from the IUseLayout vtable
    al::LayoutKeeper* keeper = layoutUser->getLayoutKeeper();

    // Get the pane manager
    long* paneMgr = *reinterpret_cast<long**>(*reinterpret_cast<long*>(keeper + 0x10 /*Layout*/) + 0x18);

    // Find the pane by name
    auto findPane = reinterpret_cast<long* (*)(long*, const char*, int)>(*reinterpret_cast<void**>(*paneMgr + 0x60));
    long* pane = findPane(paneMgr, paneName, 1);

    // Set the vertex color for indices 0 and 2
    auto setVtxColor = reinterpret_cast<void (*)(long*, int, const sead::Color4u8*)>(*reinterpret_cast<void**>(*pane + 0x28));
    setVtxColor(pane, 0, &color);
    setVtxColor(pane, 2, &color);
}

void CustomBootLoad::exeDecrease() {
    mTime += 1.f / 60.f;
    al::setPaneString(this, "TxtTip", u"Hiiiiiiii :3", 0);
    al::setPaneString(this, "TxtName", u"BTT-Studio", 0);
    al::setPaneString(this, "TxtConnecting", u"\u00A9 2025, MrKatzenGaming", 0);

    // if (al::isPadTriggerPlus(-1)) {
    //     Logger::log("Plus button pressed. Opening keyboard for IP and Port input.\n");

    //     if (Client* client = Client::get()) {
    //         Client::getKeyboard()->setHeaderText(u"Set Server IP");
    //         Client::getKeyboard()->setSubText(u"Enter IP Address below:");
    //         Client::openKeyboardIP();

    //         Client::getKeyboard()->setHeaderText(u"Set Server Port");
    //         Client::getKeyboard()->setSubText(u"Enter Port below:");
    //         Client::openKeyboardPort();

    //         SaveDataAccessFunction::startSaveDataWrite(client->getHolder().mData);
    //         client->restartConnection();

    //         mHasConnected = true;
    //         mConnectionDisplayTimer = 0.0f;

    //         al::setPaneLocalAlpha(this, "ConnectingGlobe", 1.0f);
    //         al::setPaneLocalAlpha(this, "ConnectingArrows", 1.0f);
    //         al::setPaneVtxColor(this, "TxtConnecting", sead::Color4u8{255, 255, 255, 255});
    //         al::setPaneString(this, "TxtConnecting", u"Restarted Connection!", 0);
    //     }
    // }

    mProgression = mTime / mAutoCloseAfter;

    sead::Color4u8 fixedColor = { 255, 0, 0, 255 };
    mRotTime += 0.03f;
    float rotation = cosf(mTime) * 5;

#ifdef BTTDEBUG
    // Debug stuff
    sead::WFormatFixedSafeString<0x50> debugString(
        u"Display Time: %.02f\nRot Time: %.02f\nSine Value: %.02f\nAutoClose: %.02f", mTime, mRotTime, rotation, mAutoCloseAfter
    );
    al::setPaneString(this, "TxtDebug", debugString.cstr(), 0);
    al::setPaneLocalTrans(this, "TxtDebug", sead::Vector2f(-590, 140));
#endif

    float arrowRotation = -mTime * 75.0f;
    al::setPaneLocalRotate(this, "ConnectingArrows", { 0.0f, 0.0f, arrowRotation });

    // --- Connection status display ---
    // if (!mHasConnected) {
    //     if (Logger::instance()->isConnected()) {
    //         mHasConnected = true;
    //         mConnectionDisplayTimer = 0.0f;
    //         al::setPaneString(this, "TxtConnecting", u"Server Connected!", 0);
    //     } else {
    //         mDotTimer += 1.0f / 60.0f;
    //         if (mDotTimer >= 0.5f) {
    //             mConnectingDotState = (mConnectingDotState + 1) % 4;
    //             mDotTimer = 0.0f;
    //         }

    //         const char16_t* dots[] = { u"Connecting to server", u"Connecting to server.", u"Connecting to server..", u"Connecting to server..." };
    //         al::setPaneString(this, "TxtConnecting", dots[mConnectingDotState], 0);
    //     }

    // } else {
    //     mConnectionDisplayTimer += 1.0f / 60.0f;
    //     if (mConnectionDisplayTimer > 2.0f) {
    //         float fade = sead::Mathf::clamp(1.0f - (mConnectionDisplayTimer - 2.0f) / 0.4f, 0.0f, 1.0f);
    //         u8 alpha = static_cast<u8>(fade * 255);
    //         SetVtxColor(this, "TxtConnecting", sead::Color4u8 { 255, 255, 255, alpha });
    //         al::setPaneLocalAlpha(this, "ConnectingGlobe", alpha);
    //         al::setPaneLocalAlpha(this, "ConnectingArrows", alpha);
    //     }
    // }

    // --- Loading dots animation ---
    mDotTimer += 1.0f / 60.0f;
    if (mDotTimer >= 0.5f) {
        mLoadingDotState = (mLoadingDotState + 1) % 4;
        mDotTimer = 0.0f;
    }

    const char16_t* loadingDots[] = { u"Loading", u"Loading.", u"Loading..", u"Loading..." };
    al::setPaneString(this, "TxtLoading", loadingDots[mLoadingDotState], 0);

    // --- Progress visuals ---
    // if (mProgression < 1.5f) {
    al::setPaneLocalScale(this, "PicBar", { mProgression, 2.f });
    al::setPaneLocalScale(this, "PicBarFill", { 30.f, 1.f });

    SetVtxColor(this, "PicBar", fixedColor);
    SetVtxColor(this, "PicBarFill", fixedColor);

    al::setPaneLocalRotate(this, "PicMoon", { 0.f, 0.f, rotation });
    al::setPaneLocalRotate(this, "Arrows", { 0.f, 0.f, arrowRotation });
    al::setPaneLocalRotate(this, "PicBG", { 0.f, 0.f, mTime * 2 * -3.f });
    // }

    if (mProgression >= 1.f) {
        wipe->tryStartClose(60);

        if (wipe->isCloseEnd()) {
            al::setNerve(this, &NrvCustomBootLoad.End);
        }
    }
}

void CustomBootLoad::exeEnd() {
    if (al::isFirstStep(this)) {
        btt::Menu::instance()->showMenu();
        kill();
        wipe->startOpen(30);
    }
}
} // namespace customboot
