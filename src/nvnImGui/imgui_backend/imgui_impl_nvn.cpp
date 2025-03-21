#include "imgui_impl_nvn.hpp"
// #include "helpers.h"
#include "helpers/fsHelper.h"
#include "Menu.h"
// #include "imgui_bin.h"
#include "hk/diag/diag.h"
#include "imgui_hid_mappings.h"
// #include "lib.hpp"
// #include "logger/Logger.hpp"
#include <cmath>
#include <imgui_internal.h>
#include <nn/oe.h>

#include "nn/hid.h"
#include "nn/os.h"
#include "nn/pl.h"
#include "nn/util/util_BitUtil.h"

#include "helpers/InputHelper.h"
#include "MemoryPoolMaker.h"

#define UBOSIZE 0x1000

namespace ImguiNvnBackend {
void make_identity(Matrix44f& mtx) {
    Matrix44f ident = { { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f } };
    memcpy(mtx, &ident, sizeof(Matrix44f));
}

void orthoRH_ZO(Matrix44f& mtx, float left, float right, float bottom, float top, float zNear, float zFar) {
    make_identity(mtx);
    mtx[0][0] = 2.f / (right - left);
    mtx[1][1] = 2.f / (top - bottom);
    mtx[2][2] = -1.f / (zFar - zNear);
    mtx[3][0] = -(right + left) / (right - left);
    mtx[3][1] = -(top + bottom) / (top - bottom);
    mtx[3][2] = -zNear / (zFar - zNear);
}

// WIP ImGui Functions from docking branch
static void ScaleWindow(ImGuiWindow* window, float scale) {
    ImVec2 origin = window->Viewport->Pos;
    window->Pos = ImFloor(ImVec2((window->Pos.x - origin.x) * scale + origin.x, (window->Pos.y - origin.y) * scale + origin.y));
    window->Size = ImFloor(ImVec2(window->Size.x * scale, window->Size.y * scale));
    window->SizeFull = ImFloor(ImVec2(window->SizeFull.x * scale, window->SizeFull.y * scale));
    window->ContentSize = ImFloor(ImVec2(window->ContentSize.x * scale, window->ContentSize.y * scale));
}

void ScaleWindowsInViewport(ImGuiViewport* viewport, float scale) {
    ImGuiContext& g = *GImGui;

    for (int i = 0; i != g.Windows.Size; i++)
        if (g.Windows[i]->Viewport == viewport) ScaleWindow(g.Windows[i], scale);
}

// backend impl

NvnBackendData* getBackendData() {
    NvnBackendData* result = ImGui::GetCurrentContext() ? (NvnBackendData*)ImGui::GetIO().BackendRendererUserData : nullptr;
    HK_ASSERT(result); //, "Backend has not been initialized!"
    return result;
}

bool createShaders() {
    auto bd = getBackendData();

    // Logger::log("Unable to compile shaders at runtime. falling back to pre-compiled shaders.\n");
    FsHelper::LoadData loadData = { .path = "content:/ImGuiData/imgui.bin" };

    FsHelper::loadFileFromPath(loadData);

    bd->imguiShaderBinary.size = loadData.bufSize;
    bd->imguiShaderBinary.ptr = (u8*)loadData.buffer;

    if (bd->imguiShaderBinary.size > 0) {
        return true;
    }

    return false;
}

bool loadSystemFont() {
    void* addr_std = nn::pl::GetSharedFontAddress(nn::pl::SharedFontType::STANDARD);
    u32 size_std = nn::pl::GetSharedFontSize(nn::pl::SharedFontType::STANDARD);

    void* addr_ext = nn::pl::GetSharedFontAddress(nn::pl::SharedFontType::NN_EXT);
    u32 size_ext = nn::pl::GetSharedFontSize(nn::pl::SharedFontType::NN_EXT);

    if (!((addr_std && size_std > 0) && (addr_ext && size_ext > 0))) {
        return false;
    }

    ImGuiIO& io = ImGui::GetIO();

    ImFontConfig fnt_cfg;
    strcpy(fnt_cfg.Name, "Nintendo Standard");
    fnt_cfg.FontDataOwnedByAtlas = false;
    ImWchar extended_range[] = { 0xe000, 0xe152 };

    io.Fonts->AddFontFromMemoryTTF(addr_std, size_std, 14.0f, &fnt_cfg, io.Fonts->GetGlyphRangesJapanese());

    fnt_cfg.MergeMode = true;
    io.Fonts->AddFontFromMemoryTTF(addr_ext, size_ext, 14.0f, &fnt_cfg, extended_range);

    io.Fonts->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight;
    if (!io.Fonts->Build()) {
        // Logger::log("Failed to build Font!\n");
        return false;
    }

    // Logger::log("Loaded System Font.\n");

    return true;
}

bool setupFont() {
    // Logger::log("Setting up ImGui Font.\n");

    auto bd = getBackendData();

    ImGuiIO& io = ImGui::GetIO();

    // init sampler and texture pools

    int sampDescSize = 0;
    bd->device->GetInteger(nvn::DeviceInfo::SAMPLER_DESCRIPTOR_SIZE, &sampDescSize);
    int texDescSize = 0;
    bd->device->GetInteger(nvn::DeviceInfo::TEXTURE_DESCRIPTOR_SIZE, &texDescSize);

    int sampMemPoolSize = sampDescSize * MaxSampDescriptors;
    int texMemPoolSize = texDescSize * MaxTexDescriptors;
    int totalPoolSize = nn::util::align_up(sampMemPoolSize + texMemPoolSize, 0x1000);
    if (!MemoryPoolMaker::createPool(&bd->sampTexMemPool, totalPoolSize)) {
        // Logger::log("Failed to Create Texture/Sampler Memory Pool!\n");
        return false;
    }

    if (!bd->samplerPool.Initialize(&bd->sampTexMemPool, 0, MaxSampDescriptors)) {
        // Logger::log("Failed to Create Sampler Pool!\n");
        return false;
    }

    if (!bd->texPool.Initialize(&bd->sampTexMemPool, sampMemPoolSize, MaxTexDescriptors)) {
        // Logger::log("Failed to Create Texture Pool!\n");
        return false;
    }

    // load switch font data into imgui

    //        if (!loadSystemFont()) {
    //            Logger::log("Failed to load Switch System Font! Falling back to default ImGui font.\n");
    //        }

    io.Fonts->AddFontDefault();

    // convert imgui font texels

    unsigned char* pixels;
    int width, height, pixelByteSize;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &pixelByteSize);
    int texPoolSize = pixelByteSize * width * height;

