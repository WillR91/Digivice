// ===============================================================================
// Full code for src/main.cpp - Implementing Overlapping Tile Scrolling
// Asset is assumed to be a single tile (NOW SCALED 3.7x from 384x128).
// Original: 384x128, Old Scale: 3x (1152x384), New Scale: 3.7x (1421x474 calculated)
// Scrolling achieved by drawing two instances of the tile, offset by
// an effective width (NOW 947, calculated as 1421 * 2/3), and clipping to the window.
// Scroll offset wraps around EFFECTIVE width (947).
// Using castlebackground1.h (ASSUMED UPDATED to 1421x474 dimensions & data)
// and ALL 8 Digimon.
// ===============================================================================

#include "platform/pc/pc_display.h"
#include "animation.h"

// --- Includes for Assets ---

// Digimon Assets
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


// Background Asset Header
// !!! CRITICAL DEPENDENCY !!!
// This header file MUST be updated/regenerated to contain the pixel data
// for the background image scaled by 3.7x, AND define the correct
// dimensions (CASTLEBACKGROUND1_WIDTH = 1421, CASTLEBACKGROUND1_HEIGHT = 474).
#include "castlebackground1.h"

// Standard / SDL Includes
#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <vector>
#include <cmath>
#include <limits>
#include <cstring> // For memcmp if needed elsewhere, not needed for current drawing
#include <stdint.h> // For uint16_t

// --- Window Dimensions ---
// Keep these the same unless the window size itself needs to change
const int WINDOW_WIDTH = 466;
const int WINDOW_HEIGHT = 466;

// --- Game Constants ---
const int MAX_QUEUED_STEPS = 2;
// Adjust scroll speed if desired, independent of background size changes
const float WALK_SCROLL_PIXELS_PER_FRAME = 1.5f;

// --- Background Constants (Overlapping Tile Method) ---
// ** NEW VALUES BASED ON 3.7x SCALING of 384x128 **
// Calculated New Dimensions: Width = 384*3.7 = 1420.8 -> 1421 ; Height = 128*3.7 = 473.6 -> 474

// These constants read directly from the header file.
// ** THEY RELY ON castlebackground1.h BEING CORRECTLY UPDATED! **
const int TILE_WIDTH = CASTLEBACKGROUND1_WIDTH;           // Width of the NEW background asset tile (EXPECTING 1421)
const int TILE_HEIGHT = CASTLEBACKGROUND1_HEIGHT;         // Height of the NEW background asset tile (EXPECTING 474)

// CRITICAL ASSUMPTION: The effective distance for seamless looping by overlap
// Maintain the original ratio (OldEffective/OldWidth = 768 / (384*3) = 768 / 1152 = 2/3)
// with the new scaled width read from the header.
// New Effective Width = New Tile Width * (2/3) = CASTLEBACKGROUND1_WIDTH * (2.0 / 3.0)
// Example Calculation: 1421 * (2.0 / 3.0) = 947.333... -> Rounded to 947
const int EFFECTIVE_BG_WIDTH = 947; // <<< HARDCODED based on 1421 * (2/3), ADJUST if 1421 changes
// Alternative (Calculated at runtime, less optimal but safer if header width might change slightly):
// const int EFFECTIVE_BG_WIDTH = static_cast<int>(std::round(static_cast<float>(TILE_WIDTH) * (2.0f / 3.0f)));

const float effectiveW_float = static_cast<float>(EFFECTIVE_BG_WIDTH); // Float version for wrapping (NOW ~947.0f)

// Implied overlap (for reference) = TILE_WIDTH - EFFECTIVE_BG_WIDTH
// e.g., 1421 - 947 = 474

// --- Game Enums ---
enum PlayerState { STATE_IDLE, STATE_WALKING };
enum DigimonType { DIGI_AGUMON, DIGI_GABUMON, DIGI_BIYOMON, DIGI_GATOMON, DIGI_GOMAMON, DIGI_PALMON, DIGI_TENTOMON, DIGI_PATAMON, DIGI_COUNT };

