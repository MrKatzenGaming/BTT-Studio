## clang format all files
```bash
find ./src -name "*.*" | xargs clang-format -i
```

# info stuff

```cpp
#include <typeinfo>
#include <cxxabi.h>
#include "hk/ro/RoUtil.h"
#include "al/Library/Nerve/NerveKeeper.h"
#include "al/Library/Nerve/NerveStateCtrl.h"

if (gameSeq) {
    int status;
    uintptr_t NrvAddr = 0;
    al::NerveKeeper* nerveKeeper = gameSeq->getNerveKeeper();
    char* sequenceName = nullptr;
    char* nerveName = nullptr;
    char* stateName = nullptr;
    int prefixLen = 0;

    sequenceName = abi::__cxa_demangle(typeid(*gameSeq).name(), nullptr, nullptr, &status);

    if (nerveKeeper) {
        const al::Nerve* currentNerve = nerveKeeper->getCurrentNerve();
        if (currentNerve) {
            NrvAddr = (uintptr_t)currentNerve;
            nerveName = abi::__cxa_demangle(typeid(*currentNerve).name(), nullptr, nullptr, &status);
            prefixLen = nerveName[0] == '(' ? strlen("(anonymous namespace)::") : 0;
        }
        if (gameSeq->getNerveKeeper()->mStateCtrl) {
            al::NerveStateCtrl::State* state = gameSeq->getNerveKeeper()->mStateCtrl->mCurrentState;
            if (state) stateName = abi::__cxa_demangle(typeid(*state->state).name(), nullptr, nullptr, &status);
        }
    }

    ImGui::Text("Sequence: %s", sequenceName);
    ImGui::Text("SequenceNrv: %s", nerveName + prefixLen);
    ImGui::Text("SequenceState: %s", stateName);
    ImGui::Text("SequenceNrvAddr: %lx", NrvAddr);
    ImGui::Text("SequenceNrvOffset: %lx", NrvAddr - hk::ro::getMainModule()->range().start());
    ImGui::Separator();

    if (sequenceName) free(sequenceName);
    if (nerveName) free(nerveName);
    if (stateName) free(stateName);
}
if (stageScene) {
    int status;
    uintptr_t NrvAddr = 0;
    al::NerveKeeper* nerveKeeper = stageScene->getNerveKeeper();
    char* sceneName = nullptr;
    char* nerveName = nullptr;
    char* stateName = nullptr;
    int prefixLen = 0;

    sceneName = abi::__cxa_demangle(typeid(*stageScene).name(), nullptr, nullptr, &status);

    if (nerveKeeper) {
        const al::Nerve* currentNerve = nerveKeeper->getCurrentNerve();
        if (currentNerve) {
            NrvAddr = (uintptr_t)currentNerve;
            nerveName = abi::__cxa_demangle(typeid(*currentNerve).name(), nullptr, nullptr, &status);
            prefixLen = nerveName[0] == '(' ? strlen("(anonymous namespace)::") : 0;
        }
        if (stageScene->getNerveKeeper()->mStateCtrl) {
            al::NerveStateCtrl::State* state = stageScene->getNerveKeeper()->mStateCtrl->mCurrentState;
            if (state) stateName = abi::__cxa_demangle(typeid(*state->state).name(), nullptr, nullptr, &status);
        }
    }

    ImGui::Text("StageScene: %s", sceneName);
    ImGui::Text("StageSceneNrv: %s", nerveName + prefixLen);
    ImGui::Text("StageSceneState: %s", stateName);
    ImGui::Text("StageSceneNrvAddr: %lx", NrvAddr);
    ImGui::Text("StageSceneNrvOffset: %lx", NrvAddr - hk::ro::getMainModule()->range().start());
    ImGui::Separator();

    if (sceneName) free(sceneName);
    if (nerveName) free(nerveName);
    if (stateName) free(stateName);
}
if (playerHak) {
    int status;
    uintptr_t NrvAddr = 0;
    al::NerveKeeper* nerveKeeper = playerHak->getNerveKeeper();
    char* actorName = nullptr;
    char* nerveName = nullptr;
    char* stateName = nullptr;
    int prefixLen = 0;

    actorName = abi::__cxa_demangle(typeid(*playerHak).name(), nullptr, nullptr, &status);

    if (nerveKeeper) {
        const al::Nerve* currentNerve = nerveKeeper->getCurrentNerve();
        if (currentNerve) {
            NrvAddr = (uintptr_t)currentNerve;
            nerveName = abi::__cxa_demangle(typeid(*currentNerve).name(), nullptr, nullptr, &status);
            prefixLen = nerveName[0] == '(' ? strlen("(anonymous namespace)::") : 0;
        }
        if (playerHak->getNerveKeeper()->mStateCtrl) {
            al::NerveStateCtrl::State* state = playerHak->getNerveKeeper()->mStateCtrl->mCurrentState;
            if (state) stateName = abi::__cxa_demangle(typeid(*state->state).name(), nullptr, nullptr, &status);
        }
    }

    ImGui::Text("Player: %s", actorName);
    ImGui::Text("PlayerNrv: %s", nerveName + prefixLen);
    ImGui::Text("PlayerState: %s", stateName);
    ImGui::Text("PlayerNrvAddr: %lx", NrvAddr);
    ImGui::Text("PlayerNrvOffset: %lx", NrvAddr - hk::ro::getMainModule()->range().start());
    ImGui::Separator();

    if (actorName) free(actorName);
    if (nerveName) free(nerveName);
    if (stateName) free(stateName);
}
```
## Infinite load on scene kill
StateNrvs
- SkipDemo
- Skip
States
- PauseMenu
- SnapShot

## World Traveling Status
- Init                      (Not via getWorldTravelingStatus)
- CityWorld0
- WaterfallWorld0
- LavaWorld0
- MoonWorld0
- PeachCastleWorld0
- Last
- LastAfter