    if (!MemoryPoolMaker::createPool(
            &bd->fontMemPool, nn::util::align_up(texPoolSize, 0x1000), nvn::MemoryPoolFlags::CPU_UNCACHED | nvn::MemoryPoolFlags::GPU_CACHED
        )) {
        // Logger::log("Failed to Create Font Memory Pool!\n");
        return false;
    }

    bd->texBuilder.SetDefaults()
        .SetDevice(bd->device)
        .SetTarget(nvn::TextureTarget::TARGET_2D)
        .SetFormat(nvn::Format::RGBA8)
        .SetSize2D(width, height)
        .SetStorage(&bd->fontMemPool, 0);

    if (!bd->fontTexture.Initialize(&bd->texBuilder)) {
        // Logger::log("Failed to Create Font Texture!\n");
        return false;
    }

    // setup font texture

    nvn::CopyRegion region = {
        .xoffset = 0, .yoffset = 0, .zoffset = 0, .width = bd->fontTexture.GetWidth(), .height = bd->fontTexture.GetHeight(), .depth = 1
    };

    bd->fontTexture.WriteTexels(nullptr, &region, pixels);
    bd->fontTexture.FlushTexels(nullptr, &region);

    bd->samplerBuilder.SetDefaults()
        .SetDevice(bd->device)
        .SetMinMagFilter(nvn::MinFilter::LINEAR, nvn::MagFilter::LINEAR)
        .SetWrapMode(nvn::WrapMode::CLAMP, nvn::WrapMode::CLAMP, nvn::WrapMode::CLAMP);

    if (!bd->fontSampler.Initialize(&bd->samplerBuilder)) {
        // Logger::log("Failed to Init Font Sampler!\n");
        return false;
    }

    bd->textureId = 257;
    bd->samplerId = 257;

    bd->texPool.RegisterTexture(bd->textureId, &bd->fontTexture, nullptr);
    bd->samplerPool.RegisterSampler(bd->samplerId, &bd->fontSampler);

    bd->fontTexHandle = bd->device->GetTextureHandle(bd->textureId, bd->samplerId);
    io.Fonts->SetTexID(&bd->fontTexHandle);

    // Logger::log("Finished.\n");

    return true;
}

