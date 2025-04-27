// Full code for src/main.cpp - Simplified Synchronized Scrolling
// WITH ALL ANIMATION SPEEDS HALVED (DOUBLED FRAME DURATIONS)

#include "platform/pc/pc_display.h"
#include "animation.h"
// --- Includes for assets ---
// !!! Include ALL necessary headers for ALL 8 Digimon !!!
// !!! Double-check these filenames match your assets folder !!!
// Agumon
#include "Agumon_Idle_0.h"
#include "Agumon_Idle_1.h"
#include "Agumon_Walk_0.h"
#include "Agumon_Walk_1.h"
// Gabumon
#include "Gabumon_Idle_0.h"
#include "Gabumon_Idle_1.h"
#include "Gabumon_Walk_0.h"
#include "Gabumon_Walk_1.h"
// Biyomon
#include "Biyomon_Idle_0.h"
#include "Biyomon_Idle_1.h"
#include "Biyomon_Walk_0.h"
#include "Biyomon_Walk_1.h"
// Gatomon
#include "Gatomon_Idle_0.h"
#include "Gatomon_Idle_1.h"
#include "Gatomon_Walk_0.h"
#include "Gatomon_Walk_1.h"
// Gomamon
#include "Gomamon_Idle_0.h"
#include "Gomamon_Idle_1.h"
#include "Gomamon_Walk_0.h"
#include "Gomamon_Walk_1.h"
// Palmon
#include "Palmon_Idle_0.h"
#include "Palmon_Idle_1.h"
#include "Palmon_Walk_0.h"
#include "Palmon_Walk_1.h"
// Tentomon
#include "Tentomon_Idle_0.h"
#include "Tentomon_Idle_1.h"
#include "Tentomon_Walk_0.h"
#include "Tentomon_Walk_1.h"
// Patamon
#include "Patamon_Idle_0.h"
#include "Patamon_Idle_1.h"
#include "Patamon_Walk_0.h"
#include "Patamon_Walk_1.h"

#include "bg_layer0_png.h" // Background Layer
// ---
#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <vector>
#include <cmath>
#include <limits>

// --- Window Dimensions ---
const int WINDOW_WIDTH = 466;
const int WINDOW_HEIGHT = 466;

// --- Game Constants ---
const int MAX_QUEUED_STEPS = 2;
// <<< Pixels to scroll background per game frame WHILE walking >>>
const float WALK_SCROLL_PIXELS_PER_FRAME = 1.5f; // Adjust this for desired scroll speed

// --- Game Enums ---
enum PlayerState { STATE_IDLE, STATE_WALKING };
enum DigimonType { DIGI_AGUMON, DIGI_GABUMON, DIGI_BIYOMON, DIGI_GATOMON, DIGI_GOMAMON, DIGI_PALMON, DIGI_TENTOMON, DIGI_PATAMON, DIGI_COUNT };

