/*
 * GAME SCENE
 * Copyright © 2018+ Ángel Rodríguez Ballesteros
 *
 * Distributed under the Boost Software License, version  1.0
 * See documents/LICENSE.TXT or www.boost.org/LICENSE_1_0.txt
 *
 * angel.rodriguez@esne.edu
 */

#ifndef GAME_SCENE_HEADER
#define GAME_SCENE_HEADER

    #include <map>
    #include <list>
    #include <memory>

    #include <basics/Canvas>
    #include <basics/Id>
    #include <basics/Scene>
    #include <basics/Texture_2D>
    #include <basics/Timer>

    #include "Sprite.hpp"

    namespace example
    {

        using basics::Id;
        using basics::Timer;
        using basics::Canvas;
        using basics::Texture_2D;

        class Game_Scene : public basics::Scene
        {
            typedef std::shared_ptr < Sprite     >     Sprite_Handle;
            typedef std::list< Sprite_Handle     >     Sprite_List;
            typedef std::shared_ptr< Texture_2D  >     Texture_Handle;
            typedef std::map< Id, Texture_Handle >     Texture_Map;
            typedef basics::Graphics_Context::Accessor Context;

            enum State
            {
                LOADING,
                RUNNING,
                ERROR
            };

            enum Gameplay_State
            {
                UNINITIALIZED,
                WAITING_TO_START,
                PLAYING,
                BALL_LEAVING,
            };

        private:

            static struct   Texture_Data { Id id; const char * path; } textures_data[];

            static unsigned textures_count;

        private:

            static constexpr float asteroid_speed =  200.f;
            static constexpr float     ship_speed =  500.f;
            static constexpr float     bullet_speed = 1000.f;

            static constexpr float     gravity = 20.f;
            static constexpr float     rotation_speed = 10.f;
            static constexpr float     thrust_power = 100.f;

        private:

            State              state;
            Gameplay_State  gameplay;
            bool           suspended;

            unsigned        canvas_width;
            unsigned       canvas_height;

            Texture_Map    textures;
            Sprite_List     sprites;

            Sprite       *    background;
            Sprite       *    top_border;
            Sprite       * bottom_border;
            Sprite       *   left_border;
            Sprite       *  right_border;
            Sprite       *      asteroid;
            Sprite       *   right_arrow;
            Sprite       *    left_arrow;
            Sprite       *       Uparrow;
            Sprite       *      r_button;
            Sprite       *          ship;
            Sprite       *        bullet;
            Sprite       *        blue_4;
            Sprite       *        blue_3;
            Sprite       *        blue_2;
            Sprite       *        blue_1;

            bool screen_touched_mov;
            bool     screen_touched;
            float finger_position_x;
            float finger_position_y;

            float ship_angle_right;
            float  ship_angle_left;

            float delta_x_red_button;
            float delta_y_red_button;
            float delta_x_mov_button;
            float delta_y_mov_button;
            float   delta_x_r_button;
            float   delta_y_r_button;
            float   delta_x_l_button;
            float   delta_y_l_button;

            Timer          timer;

        public:

            Game_Scene();

            basics::Size2u get_view_size () override
            {
                return { canvas_width, canvas_height };
            }

            bool initialize () override;

            void suspend () override;

            void resume () override;

            void handle (basics::Event & event) override;

            void update (float time) override;

            void render (Context & context) override;

        private:

            void load_textures ();

            void create_sprites ();

            void restart_game ();

            void start_playing ();

            void run_simulation (float time);

            void update_movement_button ();

            void update_red_button ();

            void check_ship_collisions ();

            void check_bullet_collisions ();

            void check_asteroid_collisions ();

            void spawn_mini_asteroids ();

            void render_loading (Canvas & canvas);

            void render_playfield (Canvas & canvas);
        };
    }

#endif