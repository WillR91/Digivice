// ===============================================================================
// Full code for src/main.cpp - Implementing 3-Layer Parallax Scrolling
// Assets: castlebackground0 (Foreground), castlebackground1 (Middleground),
//         castlebackground2 (Background).
// Assumed Orig Size: 384x128 each, Scaled: 3.7x (1421x474 each calculated).
// Scrolling uses overlapping tiles, offset by effective width (947 calculated),
// with individual scroll speeds for parallax.
// Character drawn BETWEEN Layer 1 and Layer 0.
// Foreground scroll speed increased.
// All headers assumed updated (1421x474 dimensions & data).
// Uses ALL 8 Digimon.
// ===============================================================================

#include "platform/pc/pc_display.h"
#include "animation.h"

// --- Includes for Assets ---

// Digimon Assets (Unchanged)
#include "Agumon_Idle_0.h"
#include "Agumon_Idle_1.h"
#include "Agumon_Walk_0.h"
#include "Agumon_Walk_1.h"
#include "Gabumon_Idle_0.h"
#include "Gabumon_Idle_1.h"
#include "Gabumon_Walk_0.h"
#include "Gabumon_Walk_1.h"
#include "Biyomon_Idle_0.h"
#include "Biyomon_Idle_1.h"
#include "Biyomon_Walk_0.h"
#include "Biyomon_Walk_1.h"
#include "Gatomon_Idle_0.h"
#include "Gatomon_Idle_1.h"
#include "Gatomon_Walk_0.h"
#include "Gatomon_Walk_1.h"
#include "Gomamon_Idle_0.h"
#include "Gomamon_Idle_1.h"
#include "Gomamon_Walk_0.h"
#include "Gomamon_Walk_1.h"
#include "Palmon_Idle_0.h"
#include "Palmon_Idle_1.h"
#include "Palmon_Walk_0.h"
#include "Palmon_Walk_1.h"
#include "Tentomon_Idle_0.h"
#include "Tentomon_Idle_1.h"
#include "Tentomon_Walk_0.h"
#include "Tentomon_Walk_1.h"
#include "Patamon_Idle_0.h"
#include "Patamon_Idle_1.h"
#include "Patamon_Walk_0.h"
#include "Patamon_Walk_1.h"

// Background Asset Headers
// !!! CRITICAL DEPENDENCY !!!
// These header files MUST be updated/regenerated to contain the pixel data
// for the background images scaled by 3.7x, AND define the correct
// dimensions (e.g., CASTLEBACKGROUND*_WIDTH = 1421, CASTLEBACKGROUND*_HEIGHT = 474).
#include "castlebackground0.h" // <<< Foreground Layer
#include "castlebackground1.h" // <<< Middleground Layer (Existing)
#include "castlebackground2.h" // <<< Background Layer

// Standard / SDL Includes
#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <vector>
#include <cmath>
#include <limits>
#include <cstring>
#include <stdint.h> // For uint16_t

// --- Window Dimensions ---
const int WINDOW_WIDTH = 466;
const int WINDOW_HEIGHT = 466;

// --- Game Constants ---
const int MAX_QUEUED_STEPS = 2;

// --- Background Constants (Parallax Layers) ---
// ** VALUES BASED ON 3.7x SCALING of 384x128 for ALL layers **
// Calculated Scaled Dimensions: Width = 1421 px, Height = 474 px

// Layer 0: Foreground (Fastest Scroll)
const int TILE_WIDTH_0 = CASTLEBACKGROUND0_WIDTH;   // Expecting 1421 from header
const int TILE_HEIGHT_0 = CASTLEBACKGROUND0_HEIGHT; // Expecting 474 from header
const int EFFECTIVE_BG_WIDTH_0 = 947;               // <<< ADJUST if TILE_WIDTH_0 is different (1421 * 2/3)
const float effectiveW_float_0 = static_cast<float>(EFFECTIVE_BG_WIDTH_0);
const float SCROLL_SPEED_0 = 3.0f;                  // <<< Speed for Layer 0 (INCREASED)