// ==========================================================================
// Main Function
// ==========================================================================
int main(int argc, char* argv[]) {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
    SDL_Log("--- Starting Digivice Sim (Overlapping Tile Scroll - 3.7x Scale) ---");
    SDL_Log("Background Asset: castlebackground1");
    SDL_Log("Expected Tile Dimensions (WxH): %d x %d", 1421, 474); // Log expected values based on calculation
    SDL_Log("Actual Dimensions from Header (WxH): %d x %d", TILE_WIDTH, TILE_HEIGHT); // Log actual values from header constant
    SDL_Log("Effective Scroll Width (Wrap distance): %d", EFFECTIVE_BG_WIDTH);
    SDL_Log("Scroll Offset Wrap Method: Using EFFECTIVE Width (%d)", EFFECTIVE_BG_WIDTH);
    SDL_Log("Drawing Method: Manual Overlapping Blit with Clipping");


    // --- Sanity Checks ---
    // Check against the constants derived from the header directly
    if (TILE_WIDTH <= 0 || TILE_HEIGHT <= 0) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: Invalid background tile dimensions from header (W:%d H:%d)", TILE_WIDTH, TILE_HEIGHT);
         return 1;
    }
     // Check the *calculated* effective width against the actual tile width from header
     if (EFFECTIVE_BG_WIDTH <= 0 || EFFECTIVE_BG_WIDTH > TILE_WIDTH) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: Effective BG Width (%d) is invalid for Tile Width from header (%d)", EFFECTIVE_BG_WIDTH, TILE_WIDTH);
         // Optional: Suggest recalculation based on actual header width if there's a mismatch
         float actual_effective_w = static_cast<float>(TILE_WIDTH) * (2.0f / 3.0f);
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Consider using effective width: %d based on actual tile width", static_cast<int>(std::round(actual_effective_w)));
         return 1; // Or handle differently
     }
     if (WINDOW_WIDTH <= 0 || WINDOW_HEIGHT <= 0) {
          SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: Invalid window dimensions (W:%d H:%d)", WINDOW_WIDTH, WINDOW_HEIGHT);
          return 1;
     }
     // Additional Sanity Check: Compare expected calculation vs actual dimensions from header
     if (TILE_WIDTH != 1421 || TILE_HEIGHT != 474) {
          SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Warning: Expected tile dimensions (1421x474) differ from header dimensions (%dx%d)! Using header values.", TILE_WIDTH, TILE_HEIGHT);
          // This might indicate an issue with the header generation or the hardcoded EFFECTIVE_BG_WIDTH calculation
     }
     // --- End Sanity Checks ---


    PCDisplay display;
    // Use a slightly different window title to confirm the new code is running
    if (!display.init("Digivice Sim - Castle Scroll (3.7x Scale Test)", WINDOW_WIDTH, WINDOW_HEIGHT)) {
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


    // --- Background Data ---
    // This pointer now points to the (assumed) larger data array from the updated header
    const uint16_t* bg_data = castlebackground1_data;
    // Scroll offset now wraps within the NEW EFFECTIVE width [0, EFFECTIVE_BG_WIDTH)
    float bg_scroll_offset = 0.0f;

    // --- Game State Variables ---
    // (These remain unchanged)
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

        // --- Handle Input ---
        // (Input handling logic remains unchanged)
        bool character_changed_this_frame = false;
        while (SDL_PollEvent(&e) != 0) {
             if (e.type == SDL_QUIT) { quit = true; }
             if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                 if (e.key.keysym.sym == SDLK_SPACE) {
                     if (queued_steps < MAX_QUEUED_STEPS) { queued_steps++; /* SDL_Log("Step Queued (%d total)", queued_steps); */ } // Quieter log
                     else { /* SDL_Log("Step queue full (%d). Input ignored.", queued_steps); */ } // Quieter log
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
        } // End Input Polling


        // --- Update Scrolling (ONLY when walking) ---
        if (current_state == STATE_WALKING) {
            bg_scroll_offset -= WALK_SCROLL_PIXELS_PER_FRAME; // Move left (speed constant)

            // --- WRAPPING LOGIC using NEW EFFECTIVE_BG_WIDTH ---
            // The logic itself doesn't change, but effectiveW_float now holds the new value (~947.0f)
            while (bg_scroll_offset < 0.0f) {
                bg_scroll_offset += effectiveW_float; // Wrap using the NEW effective width
            }
            // fmod is still suitable for wrapping
            bg_scroll_offset = std::fmod(bg_scroll_offset, effectiveW_float); // Wrap using the NEW effective width
            // --- END OF WRAPPING LOGIC ---
        }


        // --- State & Animation Selection / Update ---
        // (This logic remains unchanged)
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
             current_anim_frame_idx = 0; last_anim_update_time = current_time; /* SDL_Log("Animation pointer updated and reset."); */ // Quieter log
         }


        // --- Animation Frame Logic ---
        // (This logic remains unchanged)
        bool animation_cycle_finished = false;
        if (active_anim && !active_anim->frames.empty() && !active_anim->frame_durations_ms.empty()) {
             if (current_anim_frame_idx >= active_anim->frames.size() || current_anim_frame_idx >= active_anim->frame_durations_ms.size()) {
                 SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Animation frame index out of bounds! Resetting."); current_anim_frame_idx = 0;
                 if (active_anim->frames.empty() || active_anim->frame_durations_ms.empty()) { continue; } // Skip frame if reset needed and anim is empty
             }
            Uint32 current_frame_duration = active_anim->frame_durations_ms[current_anim_frame_idx];
            if (current_time >= last_anim_update_time + current_frame_duration) {
                current_anim_frame_idx++; last_anim_update_time = current_time;
                if (current_anim_frame_idx >= active_anim->frames.size()) {
                    animation_cycle_finished = true;
                    if (active_anim->loops) { current_anim_frame_idx = 0; }
                    else {
                        // Don't decrement if it finished, stay on last frame until state change
                        current_anim_frame_idx = active_anim->frames.size() - 1;
                        if (current_anim_frame_idx < 0) current_anim_frame_idx = 0; // Safety for 1-frame anims
                    }
                }
            }
        } else {
            current_anim_frame_idx = 0;
            if (!active_anim) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "active_anim is NULL!"); }
            else if (active_anim->frames.empty()) { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "active_anim has no frames!"); }
        }


        // --- State Transition AFTER animation check (Walking -> Idle) ---
        // (This logic remains unchanged)
        if (current_state == STATE_WALKING && animation_cycle_finished && !active_anim->loops) {
             queued_steps--; SDL_Log("Walk cycle finished. Steps remaining: %d", queued_steps);
             if (queued_steps > 0) {
                 // Reset animation to loop the walk cycle if steps remain
                 current_anim_frame_idx = 0; last_anim_update_time = current_time;
                 animation_cycle_finished = false; SDL_Log("Starting next queued walk cycle.");
             } else {
                 SDL_Log("Switching to IDLE state."); current_state = STATE_IDLE;
                 animation_needs_reset = true; // Trigger idle animation selection below
             }
        }
        // Re-select animation if needed after state transition (e.g., walking finished -> idle)
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
            /* SDL_Log("Animation pointer updated and reset (Idle)."); */ // Quieter log
        }


        // =================== DRAWING STARTS HERE ==========================
        display.clear(0x0000); // Clear screen (black)

        // --- Draw Background using Overlapping Tiles ---

        // Calculate the screen X coordinates for the top-left of the two tile instances
        // Offset is subtracted because scrolling left means drawing the tile further left (negative offset)
        // Uses the potentially updated bg_scroll_offset value
        int draw_x1_unclipped = -static_cast<int>(bg_scroll_offset);
        // Position the second tile instance using the NEW EFFECTIVE_BG_WIDTH
        int draw_x2_unclipped = draw_x1_unclipped + EFFECTIVE_BG_WIDTH; // Uses the NEW value (~947)

        // Helper lambda for drawing a potentially clipped tile portion
        // This lambda now operates correctly with the NEW TILE_WIDTH and TILE_HEIGHT
        // (read from the header) and the WINDOW_WIDTH/HEIGHT constants.
        auto drawClippedTile = [&](int dest_x_unclipped) {
            // Define the full source rectangle (the whole tile)
            int src_x = 0;
            int src_y = 0;
            // These now refer to the NEW dimensions (e.g., 1421x474) via the header constants
            int src_w = TILE_WIDTH;
            int src_h = TILE_HEIGHT;

            // Define the initial destination rectangle based on the unclipped position
            int dest_x = dest_x_unclipped;
            int dest_y = 0; // Draw background starting at the top of the window
            int dest_w = TILE_WIDTH; // Uses NEW width
            int dest_h = TILE_HEIGHT; // Uses NEW height

            // --- Perform Clipping ---

            // Clip Left edge: If tile starts left of the window (dest_x < 0)
            if (dest_x < 0) {
                int clip_amount = -dest_x; // How many pixels are off-screen left
                // Check against NEW tile width
                if (clip_amount >= TILE_WIDTH) return; // Entire tile is off-screen left, do nothing

                // Adjust source and destination rectangles
                src_x += clip_amount;    // Start reading source data further in
                src_w -= clip_amount;    // Read fewer pixels from source
                dest_w -= clip_amount;   // Draw fewer pixels on screen
                dest_x = 0;              // Clamp draw position to left edge of window
            }

            // Clip Right edge: If tile ends right of the window (dest_x + dest_w > WINDOW_WIDTH)
            if (dest_x + dest_w > WINDOW_WIDTH) {
                int clip_amount = (dest_x + dest_w) - WINDOW_WIDTH; // How many pixels are off-screen right
                 // Check against NEW tile width (unlikely to fail if left clip was correct, but safe)
                if (clip_amount >= TILE_WIDTH) return;

                // Adjust source and destination widths (no need to adjust x/y)
                src_w -= clip_amount;
                dest_w -= clip_amount;
            }

            // Clip Bottom edge: If tile is taller than the window (dest_y + dest_h > WINDOW_HEIGHT)
            // Added this because the new height (474) is > WINDOW_HEIGHT (466)
            if (dest_y + dest_h > WINDOW_HEIGHT) {
                 int clip_amount = (dest_y + dest_h) - WINDOW_HEIGHT; // How many pixels are off-screen bottom
                 // Check against NEW tile height
                 if (clip_amount >= TILE_HEIGHT) return; // Entire tile is off-screen bottom

                 // Adjust source and destination heights (no need to adjust x/y or width)
                 src_h -= clip_amount; // Read fewer rows from source
                 dest_h -= clip_amount; // Draw fewer rows to screen
            }

            // Clip Top edge (Only needed if dest_y can be < 0)
            // if (dest_y < 0) { ... similar logic ... }


            // --- Draw if anything is left ---
            // Check width AND height before drawing
            if (dest_w > 0 && src_w > 0 && dest_h > 0 && src_h > 0) {
                // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Drawing Tile Portion: Dest=(%d,%d %dx%d) Src=(%d,%d)", dest_x, dest_y, dest_w, dest_h, src_x, src_y);
                display.drawPixels(dest_x, dest_y,         // Destination on screen (clipped)
                                   dest_w, dest_h,         // Size on screen (clipped)
                                   bg_data,                // Source tile data (Points to NEW data)
                                   TILE_WIDTH, TILE_HEIGHT,// Dimensions of the *source data buffer* (Uses NEW header values)
                                   src_x, src_y);          // Top-left corner to read from in source data (clipped)
            }
        };

        // Draw the visible portions of the two potentially overlapping tile instances
        drawClippedTile(draw_x1_unclipped);
        drawClippedTile(draw_x2_unclipped);


        // --- Draw Character Sprite ---
        // (Character drawing logic remains unchanged, centering uses WINDOW_WIDTH/HEIGHT)
        if (active_anim && current_anim_frame_idx < active_anim->frames.size()) {
            const SpriteFrame& current_sprite_frame = active_anim->frames[current_anim_frame_idx];
            if (current_sprite_frame.data) { // Check if sprite data is valid
                // Center the sprite horizontally
                int draw_x = (WINDOW_WIDTH / 2) - (current_sprite_frame.width / 2);
                // Center the sprite vertically - Adjust Y if needed (e.g., align bottom)
                int draw_y = (WINDOW_HEIGHT / 2) - (current_sprite_frame.height / 2);
                // Example: Align bottom with small offset:
                // int draw_y = WINDOW_HEIGHT - current_sprite_frame.height - 10; // Adjust 10 as needed

                display.drawPixels(draw_x, draw_y, current_sprite_frame.width, current_sprite_frame.height,
                                   current_sprite_frame.data, current_sprite_frame.width, current_sprite_frame.height,
                                   0, 0); // Source x,y for sprite is always 0,0
            } else {
                 SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Attempted to draw sprite with NULL data!");
            }
        }

        // --- Update Screen ---
        display.present();
        // =================== DRAWING ENDS HERE ============================


        // Frame Limiter (approx 60 FPS)
        SDL_Delay(16); // Aim for 1000ms / 60fps ~= 16.6ms

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