bool setupShaders(u8* shaderBinary, ulong binarySize) {
    // Logger::log("Setting up ImGui Shaders.\n");

    auto bd = getBackendData();

    if (!bd->shaderProgram.Initialize(bd->device)) {
        // Logger::log("Failed to Initialize Shader Program!");
        return false;
    }

    bd->shaderMemory = IM_NEW(MemoryBuffer)(
        binarySize, shaderBinary, nvn::MemoryPoolFlags::CPU_UNCACHED | nvn::MemoryPoolFlags::GPU_CACHED | nvn::MemoryPoolFlags::SHADER_CODE
    );

    if (!bd->shaderMemory->IsBufferReady()) {
        // Logger::log("Shader Memory Pool not Ready! Unable to continue.\n");
        return false;
    }

    BinaryHeader offsetData = BinaryHeader((u32*)shaderBinary);

    nvn::BufferAddress addr = bd->shaderMemory->GetBufferAddress();

    nvn::ShaderData& vertShaderData = bd->shaderDatas[0];
    vertShaderData.data = addr + offsetData.mVertexDataOffset;
    vertShaderData.control = shaderBinary + offsetData.mVertexControlOffset;

    nvn::ShaderData& fragShaderData = bd->shaderDatas[1];
    fragShaderData.data = addr + offsetData.mFragmentDataOffset;
    fragShaderData.control = shaderBinary + offsetData.mFragmentControlOffset;

    if (!bd->shaderProgram.SetShaders(2, bd->shaderDatas)) {
        // Logger::log("Failed to Set shader data for program.\n");
        return false;
    }

    bd->shaderProgram.SetDebugLabel("ImGuiShader");

    // Uniform Block Object Memory Setup

    bd->uniformMemory = IM_NEW(MemoryBuffer)(UBOSIZE);

    if (!bd->uniformMemory->IsBufferReady()) {
        // Logger::log("Uniform Memory Pool not Ready! Unable to continue.\n");
        return false;
    }

    // setup vertex attrib & stream

    bd->attribStates[0].SetDefaults().SetFormat(nvn::Format::RG32F, offsetof(ImDrawVert, pos)); // pos
    bd->attribStates[1].SetDefaults().SetFormat(nvn::Format::RG32F, offsetof(ImDrawVert, uv));  // uv
    bd->attribStates[2].SetDefaults().SetFormat(nvn::Format::RGBA8, offsetof(ImDrawVert, col)); // color

    bd->streamState.SetDefaults().SetStride(sizeof(ImDrawVert));

    // Logger::log("Finished.\n");

    return true;
}

void InitBackend(const NvnBackendInitInfo& initInfo) {
    ImGuiIO& io = ImGui::GetIO();
    // HK_ASSERT(!io.BackendRendererUserData);//, "Already Initialized Imgui Backend!"
    if (io.BackendRendererUserData) {
        return;
    }

    io.BackendPlatformName = "Switch";
    io.BackendRendererName = "imgui_impl_nvn";
    io.IniFilename = nullptr;
    io.MouseDrawCursor = InputHelper::isMouseConnected();
    io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    bool isDockedMode = nn::oe::GetOperationMode() == nn::oe::OperationMode_Docked;
    // these might be different depending on the game, but the setCrop hook should fix it.
    if (isDockedMode) {
        io.DisplaySize = ImVec2(1600, 900);
    } else {
        io.DisplaySize = ImVec2(1280, 720);
    }

    auto* bd = IM_NEW(NvnBackendData)();
    io.BackendRendererUserData = (void*)bd;

    ImguiNvnBackend::updateProjection(io.DisplaySize);
    ImguiNvnBackend::updateScale(isDockedMode);

    bd->device = initInfo.device;
    bd->queue = initInfo.queue;
    bd->cmdBuf = initInfo.cmdBuf;
    bd->isInitialized = false;

    // set this to true if your implementation does not need to convert sRGB to linear
    bd->mShaderUBO.isUseSrgb = false;

    if (createShaders()) {
        // Logger::log("Shader Binaries Loaded! Setting up Render Data.\n");

        if (setupShaders(bd->imguiShaderBinary.ptr, bd->imguiShaderBinary.size) && setupFont()) {
            // Logger::log("Rendering Setup!\n");

            bd->isInitialized = true;

        } else {
            // Logger::log("Failed to Setup Render Data!\n");
        }
    }
}

void ShutdownBackend() {}