// Layer 1: Middleground (Medium Scroll)
const int TILE_WIDTH_1 = CASTLEBACKGROUND1_WIDTH;   // Expecting 1421 from header
const int TILE_HEIGHT_1 = CASTLEBACKGROUND1_HEIGHT; // Expecting 474 from header
const int EFFECTIVE_BG_WIDTH_1 = 947;               // <<< ADJUST if TILE_WIDTH_1 is different (1421 * 2/3)
const float effectiveW_float_1 = static_cast<float>(EFFECTIVE_BG_WIDTH_1);
const float SCROLL_SPEED_1 = 1.0f;                  // <<< Speed for Layer 1

// Layer 2: Background (Slowest Scroll)
const int TILE_WIDTH_2 = CASTLEBACKGROUND2_WIDTH;   // Expecting 1421 from header
const int TILE_HEIGHT_2 = CASTLEBACKGROUND2_HEIGHT; // Expecting 474 from header
const int EFFECTIVE_BG_WIDTH_2 = 947;               // <<< ADJUST if TILE_WIDTH_2 is different (1421 * 2/3)
const float effectiveW_float_2 = static_cast<float>(EFFECTIVE_BG_WIDTH_2);
const float SCROLL_SPEED_2 = 0.5f;                  // <<< Speed for Layer 2

// --- Game Enums ---
enum PlayerState { STATE_IDLE, STATE_WALKING };
enum DigimonType { DIGI_AGUMON, DIGI_GABUMON, DIGI_BIYOMON, DIGI_GATOMON, DIGI_GOMAMON, DIGI_PALMON, DIGI_TENTOMON, DIGI_PATAMON, DIGI_COUNT };

