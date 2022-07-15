#include "pch.h"
#include "sigscanner.h"

//#include "json.hpp"

using namespace std;
using namespace CG;
// using json = nlohmann::json;

/* static */ DWORD64 uworld_offset = 0;
// static const DWORD64 processevent_offset = 0xEE57C0;
/* static */ DWORD64 base_address = NULL;

class UWorldProxy {
   public:
    class UWorld* world;
} world_proxy;

class Timer {
   private:
    std::chrono::steady_clock::time_point previous_tick_time_;
    float period_in_ms_;

   public:
    void SetPeriod(float period_in_ms) {
        this->period_in_ms_ = period_in_ms;
    }

    void SetFrequency(float frequency) {
        this->period_in_ms_ = 1000 * (1.0 / frequency);
    }

    Timer(void) {
        this->previous_tick_time_ = std::chrono::steady_clock::time_point();
        SetPeriod(1000);
    }

    Timer(float frequency) : Timer() {
        SetFrequency(frequency);
    }

    bool IsReady(void) {
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        float delta = std::chrono::duration<float>(now - previous_tick_time_).count() * 1000.0;
        if (delta > period_in_ms_) {
            Tick(now);
            return true;
        }
        return false;
    }

    void Tick(void) {
        previous_tick_time_ = std::chrono::steady_clock::now();
    }

    void Tick(std::chrono::steady_clock::time_point now) {
        previous_tick_time_ = now;
    }

    void Reset(void) {
        previous_tick_time_ = std::chrono::steady_clock::time_point();
    }

    void Restart(void) {
        previous_tick_time_ = std::chrono::steady_clock::now();
    }
};

namespace imgui {

namespace visuals {
/* static */ enum MarkerStyle { kNone, kDot, kCircle, kFilledSquare, kSquare, kBounds, kFilledBounds };
/* static */ const char* marker_labels[] = {"None", "Dot", "Circle", "Filled square", "Square", "Bounds", "Filled bounds"};

struct Marker {
    MarkerStyle marker_style = MarkerStyle::kSquare;
    int marker_size = 5;
    int marker_thickness = 2;
    ImColor marker_colour = {255, 255, 255, 255};
};

/* static */ struct AimbotVisualSettings : Marker {
    bool scale_by_distance = true;
    int distance_for_scaling = 5000;
    int minimum_marker_size = 3;
    AimbotVisualSettings(void) {
        marker_style = MarkerStyle::kFilledBounds;
        marker_size = 9;
        marker_thickness = 2;
        marker_colour = {255, 255, 0, 125};
    }
} aimbot_visual_settings;

/* static */ struct RadarVisualSettings : Marker {
    ImColor enemy_marker_colour = {255, 0, 0, 1 * 255};
    ImColor friendly_marker_colour = {0, 255, 0, 1 * 255};
    ImColor enemy_flag_marker_colour = {255, 255, 0, 255};
    ImColor friendly_flag_marker_colour = {0, 255, 255, 255};
    ImColor window_background_colour = {25, 25, 25, 25};
    bool draw_axes = true;

    float zoom_ = 0.004;
    float zoom = 1;
    int window_size = 300 * 1.25;
    ImVec2 window_location = {100, 100};
    float border = 5;
    int axes_thickness = 1;
    RadarVisualSettings(void) {
        marker_style = MarkerStyle::kDot;
        marker_size = 3;
        marker_thickness = 1;
    }
} radar_visual_settings;

/* static */ struct ESPVisualSettings {
    struct BoundingBoxSettings {
        int box_thickness = 2;
        ImColor enemy_player_box_colour = {0, 255, 255, 1 * 125};
        ImColor friendly_player_box_colour = {0, 255, 0, 1 * 125};
    } bounding_box_settings;

    struct LineSettings {
        int line_thickness = 1;
        ImColor enemy_player_line_colour = {255, 255, 255, 65};
    } line_settings;
} esp_visual_settings;

/* static */ struct CrosshairSettings : Marker {
    bool enabled = true;

    CrosshairSettings(void) {
        marker_style = MarkerStyle::kDot;
        marker_size = 3;
        marker_thickness = 1;
        marker_colour = {0, 255, 0, 255};
    }
} crosshair_settings;

void DrawMarker(MarkerStyle marker_style, ImVec2 center, ImColor marker_colour, int marker_size, int marker_thickness) {
    ImDrawList* imgui_draw_list = ImGui::GetWindowDrawList();
    switch (marker_style) {
        case visuals::MarkerStyle::kDot:
            imgui_draw_list->AddCircleFilled(center, marker_size, marker_colour);
            break;
        case visuals::MarkerStyle::kCircle:
            imgui_draw_list->AddCircle(center, marker_size, marker_colour, 0, marker_thickness);
            break;
        case visuals::MarkerStyle::kFilledSquare:
            imgui_draw_list->AddRectFilled({center.x - marker_size, center.y - marker_size}, {center.x + marker_size, center.y + marker_size}, marker_colour, 0);
            break;
        case visuals::MarkerStyle::kSquare:
            imgui_draw_list->AddRect({center.x - marker_size, center.y - marker_size}, {center.x + marker_size, center.y + marker_size}, marker_colour, 0, 0, marker_thickness);
            break;
        default:
            break;
    }
}

}  // namespace visuals
namespace imgui_menu {}

/* static */ struct ImGuiSettings { } imgui_settings; }  // namespace imgui

namespace hooks {
namespace processevent {
#define PROCESSEVENT_HOOK_FUNCTION(x) void x(UObject* object, UFunction* function, void* params)
typedef int(__fastcall* _ProcessEvent)(UObject*, UFunction*, void*);
_ProcessEvent original_processevent = NULL;
int ProcessEvent(UObject* object, UFunction* function, void* params);
unordered_map<UFunction*, vector<_ProcessEvent>> processevent_hooks;

bool AddProcessEventHook(UFunction* function, _ProcessEvent hook) {
    if (processevent_hooks.find(function) == processevent_hooks.end()) {
        processevent_hooks[function] = vector<_ProcessEvent>();
    }
    processevent_hooks[function].push_back(hook);
    return true;
}

}  // namespace processevent
}  // namespace hooks

namespace validate {
bool IsValid(AMACharacter* character) {
    return character && character->PlayerState /*&& character->Mesh && character->GetHealth() > 0*/;
}
}  // namespace validate

namespace math {
#define M_PI 3.14159265358979323846
#define M_PI_F ((float)(M_PI))
#define PI M_PI
#define DEG2RAD(x) ((float)(x) * (float)(M_PI_F / 180.f))
#define RAD2DEG(x) ((float)(x) * (float)(180.f / M_PI_F))
#define INV_PI (0.31830988618f)
#define HALF_PI (1.57079632679f)

void SinCos(float* ScalarSin, float* ScalarCos, float Value) {
    // Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
    float quotient = (INV_PI * 0.5f) * Value;
    if (Value >= 0.0f) {
        quotient = (float)((int)(quotient + 0.5f));
    } else {
        quotient = (float)((int)(quotient - 0.5f));
    }
    float y = Value - (2.0f * PI) * quotient;

    // Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
    float sign;
    if (y > HALF_PI) {
        y = PI - y;
        sign = -1.0f;
    } else if (y < -HALF_PI) {
        y = -PI - y;
        sign = -1.0f;
    } else {
        sign = +1.0f;
    }

    float y2 = y * y;

    // 11-degree minimax approximation
    *ScalarSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

    // 10-degree minimax approximation
    float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
    *ScalarCos = sign * p;
}

FVector RotatorToVector(FRotator rotation) {
    float CP, SP, CY, SY;
    SinCos(&SP, &CP, DEG2RAD(rotation.Pitch));
    SinCos(&SY, &CY, DEG2RAD(rotation.Yaw));
    FVector V = FVector(CP * CY, CP * SY, SP);

    return V;
}

FRotator VectorToRotator(FVector v) {
    FRotator rotator;
    rotator.Yaw = RAD2DEG(atan2(v.Y, v.X));
    rotator.Pitch = RAD2DEG(atan2(v.Z, sqrt((v.X * v.X) + (v.Y * v.Y))));
    rotator.Roll = 0;  // No roll

    return rotator;
}

FVector CrossProduct(FVector U, FVector V) {
    return FVector(U.Y * V.Z - U.Z * V.Y, U.Z * V.X - U.X * V.Z, U.X * V.Y - U.Y * V.X);
}

bool IsVectorToRight(FVector base_vector, FVector vector_to_check) {
    FVector right = CrossProduct(base_vector, {0, 0, 1});

    if (right.Dot(vector_to_check) < 0) {
        return true;
    } else {
        return false;
    }
}

// Angle on the X-Y plane
float AngleBetweenVector(FVector a, FVector b) {
    // a.b = |a||b|cosx
    a.Z = 0;
    b.Z = 0;

    float dot = a.Dot(b);
    float denom = a.Magnitude() * b.Magnitude();
    float div = dot / denom;
    return acos(div);
}

}  // namespace math

namespace game_data {
/* static */ ULocalPlayer* local_player = NULL;
/* static */ AMAPlayerController* local_player_controller = NULL;
/* static */ AMACharacter* local_player_character = NULL;

/* static */ FVector2D screen_size = FVector2D();
/* static */ FVector2D screen_center = FVector2D();

enum class Weapon { none, disk, cg, gl, blaster, plasma, unknown };
enum class WeaponType { kHitscan, kProjectileLinear, kProjectileArching };

namespace information {

/* static */ struct WeaponSpeeds {
    struct Weapon {
        float bullet_speed;
        float inheritence;
    };

    struct Disk : Weapon {
        Disk() {
            bullet_speed = 6500;  //(5850 + 6050) / 2;  // 5850 //6500 IS THE REAL IN GAME SPEED
            inheritence = 0.5;    // 0.5 IS THE REAL INHERITENCE
        }
    } disk;

    struct Chaingun : Weapon {
        Chaingun() {
            bullet_speed = 52500;  // 37500;  // 52500 IS THE REAL IN GAME SPEED
            inheritence = 1;       // 1 IS THE REAL INHERITENCE
        }
    } chaingun;

    struct Grenadelauncher : Weapon {
        Grenadelauncher() {
            bullet_speed = 4700;  // 6502.13;  // 4700 IS THE REAL IN GAME SPEED
            inheritence = 0.75;   // 0.5;       // 0.75 IS THE REAL INHERITENCE
        }
    } grenadelauncher;

    struct Plasma : Weapon {
        Plasma() {
            bullet_speed = 4800;  // 4800 IS THE REAL SPEED
            inheritence = 0.5;    // 0.5 IS THE REAL INHERITENCE
        }
    } plasma;

    struct Blaster : Weapon {
        Blaster() {
            bullet_speed = 15000;  // 15000 IS THE REAL IN GAME SPEED  // GRAVITY IS -750
            inheritence = 1;       // 1 IS THE REAL INHERITENCE
        }
    } blaster;
} weapon_speeds;

struct GameActor {
   public:
    int team_id_ = -1;
    FVector location_;
    FRotator rotation_;
    FVector velocity_;
    // FVector velocity_previous_;
    FVector forward_vector_;
    FVector acceleration_;

    bool IsSameTeam(GameActor* actor) {
        return this->team_id_ == actor->team_id_;
    }
};

struct Player : public GameActor {
   public:
    ABP_BaseCharacter_C* character_ = NULL;

    int player_id_ = -1;
    float health_ = 0;
    float energy_ = 0;

    bool is_valid_ = false;
    Weapon weapon_ = Weapon::none;
    WeaponType weapon_type_ = WeaponType::kHitscan;

    void Reset(void) {
        character_ = NULL;
        is_valid_ = false;
    }

    void Setup(ABP_BaseCharacter_C* character) {
        Reset();
        if (!character)
            return;

        AMAPlayerState* player_state = (AMAPlayerState*)character->PlayerState;
        if (!player_state)
            return;

        /*
        health_ = character->GetHealth();
        energy_ = character->GetEnergy();

        if (health_ > 0) {
            is_valid_ = true;
        } else {
            is_valid_ = false;
        }
        */

        player_id_ = character->PlayerState->PlayerId;
        if (player_state->TeamState)
            team_id_ = player_state->TeamState->TeamId;

        location_ = character->RootComponent->RelativeLocation;
        rotation_ = character->RootComponent->RelativeRotation;
        velocity_ = character->RootComponent->ComponentVelocity;
        // acceleration_ = character->CharacterMovement->Acceleration;
        forward_vector_ = math::RotatorToVector(rotation_).Unit();

        character_ = character;

        is_valid_ = true;
    }
};

class Flag : public GameActor {
   public:
    AMACarriedObject* flag_;
    AMACarriedObjectBase* base_;
    bool is_held_ = false;
    bool is_valid_ = false;

