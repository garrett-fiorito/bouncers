#include <bn_core.h>
#include <bn_backdrop.h>
#include <bn_keypad.h>
#include <bn_sprite_ptr.h>
#include <bn_display.h>
#include <bn_random.h>
#include <bn_vector.h>
#include <bn_log.h>

#include "bn_sprite_items_dot.h"

// Set max/min x position to be the edges of the display
static constexpr int HALF_SCREEN_WIDTH = bn::display::width() / 2;
static constexpr bn::fixed MIN_X = -HALF_SCREEN_WIDTH;
static constexpr bn::fixed MAX_X = HALF_SCREEN_WIDTH;

static constexpr int HALF_SCREEN_HEIGHT = bn::display::height() / 2;
static constexpr bn::fixed MIN_Y = -HALF_SCREEN_HEIGHT;
static constexpr bn::fixed MAX_Y = HALF_SCREEN_HEIGHT;

// Starting speed of a bouncer
static constexpr bn::fixed BASE_SPEED = 2;

// Maximum number of bouncers on screen at once
static constexpr int MAX_BOUNCERS = 20;

// Bouncers have a lifetime of 4 seconds (60 frames per second)
static constexpr int BOUNCER_LIFETIME = 60 * 4;

class Bouncer {
    public:
        bn::sprite_ptr sprite;
        bn::fixed x_speed;
        bn::fixed y_speed;
        int frames_remaining = BOUNCER_LIFETIME;

        Bouncer(bn::fixed x_start, bn::fixed y_start, bn::random& random) : 
            sprite(bn::sprite_items::dot.create_sprite(x_start, y_start)), 
            x_speed(random.get_fixed(-BASE_SPEED, BASE_SPEED)),
            y_speed(random.get_fixed(-BASE_SPEED, BASE_SPEED)) {

            }

        void update() {
            bn::fixed x = sprite.x();
            bn::fixed y = sprite.y();

            x += x_speed;
            y += y_speed;

            // Bounce off the right and left edges
            if(x > MAX_X) {
                x = MAX_X;
                x_speed *= -1;
            }
            if(x < MIN_X) {
                x = MIN_X;
                x_speed *= -1;
            }

            // Bounce off the bottom and top edges
            if(y > MAX_Y) {
                y = MAX_Y;
                y_speed *= -1;
            }
            if(y < MIN_Y) {
                y = MIN_Y;
                y_speed *= -1;
            }

            sprite.set_x(x);
            sprite.set_y(y);
            --frames_remaining;
        }

        bool expired() const {
            return frames_remaining <= 0;
        }

    private:

};


bn::fixed average_x(bn::vector<Bouncer, MAX_BOUNCERS>& bouncers) {
    // Add all x positions together
    bn::fixed x_sum = 0;
    for(Bouncer& b : bouncers) {
        x_sum += b.sprite.x();
    }

    bn::fixed x_average= x_sum;

    // Only divide if we have 1 or more. Prevents division by 0
    if(bouncers.size() > 0) {
        x_average /= bouncers.size();
    }

    return x_average;
}

void add_bouncer(bn::vector<Bouncer, MAX_BOUNCERS>& bouncers, bn::fixed start_x, bn::fixed start_y, bn::random& random) {
    if(bouncers.size() < bouncers.max_size()) {
        bouncers.push_back(Bouncer(start_x, start_y, random));
    }
}

int main() {
        bn::core::init();
        bn::random random;

        // Sprites and x speeds of bouncers. 
        // Items with same index correspond to eachother
        //
        // 

        bn::vector<Bouncer, MAX_BOUNCERS> bouncers = {};

        bn::fixed y_spawn = 0;

        while(true) {
            // if A is pressed add a new bouncer
            if(bn::keypad::a_pressed()) {
                add_bouncer(bouncers, 0, y_spawn, random);
            }

        // if B is pressed print the average to the console
        if(bn::keypad::b_pressed()) {
            bn::fixed x_average = average_x(bouncers);
            BN_LOG("Average x: ", x_average);
        }
            
        // Backwards so removing a bouncer won't skip another one
        for(int i = bouncers.size() - 1; i >= 0; --i) {
            bouncers[i].update();
            if(bouncers[i].expired()) {
                bouncers.erase(bouncers.begin() + i);
            }
        }

            y_spawn += 1;

            if(y_spawn > MAX_Y) {
                y_spawn = MIN_Y;
            }

            bn::core::update();
        }
}