void updateTouch(ImGuiIO& io) {
    static s32 touchPosX = 0;
    static s32 touchPosY = 0;
    InputHelper::getTouchCoords(&touchPosX, &touchPosY);
    io.AddMousePosEvent(touchPosX, touchPosY);

    ImGuiMouseButton button = ImGuiMouseButton_Left;

    if (InputHelper::isPressTouch())
        io.AddMouseButtonEvent(button, true);
    else if (InputHelper::isReleaseTouch())
        io.AddMouseButtonEvent(button, false);
}

void updateMouse(ImGuiIO& io) {
    ImVec2 mousePos(0, 0);
    InputHelper::getMouseCoords(&mousePos.x, &mousePos.y);

    mousePos = ImVec2((mousePos.x / 1280.f) * io.DisplaySize.x, (mousePos.y / 720.f) * io.DisplaySize.y);

    io.AddMousePosEvent(mousePos.x, mousePos.y);

    ImVec2 scrollDelta(0, 0);
    InputHelper::getScrollDelta(&scrollDelta.x, &scrollDelta.y);

    if (scrollDelta.x != 0.0f) io.AddMouseWheelEvent(0.0f, scrollDelta.x > 0.0f ? 0.5f : -0.5f);

    for (auto [im_k, nx_k] : mouse_mapping) {
        if (InputHelper::isMousePress((nn::hid::MouseButton)nx_k))
            io.AddMouseButtonEvent((ImGuiMouseButton)im_k, true);
        else if (InputHelper::isMouseRelease((nn::hid::MouseButton)nx_k))
            io.AddMouseButtonEvent((ImGuiMouseButton)im_k, false);
    }
}

void updateKeyboard(ImGuiIO& io) {
    io.AddKeyEvent(ImGuiMod_Shift, InputHelper::isModifierActive(nn::hid::KeyboardModifier::Shift));
    io.AddKeyEvent(ImGuiMod_Ctrl, InputHelper::isModifierActive(nn::hid::KeyboardModifier::Control));
    io.AddKeyEvent(ImGuiMod_Super, InputHelper::isModifierActive(nn::hid::KeyboardModifier::Gui));
    io.AddKeyEvent(
        ImGuiMod_Alt, InputHelper::isModifierActive(nn::hid::KeyboardModifier::LeftAlt) || InputHelper::isModifierActive(nn::hid::KeyboardModifier::RightAlt)
    );

    bool isAltCode = InputHelper::isModifierActive(nn::hid::KeyboardModifier::CapsLock) || InputHelper::isModifierActive(nn::hid::KeyboardModifier::Shift);
    bool isNumLock = InputHelper::isModifierActive(nn::hid::KeyboardModifier::NumLock);

    for (auto [im_int, nx_int] : key_mapping) {
        auto nx_k = (nn::hid::KeyboardKey)nx_int;
        auto im_k = (ImGuiKey)im_int;

        if (InputHelper::isKeyPress(nx_k)) {
            io.AddKeyEvent(im_k, true);

            char keyCode = getKeyCode(im_k, isAltCode, isNumLock);
            if (keyCode != 0) io.AddInputCharacter(keyCode);
        } else if (InputHelper::isKeyRelease(nx_k)) {
            io.AddKeyEvent(im_k, false);
        }
    }
}

void updateGamepad(ImGuiIO& io) {
    for (auto [im_k, nx_k] : npad_mapping) {
        if (InputHelper::isButtonPress((nn::hid::NpadButton)nx_k))
            io.AddKeyEvent((ImGuiKey)im_k, true);
        else if (InputHelper::isButtonRelease((nn::hid::NpadButton)nx_k))
            io.AddKeyEvent((ImGuiKey)im_k, false);
    }
}

void updateInput() {
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDrawCursor = InputHelper::isMouseConnected();

    updateKeyboard(io);

    if (io.MouseDrawCursor) updateMouse(io);

    if (nn::oe::GetOperationMode() == nn::oe::OperationMode_Handheld) updateTouch(io);

    if (InputHelper::isInputToggled() && btt::Menu::instance()->mIsEnabledMenu) updateGamepad(io);
}

void updateProjection(ImVec2 dispSize) {
    orthoRH_ZO(getBackendData()->mShaderUBO.mMtx, 0.0f, dispSize.x, dispSize.y, 0.0f, -1.0f, 1.0f);
}