    Flag(AMACarriedObjectBase* base) {}

    Flag(AMACarriedObject* flag) {
        if (!flag)
            return;
        flag_ = flag;
        base_ = flag->HomeBase;
        location_ = flag->K2_GetActorLocation();
        is_held_ = !flag->IsHome();
        if (flag->TeamState) {
            team_id_ = flag->TeamState->TeamId;
        }
        is_valid_ = true;
    }
};

}  // namespace information

/* static */ class GameData {
   public:
    information::Player my_player_information;
    vector<information::Player> players;
    vector<information::Flag> flags;
    vector<APlayerController*> player_controllers;

    GameData(void) {
        ;
    }

    void Reset(void) {
        players.clear();

        flags.clear();
        my_player_information.weapon_ = Weapon::none;
        my_player_information.weapon_type_ = WeaponType::kHitscan;
        my_player_information.is_valid_ = false;  // Invalidate the player every frame
    }

} game_data;

/* static */ game_data::information::Player& my_player = game_data::game_data.my_player_information;

/* static */ Timer get_player_controllers_timer(0.1 * 5);

void GetPlayers(void) {
    local_player_character = (AMACharacter*)local_player_controller->Character;
    bool my_player_character_found = false;

    AMAGameState* game_state = ((AMAGameState*)world_proxy.world->GameState);
    if (game_state == NULL)
        return;

    /*
    if (game_state->IsA(AMACTFGameState::StaticClass())) {
        AMACTFGameState* ctf_game_state = ((AMACTFGameState*)world_proxy.world->GameState);
        for (int i = 0; i < ctf_game_state->FlagBases.Num(); i++) {
            AMACTFFlagBase* flag_base = ctf_game_state->FlagBases[i];
            if (flag_base && flag_base->CarriedObject) {
                game_data.flags.push_back(information::Flag(flag_base->CarriedObject));
            }
        }
    }
    */

    /* static */ information::Player player_information;

    TArray<APlayerState*>& player_states = game_state->PlayerArray;
    for (int i = 0; i < player_states.Num(); i++) {
        AMAPlayerState* player_state = (AMAPlayerState*)player_states[i];
        AMACharacter* character_ = (AMACharacter*)player_state->PawnPrivate;
        ABP_LightCharacter_C* character = (ABP_LightCharacter_C*)character_;

        player_information.Setup((ABP_BaseCharacter_C*)character);

        if (player_information.is_valid_) {
            if (character != local_player_character) {
                game_data.players.push_back(player_information);  // game_data.players does not include ourselves
            } else if (character == local_player_character) {
                my_player_character_found = true;
                my_player.Setup((ABP_BaseCharacter_C*)character);

                my_player.rotation_ = local_player_controller->ControlRotation;
                my_player.forward_vector_ = math::RotatorToVector(local_player_controller->ControlRotation).Unit();

            }
        }
    }

    if (!my_player_character_found) {
        my_player.forward_vector_ = math::RotatorToVector(local_player_controller->ControlRotation).Unit();
        my_player.is_valid_ = false;
    }

    return;
}

void GetGameData(void);

}  // namespace game_data

namespace W2S {
struct vMatrix {
    vMatrix() {}
    vMatrix(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23) {
        m_flMatVal[0][0] = m00;
        m_flMatVal[0][1] = m01;
        m_flMatVal[0][2] = m02;
        m_flMatVal[0][3] = m03;
        m_flMatVal[1][0] = m10;
        m_flMatVal[1][1] = m11;
        m_flMatVal[1][2] = m12;
        m_flMatVal[1][3] = m13;
        m_flMatVal[2][0] = m20;
        m_flMatVal[2][1] = m21;
        m_flMatVal[2][2] = m22;
        m_flMatVal[2][3] = m23;
    }

    float* operator[](int i) {
        /* Assert((i >= 0) && (i < 3));*/
        return m_flMatVal[i];
    }
    const float* operator[](int i) const {
        /*Assert((i >= 0) && (i < 3)); */
        return m_flMatVal[i];
    }

    float* Base() {
        return &m_flMatVal[0][0];
    }
    const float* Base() const {
        return &m_flMatVal[0][0];
    }

