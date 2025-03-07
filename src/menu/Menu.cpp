#include "Menu.h"

#include "hk/gfx/DebugRenderer.h"
#include "helpers/getHelper.h"

#include "al/Library/LiveActor/ActorPoseUtil.h"
#include "al/Library/LiveActor/ActorPoseKeeper.h"
#include "game/System/GameSystem.h"
#include "al/Library/Controller/InputFunction.h"
#include <heap/seadHeapMgr.h>
#include "nn/hid.h"

namespace btt {

SEAD_SINGLETON_DISPOSER_IMPL(Menu);

void Menu::init(sead::Heap* heap) {
    mHeap = heap;
    sead::ScopedCurrentHeapSetter heapSetter(mHeap);
}

void Menu::draw(sead::DrawContext* drawContext) {
    auto* renderer = hk::gfx::DebugRenderer::instance();

    renderer->clear();
    renderer->begin(drawContext->getCommandBuffer()->ToData()->pNvnCommandBuffer);

    renderer->setGlyphSize(0.55);

    renderer->drawQuad(
        { { 0, 360 }, { 0, 0 }, 0xaf000000 },
        { { 500, 360 }, { 1.0, 0 }, 0xaf000000 },
        { { 500, 720 }, { 1.0, 1.0 }, 0xaf000000 },
        { { 0, 720 }, { 0, 1.0 }, 0xaf000000 });

    renderer->setCursor({ 10, 370});

    
    switch (mCurrentPage) {
        case Main:
            drawMain(renderer);
            break;

        case Options:
            drawOptions(renderer);
            break;

        case Info:
            drawInfo(renderer);
            break;

        default:
            printf("Invalid page\n");
            break;
    }


    renderer->end();

    if (al::isPadTriggerDown(-1)) {
        mCurrentLine++;
    } else if (al::isPadTriggerUp(-1)) {
        mCurrentLine--;
    }

}

void Menu::drawMain(hk::gfx::DebugRenderer* renderer) {
    TITLE("BTT Studio");
    MAX_LINE(2);
    CHANGE_PAGE("Options", Options, 0);
    CHANGE_PAGE("Info", Info, 1);


}

void Menu::drawOptions(hk::gfx::DebugRenderer* renderer) {
    TITLE("Options");
    MAX_LINE(1);
    BACK_PAGE(Main, 0);
}

void Menu::drawInfo(hk::gfx::DebugRenderer* renderer) {
    TITLE("Info");
    MAX_LINE(1);
    BACK_PAGE(Main, 0);


    al::LiveActor* player = helpers::tryGetPlayerHak(GameSystemFunction::getGameSystem()->mSequence);
    if (player) {
        const sead::Vector3f& trans = al::getTrans(player);
        const sead::Vector3f& vel = player->getPoseKeeper()->getVelocity();
        
        // renderer->setCursor({ 10, 390});
        TEXT(1,"Pos: %.2f %.2f %.2f\n", trans.x, trans.y, trans.z);
        // renderer->setCursor({ 10, 410});
        TEXT(2,"Vel: %.2f %.2f %.2f\n", vel.x, vel.y, vel.z);
    } else {
        // renderer->setCursor({ 10, 390});
        TEXT(1,"No player\n");
    }
}

} // namespace btt