void updateScale(bool isDocked) {
    static float prevScale = 0.0f;

    float scale = isDocked ? 1.3f : 1.3f;

    ImGuiStyle& stylePtr = ImGui::GetStyle();
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGuiIO& io = ImGui::GetIO();

    ImVec4 prevColors[ImGuiCol_COUNT] = {};
    memcpy(&prevColors, &stylePtr.Colors, sizeof(stylePtr.Colors));

    // reset style
    stylePtr = ImGuiStyle();
    // set colors back to previous
    memcpy(&stylePtr.Colors, &prevColors, sizeof(stylePtr.Colors));
    // scale style
    ImGui::GetStyle().ScaleAllSizes(scale);
    // reset scale of windows
    if (prevScale != 0.0f) {
        ScaleWindowsInViewport(viewport, 1.f / prevScale);
    }

    // scale window
    ScaleWindowsInViewport(viewport, scale);
    prevScale = scale;
    // set font scale
    io.FontGlobalScale = scale;
}

void newFrame() {
    ImGuiIO& io = ImGui::GetIO();
    auto* bd = getBackendData();

    nn::TimeSpan curTick = nn::os::GetSystemTick().ToTimeSpan();
    nn::TimeSpan prevTick(bd->lastTick);
    io.DeltaTime = fabsf((float)(curTick.nanoseconds - prevTick.nanoseconds) / 1e9f);

    bd->lastTick = curTick;

    InputHelper::updatePadState(); // update input helper

    updateInput(); // update backend inputs
}

void setRenderStates() {
    auto bd = getBackendData();

    nvn::PolygonState polyState;
    polyState.SetDefaults();
    polyState.SetPolygonMode(nvn::PolygonMode::FILL);
    polyState.SetCullFace(nvn::Face::NONE);
    polyState.SetFrontFace(nvn::FrontFace::CCW);
    bd->cmdBuf->BindPolygonState(&polyState);

    nvn::ColorState colorState;
    colorState.SetDefaults();
    colorState.SetLogicOp(nvn::LogicOp::COPY);
    colorState.SetAlphaTest(nvn::AlphaFunc::ALWAYS);
    for (int i = 0; i < 8; ++i) {
        colorState.SetBlendEnable(i, true);
    }
    bd->cmdBuf->BindColorState(&colorState);

    nvn::BlendState blendState;
    blendState.SetDefaults();
    blendState.SetBlendFunc(nvn::BlendFunc::SRC_ALPHA, nvn::BlendFunc::ONE_MINUS_SRC_ALPHA, nvn::BlendFunc::ONE, nvn::BlendFunc::ZERO);
    blendState.SetBlendEquation(nvn::BlendEquation::ADD, nvn::BlendEquation::ADD);
    bd->cmdBuf->BindBlendState(&blendState);

    bd->cmdBuf->BindVertexAttribState(3, bd->attribStates);
    bd->cmdBuf->BindVertexStreamState(1, &bd->streamState);

    bd->cmdBuf->SetTexturePool(&bd->texPool);
    bd->cmdBuf->SetSamplerPool(&bd->samplerPool);
}