    float m_flMatVal[3][4];
};
vMatrix matrix;

vMatrix Matrix(FVector rot, FVector origin) {
    origin = FVector(0, 0, 0);
    float radPitch = (rot.X * float(PI) / 180.f);
    float radYaw = (rot.Y * float(PI) / 180.f);
    float radRoll = (rot.Z * float(PI) / 180.f);

    float SP = sinf(radPitch);
    float CP = cosf(radPitch);
    float SY = sinf(radYaw);
    float CY = cosf(radYaw);
    float SR = sinf(radRoll);
    float CR = cosf(radRoll);

    matrix[0][0] = CP * CY;
    matrix[0][1] = CP * SY;
    matrix[0][2] = SP;
    matrix[0][3] = 0.f;

    matrix[1][0] = SR * SP * CY - CR * SY;
    matrix[1][1] = SR * SP * SY + CR * CY;
    matrix[1][2] = -SR * CP;
    matrix[1][3] = 0.f;

    matrix[2][0] = -(CR * SP * CY + SR * SY);
    matrix[2][1] = CY * SR - CR * SP * SY;
    matrix[2][2] = CR * CP;
    matrix[2][3] = 0.f;

    matrix[3][0] = origin.X;
    matrix[3][1] = origin.Y;
    matrix[3][2] = origin.Z;
    matrix[3][3] = 1.f;

    return matrix;
}

FVector2D WorldToScreen(FVector world, FVector Rotation, FVector cameraPosition, float FovAngle) {
    FVector Screenlocation = FVector(0, 0, 0);

    vMatrix tempMatrix = Matrix(Rotation, Screenlocation);

    FVector vAxisX, vAxisY, vAxisZ;

    vAxisX = FVector(tempMatrix[0][0], tempMatrix[0][1], tempMatrix[0][2]);
    vAxisY = FVector(tempMatrix[1][0], tempMatrix[1][1], tempMatrix[1][2]);
    vAxisZ = FVector(tempMatrix[2][0], tempMatrix[2][1], tempMatrix[2][2]);

    FVector vDelta = world - cameraPosition;
    FVector vTransformed = FVector(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

    if (vTransformed.Z < 1.f)
        vTransformed.Z = 1.f;

    float ScreenCenterX = game_data::screen_size.X / 2.0f;
    float ScreenCenterY = game_data::screen_size.Y / 2.0f;

    auto tmpFOV = tanf(FovAngle * (float)PI / 360.f);

    FVector2D screen;
    screen.X = ScreenCenterX + vTransformed.X * (ScreenCenterX / tmpFOV) / vTransformed.Z;
    screen.Y = ScreenCenterY - vTransformed.Y * (ScreenCenterX / tanf(FovAngle * (float)PI / 360.f)) / vTransformed.Z;
    return screen;
}
}  // namespace W2S

namespace game_functions {
bool InLineOfSight(AActor* actor) {
    return false;
    /*return game_data::local_player_controller->LineOfSightTo(actor, FVector(), false);*/
}

FVector2D Project(FVector location) {
    FRotator r = game_data::local_player->PlayerController->PlayerCameraManager->CameraCachePrivate.POV.Rotation;
    FVector cam_location = game_data::local_player->PlayerController->PlayerCameraManager->CameraCachePrivate.POV.Location;
    float fov = game_data::local_player->PlayerController->PlayerCameraManager->CameraCachePrivate.POV.FOV;

    FVector rot(r.Pitch, r.Yaw, r.Roll);
    FVector loc(cam_location.X, cam_location.Y, cam_location.Z);

    FVector2D screen;
    screen = W2S::WorldToScreen(location, rot, loc, fov);
    if (screen.X >= 0 && screen.X <= game_data::screen_size.X && screen.Y >= 0 && screen.Y <= game_data::screen_size.Y)
        return screen;

    return FVector2D(0, 0);

    /*
    return FVector2D(100, 100);

    FVector2D projection;
    game_data::local_player->PlayerController->ProjectWorldLocationToScreen(location, &projection, false);
    return projection;
    */
}

bool IsInFieldOfView(FVector enemy_location) {
    FVector rotation_vector = game_data::my_player.forward_vector_;
    FVector difference_vector = enemy_location - game_data::my_player.location_;
    double dot = rotation_vector.X * difference_vector.X + rotation_vector.Y * difference_vector.Y;
    if (dot <= 0)      // dot is > 0 if vectors face same direction, 0 if vectors perpendicular, negative if facing oppposite directions
        return false;  // we want to ensure the vectors face in the same direction
    return true;
}

bool IsInHorizontalFieldOfView(FVector enemy_location, double horizontal_fov) {
    FVector rotation_vector = game_data::my_player.forward_vector_;
    FVector difference_vector = enemy_location - game_data::my_player.location_;
    double dot = rotation_vector.X * difference_vector.X + rotation_vector.Y * difference_vector.Y;
    if (dot <= 0)      // dot is > 0 if vectors face same direction, 0 if vectors perpendicular, negative if facing oppposite directions
        return false;  // we want to ensure the vectors face in the same direction

    double dot_sq = dot * dot;  // squaring the dot loses the negative sign, so we cant tell from this point onwards
                                // if the enemy is in front or behind us if using dot_sq
    double denom = (rotation_vector.X * rotation_vector.X + rotation_vector.Y * rotation_vector.Y) * (difference_vector.X * difference_vector.X + difference_vector.Y * difference_vector.Y);
    double angle_sq = dot_sq / denom;
    double v = pow(cos(DEG2RAD(horizontal_fov)), 2);

    if (angle_sq < v)
        return false;
    return true;
}

FVector GetMuzzleOffset(void) {
    return game_data::my_player.character_->Weapon->FireOffset;
}

}  // namespace game_functions

namespace other {

/* static */ struct OtherSettings { bool disable_hitmarker = false; } other_settings;

void SendLeftMouseClick(void) {
    INPUT inputs[2];
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

void Tick(void) {
    if (other::other_settings.disable_hitmarker) {
        AMAHUD* ama_hud = (AMAHUD*)game_data::local_player_controller->MyHUD;
        if (ama_hud) {
            ama_hud->LastHitMarkerInfo.Time = 1.5;
            ama_hud->LastHitMarkerInfo.bCriticalHit = true;
            ama_hud->LastHitMarkerInfo.bTeamHit = true;
            ama_hud->LastHitMarkerInfo.Damage = 0;
            // ama_hud->LastHitMarkerInfo.MaterialInstance = NULL;
            // ama_hud->HitMarkerMaterial = NULL;
        }
    }
}
}  // namespace other

namespace esp {

/* static */ struct ESPSettings {
    bool enabled = true;
    int poll_frequency = 60 * 5;
    bool show_friendlies = false;
    int player_height = 100;
    int player_width = 0;
    float width_to_height_ratio = 0.5;
    bool show_lines = true;
} esp_settings;

/* static */ Timer get_esp_data_timer(esp_settings.poll_frequency);

struct ESPInformation {
    FVector2D projection;  // center
    float height;          // height for box/rectangle
    bool is_friendly = false;
};

vector<ESPInformation> esp_information;

void Tick(void) {
    if (!esp_settings.enabled || !get_esp_data_timer.IsReady())
        return;

    esp_information.clear();

    if (!game_data::my_player.is_valid_)
        ;  // return;

    // bool is_my_player_alive = game_data::my_player.is_valid_;
    for (vector<game_data::information::Player>::iterator player = game_data::game_data.players.begin(); player != game_data::game_data.players.end(); player++) {
        if (!player->is_valid_) {
            continue;
        }

        game_data::information::Player* p = (game_data::information::Player*)&*player;
        bool same_team = game_data::my_player.IsSameTeam(p);

        if ((same_team && !esp_settings.show_friendlies) || !game_functions::IsInFieldOfView(player->location_))
            continue;

        if (!game_functions::IsInFieldOfView(player->location_))
            continue;

        FVector2D center_projection = game_functions::Project(player->location_);
        player->location_.Z += esp_settings.player_height;  // this is HALF the height in reality
        FVector2D head_projection = game_functions::Project(player->location_);
        player->location_.Z -= esp_settings.player_height;  // this is HALF the height in reality
        float height = abs(head_projection.Y - center_projection.Y);
        float width = esp_settings.width_to_height_ratio * height;
        esp_information.push_back({center_projection, height, same_team});

        // cout << height << endl;
    }
}
}  // namespace esp

namespace aimbot {

// Overshooting means the weapon bullet speed is too low
// Undershooting means the weapon bullet speed is too high

/* static */ float delta_time = 0;
vector<game_data::information::Player> players_previous;

enum AimbotMode { kClosestDistance, kClosestXhair };
/* static */ const char* mode_labels[] = {"Closest distance", "Closest to crosshair"};
/* static */ bool enabled = true;

/* static */ struct AimbotSettings {
    AimbotMode aimbot_mode = AimbotMode::kClosestXhair;

    bool enabled = true;          // enabling really just enables aimassist, this isnt really an aimbot
    bool target_everyone = true;  // if we want to do prediction on every single player

    float tempest_ping_in_ms = 0;   //-90
    float chaingun_ping_in_ms = 0;  //-50
    float grenadelauncher_ping_in_ms = 0;
    float plasmagun_ping_in_ms = 0;
    float blaster_ping_in_ms = 0;

    bool stay_locked_to_target = true;
    bool auto_lock_to_new_target = true;

    float aimbot_horizontal_fov_angle = 90;         // 30;
    float aimbot_horizontal_fov_angle_cos = 0;      // 0.86602540378;
    float aimbot_horizontal_fov_angle_cos_sqr = 0;  // 0.75;

    bool friendly_fire = false;
    bool need_line_of_sight = false;

    int aimbot_poll_frequency = 60 * 5;

    bool use_acceleration = true;
    bool use_acceleration_cg_only = false;
    // float acceleration_delta_in_ms = 30;

    bool triggerbot_enabled = true;
} aimbot_settings;

/* static */ Timer aimbot_poll_timer(aimbot_settings.aimbot_poll_frequency);

/* static */ game_data::information::Player target_player;

vector<FVector2D> projections_of_predictions;

struct AimbotInformation {
    float distance_;
    FVector2D projection_;
    float height;
};

vector<AimbotInformation> aimbot_information;

/* static */ struct WeaponAimbotParameters {
    int maximum_iterations = 2 * 10;
    float epsilon = 0.05 / 3;
} aimbot_parameters_;

bool PredictAimAtTarget(game_data::information::Player* target_player, FVector* output_vector, FVector offset) {
    float projectileSpeed;
    float inheritence;
    float ping;

    switch (game_data::my_player.weapon_) {
        using namespace game_data::information;
        case game_data::Weapon::disk:
            projectileSpeed = weapon_speeds.disk.bullet_speed;
            inheritence = weapon_speeds.disk.inheritence;
            ping = aimbot::aimbot_settings.tempest_ping_in_ms;
            break;
        case game_data::Weapon::cg:
            projectileSpeed = weapon_speeds.chaingun.bullet_speed;
            inheritence = weapon_speeds.chaingun.inheritence;
            ping = aimbot::aimbot_settings.chaingun_ping_in_ms;
            break;
        case game_data::Weapon::gl:
            projectileSpeed = weapon_speeds.grenadelauncher.bullet_speed;
            inheritence = weapon_speeds.grenadelauncher.inheritence;
            ping = aimbot::aimbot_settings.grenadelauncher_ping_in_ms;
            break;
        case game_data::Weapon::plasma:
            projectileSpeed = weapon_speeds.plasma.bullet_speed;
            inheritence = weapon_speeds.plasma.inheritence;
            ping = aimbot::aimbot_settings.plasmagun_ping_in_ms;
            break;
        case game_data::Weapon::blaster:
            projectileSpeed = weapon_speeds.blaster.bullet_speed;
            inheritence = weapon_speeds.blaster.inheritence;
            ping = aimbot::aimbot_settings.blaster_ping_in_ms;
            break;
        case game_data::Weapon::none:
        case game_data::Weapon::unknown:
        default:
            return false;
    }

    if (game_data::my_player.weapon_type_ == game_data::WeaponType::kHitscan) {
        *output_vector = target_player->location_;
        return true;
    }

    /*
    if (game_data::my_player.weapon_type_ == game_data::WeaponType::kProjectileArching) {
        return PredictAimAtTargetDL(target_player, output_vector, offset);
    }
    */

    if (game_data::my_player.weapon_type_ != game_data::WeaponType::kProjectileArching && game_data::my_player.weapon_type_ != game_data::WeaponType::kProjectileLinear)
        return false;

    FVector owner_location = game_data::my_player.location_ - offset * 0;
    FVector owner_velocity = game_data::my_player.velocity_;

    // owner_location = owner_location - owner_velocity * (weapon_parameters_.self_compensation_ping_ / 1000.0);

    FVector target_location = target_player->location_;
    FVector target_velocity = target_player->velocity_;

    /*
    a_ = (v-u)/t
    acceleration is normalised -> a = a_ / |a_|
    */
    FVector target_acceleration = FVector();
    if (aimbot::aimbot_settings.use_acceleration && (!aimbot_settings.use_acceleration_cg_only || (aimbot_settings.use_acceleration_cg_only && (game_data::game_data.my_player_information.weapon_ == game_data::Weapon::cg || game_data::game_data.my_player_information.weapon_ == game_data::Weapon::blaster)))) {
        FVector velocity_previous = FVector();
        bool player_found = false;
        for (vector<game_data::information::Player>::iterator player = players_previous.begin(); player != players_previous.end(); player++) {
            if (player->character_ == target_player->character_) {
                player_found = true;
                velocity_previous = player->velocity_;
                break;
            }
        }

        if (player_found) {
            target_acceleration = (target_velocity - velocity_previous) / (delta_time / 1000.0);  // should be dividing by 1000 to get ms in seconds  //(aimbot::aimbot_settings.acceleration_delta_in_ms/1000.0);

            // target_acceleration = target_player->character_->CharacterMovement->Acceleration.Unit() * target_acceleration.Magnitude();

            // cout << "Custom accecl: " << target_acceleration.X << ", " << target_acceleration.Y << ", " << target_acceleration.Z << endl;
            // cout << "Real accecl: " << target_player->character_->CharacterMovement->Acceleration.X << ", " << target_player->character_->CharacterMovement->Acceleration.Y << ", " << target_player->character_->CharacterMovement->Acceleration.Z << endl << endl;

            // cout << delta_time << endl;
        }
    }

    // float ping_time = weapon_parameters_.ping_ / 1000.0;

    FVector prediction = target_location;
    FVector ping_prediction = target_location;

    /* static */ vector<double> D(aimbot_parameters_.maximum_iterations, 0);
    /* static */ vector<double> dt(aimbot_parameters_.maximum_iterations, 0);

    int i = 0;
    do {
        D[i] = (owner_location - prediction).Magnitude();
        dt[i] = D[i] / projectileSpeed;
        if (i > 0 && abs(dt[i] - dt[i - 1]) < aimbot_parameters_.epsilon) {
            break;
        }

        prediction = (target_location + (target_velocity * dt[i] * 1) + (target_acceleration * pow(dt[i], 2) * 0.5) - (1 ? (owner_velocity * (inheritence * dt[i])) : FVector()));
        i++;
    } while (i < aimbot_parameters_.maximum_iterations);

    if (i == aimbot_parameters_.maximum_iterations) {
        return false;
    }

    float full_time = dt[i] + ping / 1000.0;

    ping_prediction = prediction = (target_location + (target_velocity * full_time * 1) + (target_acceleration * pow(dt[i], 2) * 0.5) - (1 ? (owner_velocity * (inheritence * full_time)) : FVector()));

    *output_vector = ping_prediction;

    return true;
}

void Enable(void) {
    enabled = true;
}

void Reset(void) {
    target_player.Reset();
    Enable();
}

void Disable(void) {
    Reset();
    enabled = false;
}

void Toggle(void) {
    aimbot_settings.enabled = !aimbot_settings.enabled;
    if (!aimbot_settings.enabled) {
        Reset();
    }
    return;

    if (enabled) {
        Disable();
    } else {
        Enable();
    }
}

bool FindTarget(void) {
    ABP_BaseCharacter_C* current_target_character = target_player.character_;
    target_player.Setup(current_target_character);

    bool need_to_find_target = true;  //! aimbot_settings.target_everyone;

    if (aimbot_settings.stay_locked_to_target) {
        if (!target_player.is_valid_) {
            if (!aimbot_settings.auto_lock_to_new_target && current_target_character != NULL) {
                Disable();
                return false;
            }
        } else {
            need_to_find_target = false;
        }
    }

    if (need_to_find_target) {
        current_target_character = NULL;
        FVector rotation_vector = game_data::my_player.forward_vector_;
        for (vector<game_data::information::Player>::iterator player = game_data::game_data.players.begin(); player != game_data::game_data.players.end(); player++) {
            if (!player->is_valid_) {
                continue;
            }

            game_data::information::Player* p = (game_data::information::Player*)&*player;
            bool same_team = game_data::my_player.IsSameTeam(p);
            bool line_of_sight = game_functions::InLineOfSight(player->character_);
            if ((same_team && !aimbot_settings.friendly_fire) || (!game_functions::IsInFieldOfView(player->location_) && aimbot_settings.aimbot_mode == AimbotMode::kClosestXhair) || (!line_of_sight && aimbot_settings.need_line_of_sight))
                continue;

            switch (aimbot_settings.aimbot_mode) {
                case AimbotMode::kClosestXhair: {
                    static double distance = 0;
                    FVector enemy_location = player->location_;

                    if (!game_functions::IsInHorizontalFieldOfView(player->location_, aimbot_settings.aimbot_horizontal_fov_angle))
                        continue;

                    FVector2D projection_vector = game_functions::Project(enemy_location);
                    if (!current_target_character) {
                        current_target_character = player->character_;
                        distance = (projection_vector - game_data::screen_center).MagnitudeSqr();
                    } else {
                        double current_distance = (projection_vector - game_data::screen_center).MagnitudeSqr();
                        if (current_distance < distance) {
                            current_target_character = player->character_;
                            distance = current_distance;
                        }
                    }
                } break;

                case AimbotMode::kClosestDistance: {
                    static double distance = 0;
                    if (!current_target_character) {
                        current_target_character = player->character_;
                        distance = (game_data::my_player.location_ - player->location_).MagnitudeSqr();
                    } else {
                        double current_distance = (game_data::my_player.location_ - player->location_).MagnitudeSqr();
                        if (current_distance < distance) {
                            current_target_character = player->character_;
                            distance = current_distance;
                        }
                    }
                } break;
            }
        }
    }

    target_player.Setup(current_target_character);

    return current_target_character != NULL;
}

/* static */ std::chrono::steady_clock::time_point previous_tick = std::chrono::steady_clock::now();

void Tick(void) {
    if (!aimbot_settings.enabled /*|| !enabled*/ || !aimbot_poll_timer.IsReady())
        return;

    // static std::chrono::steady_clock::time_point previous_tick = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    delta_time = std::chrono::duration<float>(now - previous_tick).count() * 1000.0;
    previous_tick = now;

    projections_of_predictions.clear();
    aimbot_information.clear();

    if (!game_data::my_player.is_valid_ || game_data::my_player.weapon_ == game_data::Weapon::none || game_data::my_player.weapon_ == game_data::Weapon::unknown)
        return;

    /* static */ FVector prediction;
    FVector muzzle_offset = game_data::my_player.character_->Weapon->FireOffset;  // game_functions::GetMuzzleOffset();

    if (!aimbot_settings.target_everyone) {
        if (enabled && FindTarget() /*&& target_player.character_*/) {
            bool result = PredictAimAtTarget(&target_player, &prediction, muzzle_offset);

            if (result) {
                FVector2D projection = game_functions::Project(prediction);
                float height = -1;

                if (((imgui::visuals::aimbot_visual_settings.marker_style == imgui::visuals::MarkerStyle::kBounds || imgui::visuals::aimbot_visual_settings.marker_style == imgui::visuals::MarkerStyle::kFilledBounds) && height == -1) || aimbot_settings.triggerbot_enabled) {
                    FVector2D center_projection = game_functions::Project(target_player.location_);
                    target_player.location_.Z += esp::esp_settings.player_height;  // this is HALF the height in reality
                    FVector2D head_projection = game_functions::Project(target_player.location_);
                    target_player.location_.Z -= esp::esp_settings.player_height;  // this is HALF the height in reality
                    height = abs(head_projection.Y - center_projection.Y);

                    if (aimbot_settings.triggerbot_enabled) {
                        float width = esp::esp_settings.width_to_height_ratio * height;
                        if (abs(game_data::screen_center.X - projection.X) < width && abs(game_data::screen_center.Y - projection.Y) < height) {
                            if (game_data::my_player.weapon_ == game_data::Weapon::disk || game_data::my_player.weapon_ == game_data::Weapon::gl || game_data::my_player.weapon_ == game_data::Weapon::plasma || game_data::my_player.weapon_ == game_data::Weapon::blaster) {
                                other::SendLeftMouseClick();
                            }
                        }
                    }
                }

                projections_of_predictions.push_back(projection);
                aimbot_information.push_back({(prediction - game_data::my_player.location_).Magnitude(), projection, height});

                // cout << (int)target_player.character_->AccelInfo << endl;
            }
        }
    } else {
        for (vector<game_data::information::Player>::iterator player = game_data::game_data.players.begin(); player != game_data::game_data.players.end(); player++) {
            if (!player->is_valid_ || player->character_ == game_data::my_player.character_) {
                continue;
            }

            game_data::information::Player* p = (game_data::information::Player*)&*player;
            bool same_team = game_data::my_player.IsSameTeam(p);
            bool line_of_sight = game_functions::InLineOfSight(player->character_);

            if ((same_team && !aimbot_settings.friendly_fire) || (!game_functions::IsInFieldOfView(player->location_) && aimbot_settings.aimbot_mode == AimbotMode::kClosestXhair) || (!line_of_sight && aimbot_settings.need_line_of_sight))
                continue;

            if (!game_functions::IsInHorizontalFieldOfView(player->location_, aimbot_settings.aimbot_horizontal_fov_angle))
                continue;

            // aimbot::aimbot_settings.use_acceleration = false;

            bool result = PredictAimAtTarget(&*p, &prediction, muzzle_offset);

            if (result) {
                FVector2D projection = game_functions::Project(prediction);
                float height = -1;

                if (((imgui::visuals::aimbot_visual_settings.marker_style == imgui::visuals::MarkerStyle::kBounds || imgui::visuals::aimbot_visual_settings.marker_style == imgui::visuals::MarkerStyle::kFilledBounds) && height == -1) || aimbot_settings.triggerbot_enabled) {
                    FVector2D center_projection = game_functions::Project(player->location_);
                    player->location_.Z += esp::esp_settings.player_height;  // this is HALF the height in reality
                    FVector2D head_projection = game_functions::Project(player->location_);
                    player->location_.Z -= esp::esp_settings.player_height;  // this is HALF the height in reality
                    height = abs(head_projection.Y - center_projection.Y);

                    if (aimbot_settings.triggerbot_enabled) {
                        float width = esp::esp_settings.width_to_height_ratio * height;
                        if (abs(game_data::screen_center.X - projection.X) < width && abs(game_data::screen_center.Y - projection.Y) < height) {
                            if (game_data::my_player.weapon_ == game_data::Weapon::disk || game_data::my_player.weapon_ == game_data::Weapon::gl || game_data::my_player.weapon_ == game_data::Weapon::plasma || game_data::my_player.weapon_ == game_data::Weapon::blaster) {
                                other::SendLeftMouseClick();
                            }
                        }
                    }
                }

                projections_of_predictions.push_back(projection);
                aimbot_information.push_back({(prediction - game_data::my_player.location_).Magnitude(), projection, height});
            }

            /*
            aimbot::aimbot_settings.use_acceleration = true;

            result = PredictAimAtTarget(&*p, &prediction, muzzle_offset);

            if (result) {
                FVector2D projection = game_functions::Project(prediction);
                float height = -1;

                if ((imgui::visuals::aimbot_visual_settings.marker_style == imgui::visuals::MarkerStyle::kBounds || imgui::visuals::aimbot_visual_settings.marker_style == imgui::visuals::MarkerStyle::kFilledBounds) && height == -1) {
                    FVector2D center_projection = game_functions::Project(player->location_);
                    player->location_.Z += esp::esp_settings.player_height;  // this is HALF the height in reality
                    FVector2D head_projection = game_functions::Project(player->location_);
                    player->location_.Z -= esp::esp_settings.player_height;  // this is HALF the height in reality
                    height = abs(head_projection.Y - center_projection.Y);
                }

                projections_of_predictions.push_back(projection);
                aimbot_information.push_back({(prediction - game_data::my_player.location_).Magnitude(), projection, height});
            }

            */
        }
    }

    players_previous = game_data::game_data.players;
}

}  // namespace aimbot

namespace radar {
/* static */ struct RadarSettings {
    bool enabled = true;
    int radar_poll_frequency = 60 * 5;
    bool show_friendlies = false;
    bool show_flags = true;
} radar_settings;

/* static */ Timer get_radar_data_timer(radar_settings.radar_poll_frequency);

struct RadarLocation {  // polar coordinates
    float r = 0;
    float theta = 0;
    bool right = 0;

