## Demangle
    see helpers::isGetShineState()

## clang format all files
    find ./src -name "*.*" | xargs clang-format -i

## sequence info
    if (sequence) {
        uintptr_t NrvAddr = 0;
        NrvAddr = (uintptr_t)sequence->getNerveKeeper()->getCurrentNerve();
        ImGui::Text("Sequence: %s", helpers::getSequenceName(sequence));
        ImGui::Text("SequenceNrv: %s", helpers::getNerveName(sequence->getNerveKeeper()->getCurrentNerve()));
        ImGui::Text("SequenceState: %s", helpers::getStateName(sequence->getNerveKeeper()->mStateCtrl->mCurrentState));
        ImGui::Text("SequenceNrvAddr: %lx", NrvAddr);
        ImGui::Text("SequenceNrvOffset: %lx", NrvAddr - hk::ro::getMainModule()->range().start());
        ImGui::Separator();
    }

## scene info
    if (scene){
        uintptr_t NrvAddr = 0;
        NrvAddr = (uintptr_t)scene->getNerveKeeper()->getCurrentNerve();
        ImGui::Text("Scene: %s", helpers::getSceneName(scene));
        ImGui::Text("SceneNerve: %s", helpers::getNerveName(scene->getNerveKeeper()->getCurrentNerve()));
        ImGui::Text("SceneState: %s", helpers::getStateName(scene->getNerveKeeper()->mStateCtrl->mCurrentState));
        ImGui::Text("SceneNerveAddr: %lx", NrvAddr);
        ImGui::Text("SceneNrvOffset: %lx", NrvAddr - hk::ro::getMainModule()->range().start());
        ImGui::Separator();
    }

## player info 
    if (player) {
        uintptr_t NrvAddr = 0;
        NrvAddr = (uintptr_t)player->getNerveKeeper()->getCurrentNerve();
        ImGui::Text("Player: %s", helpers::getLiveActorName(player));
        ImGui::Text("PlayerNerve: %s", helpers::getNerveName(player->getNerveKeeper()->getCurrentNerve()));
        ImGui::Text("PlayerState: %s", helpers::getStateName(player->getNerveKeeper()->mStateCtrl->mCurrentState));
        ImGui::Text("PlayerNrvAddr: %lx", NrvAddr);
        ImGui::Text("PlayerNrvOffset: %lx", NrvAddr - hk::ro::getMainModule()->range().start());
        ImGui::Separator();
    }