int main(int argc, char* argv[]) {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
    SDL_Log("--- Starting Synced Scroll Test ---");

    PCDisplay display;
    if (!display.init("Digivice Sim - Synced Scroll!", WINDOW_WIDTH, WINDOW_HEIGHT)) { return 1; }

    // --- Define ALL Sprite Frames ---
    // !!! Double check internal header names match these usages !!!
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

    // --- Define Animations for EACH Digimon with DOUBLED frame durations ---
    // Agumon
    Animation agumon_idle_anim; 
    agumon_idle_anim.addFrame(agumon_idle_0_sf, 1000); // Changed from 500 to 1000
    agumon_idle_anim.addFrame(agumon_idle_1_sf, 1000); // Changed from 500 to 1000
    agumon_idle_anim.loops = true;
    
    Animation agumon_walk_anim; 
    agumon_walk_anim.addFrame(agumon_walk_0_sf, 300); // Changed from 150 to 300
    agumon_walk_anim.addFrame(agumon_walk_1_sf, 300); // Changed from 150 to 300
    agumon_walk_anim.addFrame(agumon_walk_0_sf, 300); // Changed from 150 to 300
    agumon_walk_anim.addFrame(agumon_walk_1_sf, 300); // Changed from 150 to 300
    agumon_walk_anim.loops = false;

    // Gabumon
    Animation gabumon_idle_anim; 
    gabumon_idle_anim.addFrame(gabumon_idle_0_sf, 1100); // Changed from 550 to 1100
    gabumon_idle_anim.addFrame(gabumon_idle_1_sf, 1100); // Changed from 550 to 1100
    gabumon_idle_anim.loops = true;
    
    Animation gabumon_walk_anim; 
    gabumon_walk_anim.addFrame(gabumon_walk_0_sf, 320); // Changed from 160 to 320
    gabumon_walk_anim.addFrame(gabumon_walk_1_sf, 320); // Changed from 160 to 320
    gabumon_walk_anim.addFrame(gabumon_walk_0_sf, 320); // Changed from 160 to 320
    gabumon_walk_anim.addFrame(gabumon_walk_1_sf, 320); // Changed from 160 to 320
    gabumon_walk_anim.loops = false;

    // Biyomon
    Animation biyomon_idle_anim; 
    biyomon_idle_anim.addFrame(biyomon_idle_0_sf, 960); // Changed from 480 to 960
    biyomon_idle_anim.addFrame(biyomon_idle_1_sf, 960); // Changed from 480 to 960
    biyomon_idle_anim.loops = true;
    
    Animation biyomon_walk_anim; 
    biyomon_walk_anim.addFrame(biyomon_walk_0_sf, 280); // Changed from 140 to 280
    biyomon_walk_anim.addFrame(biyomon_walk_1_sf, 280); // Changed from 140 to 280
    biyomon_walk_anim.addFrame(biyomon_walk_0_sf, 280); // Changed from 140 to 280
    biyomon_walk_anim.addFrame(biyomon_walk_1_sf, 280); // Changed from 140 to 280
    biyomon_walk_anim.loops = false;

    // Gatomon
    Animation gatomon_idle_anim; 
    gatomon_idle_anim.addFrame(gatomon_idle_0_sf, 1200); // Changed from 600 to 1200
    gatomon_idle_anim.addFrame(gatomon_idle_1_sf, 1200); // Changed from 600 to 1200
    gatomon_idle_anim.loops = true;
    
    Animation gatomon_walk_anim; 
    gatomon_walk_anim.addFrame(gatomon_walk_0_sf, 340); // Changed from 170 to 340
    gatomon_walk_anim.addFrame(gatomon_walk_1_sf, 340); // Changed from 170 to 340
    gatomon_walk_anim.addFrame(gatomon_walk_0_sf, 340); // Changed from 170 to 340
    gatomon_walk_anim.addFrame(gatomon_walk_1_sf, 340); // Changed from 170 to 340
    gatomon_walk_anim.loops = false;

    // Gomamon
    Animation gomamon_idle_anim; 
    gomamon_idle_anim.addFrame(gomamon_idle_0_sf, 1040); // Changed from 520 to 1040
    gomamon_idle_anim.addFrame(gomamon_idle_1_sf, 1040); // Changed from 520 to 1040
    gomamon_idle_anim.loops = true;
    
    Animation gomamon_walk_anim; 
    gomamon_walk_anim.addFrame(gomamon_walk_0_sf, 310); // Changed from 155 to 310
    gomamon_walk_anim.addFrame(gomamon_walk_1_sf, 310); // Changed from 155 to 310
    gomamon_walk_anim.addFrame(gomamon_walk_0_sf, 310); // Changed from 155 to 310
    gomamon_walk_anim.addFrame(gomamon_walk_1_sf, 310); // Changed from 155 to 310
    gomamon_walk_anim.loops = false;

    // Palmon
    Animation palmon_idle_anim; 
    palmon_idle_anim.addFrame(palmon_idle_0_sf, 1080); // Changed from 540 to 1080
    palmon_idle_anim.addFrame(palmon_idle_1_sf, 1080); // Changed from 540 to 1080
    palmon_idle_anim.loops = true;
    
    Animation palmon_walk_anim; 
    palmon_walk_anim.addFrame(palmon_walk_0_sf, 330); // Changed from 165 to 330
    palmon_walk_anim.addFrame(palmon_walk_1_sf, 330); // Changed from 165 to 330
    palmon_walk_anim.addFrame(palmon_walk_0_sf, 330); // Changed from 165 to 330
    palmon_walk_anim.addFrame(palmon_walk_1_sf, 330); // Changed from 165 to 330
    palmon_walk_anim.loops = false;

    // Tentomon
    Animation tentomon_idle_anim; 
    tentomon_idle_anim.addFrame(tentomon_idle_0_sf, 920); // Changed from 460 to 920
    tentomon_idle_anim.addFrame(tentomon_idle_1_sf, 920); // Changed from 460 to 920
    tentomon_idle_anim.loops = true;
    
    Animation tentomon_walk_anim; 
    tentomon_walk_anim.addFrame(tentomon_walk_0_sf, 290); // Changed from 145 to 290
    tentomon_walk_anim.addFrame(tentomon_walk_1_sf, 290); // Changed from 145 to 290
    tentomon_walk_anim.addFrame(tentomon_walk_0_sf, 290); // Changed from 145 to 290
    tentomon_walk_anim.addFrame(tentomon_walk_1_sf, 290); // Changed from 145 to 290
    tentomon_walk_anim.loops = false;

    // Patamon
    Animation patamon_idle_anim; 
    patamon_idle_anim.addFrame(patamon_idle_0_sf, 1060); // Changed from 530 to 1060
    patamon_idle_anim.addFrame(patamon_idle_1_sf, 1060); // Changed from 530 to 1060
    patamon_idle_anim.loops = true;
    
    Animation patamon_walk_anim; 
    patamon_walk_anim.addFrame(patamon_walk_0_sf, 300); // Changed from 150 to 300
    patamon_walk_anim.addFrame(patamon_walk_1_sf, 300); // Changed from 150 to 300
    patamon_walk_anim.addFrame(patamon_walk_0_sf, 300); // Changed from 150 to 300
    patamon_walk_anim.addFrame(patamon_walk_1_sf, 300); // Changed from 150 to 300
    patamon_walk_anim.loops = false;

    // --- Background Data ---
    const int BG_WIDTH = BG_LAYER0_PNG_WIDTH; 
    const int BG_HEIGHT = BG_LAYER0_PNG_HEIGHT; 
    const uint16_t* bg_data = bg_layer0_png_data;
    float bg_scroll_offset = 0.0f; // Only need current offset

    // --- Game State Variables ---
    bool quit = false; 
    SDL_Event e;
    DigimonType current_digimon = DIGI_AGUMON; 
    PlayerState current_state = STATE_IDLE;
    Animation* active_anim = &agumon_idle_anim;
    int current_anim_frame_idx = 0; 
    Uint32 last_anim_update_time = 0; 
    int queued_steps = 0;

    SDL_Log("--- Entering main loop ---");
    while (!quit) {
        Uint32 current_time = SDL_GetTicks();

        // --- Handle Input ---
        bool character_changed_this_frame = false;
        while (SDL_PollEvent(&e) != 0) {
             if (e.type == SDL_QUIT) { quit = true; }
            if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                if (e.key.keysym.sym == SDLK_SPACE) {
                    if (queued_steps < MAX_QUEUED_STEPS) {
                        queued_steps++; 
                        SDL_Log("Step Queued (%d total)", queued_steps);
                    } else { 
                        SDL_Log("Step queue full (%d). Input ignored.", queued_steps); 
                    }
                } else if (e.key.keysym.sym >= SDLK_1 && e.key.keysym.sym <= SDLK_8) {
                    int key_num = e.key.keysym.sym - SDLK_1;
                    if (key_num < DIGI_COUNT) {
                        DigimonType selected_digi = static_cast<DigimonType>(key_num);
                        if (selected_digi != current_digimon) {
                             current_digimon = selected_digi; 
                             SDL_Log("Switched character to %d", current_digimon);
                             character_changed_this_frame = true;
                             // Reset state/anim when character changes
                             current_state = STATE_IDLE;
                             queued_steps = 0; // Clear step queue on character change
                        }
                    }
                }
            }
        }

        // --- Update Scrolling (ONLY when walking) ---
        if (current_state == STATE_WALKING) {
            bg_scroll_offset -= WALK_SCROLL_PIXELS_PER_FRAME; // Move left
            // Handle wrap
            while (bg_scroll_offset < 0.0f) {
                bg_scroll_offset += static_cast<float>(BG_WIDTH);
            }
             bg_scroll_offset = std::fmod(bg_scroll_offset, static_cast<float>(BG_WIDTH));
        }

        // --- State & Animation Selection / Update ---
        bool animation_needs_reset = character_changed_this_frame;
        if (current_state == STATE_IDLE && queued_steps > 0) { 
            current_state = STATE_WALKING; 
            animation_needs_reset = true; 
            SDL_Log("State changed to WALKING"); 
        }
        if (animation_needs_reset) {
             if (current_state == STATE_IDLE) {
                 switch(current_digimon) {
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
             } else { // STATE_WALKING
                  switch(current_digimon) {
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
             current_anim_frame_idx = 0; 
             last_anim_update_time = current_time; 
             SDL_Log("Animation pointer updated.");
        }

        // --- Animation Frame Logic ---
        bool animation_cycle_finished = false;
        if (active_anim && !active_anim->frames.empty() && !active_anim->frame_durations_ms.empty()) { 
            if (current_anim_frame_idx >= active_anim->frames.size() || current_anim_frame_idx >= active_anim->frame_durations_ms.size()) { 
                current_anim_frame_idx = 0; 
                if (active_anim->frames.empty() || active_anim->frame_durations_ms.empty()){ continue; } 
            } 
            Uint32 current_frame_duration = active_anim->frame_durations_ms[current_anim_frame_idx]; 
            if (current_time > last_anim_update_time + current_frame_duration) { 
                current_anim_frame_idx++; 
                last_anim_update_time = current_time; 
                if (current_anim_frame_idx >= active_anim->frames.size()) { 
                    animation_cycle_finished = true; 
                    if (active_anim->loops) { 
                        current_anim_frame_idx = 0; 
                    } else { 
                        current_anim_frame_idx--; 
                    } 
                } 
            } 
        } else { 
            current_anim_frame_idx = 0; 
        }

        // --- State Transition AFTER animation check (Walking -> Idle) ---
        if (current_state == STATE_WALKING && animation_cycle_finished && !active_anim->loops) {
             queued_steps--; 
             SDL_Log("Walk cycle finished. Steps remaining: %d", queued_steps);
             if (queued_steps > 0) { // More steps queued? Restart walk
                 current_anim_frame_idx = 0; 
                 last_anim_update_time = current_time; 
                 SDL_Log("Starting next queued walk cycle.");
             } else { // No more steps? Go back to idle
                 SDL_Log("Switching to IDLE state."); 
                 current_state = STATE_IDLE;
                 switch(current_digimon) { // Select correct idle animation
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
                 current_anim_frame_idx = 0; 
                 last_anim_update_time = current_time;
             }
        }

        // --- Drawing ---
        display.clear(0x0000);
        // Draw Background
        int bg_src_x = static_cast<int>(bg_scroll_offset); 
        int bg_remaining_width = BG_WIDTH - bg_src_x; 
        if (bg_remaining_width >= WINDOW_WIDTH) { 
            display.drawPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, bg_data, BG_WIDTH, BG_HEIGHT, bg_src_x, 0); 
        } else { 
            int w1 = bg_remaining_width; 
            int w2 = WINDOW_WIDTH - w1; 
            if (w1 > 0) display.drawPixels(0, 0, w1, WINDOW_HEIGHT, bg_data, BG_WIDTH, BG_HEIGHT, bg_src_x, 0); 
            if (w2 > 0) display.drawPixels(w1, 0, w2, WINDOW_HEIGHT, bg_data, BG_WIDTH, BG_HEIGHT, 0, 0); 
        }
        // Draw Character Sprite
        if (active_anim && current_anim_frame_idx < active_anim->frames.size()) { 
            const SpriteFrame& current_sprite_frame = active_anim->frames[current_anim_frame_idx]; 
            int draw_x = (WINDOW_WIDTH / 2) - (current_sprite_frame.width / 2); 
            int draw_y = (WINDOW_HEIGHT / 2) - (current_sprite_frame.height / 2); 
            display.drawPixels(draw_x, draw_y, current_sprite_frame.width, current_sprite_frame.height, 
                             current_sprite_frame.data, current_sprite_frame.width, current_sprite_frame.height, 0, 0); 
        }
        // Update Screen
        display.present();
        SDL_Delay(16);
    }

    // Cleanup
    SDL_Log("--- Exited main loop ---"); 
    display.close(); 
    SDL_Log("--- display.close() called ---"); 
    SDL_Log("--- Simulator finished ---"); 
    return 0;
}