void renderDrawData(ImDrawData* drawData) {
    // we dont need to process any data if it isnt valid
    if (!drawData->Valid) {
        //            Logger::log("Draw Data was Invalid! Skipping Render.");
        return;
    }
    // if we dont have any command lists to draw, we can stop here
    if (drawData->CmdListsCount == 0) {
        //            Logger::log("Command List was Empty! Skipping Render.\n");
        return;
    }

    // get both the main backend data and IO from ImGui
    auto bd = getBackendData();
    ImGuiIO& io = ImGui::GetIO();

    // if something went wrong during backend setup, don't try to render anything
    if (!bd->isInitialized) {
        // Logger::log("Backend Data was not fully initialized!\n");
        return;
    }

    // initializes/resizes buffer used for all vertex data created by ImGui
    size_t totalVtxSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
    if (!bd->vtxBuffer || bd->vtxBuffer->GetPoolSize() < totalVtxSize) {
        if (bd->vtxBuffer) {
            bd->vtxBuffer->Finalize();
            IM_FREE(bd->vtxBuffer);
            // Logger::log("Resizing Vertex Buffer to Size: %d\n", totalVtxSize);
        } else {
            // Logger::log("Initializing Vertex Buffer to Size: %d\n", totalVtxSize);
        }

        bd->vtxBuffer = IM_NEW(MemoryBuffer)(totalVtxSize);
    }

    // initializes/resizes buffer used for all index data created by ImGui
    size_t totalIdxSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);
    if (!bd->idxBuffer || bd->idxBuffer->GetPoolSize() < totalIdxSize) {
        if (bd->idxBuffer) {
            bd->idxBuffer->Finalize();
            IM_FREE(bd->idxBuffer);

            // Logger::log("Resizing Index Buffer to Size: %d\n", totalIdxSize);
        } else {
            // Logger::log("Initializing Index Buffer to Size: %d\n", totalIdxSize);
        }

        bd->idxBuffer = IM_NEW(MemoryBuffer)(totalIdxSize);
    }

    // if we fail to resize/init either buffers, end execution before we try to use said invalid buffer(s)
    if (!(bd->vtxBuffer->IsBufferReady() && bd->idxBuffer->IsBufferReady())) {
        // Logger::log("Cannot Draw Data! Buffers are not Ready.\n");
        return;
    }

    bd->cmdBuf->BeginRecording(); // start recording our commands to the cmd buffer

    bd->cmdBuf->BindProgram(&bd->shaderProgram, nvn::ShaderStageBits::VERTEX | nvn::ShaderStageBits::FRAGMENT); // bind main imgui shader

    bd->cmdBuf->BindUniformBuffer(nvn::ShaderStage::VERTEX, 0, *bd->uniformMemory,
                                  UBOSIZE); // bind uniform block ptr to vertex stage

    bd->cmdBuf->BindUniformBuffer(nvn::ShaderStage::FRAGMENT, 0, *bd->uniformMemory,
                                  UBOSIZE); // bind uniform block ptr to fragment stage

    bd->cmdBuf->UpdateUniformBuffer(*bd->uniformMemory, UBOSIZE, 0, sizeof(bd->mShaderUBO),
                                    &bd->mShaderUBO); // add projection matrix data to uniform data

    setRenderStates(); // sets up the rest of the render state, required so that our shader properly gets drawn to the screen

    size_t vtxOffset = 0, idxOffset = 0;
    nvn::TextureHandle boundTextureHandle = 0;

    // load data into buffers, and process draw commands
    for (size_t i = 0; i < drawData->CmdListsCount; i++) {
        auto cmdList = drawData->CmdLists[i];

        // calc vertex and index buffer sizes
        size_t vtxSize = cmdList->VtxBuffer.Size * sizeof(ImDrawVert);
        size_t idxSize = cmdList->IdxBuffer.Size * sizeof(ImDrawIdx);

        // bind vtx buffer at the current offset
        bd->cmdBuf->BindVertexBuffer(0, (*bd->vtxBuffer) + vtxOffset, vtxSize);

        // copy data from imgui command list into our gpu dedicated memory
        memcpy(bd->vtxBuffer->GetMemPtr() + vtxOffset, cmdList->VtxBuffer.Data, vtxSize);
        memcpy(bd->idxBuffer->GetMemPtr() + idxOffset, cmdList->IdxBuffer.Data, idxSize);

        for (auto cmd : cmdList->CmdBuffer) {
            ImVec4 clipRect = cmd.ClipRect;

            ImVec2 clip_min(clipRect.x, clipRect.y);
            ImVec2 clip_max(clipRect.z, clipRect.w);
            ImVec2 clip_size(clip_max.x - clip_min.x, clip_max.y - clip_min.y);

            if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y) continue;

            bd->cmdBuf->SetScissor((int)clip_min.x, (int)clip_min.y, (int)clip_size.x, (int)clip_size.y);

            // get texture ID from the command
            nvn::TextureHandle TexID = *(nvn::TextureHandle*)cmd.GetTexID();
            // if our previous handle is different from the current, bind the texture
            if (boundTextureHandle != TexID) {
                boundTextureHandle = TexID;
                bd->cmdBuf->BindTexture(nvn::ShaderStage::FRAGMENT, 1, TexID);
            }
            // draw our vertices using the indices stored in the buffer, offset by the current command index offset,
            // as well as the current offset into our buffer.
            bd->cmdBuf->DrawElementsBaseVertex(
                nvn::DrawPrimitive::TRIANGLES, nvn::IndexType::UNSIGNED_SHORT, cmd.ElemCount,
                (*bd->idxBuffer) + (cmd.IdxOffset * sizeof(ImDrawIdx)) + idxOffset, cmd.VtxOffset
            );
        }

        vtxOffset += vtxSize;
        idxOffset += idxSize;
    }

    // end the command recording and submit to queue.
    auto handle = bd->cmdBuf->EndRecording();
    bd->queue->SubmitCommands(1, &handle);
}

} // namespace ImguiNvnBackend
