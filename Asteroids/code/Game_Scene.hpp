/*
 * GAME SCENE
 * Copyright � 2021+ Marcelo L�pez de Lerma
 * marcelolopezdelerma@gmail.com
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
            };

        private:

            static struct   Texture_Data { Id id; const char * path; } textures_data[];

            static unsigned textures_count;

        private:

            static constexpr float   bullet_speed = 400.f;
            static constexpr float   asteroid_speed = 300.f;
            static constexpr float   ship_speed = 2000.f;
            static constexpr float   Red_Button_speed = 2000.f;

            struct Touch
            {
                int32_t id;
            };

        private:

            State          state;
            Gameplay_State gameplay;
            bool           suspended;

            unsigned       canvas_width;
            unsigned       canvas_height;

            Texture_Map    textures;
            Sprite_List    sprites;

            Sprite       * bg_image;
            Sprite       * top_border;
            Sprite       * bottom_border;
            Sprite       * right_border;
            Sprite       * left_border;
            Sprite       * Handle_Outline;
            Sprite       * Handle_Ridged;
            Sprite       * Red_Button;
            Sprite       * ship;
            Sprite       * asteroid;
            Sprite       * bullet;

            bool           Start_redButton;
            bool           screen_touched;
            float          finger_position_x;
            float          finger_position_y;

            Timer          timer;

            Touch    touches[10];

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

            void update_user ();

            void sapwn_bullets ();

            void update_facing_position ();

            void check_asteroid_collision ();

            void render_loading (Canvas & canvas);

            void render_playfield (Canvas & canvas);
        };
    }

#endif