    /*
    void Clear(void) {
        r = 0;
        theta = 0;
        right = 0;
    }
    */
};

struct RadarInformation : RadarLocation {
    bool is_friendly = false;
};

/* static */ vector<RadarInformation> player_locations;
/* static */ vector<RadarInformation> flag_locations;

void Tick(void) {
    if (!radar_settings.enabled || !get_radar_data_timer.IsReady())
        return;

    player_locations.clear();
    flag_locations.clear();

    if (!game_data::my_player.is_valid_)
        ;  // return;

    for (vector<game_data::information::Player>::iterator player = game_data::game_data.players.begin(); player != game_data::game_data.players.end(); player++) {
        if (!player->is_valid_) {
            continue;
        }

        game_data::information::Player* p = (game_data::information::Player*)&*player;
        bool same_team = game_data::my_player.IsSameTeam(p);
        if (same_team && !radar_settings.show_friendlies) {
            continue;
        }

        FVector difference_vector = player->location_ - game_data::my_player.location_;
        float angle = math::AngleBetweenVector(game_data::my_player.forward_vector_, difference_vector);
        float magnitude = difference_vector.Magnitude();
        bool right = math::IsVectorToRight(game_data::my_player.forward_vector_, difference_vector);
        RadarInformation radar_information = {magnitude, angle, right, same_team};

        float delta = magnitude * imgui::visuals::radar_visual_settings.zoom_ * imgui::visuals::radar_visual_settings.zoom;
        if (delta > imgui::visuals::radar_visual_settings.window_size / 2 - imgui::visuals::radar_visual_settings.border) {
            continue;
        }

        player_locations.push_back(radar_information);
    }

    if (radar_settings.show_flags) {
        for (vector<game_data::information::Flag>::iterator flag = game_data::game_data.flags.begin(); flag != game_data::game_data.flags.end(); flag++) {
            if (!flag->is_valid_)
                continue;

            FVector difference_vector = flag->location_ - game_data::my_player.location_;
            float angle = math::AngleBetweenVector(game_data::my_player.forward_vector_, difference_vector);
            float magnitude = difference_vector.Magnitude();
            bool right = math::IsVectorToRight(game_data::my_player.forward_vector_, difference_vector);

            game_data::information::Flag* f = (game_data::information::Flag*)&*flag;
            bool same_team = game_data::my_player.IsSameTeam(f);

            RadarInformation radar_information = {magnitude, angle, right, same_team};

            float delta = magnitude * imgui::visuals::radar_visual_settings.zoom_ * imgui::visuals::radar_visual_settings.zoom;
            if (delta > imgui::visuals::radar_visual_settings.window_size / 2 - imgui::visuals::radar_visual_settings.border) {
                continue;
            }

            flag_locations.push_back(radar_information);
        }
    }
}

}  // namespace radar

namespace aimtracker {

/* static */ struct AimTrackerSettings {
    bool enabled = false;
    ImVec2 window_location = {-100, 100};
    ImVec2 window_size = {400, 400};
} aimtracker_settings;

class AimTrackerTick {
   private:
    float f_pitch, f_yaw, f_roll;
    std::chrono::steady_clock::time_point t_tick_time;

   public:
    AimTrackerTick(float pitch, float yaw, float roll) {
        this->f_pitch = pitch;
        this->f_yaw = FixYaw(yaw);
        this->f_roll = roll;
        this->t_tick_time = std::chrono::steady_clock::now();
    }

    static float FixYaw(float yaw) {
        return yaw;
    }

    // Getters
    float GetPitch(void) {
        return f_pitch;
    }

    float GetYaw(void) {
        return f_yaw;
    }

    float GetRoll(void) {
        return f_roll;
    }

    std::chrono::steady_clock::time_point GetTickTime(void) {
        return t_tick_time;
    }

    // Setters
    void SetPitch(float pitch) {
        this->f_pitch = pitch;
    }

    void SetYaw(float yaw) {
        this->f_yaw = yaw;
    }

    void SetRoll(float roll) {
        this->f_roll = roll;
    }

    void SetTickTime(std::chrono::steady_clock::time_point time) {
        this->t_tick_time = time;
    }
};

static double GetTimeDifference(const std::chrono::steady_clock::time_point& end_time, const std::chrono::steady_clock::time_point& start_time) {
    return std::chrono::duration<double>(end_time - start_time).count();
}

class AimTrackerTickManager {
   private:
    std::vector<AimTrackerTick> v_aim_ticks;
    int i_aim_tick_index;
    int i_aim_tick_max_count;
    int i_aim_tick_count;
    float f_lifetime;
    float f_zoom_yaw;
    float f_zoom_pitch;
    int i_ticks_per_second;
    float f_delay_per_tick;
    std::chrono::steady_clock::time_point t_last_tick_time;

   public:
    // static float f_lifetime;

    AimTrackerTickManager(void) {
        SetMaxCount(150 * 3);
        SetLifeTime(3.0);
        SetZoomPitch(2.0);
        SetZoomYaw(2.0);
        SetTicksPerSecond(150);
        t_last_tick_time = std::chrono::steady_clock::now();
    }

    void AddTick(AimTrackerTick tick) {
        /*
        AimTrackerTick* existing_tick = v_aim_ticks[i_aim_tick_index];
        if (existing_tick){
                delete existing_tick;
        }
        */

        // v_aim_ticks[i_aim_tick_index] = tick;

        if (i_aim_tick_count < i_aim_tick_max_count) {
            v_aim_ticks.push_back(AimTrackerTick(0, 0, 0));
        }

        AimTrackerTick& existing_tick = v_aim_ticks[i_aim_tick_index];
        existing_tick.SetPitch(tick.GetPitch());
        existing_tick.SetYaw(tick.GetYaw());
        existing_tick.SetRoll(tick.GetRoll());
        existing_tick.SetTickTime(std::chrono::steady_clock::now());

        existing_tick = AimTrackerTick(tick.GetPitch(), tick.GetYaw(), tick.GetRoll());

        i_aim_tick_index++;
        i_aim_tick_index = i_aim_tick_index % i_aim_tick_max_count;

        i_aim_tick_count++;
        if (i_aim_tick_count >= i_aim_tick_max_count) {
            i_aim_tick_count = i_aim_tick_max_count;
        }

        t_last_tick_time = std::chrono::steady_clock::now();
    }

    // Setters

    void SetTicksPerSecond(int ticks_per_second) {
        this->i_ticks_per_second = ticks_per_second;
        this->f_delay_per_tick = 1.0 / ticks_per_second;
    }

    void SetMaxCount(int max_count) {
        i_aim_tick_index = 0;
        i_aim_tick_max_count = max_count;
        v_aim_ticks.clear();
        v_aim_ticks.reserve(i_aim_tick_max_count);
        /*
        for (int i=0;i<i_aim_tick_max_count;i++){
                v_aim_ticks[i] = NULL;
        }
        */
        // v_aim_ticks.push_back(AimTrackerTick(0, 0, 0));
        i_aim_tick_count = 0;
    }

    void SetLifeTime(float lifetime) {
        this->f_lifetime = lifetime;
    }

    void SetZoomYaw(float zoom) {
        this->f_zoom_yaw = zoom;
    }

    void SetZoomPitch(float zoom) {
        this->f_zoom_pitch = zoom;
    }

    // Getters
    std::chrono::steady_clock::time_point GetLastTickTime(void) {
        return t_last_tick_time;
    }

    int GetTicksPerSecond(void) {
        return i_ticks_per_second;
    }

    float GetDelayPerTick(void) {
        return f_delay_per_tick;
    }

    float GetZoomPitch(void) {
        return f_zoom_pitch;
    }

    float GetZoomYaw(void) {
        return f_zoom_yaw;
    }

    float GetLifeTime(void) {
        return f_lifetime;
    }

