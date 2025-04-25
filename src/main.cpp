// Full code for src/main.cpp - Walk Animation with Step Queuing

#include "platform/pc/pc_display.h"
#include "animation.h" // Include animation structures
// --- Include the generated headers ---
// !!! Adjust names if needed !!!
#include "TaiTest_0.h"
#include "TaiTest2_0.h"
#include "walk_frame_0.h"
#include "walk_frame_1.h"
#include "bg_layer0_png.h" // Background Layer
// ---
#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <vector>
#include <cmath>

// --- Window Dimensions ---
const int WINDOW_WIDTH = 466;
const int WINDOW_HEIGHT = 466;

// --- Player State ---
enum PlayerState {
    STATE_IDLE,
    STATE_WALKING // Now means "currently executing a walk animation cycle"
};

int main(int argc, char* argv[]) {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
    SDL_Log("--- Starting Step Queued Animation Test ---");

    PCDisplay display;
    SDL_Log("--- PCDisplay object created ---");

    SDL_Log("--- Calling display.init() ---");
    if (!display.init("Digivice Sim - Step Queue!", WINDOW_WIDTH, WINDOW_HEIGHT)) {
        return 1; // Error handling omitted for brevity
    }
    SDL_Log("--- display.init() succeeded ---");

    // --- Define Sprite Frames ---
    // !!! Adjust names if needed !!!
    SpriteFrame idle_frame_0 = {TAITEST_0_WIDTH, TAITEST_0_HEIGHT, TaiTest_0_data};
    SpriteFrame idle_frame_1 = {TAITEST2_0_WIDTH, TAITEST2_0_HEIGHT, TaiTest2_0_data};
    SpriteFrame walk_frame_0 = {WALK_FRAME_0_WIDTH, WALK_FRAME_0_HEIGHT, walk_frame_0_data};
    SpriteFrame walk_frame_1 = {WALK_FRAME_1_WIDTH, WALK_FRAME_1_HEIGHT, walk_frame_1_data};

    // --- Define Animations ---
    Animation idle_animation;
    idle_animation.addFrame(idle_frame_0, 500);
    idle_animation.addFrame(idle_frame_1, 500);
    idle_animation.loops = true; // Idle loops

    Animation walk_animation; // Represents ONE full walk cycle (e.g., 0 -> 1 -> 0 -> 1)
    walk_animation.addFrame(walk_frame_0, 150);
    walk_animation.addFrame(walk_frame_1, 150);
    walk_animation.addFrame(walk_frame_0, 150);
    walk_animation.addFrame(walk_frame_1, 150);
    walk_animation.loops = false; // A single walk cycle does NOT loop by itself

    // --- Background Data ---
    // !!! Adjust names if needed !!!
    const int BG_WIDTH = BG_LAYER0_PNG_WIDTH;
    const int BG_HEIGHT = BG_LAYER0_PNG_HEIGHT;
    const uint16_t* bg_data = bg_layer0_png_data;
    float bg_scroll_offset = 0.0f;
    const float BG_SCROLL_SPEED = 30.0f; // Scroll speed per step *taken*
    const int MAX_QUEUED_STEPS = 2; // Adjust to whatever makes sense for your game


    // --- Game State Variables ---
    bool quit = false;
    SDL_Event e;
    PlayerState current_state = STATE_IDLE;
    Animation* active_anim = &idle_animation; // Start idle
    int current_anim_frame_idx = 0;
    Uint32 last_anim_update_time = 0;
    int queued_steps = 0; // <<< Counter for buffered steps

    SDL_Log("--- Entering main loop ---");
    while (!quit) {
        // --- Handle Input ---
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            // Add a step to the queue on Spacebar press down
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE && e.key.repeat == 0) {
                 SDL_Log("SPACE Down - Step Queued (%d total)", queued_steps + 1);
                 queued_steps++; // Add a step to the queue
                 // Scroll the background immediately when a step is queued
                 bg_scroll_offset += BG_SCROLL_SPEED;
                 if (bg_scroll_offset >= static_cast<float>(BG_WIDTH)) {
                     bg_scroll_offset = std::fmod(bg_scroll_offset, static_cast<float>(BG_WIDTH));
                 }
            }
        }

        // --- State & Animation Selection ---
        // This runs every frame, deciding which animation *should* be playing
        if (current_state == STATE_IDLE) {
            // If we are idle, check if steps have been queued
            if (queued_steps > 0) {
                 SDL_Log("Switching to WALKING state");
                 current_state = STATE_WALKING;
                 active_anim = &walk_animation;
                 current_anim_frame_idx = 0; // Start walk anim from beginning
                 last_anim_update_time = SDL_GetTicks(); // Start timer
            } else {
                // Otherwise, stay idle
                active_anim = &idle_animation;
            }
        } else { // current_state == STATE_WALKING
             active_anim = &walk_animation; // Ensure walk anim is set if we are in this state
             // (The logic below handles switching back to IDLE when the anim cycle finishes)
        }


        // --- Animation Frame Logic ---
        Uint32 current_time = SDL_GetTicks();
        bool animation_cycle_finished = false; // Flag to check if a sequence ended

        if (active_anim && !active_anim->frames.empty() && !active_anim->frame_durations_ms.empty()) {
            // Safety check index
            if (current_anim_frame_idx >= active_anim->frames.size() || current_anim_frame_idx >= active_anim->frame_durations_ms.size()) {
                 current_anim_frame_idx = 0;
                 if (active_anim->frames.empty() || active_anim->frame_durations_ms.empty()) { continue; }
            }

            Uint32 current_frame_duration = active_anim->frame_durations_ms[current_anim_frame_idx];

            // Check if time for current frame has passed
            if (current_time > last_anim_update_time + current_frame_duration) {
                current_anim_frame_idx++; // Advance frame index
                last_anim_update_time = current_time; // Reset timer for next frame

                // Check if we finished the *entire sequence*
                if (current_anim_frame_idx >= active_anim->frames.size()) {
                    animation_cycle_finished = true; // Mark sequence as finished
                    if (active_anim->loops) {
                        current_anim_frame_idx = 0; // Loop back to start
                    } else {
                        // If a non-looping animation finished (our walk anim)
                        // We don't immediately switch state here, we do it below
                        current_anim_frame_idx--; // Stay on last frame visually until state change
                    }
                }
            }
        } else {
            current_anim_frame_idx = 0;
        }

        // --- State Transition Based on Animation Completion ---
        // If we were walking AND the walk animation cycle just finished...
        if (current_state == STATE_WALKING && animation_cycle_finished) {
             queued_steps--; // Consume one queued step
             SDL_Log("Walk cycle finished. Steps remaining in queue: %d", queued_steps);
             if (queued_steps > 0) {
                 // If more steps are queued, restart the walk animation immediately
                 current_anim_frame_idx = 0;
                 last_anim_update_time = SDL_GetTicks(); // Start timer for frame 0 now
                 SDL_Log("Starting next queued walk cycle.");
                 // State remains STATE_WALKING
             } else {
                 // If no more steps queued, switch back to IDLE
                 SDL_Log("No more steps queued, switching to IDLE.");
                 current_state = STATE_IDLE;
                 active_anim = &idle_animation;
                 current_anim_frame_idx = 0;
                 last_anim_update_time = SDL_GetTicks(); // Start timer for idle frame 0
             }
        }


        // --- Drawing ---
        display.clear(0x0000);

        // --- Draw Scrolling Background ---
        int src_x = static_cast<int>(bg_scroll_offset);
        int remaining_width = BG_WIDTH - src_x;
        if (remaining_width >= WINDOW_WIDTH) {
            display.drawPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, bg_data, BG_WIDTH, BG_HEIGHT, src_x, 0);
        } else {
            int w1 = remaining_width; int w2 = WINDOW_WIDTH - w1;
            if (w1 > 0) { display.drawPixels(0, 0, w1, WINDOW_HEIGHT, bg_data, BG_WIDTH, BG_HEIGHT, src_x, 0); }
            if (w2 > 0) { display.drawPixels(w1, 0, w2, WINDOW_HEIGHT, bg_data, BG_WIDTH, BG_HEIGHT, 0, 0); }
        }

        // --- Draw Character Sprite (Centered) ---
        if (active_anim && current_anim_frame_idx < active_anim->frames.size()) {
            const SpriteFrame& current_sprite_frame = active_anim->frames[current_anim_frame_idx];
            int draw_x = (WINDOW_WIDTH / 2) - (current_sprite_frame.width / 2);
            int draw_y = (WINDOW_HEIGHT / 2) - (current_sprite_frame.height / 2);
            display.drawPixels(draw_x, draw_y, current_sprite_frame.width, current_sprite_frame.height,
                               current_sprite_frame.data, current_sprite_frame.width, current_sprite_frame.height, 0, 0);
        }

        // --- Update Screen ---
        display.present();
        SDL_Delay(16);
    }

    // --- Cleanup ---
    SDL_Log("--- Exited main loop ---"); display.close(); SDL_Log("--- display.close() called ---"); SDL_Log("--- Simulator finished ---"); return 0;
}