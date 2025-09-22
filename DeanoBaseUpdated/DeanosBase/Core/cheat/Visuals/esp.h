#pragma once 
#include "../Utilities/util.h"
#include "../Engine/sdk.h"
#include "../Engine/offsets.hpp"
#include "../Utilities/settings.h"
#include "../aimbot/aimbot.h"

void UpdateWorldCache() {
    WorldCache::AcknowledgedPawn = LocalPtrs::Player;
    if (WorldCache::AcknowledgedPawn != 0) {
        WorldCache::Mesh = read<uint64_t>(WorldCache::AcknowledgedPawn + offsets::Mesh);
        WorldCache::PlayerState = read<uint64_t>(WorldCache::AcknowledgedPawn + offsets::PlayerState);
        WorldCache::RootComponent = read<uint64_t>(WorldCache::AcknowledgedPawn + offsets::RootComponent);
        WorldCache::LocalWeapon = read<uint64_t>(WorldCache::AcknowledgedPawn + offsets::current_weapon);
        WorldCache::LocalVehicle = read<uint64_t>(WorldCache::AcknowledgedPawn + offsets::current_vehicle);

        if (WorldCache::PlayerState != 0)
            WorldCache::TeamIndex = read<char>(WorldCache::PlayerState + offsets::TeamIndex);
    }
    else {
        WorldCache::Mesh = 0;
        WorldCache::PlayerState = 0;
        WorldCache::RootComponent = 0;
        WorldCache::LocalWeapon = 0;
        WorldCache::LocalVehicle = 0;
        WorldCache::TeamIndex = 0;
    }
}