    int GetCurrentIndex(void) {
        return i_aim_tick_index;
    }

    int GetCount(void) {
        return i_aim_tick_count;
    }

    int GetMaxCount(void) {
        return i_aim_tick_max_count;
    }

    std::vector<AimTrackerTick>& GetTicks(void) {
        return v_aim_ticks;
    }

    AimTrackerTick& GetTick(int index) {
        int i_index = i_aim_tick_index - 1 - index;
        if (i_index >= 0) {
            return v_aim_ticks[i_index];
        } else {
            return v_aim_ticks[i_aim_tick_max_count + i_index];
        }

        // return v_aim_ticks[(i_aim_tick_index + index)%i_aim_tick_max_count];
    }
} aim_tracker_tick_manager;

void Tick(void) {
    if (aimtracker_settings.enabled && GetTimeDifference(std::chrono::steady_clock::now(), aim_tracker_tick_manager.GetLastTickTime()) > aim_tracker_tick_manager.GetDelayPerTick()) {
        if (game_data::game_data.my_player_information.is_valid_) {
            FRotator r = game_data::game_data.my_player_information.rotation_;

            // Math::printRotator(r, "Rotation");
            if (r.Pitch > 270) {  // Looking down
                r.Pitch = -(360 - r.Pitch);
            }

            if (r.Yaw > 360 / 2) {
                r.Yaw = (360 - r.Yaw);
            }
            aim_tracker_tick_manager.AddTick(AimTrackerTick(r.Pitch, r.Yaw - 90, 0));
        }
    }
}
}  // namespace aimtracker

namespace imgui {
namespace imgui_menu {
enum LeftMenuButtons { kAimAssist, kESP, kRadar, kOther, kInformation };
/* static */ const char* button_text[] = {"Aim assist", "ESP", "Radar", "Other", "Information"};
/* static */ const int buttons_num = sizeof(button_text) / sizeof(char*);
/* static */ int selected_index = LeftMenuButtons::kInformation;  // LeftMenuButtons::kAimAssist;
/* static */ float item_width = -150;

void DrawInformationMenuNew(void) {
    static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | (ImGuiTableFlags_ContextMenuInBody & 0) | (ImGuiTableFlags_NoBordersInBody & 0) | ImGuiTableFlags_BordersOuter;
    if (ImGui::BeginTable("descensiontable", 1, flags, ImVec2(0, ImGui::GetContentRegionAvail().y))) {
        ImGui::TableSetupColumn("descension", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::PushItemWidth(item_width);
        ImGui::Indent();
        const char* info0 =
            "descension v1.5 (Public)\n"
            "Released: 14/07/2022\n";
        //"Game version: -";

        const char* info1 =
            "https://github.com/MuhanjalaRE\n"
            "https://twitter.com/Muhanjala\n"
            "https://dll.rip";
        ImGui::Text(info0);
        ImGui::Separator();
        ImGui::Text(info1);

        ImGui::Unindent();
        ImGui::EndTable();
    }
}

void DrawAimAssistMenuNew(void) {
    static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | (ImGuiTableFlags_ContextMenuInBody & 0) | (ImGuiTableFlags_NoBordersInBody & 0) | ImGuiTableFlags_BordersOuter;
    if (ImGui::BeginTable("aimassisttable", 1, flags, ImVec2(0, ImGui::GetContentRegionAvail().y))) {
        ImGui::TableSetupColumn("Aim assist", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::PushItemWidth(item_width);
        if (ImGui::CollapsingHeader("General settings")) {
            ImGui::Indent();
            ImGui::Checkbox("Enable aim assist", &aimbot::aimbot_settings.enabled);
            ImGui::Combo("Mode##aim_assist_mode_combo", (int*)&aimbot::aimbot_settings.aimbot_mode, aimbot::mode_labels, IM_ARRAYSIZE(aimbot::mode_labels));

            if (aimbot::aimbot_settings.aimbot_mode == aimbot::AimbotMode::kClosestXhair) {
                ImGui::SliderFloat("Horizontal FOV", &aimbot::aimbot_settings.aimbot_horizontal_fov_angle, 1, 89);
                aimbot::aimbot_settings.aimbot_horizontal_fov_angle_cos = cos(aimbot::aimbot_settings.aimbot_horizontal_fov_angle * PI / 180.0);
                aimbot::aimbot_settings.aimbot_horizontal_fov_angle_cos_sqr = pow(aimbot::aimbot_settings.aimbot_horizontal_fov_angle_cos, 2);
            }

            if (ImGui::SliderInt("Poll rate (Hz)", &aimbot::aimbot_settings.aimbot_poll_frequency, 1, 300)) {
                aimbot::aimbot_poll_timer.SetFrequency(aimbot::aimbot_settings.aimbot_poll_frequency);
            }

            // ImGui::Checkbox("Factor target acceleration (Chaingun only)", &aimbot::aimbot_settings.use_acceleration);
            ImGui::Checkbox("Factor target acceleration", &aimbot::aimbot_settings.use_acceleration);
            if (aimbot::aimbot_settings.use_acceleration) {
                ImGui::Indent();
                ImGui::Checkbox("Factor for Chaingun only", &aimbot::aimbot_settings.use_acceleration_cg_only);
                ImGui::Unindent();
            }

            // ImGui::Text("Factor target accel")
            if (aimbot::aimbot_settings.use_acceleration) {
                // ImGui::SliderFloat("Acceleration delta (ms)", &aimbot::aimbot_settings.acceleration_delta_in_ms, 1, 1000);
            }

            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Target settings")) {
            ImGui::Indent();
            ImGui::Checkbox("Friendly fire", &aimbot::aimbot_settings.friendly_fire);
            // ImGui::Checkbox("Need line of sight", &aimbot::aimbot_settings.need_line_of_sight);
            ImGui::Checkbox("Target everyone", &aimbot::aimbot_settings.target_everyone);
            if (!aimbot::aimbot_settings.target_everyone) {
                ImGui::Checkbox("Stay locked on to target", &aimbot::aimbot_settings.stay_locked_to_target);
                ImGui::Checkbox("Auto lock to new target", &aimbot::aimbot_settings.auto_lock_to_new_target);
            }
            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Weapon settings")) {
            ImGui::Indent();
            if (ImGui::CollapsingHeader("Pings")) {
                ImGui::Indent();
                ImGui::SliderFloat("Tempest ping", &aimbot::aimbot_settings.tempest_ping_in_ms, -300, 300);
                ImGui::SliderFloat("Chaingun ping", &aimbot::aimbot_settings.chaingun_ping_in_ms, -300, 300);
                ImGui::SliderFloat("Grenade Launcher ping", &aimbot::aimbot_settings.grenadelauncher_ping_in_ms, -300, 300);
                ImGui::SliderFloat("Plasma Gun ping", &aimbot::aimbot_settings.plasmagun_ping_in_ms, -300, 300);
                ImGui::SliderFloat("Blaster ping", &aimbot::aimbot_settings.blaster_ping_in_ms, -300, 300);
                ImGui::Unindent();
            }

            if (ImGui::CollapsingHeader("Bullet speeds")) {
                ImGui::Indent();
                ImGui::SliderFloat("Tempest speed", &game_data::information::weapon_speeds.disk.bullet_speed, 0, 1E5);
                ImGui::SliderFloat("Chaingun speed", &game_data::information::weapon_speeds.chaingun.bullet_speed, 0, 1E5);
                ImGui::SliderFloat("Grenadelauncher speed", &game_data::information::weapon_speeds.grenadelauncher.bullet_speed, 0, 1E5);
                ImGui::SliderFloat("Plasma speed", &game_data::information::weapon_speeds.plasma.bullet_speed, 0, 1E5);
                ImGui::SliderFloat("Blaster speed", &game_data::information::weapon_speeds.blaster.bullet_speed, 0, 1E5);
                ImGui::Unindent();
            }

            if (ImGui::CollapsingHeader("Inheritence")) {
                ImGui::Indent();
                ImGui::SliderFloat("Tempest inheritence", &game_data::information::weapon_speeds.disk.inheritence, 0, 1);
                ImGui::SliderFloat("Chaingun inheritence", &game_data::information::weapon_speeds.chaingun.inheritence, 0, 1);
                ImGui::SliderFloat("Grenadelauncher inheritence", &game_data::information::weapon_speeds.grenadelauncher.inheritence, 0, 1);
                ImGui::SliderFloat("Plasma inheritence", &game_data::information::weapon_speeds.plasma.inheritence, 0, 1);
                ImGui::SliderFloat("Blaster inheritence", &game_data::information::weapon_speeds.blaster.inheritence, 0, 1);
                ImGui::Unindent();
            }

            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Markers")) {
            float marker_preview_size = 100;
            ImGui::Combo("Style##aim_assist_combo", (int*)&visuals::aimbot_visual_settings.marker_style, visuals::marker_labels, IM_ARRAYSIZE(visuals::marker_labels));
            ImGui::ColorEdit4("Colour", &visuals::aimbot_visual_settings.marker_colour.Value.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_None | ImGuiColorEditFlags_AlphaBar);

            if (visuals::aimbot_visual_settings.marker_style == imgui::visuals::MarkerStyle::kBounds || visuals::aimbot_visual_settings.marker_style == imgui::visuals::MarkerStyle::kFilledBounds) {
                if (visuals::aimbot_visual_settings.marker_style == imgui::visuals::MarkerStyle::kBounds) {
                    ImGui::SliderInt("Thickness", &visuals::aimbot_visual_settings.marker_thickness, 1, 10);
                }
            } else {
                ImGui::SliderInt("Radius", &visuals::aimbot_visual_settings.marker_size, 1, 10);

                if (visuals::aimbot_visual_settings.marker_style == visuals::MarkerStyle::kCircle || visuals::aimbot_visual_settings.marker_style == visuals::MarkerStyle::kSquare) {
                    ImGui::SliderInt("Thickness", &visuals::aimbot_visual_settings.marker_thickness, 1, 10);
                }

                ImGui::Text("Marker preview");

                ImVec2 window_position = ImGui::GetWindowPos();
                ImVec2 window_size = ImGui::GetWindowSize();

                ImDrawList* imgui_draw_list = ImGui::GetWindowDrawList();
                ImVec2 current_cursor_pos = ImGui::GetCursorPos();
                ImVec2 local_cursor_pos = {window_position.x + ImGui::GetCursorPosX(), window_position.y + ImGui::GetCursorPosY() - ImGui::GetScrollY()};
                imgui_draw_list->AddRectFilled(local_cursor_pos, {local_cursor_pos.x + marker_preview_size, local_cursor_pos.y + marker_preview_size}, ImColor(0, 0, 0, 255), 0, 0);
                ImVec2 center = {local_cursor_pos.x + marker_preview_size / 2, local_cursor_pos.y + marker_preview_size / 2};

                float box_size_height = 40;
                float box_size_width = box_size_height * esp::esp_settings.width_to_height_ratio;

                imgui::visuals::DrawMarker((imgui::visuals::MarkerStyle)visuals::aimbot_visual_settings.marker_style, center, visuals::aimbot_visual_settings.marker_colour, visuals::aimbot_visual_settings.marker_size, visuals::aimbot_visual_settings.marker_thickness);

                current_cursor_pos.y += marker_preview_size;
                ImGui::SetCursorPos(current_cursor_pos);

                ImGui::Spacing();
                ImGui::Checkbox("Scale by distance", &visuals::aimbot_visual_settings.scale_by_distance);
                if (visuals::aimbot_visual_settings.scale_by_distance) {
                    ImGui::SliderInt("Distance for scaling", &visuals::aimbot_visual_settings.distance_for_scaling, 1, 15000);
                    ImGui::SliderInt("Minimum marker size", &visuals::aimbot_visual_settings.minimum_marker_size, 1, 10);
                }
            }
        }

        if (ImGui::CollapsingHeader("Triggerbot settings")) {
            ImGui::Indent();
            ImGui::Checkbox("Enable triggerbot", &aimbot::aimbot_settings.triggerbot_enabled);
            ImGui::Text("Triggerbot does not work for the Chaingun as it is a hold to fire weapon.");
            ImGui::Unindent();
        }

        ImGui::EndTable();
    }
}

void DrawRadarMenuNew(void) {
    static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | (ImGuiTableFlags_ContextMenuInBody & 0) | (ImGuiTableFlags_NoBordersInBody & 0) | ImGuiTableFlags_BordersOuter;
    if (ImGui::BeginTable("radartable", 1, flags, ImVec2(0, ImGui::GetContentRegionAvail().y))) {
        ImGui::TableSetupColumn("Radar", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::PushItemWidth(item_width);
        if (ImGui::CollapsingHeader("General settings")) {
            ImGui::Indent();
            ImGui::Checkbox("Enabled##radar_enabled", &radar::radar_settings.enabled);
            if (ImGui::SliderInt("Poll rate (Hz)##radar", &radar::radar_settings.radar_poll_frequency, 1, 300)) {
                radar::get_radar_data_timer.SetFrequency(radar::radar_settings.radar_poll_frequency);
            }

            ImGui::SliderFloat("Zoom", &visuals::radar_visual_settings.zoom, 0, 10);
            ImGui::Checkbox("Show friendlies", &radar::radar_settings.show_friendlies);
            ImGui::Checkbox("Show flags", &radar::radar_settings.show_flags);
            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Visual settings")) {
            ImGui::Indent();
            ImGui::ColorEdit4("Backgrond Colour", &visuals::radar_visual_settings.window_background_colour.Value.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_None | ImGuiColorEditFlags_AlphaBar);
            ImGui::Checkbox("Draw axes", &visuals::radar_visual_settings.draw_axes);
            if (visuals::radar_visual_settings.draw_axes)
                ImGui::SliderInt("Axes thickness", &visuals::radar_visual_settings.axes_thickness, 1, 4);
            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Markers")) {
            ImGui::Indent();
            float marker_preview_size = 100;

            ImGui::Combo("Style##radar_combo", (int*)&visuals::radar_visual_settings.marker_style, visuals::marker_labels, IM_ARRAYSIZE(visuals::marker_labels) - 2);
            ImGui::SliderInt("Radius", &visuals::radar_visual_settings.marker_size, 1, 50);

            if (visuals::radar_visual_settings.marker_style == visuals::MarkerStyle::kCircle || visuals::radar_visual_settings.marker_style == visuals::MarkerStyle::kSquare) {
                ImGui::SliderInt("Thickness", &visuals::radar_visual_settings.marker_thickness, 1, 10);
            }

            ImGui::Text("Marker preview");

            ImVec2 window_position = ImGui::GetWindowPos();
            ImVec2 window_size = ImGui::GetWindowSize();

            ImDrawList* imgui_draw_list = ImGui::GetWindowDrawList();
            ImVec2 current_cursor_pos = ImGui::GetCursorPos();
            ImVec2 local_cursor_pos = {window_position.x + ImGui::GetCursorPosX(), window_position.y + ImGui::GetCursorPosY() - ImGui::GetScrollY()};
            imgui_draw_list->AddRectFilled(local_cursor_pos, {local_cursor_pos.x + marker_preview_size, local_cursor_pos.y + marker_preview_size}, ImColor(0, 0, 0, 255), 0, 0);
            ImVec2 center = {local_cursor_pos.x + marker_preview_size / 2, local_cursor_pos.y + marker_preview_size / 2};

            imgui::visuals::DrawMarker((imgui::visuals::MarkerStyle)visuals::radar_visual_settings.marker_style, center, visuals::radar_visual_settings.enemy_marker_colour, visuals::radar_visual_settings.marker_size, visuals::radar_visual_settings.marker_thickness);

            current_cursor_pos.y += marker_preview_size;
            ImGui::SetCursorPos(current_cursor_pos);
            ImGui::Spacing();

            ImGui::ColorEdit4("Friendly player Colour", &visuals::radar_visual_settings.friendly_marker_colour.Value.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_None | ImGuiColorEditFlags_AlphaBar);
            ImGui::ColorEdit4("Enemy player Colour", &visuals::radar_visual_settings.enemy_marker_colour.Value.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_None | ImGuiColorEditFlags_AlphaBar);
            ImGui::ColorEdit4("Friendly flag Colour", &visuals::radar_visual_settings.friendly_flag_marker_colour.Value.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_None | ImGuiColorEditFlags_AlphaBar);
            ImGui::ColorEdit4("Enemy flag Colour", &visuals::radar_visual_settings.enemy_flag_marker_colour.Value.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_None | ImGuiColorEditFlags_AlphaBar);
            ImGui::Unindent();
        }

        ImGui::EndTable();
    }
}

void DrawESPMenuNew(void) {
    static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | (ImGuiTableFlags_ContextMenuInBody & 0) | (ImGuiTableFlags_NoBordersInBody & 0) | ImGuiTableFlags_BordersOuter;
    if (ImGui::BeginTable("esptable", 1, flags, ImVec2(0, ImGui::GetContentRegionAvail().y))) {
        ImGui::TableSetupColumn("ESP", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::PushItemWidth(item_width);
        if (ImGui::CollapsingHeader("General settings")) {
            ImGui::Indent();
            ImGui::Checkbox("Enabled", &esp::esp_settings.enabled);
            if (ImGui::SliderInt("Poll rate (Hz)", &esp::esp_settings.poll_frequency, 1, 300)) {
                esp::get_esp_data_timer.SetFrequency(esp::esp_settings.poll_frequency);
            }
            ImGui::Checkbox("Show friendlies", &esp::esp_settings.show_friendlies);
            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("BoundingBox settings")) {
            ImGui::Indent();
            ImGui::SliderInt("Box thickness", &visuals::esp_visual_settings.bounding_box_settings.box_thickness, 1, 20);
            ImGui::ColorEdit4("Friendly Colour##box", &visuals::esp_visual_settings.bounding_box_settings.friendly_player_box_colour.Value.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_None | ImGuiColorEditFlags_AlphaBar);
            ImGui::ColorEdit4("Enemy Colour##box", &visuals::esp_visual_settings.bounding_box_settings.enemy_player_box_colour.Value.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_None | ImGuiColorEditFlags_AlphaBar);
            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Snapline settings")) {
            ImGui::Indent();
            ImGui::Checkbox("Show snap lines", &esp::esp_settings.show_lines);
            ImGui::ColorEdit4("Enemy Colour##line", &visuals::esp_visual_settings.line_settings.enemy_player_line_colour.Value.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_None | ImGuiColorEditFlags_AlphaBar);
            ImGui::SliderInt("Line thickness", &visuals::esp_visual_settings.line_settings.line_thickness, 1, 20);
            ImGui::Unindent();
        }

        ImGui::EndTable();
    }
}

void DrawOtherMenuNew(void) {
    static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | (ImGuiTableFlags_ContextMenuInBody & 0) | (ImGuiTableFlags_NoBordersInBody & 0) | ImGuiTableFlags_BordersOuter;
    if (ImGui::BeginTable("othertable", 1, flags, ImVec2(0, ImGui::GetContentRegionAvail().y))) {
        ImGui::TableSetupColumn("Other", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::PushItemWidth(item_width);
        if (ImGui::CollapsingHeader("Crosshair settings")) {
            ImGui::Indent();

            if (ImGui::CollapsingHeader("General settings##crosshair")) {
                float marker_preview_size = 100;
                ImGui::Indent();
                ImGui::Checkbox("Enabled##crosshair_enabled", &visuals::crosshair_settings.enabled);
                ImGui::Combo("Style##crosshair_combo", (int*)&visuals::crosshair_settings.marker_style, visuals::marker_labels, IM_ARRAYSIZE(visuals::marker_labels) - 2);
                ImGui::ColorEdit4("Colour", &visuals::crosshair_settings.marker_colour.Value.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_None | ImGuiColorEditFlags_AlphaBar);
                ImGui::SliderInt("Radius", &visuals::crosshair_settings.marker_size, 1, 10);

                if (visuals::crosshair_settings.marker_style == visuals::MarkerStyle::kCircle || visuals::crosshair_settings.marker_style == visuals::MarkerStyle::kSquare) {
                    ImGui::SliderInt("Thickness", &visuals::crosshair_settings.marker_thickness, 1, 10);
                }

                ImGui::Text("Crosshair preview");

                ImVec2 window_position = ImGui::GetWindowPos();
                ImVec2 window_size = ImGui::GetWindowSize();

                ImDrawList* imgui_draw_list = ImGui::GetWindowDrawList();
                ImVec2 current_cursor_pos = ImGui::GetCursorPos();
                ImVec2 local_cursor_pos = {window_position.x + ImGui::GetCursorPosX(), window_position.y + ImGui::GetCursorPosY()};
                imgui_draw_list->AddRectFilled(local_cursor_pos, {local_cursor_pos.x + 100, local_cursor_pos.y + 100}, ImColor(0, 0, 0, 255), 0, 0);
                ImVec2 center = {local_cursor_pos.x + 100 / 2, local_cursor_pos.y + 100 / 2};

                imgui::visuals::DrawMarker((imgui::visuals::MarkerStyle)visuals::crosshair_settings.marker_style, center, visuals::crosshair_settings.marker_colour, visuals::crosshair_settings.marker_size, visuals::crosshair_settings.marker_thickness);

                current_cursor_pos.y += marker_preview_size;
                ImGui::SetCursorPos(current_cursor_pos);
                ImGui::Spacing();

                ImGui::Unindent();
            }

            ImGui::Unindent();
        }

        /*
        if (ImGui::CollapsingHeader("Other settings")) {
            ImGui::Indent();

            //ImGui::Checkbox("Disable hitmarkers", &other::other_settings.disable_hitmarker);

            ImGui::Unindent();
        }*/

        /*
        if (ImGui::CollapsingHeader("Other options")) {
            ImGui::Indent();
            //
            ImGui::Unindent();
        }
        */

        if (ImGui::CollapsingHeader("Aim tracker")) {
            ImGui::Indent();

            ImGui::Checkbox("Enabled", &aimtracker::aimtracker_settings.enabled);

            static float f_lifetime = aimtracker::aim_tracker_tick_manager.GetLifeTime();
            static int i_maxcount = aimtracker::aim_tracker_tick_manager.GetMaxCount();
            static int i_ticks_per_second = aimtracker::aim_tracker_tick_manager.GetTicksPerSecond();

            static float f_zoom_pitch = aimtracker::aim_tracker_tick_manager.GetZoomPitch();
            static float f_zoom_yaw = aimtracker::aim_tracker_tick_manager.GetZoomYaw();

            static float f_tool_window_size = aimtracker::aimtracker_settings.window_size.x;

            // ImGui::PushItemWidth(100);

            if (ImGui::SliderInt("Max items", &i_maxcount, 1, 10000)) {
                aimtracker::aim_tracker_tick_manager.SetMaxCount(i_maxcount);
            }

            if (ImGui::SliderFloat("Lifetime", &f_lifetime, 0, 60)) {
                aimtracker::aim_tracker_tick_manager.SetLifeTime(f_lifetime);
            }

            if (ImGui::SliderInt("Ticks per second", &i_ticks_per_second, 1, 300)) {
                aimtracker::aim_tracker_tick_manager.SetTicksPerSecond(i_ticks_per_second);
            }

            if (ImGui::SliderFloat("Pitch zoom", &f_zoom_pitch, 1, 5)) {
                aimtracker::aim_tracker_tick_manager.SetZoomPitch(f_zoom_pitch);
            }

            if (ImGui::SliderFloat("Yaw zoom", &f_zoom_yaw, 1, 5)) {
                aimtracker::aim_tracker_tick_manager.SetZoomYaw(f_zoom_yaw);
            }
            /*
            if (ImGui::SliderFloat("Window size", &f_tool_window_size, 200, 1200)) {
                aimtracker::aimtracker_settings.window_size = {f_tool_window_size, f_tool_window_size};
            }
            */

            // ImGui::Unindent();
        }

        ImGui::EndTable();
    }
}

void DrawAimTrackerMenu(void) {
    ImGui::BeginGroup();

    ImGui::Checkbox("Enabled", &aimtracker::aimtracker_settings.enabled);

    static float f_lifetime = aimtracker::aim_tracker_tick_manager.GetLifeTime();
    static int i_maxcount = aimtracker::aim_tracker_tick_manager.GetMaxCount();
    static int i_ticks_per_second = aimtracker::aim_tracker_tick_manager.GetTicksPerSecond();

    static float f_zoom_pitch = aimtracker::aim_tracker_tick_manager.GetZoomPitch();
    static float f_zoom_yaw = aimtracker::aim_tracker_tick_manager.GetZoomYaw();

    static float f_tool_window_size = aimtracker::aimtracker_settings.window_size.x;

    ImGui::PushItemWidth(100);

    if (ImGui::SliderInt("Max items", &i_maxcount, 1, 10000)) {
        aimtracker::aim_tracker_tick_manager.SetMaxCount(i_maxcount);
    }

    if (ImGui::SliderFloat("Lifetime", &f_lifetime, 0, 60)) {
        aimtracker::aim_tracker_tick_manager.SetLifeTime(f_lifetime);
    }

    if (ImGui::SliderInt("Ticks per second", &i_ticks_per_second, 1, 300)) {
        aimtracker::aim_tracker_tick_manager.SetTicksPerSecond(i_ticks_per_second);
    }

    if (ImGui::SliderFloat("Pitch zoom", &f_zoom_pitch, 1, 5)) {
        aimtracker::aim_tracker_tick_manager.SetZoomPitch(f_zoom_pitch);
    }

    if (ImGui::SliderFloat("Yaw zoom", &f_zoom_yaw, 1, 5)) {
        aimtracker::aim_tracker_tick_manager.SetZoomYaw(f_zoom_yaw);
    }

    if (ImGui::SliderFloat("Window size", &f_tool_window_size, 200, 1200)) {
        aimtracker::aimtracker_settings.window_size = {f_tool_window_size, f_tool_window_size};
    }

    ImGui::PopItemWidth();

    ImGui::EndGroup();
}

}  // namespace imgui_menu

}  // namespace imgui

namespace ue {

PROCESSEVENT_HOOK_FUNCTION(UEHookMain) {
    DWORD dwWaitResult = WaitForSingleObject(dx11::game_dx_mutex, INFINITE);
    if (ue::frame_is_ready) {
        static bool got_resolution = false;

        ue::frame_is_ready = false;

        static int prev_total_aactor_count = 0;

        UWorld* world = (UWorld*)(*(DWORD64*)(base_address + uworld_offset));

        if (world) {
            if (world_proxy.world != world || world_proxy.world == NULL) {
                world_proxy.world = world;
                aimbot::Reset();  // Clear target_player because its character_ value will be invalid, cant use the object because when it checks to see if its valid it will just crash since garbage value in the pointer
                prev_total_aactor_count = 0;
            }
            game_data::local_player = ((ULocalPlayer*)world_proxy.world->OwningGameInstance->LocalPlayers[0]);
            game_data::local_player_controller = (AMAPlayerController*)game_data::local_player->PlayerController;
        }

        if (!world || !game_data::local_player || !game_data::local_player_controller) {
            ReleaseMutex(dx11::game_dx_mutex);
            return;
        }

        int total_aactor_count = world_proxy.world->PersistentLevel->all_aactors.Num();
        if (total_aactor_count < prev_total_aactor_count) {
            aimbot::Reset();  // Out of warm up
        }

        prev_total_aactor_count = total_aactor_count;

        game_data::GetGameData();

        aimbot::Tick();
        radar::Tick();
        esp::Tick();
        other::Tick();
        aimtracker::Tick();

        // Just get screen resolution every frame, who cares
        if (!got_resolution && game_data::local_player_controller && game_data::my_player.is_valid_) {
            int x, y;
            ImVec2 display_size = ImGui::GetIO().DisplaySize;
            // cout << display_size.x << ", " << display_size.y << endl;
            x = display_size.x;
            y = display_size.y;

            // game_data::local_player_controller->GetViewportSize(&x, &y);
            game_data::screen_size = {x * 1.0f, y * 1.0f};
            game_data::screen_center = {x * 0.5f, y * 0.5f};
            // got_resolution = true;
        }
    }
    ReleaseMutex(dx11::game_dx_mutex);
}

void UE4_(void) {
    UEHookMain(NULL, NULL, NULL);
}

void HookUnrealEngine4(void) {
    base_address = (DWORD64)GetModuleHandle(0);

    // UObject::GObjects = reinterpret_cast<CG::TUObjectArray*>(base_address + 0x5283F20);
    // FName::GNames = reinterpret_cast<CG::GNAME_TYPE*>(base_address + 0x5266340);

    // InitSdk();

    DWORD uworld_rip_offset_ = 0;
    LPCSTR uworld_signature = "\x74\x4E\x48\x8B\x1D\x00\x00\x00\x00\x48\x85\xDB\x74\x3B";
    LPCSTR uworld_Mask = "xxxxx????xxxxx";
    DWORD64 uworld_signature_address = 0;
    DWORD64 uworld_address = 0;

    /*
    LPCSTR processevent_signature = "\x40\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x81\xEC\xF0\x00\x00\x00\x48\x8D\x6C\x24\x30";
    LPCSTR processevent_mask = "xxxxxxxxxxxxxxxxxxxxxxxx";
    DWORD64 processevent_address = 0;
    */

    /* static */ SignatureScanner SigScanner;
    if (SigScanner.GetProcess("MidairCE-Win64-Test.exe")) {
        module mod = SigScanner.GetModule("MidairCE-Win64-Test.exe");
        uworld_signature_address = SigScanner.FindSignature(mod.dwBase, mod.dwSize, uworld_signature, uworld_Mask);
        if (!uworld_signature_address) {
            // cout << "Error finding UWorld signature" << endl;
            // Sleep(1000000);
            *((DWORD64*)0x0) = 0x9999;
        }

        /*
        processevent_address = SigScanner.FindSignature(mod.dwBase, mod.dwSize, processevent_signature, processevent_mask);
        if (!processevent_address) {
            //cout << "Error finding ProcessEvent signature" << endl;
            Sleep(1000000);
            *((DWORD64*)0x0) = 0x9999;
        }
        */
    }

    uworld_rip_offset_ = *((DWORD*)(uworld_signature_address + 2 + 3));      // 2 forward to get instruction, then 3 for opcode, then offset
    uworld_address = uworld_signature_address + 2 + uworld_rip_offset_ + 7;  // signature address -> 2 forward to get to the instruction, then 7 extra because RIP calculation is after the instruction which is 7 bytes long

    uworld_offset = uworld_address - base_address;

    world_proxy.world = (CG::UWorld*)(*(DWORD64*)(base_address + uworld_offset));

    game_data::local_player = ((ULocalPlayer*)world_proxy.world->OwningGameInstance->LocalPlayers[0]);
    game_data::local_player_controller = (AMAPlayerController*)game_data::local_player->PlayerController;

    /*
    DWORD64 processevent_address = base_address + processevent_offset;
    hooks::processevent::original_processevent = (hooks::processevent::_ProcessEvent)processevent_address;

    UFunction* ufunction = UObject::FindObject<UFunction>(ue::ufunction_to_hook);
    hooks::processevent::AddProcessEventHook(ufunction, (hooks::processevent::_ProcessEvent)UEHookMain);

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)hooks::processevent::original_processevent, hooks::processevent::ProcessEvent);
    DetourTransactionCommit();
    */

    game_data::screen_size = FVector2D(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
    game_data::screen_center = FVector2D(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2);
}

void DrawImGuiInUE4(void) {
    // No unreal engine hooking, just do everything through present :brolmfao:

    using namespace imgui;

    if (aimbot::aimbot_settings.enabled) {
        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize({game_data::screen_size.X, game_data::screen_size.Y});
        ImGui::Begin("aim_assist", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus);

        int marker_size = visuals::aimbot_visual_settings.marker_size;

        vector<aimbot::AimbotInformation>& aimbot_informations = aimbot::aimbot_information;

        for (vector<aimbot::AimbotInformation>::iterator i = aimbot_informations.begin(); i != aimbot_informations.end(); i++) {
            ImVec2 v(i->projection_.X, i->projection_.Y);

            if (visuals::aimbot_visual_settings.scale_by_distance) {
                marker_size = (visuals::aimbot_visual_settings.marker_size - visuals::aimbot_visual_settings.minimum_marker_size) * exp(-i->distance_ / visuals::aimbot_visual_settings.distance_for_scaling) + visuals::aimbot_visual_settings.minimum_marker_size;
            }

            float box_size_height = i->height;
            float box_size_width = box_size_height * esp::esp_settings.width_to_height_ratio;

            if (visuals::aimbot_visual_settings.marker_style == imgui::visuals::MarkerStyle::kBounds) {
                ImDrawList* imgui_draw_list = ImGui::GetWindowDrawList();
                imgui_draw_list->AddRect({v.x - box_size_width, v.y - box_size_height}, {v.x + box_size_width, v.y + box_size_height}, visuals::aimbot_visual_settings.marker_colour, 0, 0, visuals::aimbot_visual_settings.marker_thickness);
            } else if (visuals::aimbot_visual_settings.marker_style == imgui::visuals::MarkerStyle::kFilledBounds) {
                ImDrawList* imgui_draw_list = ImGui::GetWindowDrawList();
                imgui_draw_list->AddRectFilled({v.x - box_size_width, v.y - box_size_height}, {v.x + box_size_width, v.y + box_size_height}, visuals::aimbot_visual_settings.marker_colour, 0);
            } else {
                imgui::visuals::DrawMarker((imgui::visuals::MarkerStyle)visuals::aimbot_visual_settings.marker_style, v, visuals::aimbot_visual_settings.marker_colour, marker_size, visuals::aimbot_visual_settings.marker_thickness);
            }
        }

        ImGui::End();
    }

    if (esp::esp_settings.enabled) {
        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize({game_data::screen_size.X, game_data::screen_size.Y});
        ImGui::Begin("esp", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImDrawList* imgui_draw_list = ImGui::GetWindowDrawList();

        ImColor friendly_box_colour = visuals::esp_visual_settings.bounding_box_settings.friendly_player_box_colour;
        ImColor enemy_box_colour = visuals::esp_visual_settings.bounding_box_settings.enemy_player_box_colour;
        int box_thickness = visuals::esp_visual_settings.bounding_box_settings.box_thickness;

        /* static */ ImColor colour = enemy_box_colour;
        for (vector<esp::ESPInformation>::iterator esp_information = esp::esp_information.begin(); esp_information != esp::esp_information.end(); esp_information++) {
            if (esp_information->projection.X <= 0 && esp_information->projection.Y <= 0)
                continue;

            colour = (esp_information->is_friendly) ? friendly_box_colour : enemy_box_colour;
            ImVec2 projection(esp_information->projection.X, esp_information->projection.Y);
            float box_size_height = esp_information->height;
            float box_size_width = box_size_height * esp::esp_settings.width_to_height_ratio;

            imgui_draw_list->AddRect({projection.x - box_size_width, projection.y - box_size_height}, {projection.x + box_size_width, projection.y + box_size_height}, colour, 0, 0, box_thickness);

            if (esp::esp_settings.show_lines && !esp_information->is_friendly) {
                imgui_draw_list->AddLine({game_data::screen_size.X / 2, game_data::screen_size.Y}, {projection.x, projection.y + box_size_height}, visuals::esp_visual_settings.line_settings.enemy_player_line_colour, visuals::esp_visual_settings.line_settings.line_thickness);
            }
        }

        ImGui::End();
    }

    if (visuals::crosshair_settings.enabled) {
        /* static */ ImVec2 window_size(30, 30);

        ImVec2 display_size = ImGui::GetIO().DisplaySize;

        ImGui::SetNextWindowPos({display_size.x / 2 - window_size.x / 2, display_size.y / 2 - window_size.y / 2});
        ImGui::SetNextWindowSize(window_size);
        ImGui::Begin("crosshair", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        imgui::visuals::DrawMarker((imgui::visuals::MarkerStyle)visuals::crosshair_settings.marker_style, {display_size.x / 2, display_size.y / 2}, visuals::crosshair_settings.marker_colour, visuals::crosshair_settings.marker_size, visuals::crosshair_settings.marker_thickness);

        ImGui::End();
    }

    if (radar::radar_settings.enabled) {
        ImGui::SetNextWindowPos(visuals::radar_visual_settings.window_location, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize({(float)visuals::radar_visual_settings.window_size, (float)visuals::radar_visual_settings.window_size});

        if (dx11::imgui_show_menu) {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(0.0f, 0.0f, 0.0f, 0.0f).Value);
            ImGui::Begin("Radar##radar", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse /*| ImGuiWindowFlags_NoBringToFrontOnFocus*/);
        } else {
            ImGui::Begin("Radar##radar", NULL, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse /*| ImGuiWindowFlags_NoBringToFrontOnFocus*/);
        }

        ImVec2 window_position = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();
        ImVec2 center(window_position.x + window_size.x / 2, window_position.y + window_size.y / 2);

        visuals::radar_visual_settings.window_size = (window_size.x >= window_size.y) ? window_size.x : window_size.y;
        float border = visuals::radar_visual_settings.border;

        visuals::radar_visual_settings.window_location = window_position;
        int axes_thickness = visuals::radar_visual_settings.axes_thickness;

        ImDrawList* imgui_draw_list = ImGui::GetWindowDrawList();
        imgui_draw_list->AddCircleFilled(center, window_size.x / 2 - border, visuals::radar_visual_settings.window_background_colour, 0);
        imgui_draw_list->AddCircle(center, window_size.x / 2 - border, visuals::radar_visual_settings.window_background_colour, 0, axes_thickness);

        if (visuals::radar_visual_settings.draw_axes) {
            imgui_draw_list->AddLine({window_position.x + border, window_position.y + window_size.y / 2}, {window_position.x + window_size.x - border, window_position.y + window_size.y / 2}, ImColor(65, 65, 65, 255), axes_thickness);
            imgui_draw_list->AddLine({window_position.x + window_size.x / 2, window_position.y + border}, {window_position.x + window_size.x / 2, window_position.y + window_size.y - border}, ImColor(65, 65, 65, 255), axes_thickness);
            imgui_draw_list->AddCircleFilled(center, axes_thickness + 1, ImColor(65, 65, 65, 125));
        }

        ImColor friendly_marker_colour = visuals::radar_visual_settings.friendly_marker_colour;
        ImColor enemy_marker_colour = visuals::radar_visual_settings.enemy_marker_colour;
        /* static */ ImColor player_marker_colour = enemy_marker_colour;
        for (vector<radar::RadarInformation>::iterator radar_information = radar::player_locations.begin(); radar_information != radar::player_locations.end(); radar_information++) {
            float theta = radar_information->theta;
            float y = radar_information->r * cos(theta) * visuals::radar_visual_settings.zoom_ * visuals::radar_visual_settings.zoom;
            float x = radar_information->r * sin(theta) * visuals::radar_visual_settings.zoom_ * visuals::radar_visual_settings.zoom;

            if (!radar_information->right)
                x = -abs(x);

            player_marker_colour = (radar_information->is_friendly) ? friendly_marker_colour : enemy_marker_colour;

            imgui::visuals::DrawMarker((imgui::visuals::MarkerStyle)visuals::radar_visual_settings.marker_style, {center.x + x, center.y - y}, player_marker_colour, visuals::radar_visual_settings.marker_size, visuals::radar_visual_settings.marker_thickness);
        }

        if (radar::radar_settings.show_flags) {
            ImColor friendly_flag_marker_colour = visuals::radar_visual_settings.friendly_flag_marker_colour;
            ImColor enemy_flag_marker_colour = visuals::radar_visual_settings.enemy_flag_marker_colour;

            for (vector<radar::RadarInformation>::iterator radar_information = radar::flag_locations.begin(); radar_information != radar::flag_locations.end(); radar_information++) {
                float theta = radar_information->theta;
                float y = radar_information->r * cos(theta) * visuals::radar_visual_settings.zoom_ * visuals::radar_visual_settings.zoom;
                float x = radar_information->r * sin(theta) * visuals::radar_visual_settings.zoom_ * visuals::radar_visual_settings.zoom;

                if (!radar_information->right)
                    x = -abs(x);

                ImColor flag_colour = (radar_information->is_friendly) ? friendly_flag_marker_colour : enemy_flag_marker_colour;

                imgui::visuals::DrawMarker((imgui::visuals::MarkerStyle)visuals::radar_visual_settings.marker_style, {center.x + x, center.y - y}, flag_colour, visuals::radar_visual_settings.marker_size, visuals::radar_visual_settings.marker_thickness);
            }
        }

        if (dx11::imgui_show_menu) {
            ImGui::PopStyleColor();
        }
        ImGui::End();
    }

    if (aimtracker::aimtracker_settings.enabled) {
        ImVec2 screen_size = ImGui::GetIO().DisplaySize;
        ImGui::SetNextWindowPos({screen_size.x - aimtracker::aimtracker_settings.window_size.x - 100, 100});
        ImGui::SetNextWindowSize(aimtracker::aimtracker_settings.window_size);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, visuals::radar_visual_settings.window_background_colour.Value);

        ImGui::Begin("aimtracker", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImVec2 window_position = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();

        ImVec2 center(window_position.x + window_size.x / 2, window_position.y + window_size.y / 2);

        ImDrawList* pDrawList = ImGui::GetWindowDrawList();

        pDrawList->AddLine({window_position.x, window_position.y + window_size.y / 2}, {window_position.x + window_size.x, window_position.y + window_size.y / 2}, ImColor(65, 65, 65, 255), 2);
        pDrawList->AddLine({window_position.x + window_size.x / 2, window_position.y}, {window_position.x + window_size.x / 2, window_position.y + window_size.y}, ImColor(65, 65, 65, 255), 2);
        pDrawList->AddCircleFilled(center, 3, ImColor(65, 65, 65, 125));

        for (int i = 0; i < aimtracker::aim_tracker_tick_manager.GetCount(); i++) {
            aimtracker::AimTrackerTick& tick = aimtracker::aim_tracker_tick_manager.GetTick(i);
            double d = aimtracker::GetTimeDifference(std::chrono::steady_clock::now(), tick.GetTickTime());

            if (d > aimtracker::aim_tracker_tick_manager.GetLifeTime())
                break;

            pDrawList->AddCircleFilled({center.x + tick.GetYaw() * aimtracker::aim_tracker_tick_manager.GetZoomYaw(), center.y - tick.GetPitch() * aimtracker::aim_tracker_tick_manager.GetZoomPitch()}, (1 - d / aimtracker::aim_tracker_tick_manager.GetLifeTime()) * 3, ImColor(d / aimtracker::aim_tracker_tick_manager.GetLifeTime() * 0, 255, d / aimtracker::aim_tracker_tick_manager.GetLifeTime() * 0, (1 - d / aimtracker::aim_tracker_tick_manager.GetLifeTime()) * 255));
            // pDrawList->AddText({center.x + tick.GetYaw() * aimtracker::aim_tracker_tick_manager.GetZoomYaw(), center.y - tick.GetPitch() * aimtracker::aim_tracker_tick_manager.GetZoomPitch()}, ImCol(255,255,255,255), to_string()
        }

        for (int i = 0; i < aimtracker::aim_tracker_tick_manager.GetCount(); i++) {
            aimtracker::AimTrackerTick& tick = aimtracker::aim_tracker_tick_manager.GetTick(i);
            double d = aimtracker::GetTimeDifference(std::chrono::steady_clock::now(), tick.GetTickTime());

            if (d > aimtracker::aim_tracker_tick_manager.GetLifeTime())
                break;

            pDrawList->AddCircleFilled({center.x + tick.GetYaw() * aimtracker::aim_tracker_tick_manager.GetZoomYaw(), center.y - tick.GetPitch() * aimtracker::aim_tracker_tick_manager.GetZoomPitch()}, (1 - d / aimtracker::aim_tracker_tick_manager.GetLifeTime()) * 3, ImColor(255, d / aimtracker::aim_tracker_tick_manager.GetLifeTime() * 255, d / aimtracker::aim_tracker_tick_manager.GetLifeTime() * 255, (1 - d / aimtracker::aim_tracker_tick_manager.GetLifeTime()) * 255));
        }

        ImGui::End();

        ImGui::PopStyleColor();
    }

    if (dx11::imgui_show_menu) {
        // static bool unused_boolean = true;

        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // ImGui::SetNextWindowPos({300, 300}, ImGuiCond_FirstUseEver);

        ImVec2 window_size_(800, 500);

        ImGui::SetNextWindowSize(window_size_, ImGuiCond_FirstUseEver);
        ImVec2 display_size = ImGui::GetIO().DisplaySize;
        ImGui::SetNextWindowPos({display_size.x / 2 - window_size_.x / 2, display_size.y / 2 - window_size_.y / 2}, ImGuiCond_FirstUseEver);

        /* static */ ImVec2 padding = ImGui::GetStyle().FramePadding;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padding.x, 8));
        ImGui::Begin("descension v1.5", NULL, ImGuiWindowFlags_AlwaysAutoResize & 0);
        ImGui::PopStyleVar();

        ImVec2 window_position = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();
        ImVec2 center(window_position.x + window_size.x / 2, window_position.y + window_size.y / 2);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar & 0;

        /* static */ float left_menu_width = 125;
        /* static */ float child_height_offset = 10;

        ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ChildBg, ImColor(0, 0, 0, 0).Value);
        ImGui::BeginChild("MenuL", ImVec2(left_menu_width, ImGui::GetContentRegionAvail().y - child_height_offset), false, window_flags);

        for (int i = 0; i < imgui_menu::buttons_num; i++) {
            ImVec2 size = ImVec2(left_menu_width * 0.75, 0);
            bool b_selected = i == imgui_menu::selected_index;
            if (b_selected) {
                size.x = left_menu_width * 0.95;
                ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_::ImGuiCol_ButtonHovered]);
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(1, 0));
            }

            if (*imgui_menu::button_text[i] != '-') {
                if (ImGui::Button(imgui_menu::button_text[i], size)) {
                    imgui_menu::selected_index = i;
                }
            }

            if (b_selected) {
                ImGui::PopStyleColor();
                ImGui::PopStyleVar();
            }
        }

        ImGui::EndChild();
        ImGui::SameLine();

        ImGui::BeginChild("MenuR", ImVec2(ImGui::GetContentRegionAvailWidth(), ImGui::GetContentRegionAvail().y - child_height_offset), false, window_flags);
        ImGui::PopStyleColor();

        switch (imgui_menu::selected_index) {
            case imgui_menu::LeftMenuButtons::kInformation:
                imgui_menu::DrawInformationMenuNew();
                break;
            case imgui_menu::LeftMenuButtons::kAimAssist:
                imgui_menu::DrawAimAssistMenuNew();
                break;
            case imgui_menu::LeftMenuButtons::kRadar:
                imgui_menu::DrawRadarMenuNew();
                break;
            case imgui_menu::LeftMenuButtons::kESP:
                imgui_menu::DrawESPMenuNew();
                break;
            case imgui_menu::LeftMenuButtons::kOther:
                imgui_menu::DrawOtherMenuNew();
                break;
        }

        ImGui::EndChild();
        ImGui::End();
    }

}  // namespace ue
}  // namespace ue

namespace game_data {

void GetWeapon(void) {
    if (!my_player.is_valid_) {
        return;
    }

    AMAWeapon* weapon = my_player.character_->Weapon;
    if (weapon && weapon->ProjectileClass) {
        int max_ammo = weapon->MaxAmmo;
        if (max_ammo == 25) {
            my_player.weapon_ = Weapon::disk;
            my_player.weapon_type_ = WeaponType::kProjectileLinear;
        } else if (max_ammo == 300) {
            my_player.weapon_ = Weapon::cg;
            my_player.weapon_type_ = WeaponType::kProjectileLinear;
        } else if (max_ammo == 21) {
            my_player.weapon_ = Weapon::gl;
            my_player.weapon_type_ = WeaponType::kProjectileArching;
        } else if (max_ammo == 120) {
            my_player.weapon_ = Weapon::blaster;
            my_player.weapon_type_ = WeaponType::kProjectileLinear;
        } else if (max_ammo == 48) {
            my_player.weapon_ = Weapon::plasma;
            my_player.weapon_type_ = WeaponType::kProjectileLinear;
        } /*else if (weapon->IsA(ABP_PlasmaGun_C::StaticClass())) {
            my_player.weapon_ = Weapon::plasma;
            my_player.weapon_type_ = WeaponType::kProjectileLinear;
        } else if (weapon->IsA(ABP_Blaster_C::StaticClass())) {
            my_player.weapon_ = Weapon::blaster;
            my_player.weapon_type_ = WeaponType::kProjectileLinear;
        }*/
        else {
            my_player.weapon_ == Weapon::unknown;
            my_player.weapon_type_ = WeaponType::kHitscan;
        }
    } else {
        my_player.weapon_ == Weapon::none;
        my_player.weapon_type_ = WeaponType::kHitscan;
    }
}

void GetGameData(void) {
    game_data.Reset();
    GetPlayers();
    GetWeapon();
}
}  // namespace game_data

namespace hooks {
namespace processevent {
int ProcessEvent(UObject* object, UFunction* function, void* params) {
    if (processevent_hooks.find(function) != processevent_hooks.end()) {
        vector<_ProcessEvent>& hooks = processevent_hooks[function];
        for (vector<_ProcessEvent>::iterator hook = hooks.begin(); hook != hooks.end(); hook++) {
            (*hook)(object, function, params);
        }
    }
    return original_processevent(object, function, params);
}
}  // namespace processevent
}  // namespace hooks