// ==========================================================================
// Main Function
// ==========================================================================
int main(int argc, char* argv[]) {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
    SDL_Log("--- Starting Digivice Sim (3-Layer Parallax Scroll - 3.7x Scale v2) ---");
    SDL_Log("Background Assets: castlebackground0, castlebackground1, castlebackground2");
    SDL_Log("Drawing Order: BG(2) -> MG(1) -> Character -> FG(0)"); // Log new order
    SDL_Log("Expected Tile Dimensions (WxH) for all layers: %d x %d", 1421, 474); // Log expected values
    SDL_Log("Actual Dimensions Layer 0 (WxH): %d x %d", TILE_WIDTH_0, TILE_HEIGHT_0);
    SDL_Log("Actual Dimensions Layer 1 (WxH): %d x %d", TILE_WIDTH_1, TILE_HEIGHT_1);
    SDL_Log("Actual Dimensions Layer 2 (WxH): %d x %d", TILE_WIDTH_2, TILE_HEIGHT_2);
    SDL_Log("Effective Scroll Widths (0, 1, 2): %d, %d, %d", EFFECTIVE_BG_WIDTH_0, EFFECTIVE_BG_WIDTH_1, EFFECTIVE_BG_WIDTH_2);
    SDL_Log("Scroll Speeds (0, 1, 2): %.2f, %.2f, %.2f", SCROLL_SPEED_0, SCROLL_SPEED_1, SCROLL_SPEED_2); // Log new speeds

    // --- Sanity Checks --- (Unchanged)
    auto checkLayer = [](int layerNum, int width, int height, int effectiveWidth) {
        if (width <= 0 || height <= 0) {
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: Invalid tile dimensions for Layer %d from header (W:%d H:%d)", layerNum, width, height);
             return false;
        }
        if (effectiveWidth <= 0 || effectiveWidth > width) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: Effective BG Width (%d) is invalid for Layer %d Tile Width (%d)", effectiveWidth, layerNum, width);
             float actual_effective_w = static_cast<float>(width) * (2.0f / 3.0f);
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Consider using effective width: %d based on actual tile width for Layer %d", static_cast<int>(std::round(actual_effective_w)), layerNum);
            return false;
        }
         if (width != 1421 || height != 474) {
              SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Warning: Expected dims (1421x474) differ from header dims for Layer %d (%dx%d)!", layerNum, width, height);
         }
         return true;
    };

    bool config_ok = true;
    config_ok &= checkLayer(0, TILE_WIDTH_0, TILE_HEIGHT_0, EFFECTIVE_BG_WIDTH_0);
    config_ok &= checkLayer(1, TILE_WIDTH_1, TILE_HEIGHT_1, EFFECTIVE_BG_WIDTH_1);
    config_ok &= checkLayer(2, TILE_WIDTH_2, TILE_HEIGHT_2, EFFECTIVE_BG_WIDTH_2);

     if (WINDOW_WIDTH <= 0 || WINDOW_HEIGHT <= 0) {
          SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: Invalid window dimensions (W:%d H:%d)", WINDOW_WIDTH, WINDOW_HEIGHT);
          config_ok = false;
     }
     if (!config_ok) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Configuration errors found. Exiting.");
         return 1;
     }
     // --- End Sanity Checks ---


    PCDisplay display;
    if (!display.init("Digivice Sim - Parallax Scroll v2", WINDOW_WIDTH, WINDOW_HEIGHT)) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Display initialization failed!");
         return 1;
    }

    // --- Define ALL Sprite Frames ---
    // (These definitions remain unchanged)
    SpriteFrame agumon_idle_0_sf = {AGUMON_IDLE_0_WIDTH, AGUMON_IDLE_0_HEIGHT, Agumon_Idle_0_data};
    SpriteFrame agumon_idle_1_sf = {AGUMON_IDLE_1_WIDTH, AGUMON_IDLE_1_HEIGHT, Agumon_Idle_1_data};
    SpriteFrame agumon_walk_0_sf = {AGUMON_WALK_0_WIDTH, AGUMON_WALK_0_HEIGHT, Agumon_Walk_0_data};
    SpriteFrame agumon_walk_1_sf = {AGUMON_WALK_1_WIDTH, AGUMON_WALK_1_HEIGHT, Agumon_Walk_1_data};
    SpriteFrame gabumon_idle_0_sf = {GABUMON_IDLE_0_WIDTH, GABUMON_IDLE_0_HEIGHT, Gabumon_Idle_0_data};
    SpriteFrame gabumon_idle_1_sf = {GABUMON_IDLE_1_WIDTH, GABUMON_IDLE_1_HEIGHT, Gabumon_Idle_1_data};
    SpriteFrame gabumon_walk_0_sf = {GABUMON_WALK_0_WIDTH, GABUMON_WALK_0_HEIGHT, Gabumon_Walk_0_data};
    SpriteFrame gabumon_walk_1_sf = {GABUMON_WALK_1_WIDTH, GABUMON_WALK_1_HEIGHT, Gabumon_Walk_1_data};
    SpriteFrame biyomon_idle_0_sf = {BIYOMON_IDLE_0_WIDTH, BIYOMON_IDLE_0_HEIGHT, Biyomon_Idle_0_data};
    SpriteFrame biyomon_idle_1_sf = {BIYOMON_IDLE_1_WIDTH, BIYOMON_IDLE_1_HEIGHT, Biyomon_Idle_1_data};
    SpriteFrame biyomon_walk_0_sf = {BIYOMON_WALK_0_WIDTH, BIYOMON_WALK_0_HEIGHT, Biyomon_Walk_0_data};
    SpriteFrame biyomon_walk_1_sf = {BIYOMON_WALK_1_WIDTH, BIYOMON_WALK_1_HEIGHT, Biyomon_Walk_1_data};
    SpriteFrame gatomon_idle_0_sf = {GATOMON_IDLE_0_WIDTH, GATOMON_IDLE_0_HEIGHT, Gatomon_Idle_0_data};
    SpriteFrame gatomon_idle_1_sf = {GATOMON_IDLE_1_WIDTH, GATOMON_IDLE_1_HEIGHT, Gatomon_Idle_1_data};
    SpriteFrame gatomon_walk_0_sf = {GATOMON_WALK_0_WIDTH, GATOMON_WALK_0_HEIGHT, Gatomon_Walk_0_data};
    SpriteFrame gatomon_walk_1_sf = {GATOMON_WALK_1_WIDTH, GATOMON_WALK_1_HEIGHT, Gatomon_Walk_1_data};
    SpriteFrame gomamon_idle_0_sf = {GOMAMON_IDLE_0_WIDTH, GOMAMON_IDLE_0_HEIGHT, Gomamon_Idle_0_data};
    SpriteFrame gomamon_idle_1_sf = {GOMAMON_IDLE_1_WIDTH, GOMAMON_IDLE_1_HEIGHT, Gomamon_Idle_1_data};
    SpriteFrame gomamon_walk_0_sf = {GOMAMON_WALK_0_WIDTH, GOMAMON_WALK_0_HEIGHT, Gomamon_Walk_0_data};
    SpriteFrame gomamon_walk_1_sf = {GOMAMON_WALK_1_WIDTH, GOMAMON_WALK_1_HEIGHT, Gomamon_Walk_1_data};
    SpriteFrame palmon_idle_0_sf = {PALMON_IDLE_0_WIDTH, PALMON_IDLE_0_HEIGHT, Palmon_Idle_0_data};
    SpriteFrame palmon_idle_1_sf = {PALMON_IDLE_1_WIDTH, PALMON_IDLE_1_HEIGHT, Palmon_Idle_1_data};
    SpriteFrame palmon_walk_0_sf = {PALMON_WALK_0_WIDTH, PALMON_WALK_0_HEIGHT, Palmon_Walk_0_data};
    SpriteFrame palmon_walk_1_sf = {PALMON_WALK_1_WIDTH, PALMON_WALK_1_HEIGHT, Palmon_Walk_1_data};
    SpriteFrame tentomon_idle_0_sf = {TENTOMON_IDLE_0_WIDTH, TENTOMON_IDLE_0_HEIGHT, Tentomon_Idle_0_data};
    SpriteFrame tentomon_idle_1_sf = {TENTOMON_IDLE_1_WIDTH, TENTOMON_IDLE_1_HEIGHT, Tentomon_Idle_1_data};
    SpriteFrame tentomon_walk_0_sf = {TENTOMON_WALK_0_WIDTH, TENTOMON_WALK_0_HEIGHT, Tentomon_Walk_0_data};
    SpriteFrame tentomon_walk_1_sf = {TENTOMON_WALK_1_WIDTH, TENTOMON_WALK_1_HEIGHT, Tentomon_Walk_1_data};
    SpriteFrame patamon_idle_0_sf = {PATAMON_IDLE_0_WIDTH, PATAMON_IDLE_0_HEIGHT, Patamon_Idle_0_data};
    SpriteFrame patamon_idle_1_sf = {PATAMON_IDLE_1_WIDTH, PATAMON_IDLE_1_HEIGHT, Patamon_Idle_1_data};
    SpriteFrame patamon_walk_0_sf = {PATAMON_WALK_0_WIDTH, PATAMON_WALK_0_HEIGHT, Patamon_Walk_0_data};
    SpriteFrame patamon_walk_1_sf = {PATAMON_WALK_1_WIDTH, PATAMON_WALK_1_HEIGHT, Patamon_Walk_1_data};


    // --- Define Animations for EACH Digimon ---
    // (These definitions remain unchanged)
    Animation agumon_idle_anim;
    agumon_idle_anim.addFrame(agumon_idle_0_sf, 1000); agumon_idle_anim.addFrame(agumon_idle_1_sf, 1000); agumon_idle_anim.loops = true;
    Animation agumon_walk_anim;
    agumon_walk_anim.addFrame(agumon_walk_0_sf, 300); agumon_walk_anim.addFrame(agumon_walk_1_sf, 300); agumon_walk_anim.addFrame(agumon_walk_0_sf, 300); agumon_walk_anim.addFrame(agumon_walk_1_sf, 300); agumon_walk_anim.loops = false;
    Animation gabumon_idle_anim;
    gabumon_idle_anim.addFrame(gabumon_idle_0_sf, 1100); gabumon_idle_anim.addFrame(gabumon_idle_1_sf, 1100); gabumon_idle_anim.loops = true;
    Animation gabumon_walk_anim;
    gabumon_walk_anim.addFrame(gabumon_walk_0_sf, 320); gabumon_walk_anim.addFrame(gabumon_walk_1_sf, 320); gabumon_walk_anim.addFrame(gabumon_walk_0_sf, 320); gabumon_walk_anim.addFrame(gabumon_walk_1_sf, 320); gabumon_walk_anim.loops = false;
    Animation biyomon_idle_anim;
    biyomon_idle_anim.addFrame(biyomon_idle_0_sf, 960); biyomon_idle_anim.addFrame(biyomon_idle_1_sf, 960); biyomon_idle_anim.loops = true;
    Animation biyomon_walk_anim;
    biyomon_walk_anim.addFrame(biyomon_walk_0_sf, 280); biyomon_walk_anim.addFrame(biyomon_walk_1_sf, 280); biyomon_walk_anim.addFrame(biyomon_walk_0_sf, 280); biyomon_walk_anim.addFrame(biyomon_walk_1_sf, 280); biyomon_walk_anim.loops = false;
    Animation gatomon_idle_anim;
    gatomon_idle_anim.addFrame(gatomon_idle_0_sf, 1200); gatomon_idle_anim.addFrame(gatomon_idle_1_sf, 1200); gatomon_idle_anim.loops = true;
    Animation gatomon_walk_anim;
    gatomon_walk_anim.addFrame(gatomon_walk_0_sf, 340); gatomon_walk_anim.addFrame(gatomon_walk_1_sf, 340); gatomon_walk_anim.addFrame(gatomon_walk_0_sf, 340); gatomon_walk_anim.addFrame(gatomon_walk_1_sf, 340); gatomon_walk_anim.loops = false;
    Animation gomamon_idle_anim;
    gomamon_idle_anim.addFrame(gomamon_idle_0_sf, 1040); gomamon_idle_anim.addFrame(gomamon_idle_1_sf, 1040); gomamon_idle_anim.loops = true;
    Animation gomamon_walk_anim;
    gomamon_walk_anim.addFrame(gomamon_walk_0_sf, 310); gomamon_walk_anim.addFrame(gomamon_walk_1_sf, 310); gomamon_walk_anim.addFrame(gomamon_walk_0_sf, 310); gomamon_walk_anim.addFrame(gomamon_walk_1_sf, 310); gomamon_walk_anim.loops = false;
    Animation palmon_idle_anim;
    palmon_idle_anim.addFrame(palmon_idle_0_sf, 1080); palmon_idle_anim.addFrame(palmon_idle_1_sf, 1080); palmon_idle_anim.loops = true;
    Animation palmon_walk_anim;
    palmon_walk_anim.addFrame(palmon_walk_0_sf, 330); palmon_walk_anim.addFrame(palmon_walk_1_sf, 330); palmon_walk_anim.addFrame(palmon_walk_0_sf, 330); palmon_walk_anim.addFrame(palmon_walk_1_sf, 330); palmon_walk_anim.loops = false;
    Animation tentomon_idle_anim;
    tentomon_idle_anim.addFrame(tentomon_idle_0_sf, 920); tentomon_idle_anim.addFrame(tentomon_idle_1_sf, 920); tentomon_idle_anim.loops = true;
    Animation tentomon_walk_anim;
    tentomon_walk_anim.addFrame(tentomon_walk_0_sf, 290); tentomon_walk_anim.addFrame(tentomon_walk_1_sf, 290); tentomon_walk_anim.addFrame(tentomon_walk_0_sf, 290); tentomon_walk_anim.addFrame(tentomon_walk_1_sf, 290); tentomon_walk_anim.loops = false;
    Animation patamon_idle_anim;
    patamon_idle_anim.addFrame(patamon_idle_0_sf, 1060); patamon_idle_anim.addFrame(patamon_idle_1_sf, 1060); patamon_idle_anim.loops = true;
    Animation patamon_walk_anim;
    patamon_walk_anim.addFrame(patamon_walk_0_sf, 300); patamon_walk_anim.addFrame(patamon_walk_1_sf, 300); patamon_walk_anim.addFrame(patamon_walk_0_sf, 300); patamon_walk_anim.addFrame(patamon_walk_1_sf, 300); patamon_walk_anim.loops = false;


    // --- Background Data Pointers --- (Unchanged)
    const uint16_t* bg_data_0 = castlebackground0_data; // Foreground
    const uint16_t* bg_data_1 = castlebackground1_data; // Middleground
    const uint16_t* bg_data_2 = castlebackground2_data; // Background

    // --- Background Scroll State --- (Unchanged)
    float bg_scroll_offset_0 = 0.0f;
    float bg_scroll_offset_1 = 0.0f;
    float bg_scroll_offset_2 = 0.0f;

    // --- Game State Variables --- (Unchanged)
    bool quit = false;
    SDL_Event e;
    DigimonType current_digimon = DIGI_AGUMON;
    PlayerState current_state = STATE_IDLE;
    Animation* active_anim = &agumon_idle_anim;
    int current_anim_frame_idx = 0;
    Uint32 last_anim_update_time = 0;
    int queued_steps = 0;

    SDL_Log("--- Entering main loop ---");
    // ==========================================================================
    // Main Game Loop
    // ==========================================================================
    while (!quit) {
        Uint32 current_time = SDL_GetTicks();

        // --- Handle Input --- (Unchanged)
        bool character_changed_this_frame = false;
        while (SDL_PollEvent(&e) != 0) {
             if (e.type == SDL_QUIT) { quit = true; }
             if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                 if (e.key.keysym.sym == SDLK_SPACE) {
                     if (queued_steps < MAX_QUEUED_STEPS) { queued_steps++; }
                 } else if (e.key.keysym.sym >= SDLK_1 && e.key.keysym.sym <= SDLK_8) {
                     int key_num = e.key.keysym.sym - SDLK_1;
                     if (key_num < DIGI_COUNT) {
                         DigimonType selected_digi = static_cast<DigimonType>(key_num);
                         if (selected_digi != current_digimon) {
                             current_digimon = selected_digi; SDL_Log("Switched character to %d", current_digimon);
                             character_changed_this_frame = true; current_state = STATE_IDLE; queued_steps = 0;
                         }
                     }
                 } else if (e.key.keysym.sym == SDLK_ESCAPE) { quit = true; }
             }
        }


        // --- Update Parallax Scrolling (ONLY when walking) --- (Unchanged)
        if (current_state == STATE_WALKING) {
            // Update Layer 0 (Foreground) - Uses SCROLL_SPEED_0
            bg_scroll_offset_0 -= SCROLL_SPEED_0;
            while (bg_scroll_offset_0 < 0.0f) { bg_scroll_offset_0 += effectiveW_float_0; }
            bg_scroll_offset_0 = std::fmod(bg_scroll_offset_0, effectiveW_float_0);

            // Update Layer 1 (Middleground) - Uses SCROLL_SPEED_1
            bg_scroll_offset_1 -= SCROLL_SPEED_1;
            while (bg_scroll_offset_1 < 0.0f) { bg_scroll_offset_1 += effectiveW_float_1; }
            bg_scroll_offset_1 = std::fmod(bg_scroll_offset_1, effectiveW_float_1);

            // Update Layer 2 (Background) - Uses SCROLL_SPEED_2
            bg_scroll_offset_2 -= SCROLL_SPEED_2;
            while (bg_scroll_offset_2 < 0.0f) { bg_scroll_offset_2 += effectiveW_float_2; }
            bg_scroll_offset_2 = std::fmod(bg_scroll_offset_2, effectiveW_float_2);
        }


        // --- State & Animation Selection / Update --- (Unchanged)
         bool animation_needs_reset = character_changed_this_frame;
         if (current_state == STATE_IDLE && queued_steps > 0) {
             current_state = STATE_WALKING; animation_needs_reset = true; SDL_Log("State changed to WALKING");
         }
         if (animation_needs_reset) {
             if (current_state == STATE_IDLE) {
                 switch(current_digimon) { /* ... Set Idle Anim ... */
                    case DIGI_AGUMON:   active_anim = &agumon_idle_anim; break;
                    case DIGI_GABUMON:  active_anim = &gabumon_idle_anim; break;
                    case DIGI_BIYOMON:  active_anim = &biyomon_idle_anim; break;
                    case DIGI_GATOMON:  active_anim = &gatomon_idle_anim; break;
                    case DIGI_GOMAMON:  active_anim = &gomamon_idle_anim; break;
                    case DIGI_PALMON:   active_anim = &palmon_idle_anim; break;
                    case DIGI_TENTOMON: active_anim = &tentomon_idle_anim; break;
                    case DIGI_PATAMON:  active_anim = &patamon_idle_anim; break;
                    default:            active_anim = &agumon_idle_anim;
                 }
             } else { /* STATE_WALKING */
                  switch(current_digimon) { /* ... Set Walk Anim ... */
                    case DIGI_AGUMON:   active_anim = &agumon_walk_anim; break;
                    case DIGI_GABUMON:  active_anim = &gabumon_walk_anim; break;
                    case DIGI_BIYOMON:  active_anim = &biyomon_walk_anim; break;
                    case DIGI_GATOMON:  active_anim = &gatomon_walk_anim; break;
                    case DIGI_GOMAMON:  active_anim = &gomamon_walk_anim; break;
                    case DIGI_PALMON:   active_anim = &palmon_walk_anim; break;
                    case DIGI_TENTOMON: active_anim = &tentomon_walk_anim; break;
                    case DIGI_PATAMON:  active_anim = &patamon_walk_anim; break;
                    default:            active_anim = &agumon_walk_anim;
                 }
             }
             current_anim_frame_idx = 0; last_anim_update_time = current_time;
         }


        // --- Animation Frame Logic --- (Unchanged)
        bool animation_cycle_finished = false;
        if (active_anim && !active_anim->frames.empty() && !active_anim->frame_durations_ms.empty()) {
             if (current_anim_frame_idx >= active_anim->frames.size() || current_anim_frame_idx >= active_anim->frame_durations_ms.size()) {
                 SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Animation frame index out of bounds! Resetting."); current_anim_frame_idx = 0;
                 if (active_anim->frames.empty() || active_anim->frame_durations_ms.empty()) { continue; }
             }
            Uint32 current_frame_duration = active_anim->frame_durations_ms[current_anim_frame_idx];
            if (current_time >= last_anim_update_time + current_frame_duration) {
                current_anim_frame_idx++; last_anim_update_time = current_time;
                if (current_anim_frame_idx >= active_anim->frames.size()) {
                    animation_cycle_finished = true;
                    if (active_anim->loops) { current_anim_frame_idx = 0; }
                    else {
                        current_anim_frame_idx = active_anim->frames.size() - 1;
                        if (current_anim_frame_idx < 0) current_anim_frame_idx = 0;
                    }
                }
            }
        } else {
            current_anim_frame_idx = 0;
            if (!active_anim) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "active_anim is NULL!"); }
            else if (active_anim->frames.empty()) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "active_anim has no frames!"); }
        }


        // --- State Transition AFTER animation check (Walking -> Idle) --- (Unchanged)
        if (current_state == STATE_WALKING && animation_cycle_finished && !active_anim->loops) {
             queued_steps--; SDL_Log("Walk cycle finished. Steps remaining: %d", queued_steps);
             if (queued_steps > 0) {
                 current_anim_frame_idx = 0; last_anim_update_time = current_time;
                 animation_cycle_finished = false; SDL_Log("Starting next queued walk cycle.");
             } else {
                 SDL_Log("Switching to IDLE state."); current_state = STATE_IDLE;
                 animation_needs_reset = true;
             }
        }
        if (animation_needs_reset && current_state == STATE_IDLE) {
             switch(current_digimon) { /* ... Set Idle Anim ... */
                case DIGI_AGUMON:   active_anim = &agumon_idle_anim; break;
                case DIGI_GABUMON:  active_anim = &gabumon_idle_anim; break;
                case DIGI_BIYOMON:  active_anim = &biyomon_idle_anim; break;
                case DIGI_GATOMON:  active_anim = &gatomon_idle_anim; break;
                case DIGI_GOMAMON:  active_anim = &gomamon_idle_anim; break;
                case DIGI_PALMON:   active_anim = &palmon_idle_anim; break;
                case DIGI_TENTOMON: active_anim = &tentomon_idle_anim; break;
                case DIGI_PATAMON:  active_anim = &patamon_idle_anim; break;
                default:            active_anim = &agumon_idle_anim;
            }
            current_anim_frame_idx = 0; last_anim_update_time = current_time;
        }


        // =================== DRAWING STARTS HERE ==========================
        display.clear(0x0000); // Clear screen (black)

        // --- Helper Lambda for Drawing Background Tiles (Parameterized) --- (Unchanged)
        auto drawClippedTile = [&](
            int dest_x_unclipped, const uint16_t* tile_data,
            int layer_tile_width, int layer_tile_height
        ) {
            int src_x = 0, src_y = 0;
            int src_w = layer_tile_width, src_h = layer_tile_height;
            int dest_x = dest_x_unclipped, dest_y = 0;
            int dest_w = layer_tile_width, dest_h = layer_tile_height;

            // Clip Left
            if (dest_x < 0) {
                int clip = -dest_x; if (clip >= layer_tile_width) return;
                src_x += clip; src_w -= clip; dest_w -= clip; dest_x = 0;
            }
            // Clip Right
            if (dest_x + dest_w > WINDOW_WIDTH) {
                int clip = (dest_x + dest_w) - WINDOW_WIDTH; if (clip >= layer_tile_width) return;
                src_w -= clip; dest_w -= clip;
            }
            // Clip Bottom
            if (dest_y + dest_h > WINDOW_HEIGHT) {
                 int clip = (dest_y + dest_h) - WINDOW_HEIGHT; if (clip >= layer_tile_height) return;
                 src_h -= clip; dest_h -= clip;
            }
             // Clip Top (if dest_y could be < 0)

            // Draw if visible
            if (dest_w > 0 && src_w > 0 && dest_h > 0 && src_h > 0) {
                display.drawPixels(dest_x, dest_y, dest_w, dest_h, tile_data,
                                   layer_tile_width, layer_tile_height, src_x, src_y);
            }
        }; // End of drawClippedTile lambda

        // --- Draw Layers and Character in Correct Order ---

        // Layer 2: Background (Slowest)
        int draw2_x1_unclipped = -static_cast<int>(bg_scroll_offset_2);
        int draw2_x2_unclipped = draw2_x1_unclipped + EFFECTIVE_BG_WIDTH_2;
        drawClippedTile(draw2_x1_unclipped, bg_data_2, TILE_WIDTH_2, TILE_HEIGHT_2);
        drawClippedTile(draw2_x2_unclipped, bg_data_2, TILE_WIDTH_2, TILE_HEIGHT_2);

        // Layer 1: Middleground (Medium)
        int draw1_x1_unclipped = -static_cast<int>(bg_scroll_offset_1);
        int draw1_x2_unclipped = draw1_x1_unclipped + EFFECTIVE_BG_WIDTH_1;
        drawClippedTile(draw1_x1_unclipped, bg_data_1, TILE_WIDTH_1, TILE_HEIGHT_1);
        drawClippedTile(draw1_x2_unclipped, bg_data_1, TILE_WIDTH_1, TILE_HEIGHT_1);

        // *** Draw Character Sprite HERE (Before Foreground) ***
        if (active_anim && current_anim_frame_idx < active_anim->frames.size()) {
            const SpriteFrame& current_sprite_frame = active_anim->frames[current_anim_frame_idx];
            if (current_sprite_frame.data) {
                int draw_x = (WINDOW_WIDTH / 2) - (current_sprite_frame.width / 2);
                int draw_y = (WINDOW_HEIGHT / 2) - (current_sprite_frame.height / 2);
                // Adjust draw_y if needed, e.g., align bottom:
                // int draw_y = WINDOW_HEIGHT - current_sprite_frame.height - 10;

                display.drawPixels(draw_x, draw_y, current_sprite_frame.width, current_sprite_frame.height,
                                   current_sprite_frame.data, current_sprite_frame.width, current_sprite_frame.height,
                                   0, 0);
            } else {
                 SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Attempted to draw sprite with NULL data!");
            }
        }

        // Layer 0: Foreground (Fastest) - Drawn last, appears on top of character
        int draw0_x1_unclipped = -static_cast<int>(bg_scroll_offset_0);
        int draw0_x2_unclipped = draw0_x1_unclipped + EFFECTIVE_BG_WIDTH_0;
        drawClippedTile(draw0_x1_unclipped, bg_data_0, TILE_WIDTH_0, TILE_HEIGHT_0);
        drawClippedTile(draw0_x2_unclipped, bg_data_0, TILE_WIDTH_0, TILE_HEIGHT_0);


        // --- Update Screen ---
        display.present();
        // =================== DRAWING ENDS HERE ============================


        // Frame Limiter (approx 60 FPS)
        SDL_Delay(16);

    } // End Main Game Loop

    // ==========================================================================
    // Cleanup
    // ==========================================================================
    SDL_Log("--- Exited main loop ---");
    display.close();
    SDL_Log("--- display.close() called ---");
    SDL_Log("--- Simulator finished ---");
    return 0;
}