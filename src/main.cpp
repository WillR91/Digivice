// Full code for src/main.cpp - Smooth Background Scroll with Step Queuing

#include "platform/pc/pc_display.h"
#include "animation.h"
#include "TaiTest_0.h"
#include "TaiTest2_0.h"
#include "walk_frame_0.h"
#include "walk_frame_1.h"
#include "bg_layer0_png.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <vector>
#include <cmath>

const int WINDOW_WIDTH = 466;
const int WINDOW_HEIGHT = 466;

const int MAX_QUEUED_STEPS = 2; // Maximum steps that can be queued

enum PlayerState {
    STATE_IDLE,
    STATE_WALKING
};

int main(int argc, char* argv[]) {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
    SDL_Log("--- Starting Step Queued Animation Test ---");

    PCDisplay display;
    SDL_Log("--- PCDisplay object created ---");

    SDL_Log("--- Calling display.init() ---");
    if (!display.init("Digivice Sim - Step Queue!", WINDOW_WIDTH, WINDOW_HEIGHT)) {
        return 1;
    }
    SDL_Log("--- display.init() succeeded ---");

    SpriteFrame idle_frame_0 = {TAITEST_0_WIDTH, TAITEST_0_HEIGHT, TaiTest_0_data};
    SpriteFrame idle_frame_1 = {TAITEST2_0_WIDTH, TAITEST2_0_HEIGHT, TaiTest2_0_data};
    SpriteFrame walk_frame_0 = {WALK_FRAME_0_WIDTH, WALK_FRAME_0_HEIGHT, walk_frame_0_data};
    SpriteFrame walk_frame_1 = {WALK_FRAME_1_WIDTH, WALK_FRAME_1_HEIGHT, walk_frame_1_data};

    Animation idle_animation;
    idle_animation.addFrame(idle_frame_0, 500);
    idle_animation.addFrame(idle_frame_1, 500);
    idle_animation.loops = true;

    Animation walk_animation;
    walk_animation.addFrame(walk_frame_0, 150);
    walk_animation.addFrame(walk_frame_1, 150);
    walk_animation.addFrame(walk_frame_0, 150);
    walk_animation.addFrame(walk_frame_1, 150);
    walk_animation.loops = false;

    const int BG_WIDTH = BG_LAYER0_PNG_WIDTH;
    const int BG_HEIGHT = BG_LAYER0_PNG_HEIGHT;
    const uint16_t* bg_data = bg_layer0_png_data;
    float bg_scroll_offset = 0.0f;
    float bg_scroll_target_offset = 0.0f;
    const float BG_SCROLL_SPEED = 35.0f;
    const float SCROLL_STEP_PER_FRAME = 1.0f;
    bool is_scrolling = false;

    bool quit = false;
    SDL_Event e;
    PlayerState current_state = STATE_IDLE;
    Animation* active_anim = &idle_animation;
    int current_anim_frame_idx = 0;
    Uint32 last_anim_update_time = 0;
    int queued_steps = 0;

    SDL_Log("--- Entering main loop ---");
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE && e.key.repeat == 0) {
                if (queued_steps < MAX_QUEUED_STEPS) {
                    queued_steps++;
                    SDL_Log("SPACE Down - Step Queued (%d total)", queued_steps);
                } else {
                    SDL_Log("Step queue full (%d). Input ignored.", queued_steps);
                }
            }
        }

        if (current_state == STATE_IDLE) {
            if (queued_steps > 0) {
                SDL_Log("Switching to WALKING state");
                current_state = STATE_WALKING;
                active_anim = &walk_animation;
                current_anim_frame_idx = 0;
                last_anim_update_time = SDL_GetTicks();

                bg_scroll_target_offset = bg_scroll_offset + BG_SCROLL_SPEED;
                if (bg_scroll_target_offset >= static_cast<float>(BG_WIDTH)) {
                    bg_scroll_target_offset = std::fmod(bg_scroll_target_offset, static_cast<float>(BG_WIDTH));
                }
                is_scrolling = true;
            } else {
                active_anim = &idle_animation;
            }
        } else {
            active_anim = &walk_animation;
        }

        Uint32 current_time = SDL_GetTicks();
        bool animation_cycle_finished = false;

        if (active_anim && !active_anim->frames.empty() && !active_anim->frame_durations_ms.empty()) {
            if (current_anim_frame_idx >= active_anim->frames.size() || current_anim_frame_idx >= active_anim->frame_durations_ms.size()) {
                current_anim_frame_idx = 0;
                if (active_anim->frames.empty() || active_anim->frame_durations_ms.empty()) { continue; }
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

        if (is_scrolling) {
            if (bg_scroll_offset < bg_scroll_target_offset) {
                bg_scroll_offset += SCROLL_STEP_PER_FRAME;
                if (bg_scroll_offset > bg_scroll_target_offset) {
                    bg_scroll_offset = bg_scroll_target_offset;
                    is_scrolling = false;
                }
            } else if (bg_scroll_offset > bg_scroll_target_offset) {
                bg_scroll_offset += SCROLL_STEP_PER_FRAME;
                if (bg_scroll_offset >= static_cast<float>(BG_WIDTH)) {
                    bg_scroll_offset = std::fmod(bg_scroll_offset, static_cast<float>(BG_WIDTH));
                    if (bg_scroll_offset >= bg_scroll_target_offset) {
                        is_scrolling = false;
                    }
                }
            }
        }

        if (current_state == STATE_WALKING && animation_cycle_finished) {
            queued_steps--;
            SDL_Log("Walk cycle finished. Steps remaining in queue: %d", queued_steps);
            if (queued_steps > 0) {
                current_anim_frame_idx = 0;
                last_anim_update_time = SDL_GetTicks();
                SDL_Log("Starting next queued walk cycle.");

                bg_scroll_target_offset = bg_scroll_offset + BG_SCROLL_SPEED;
                if (bg_scroll_target_offset >= static_cast<float>(BG_WIDTH)) {
                    bg_scroll_target_offset = std::fmod(bg_scroll_target_offset, static_cast<float>(BG_WIDTH));
                }
                is_scrolling = true;
            } else {
                SDL_Log("No more steps queued, switching to IDLE.");
                current_state = STATE_IDLE;
                active_anim = &idle_animation;
                current_anim_frame_idx = 0;
                last_anim_update_time = SDL_GetTicks();
            }
        }

        display.clear(0x0000);

        int src_x = static_cast<int>(bg_scroll_offset);
        int remaining_width = BG_WIDTH - src_x;
        if (remaining_width >= WINDOW_WIDTH) {
            display.drawPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, bg_data, BG_WIDTH, BG_HEIGHT, src_x, 0);
        } else {
            int w1 = remaining_width; int w2 = WINDOW_WIDTH - w1;
            if (w1 > 0) { display.drawPixels(0, 0, w1, WINDOW_HEIGHT, bg_data, BG_WIDTH, BG_HEIGHT, src_x, 0); }
            if (w2 > 0) { display.drawPixels(w1, 0, w2, WINDOW_HEIGHT, bg_data, BG_WIDTH, BG_HEIGHT, 0, 0); }
        }

        if (active_anim && current_anim_frame_idx < active_anim->frames.size()) {
            const SpriteFrame& current_sprite_frame = active_anim->frames[current_anim_frame_idx];
            int draw_x = (WINDOW_WIDTH / 2) - (current_sprite_frame.width / 2);
            int draw_y = (WINDOW_HEIGHT / 2) - (current_sprite_frame.height / 2);
            display.drawPixels(draw_x, draw_y, current_sprite_frame.width, current_sprite_frame.height,
                               current_sprite_frame.data, current_sprite_frame.width, current_sprite_frame.height, 0, 0);
        }

        display.present();
        SDL_Delay(16);
    }

    SDL_Log("--- Exited main loop ---");
    display.close();
    SDL_Log("--- display.close() called ---");
    SDL_Log("--- Simulator finished ---");
    return 0;
}