void ActorLoop() {
    LocalPtrs::Gworld = read<uint64_t>(Base + offsets::UWorld);
    if (!LocalPtrs::Gworld) return;

    const uintptr_t GameInstance = read<uint64_t>(LocalPtrs::Gworld + offsets::OwningGameInstance);
    if (!GameInstance) return;

    const uintptr_t LocalPlayerArray = read<uint64_t>(GameInstance + offsets::LocalPlayers);
    if (!LocalPlayerArray) return;

    LocalPtrs::LocalPlayers = read<uint64_t>(LocalPlayerArray);
    if (!LocalPtrs::LocalPlayers) return;

    LocalPtrs::PlayerController = read<uint64_t>(LocalPtrs::LocalPlayers + offsets::PlayerController);
    if (!LocalPtrs::PlayerController) return;

    LocalPtrs::Player = read<uint64_t>(LocalPtrs::PlayerController + offsets::AcknowledgedPawn);
    if (!LocalPtrs::Player) return;

    LocalPtrs::RootComponent = read<uint64_t>(LocalPtrs::Player + offsets::RootComponent);

    UpdateWorldCache();

    const uintptr_t GameState = read<uintptr_t>(LocalPtrs::Gworld + offsets::GameState);
    if (!GameState) return;

    const uintptr_t PlayerArray = read<uintptr_t>(GameState + offsets::PlayerArray);
    const int PlayerCount = read<int>(GameState + offsets::PlayerArray + sizeof(uintptr_t));

    uintptr_t FinalTargetEntity = 0;

    for (int i = 0; i < PlayerCount; i++) {
        const uintptr_t PlayerState = read<uintptr_t>(PlayerArray + (i * sizeof(uintptr_t)));
        if (!PlayerState) continue;

        const uintptr_t OtherPlayer = read<uintptr_t>(PlayerState + offsets::PawnPrivate);
        if (!OtherPlayer || OtherPlayer == LocalPtrs::Player) continue;

        const uintptr_t Mesh = read<uintptr_t>(OtherPlayer + offsets::Mesh);
        if (!Mesh) continue;

        Vector3 Head3D = GetBoneWithRotation(Mesh, 110);
        Vector3 Bottom3D = GetBoneWithRotation(Mesh, 0);
        Vector2 Head2D = ProjectWorldToScreen({ Head3D.x, Head3D.y, Head3D.z + 20 });
        Vector2 Bottom2D = ProjectWorldToScreen(Bottom3D);
        float Distance = vCamera.Location.Distance(Bottom3D) / 100.f;
        float BoxHeight = Head2D.y - Bottom2D.y;
        float BoxWidth = BoxHeight * 0.30f;

        ImColor BoxColor = IsEnemyVisible(Mesh) ? settings::colors.visible : settings::colors.notVisible;

        if (settings::esp.box)
            DrawCornerBox(Head2D.x - BoxWidth / 2.0f, Head2D.y, BoxWidth, std::abs(BoxHeight), BoxColor, 1.5f);

        if (settings::esp.snapline)
            ImGui::GetBackgroundDrawList()->AddLine(ImVec2(Width / 2.0f, Height), ImVec2(Bottom2D.x, Bottom2D.y), BoxColor, 1.f);

        if (settings::esp.distance) {
            char buffer[32];
            std::snprintf(buffer, sizeof(buffer), "%d m", static_cast<int>(Distance));
            ImGui::GetBackgroundDrawList()->AddText(ImVec2(Bottom2D.x, Bottom2D.y), IM_COL32(255, 255, 255, 255), buffer);
        }

        auto IsValidTarget = [&](uintptr_t target) -> bool {
            if (!target) return false;
            auto targetMesh = read<uint64_t>(target + offsets::Mesh);
            if (!targetMesh) return false;
            auto targetPawn = read<uintptr_t>(target + offsets::PawnPrivate);
            if (is_dead(targetPawn)) return false;
            Vector3 headBone = GetBoneWithRotation(targetMesh, 110);
            Vector2 headScreen = ProjectWorldToScreen(headBone);
            double dx = headScreen.x - (Width / 2);
            double dy = headScreen.y - (Height / 2);
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist > settings::aimbot.fovsize) return false;
            if (settings::aimbot.vischeck && !IsEnemyVisible(targetMesh)) return false;
            return true;
            };

        if (settings::aimbot.enabled && IsValidTarget(OtherPlayer)) {
            if (!FinalTargetEntity ||
                (settings::aimbot.targetPriority == 0 && GetDistanceToCrosshair(Mesh) < GetDistanceToCrosshair(FinalTargetEntity)) ||
                (settings::aimbot.targetPriority == 1 && GetFOVDistance(Mesh) < GetFOVDistance(FinalTargetEntity)))
            {
                FinalTargetEntity = OtherPlayer;
            }
        }
    }

    if (settings::aimbot.enabled && FinalTargetEntity) {
        auto targetMesh = read<uint64_t>(FinalTargetEntity + offsets::Mesh);
        if (!targetMesh) return;

        Vector3 hitbox;
        switch (settings::aimbot.Hitbox) {
        case 0: hitbox = GetBoneWithRotation(targetMesh, 110); break;
        case 1: hitbox = GetBoneWithRotation(targetMesh, 66); break;
        case 2: hitbox = GetBoneWithRotation(targetMesh, 36); break;
        case 3: hitbox = GetBoneWithRotation(targetMesh, 2); break;
        case 4: {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 3);
            int randomHitbox = dis(gen);
            hitbox = GetBoneWithRotation(targetMesh, randomHitbox == 0 ? 110 : randomHitbox == 1 ? 66 : randomHitbox == 2 ? 36 : 2);
            break;
        }
        }

        Vector2 hitboxScreen = ProjectWorldToScreen(hitbox);

        if ((hitboxScreen.x != 0 || hitboxScreen.y != 0) &&
            get_cross_distance(hitboxScreen.x, hitboxScreen.y, Width / 2, Height / 2) <= settings::aimbot.fovsize)
        {
            if (!settings::aimbot.vischeck || IsEnemyVisible(targetMesh)) {
                if (GetAsyncKeyState(settings::aimbot.aimkey) && settings::aimbot.mouse) {
                    if (settings::aimbot.prediction) {
                        auto root = read<uintptr_t>(FinalTargetEntity + offsets::RootComponent);
                        auto distance = vCamera.Location.Distance(hitbox);
                        Vector3 velocity = read<Vector3>(root + 0x188);

                        float projectileSpeed = read<float>(WorldCache::LocalWeapon + 0x2468);
                        float projectileGravityScale = read<float>(WorldCache::LocalWeapon + 0x246c);

                        if (projectileSpeed > 1000.f)
                            hitbox = PredictLocation(hitbox, velocity, projectileSpeed, projectileGravityScale, distance);

                        hitboxScreen = ProjectWorldToScreen(hitbox);
                    }
                    aim(targetMesh, hitboxScreen);
                }
            }
        }
    }
}