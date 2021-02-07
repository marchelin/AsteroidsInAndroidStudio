/*
 * GAME SCENE
 * Copyright � 2021+ Marcelo L�pez de Lerma
 * marcelolopezdelerma@gmail.com
 */

#include "Game_Scene.hpp"

#include <cstdlib>
#include <basics/Canvas>
#include <basics/Director>
#include <math.h>

using namespace basics;
using namespace std;

namespace example
{
    Game_Scene::Texture_Data Game_Scene::textures_data[] =
    {
        { ID(loading),        "game-scene/loading.png"               },
        { ID(background),     "game-scene/background.png"            },
        { ID(top_border),     "game-scene/top_border.png"            },
        { ID(bottom_border),  "game-scene/bottom_border.png"         },
        { ID(right_border),   "game-scene/right_border.png"          },
        { ID(left_border),    "game-scene/left_border.png"           },
        { ID(Handle_Outline), "game-scene/Handles/Red_Button.png"    },
        { ID(Handle_Ridged),  "game-scene/Handles/Handle_Ridged.png" },
        { ID(Red_Button),     "game-scene/Handles/Handle_Outline.png"},
        { ID(asteroid),       "game-scene/asteroid.png"              },
        { ID(ship),           "game-scene/ship.png"                  },
        { ID(bullet),         "game-scene/bullet.png"                },
    };

    unsigned Game_Scene::textures_count = sizeof(textures_data) / sizeof(Texture_Data);

    constexpr float Game_Scene:: asteroid_speed;
    constexpr float Game_Scene::     ship_speed;
    constexpr float Game_Scene::   bullet_speed;
    constexpr float Game_Scene::   Red_Button_speed;

    Game_Scene::Game_Scene()
    {
        canvas_width  = 1280;
        canvas_height =  720;

        srand (unsigned(time(nullptr)));

        initialize ();
    }

    bool Game_Scene::initialize ()
    {
        state     = LOADING;
        suspended = true;
        gameplay  = UNINITIALIZED;

        return true;
    }

    void Game_Scene::suspend ()
    {
        suspended = true;
    }

    void Game_Scene::resume ()
    {
        suspended = false;
    }

    void Game_Scene::handle (Event & event)
    {
        if (state == RUNNING)
        {
            if (gameplay == WAITING_TO_START)
            {
                start_playing ();
            }
            else switch (event.id)
            {
                case ID(touch-started):
                {
                    screen_touched = true;
                    break;
                }
                case ID(touch-moved):
                {
                    finger_position_x = *event[ID(x)].as< var::Float > ();
                    finger_position_y = *event[ID(y)].as< var::Float > ();
                    break;
                }
                case ID(touch-ended):
                {
                    screen_touched = false;
                    break;
                }
            }
        }
    }

    void Game_Scene::update (float time)
    {
        if (!suspended) switch (state)
        {
            case LOADING: load_textures  ();     break;
            case RUNNING: run_simulation (time); break;
            case ERROR:   break;
        }
    }

    void Game_Scene::render (Context & context)
    {
        if (!suspended)
        {
            Canvas * canvas = context->get_renderer< Canvas > (ID(canvas));

            if (!canvas)
            {
                canvas = Canvas::create (ID(canvas), context, {{ canvas_width, canvas_height }});
            }

            if (canvas)
            {
                canvas->clear ();

                switch (state)
                {
                    case LOADING: render_loading   (*canvas); break;
                    case RUNNING: render_playfield (*canvas); break;
                    case ERROR:   break;
                }
            }
        }
    }

    void Game_Scene::load_textures ()
    {
        if (textures.size () < textures_count)
        {
            Graphics_Context::Accessor context = director.lock_graphics_context ();

            if (context)
            {
                Texture_Data   & texture_data = textures_data[textures.size ()];
                Texture_Handle & texture      = textures[texture_data.id] = Texture_2D::create (texture_data.id, context, texture_data.path);

                if (texture)
                {
                    context->add (texture);
                }

            }
        }
        else if (timer.get_elapsed_seconds () > 1.f)
        {
            create_sprites ();
            restart_game   ();

            state = RUNNING;
        }
    }

    void Game_Scene::create_sprites ()
    {
        //float handle_outline_position_x = Handle_Outline->get_position_x ();
        //float handle_outline_position_y = Handle_Outline->get_position_y ();
        //Handle_Ridged->set_position ({ handle_outline_position_x, handle_outline_position_y });

        Sprite_Handle    background(new Sprite( textures[ID(background)].get () ));
        Sprite_Handle    top_bar(new Sprite( textures[ID(top_border)].get () ));
        Sprite_Handle bottom_bar(new Sprite( textures[ID(bottom_border)].get () ));
        Sprite_Handle    right_bar(new Sprite( textures[ID(right_border)].get () ));
        Sprite_Handle left_bar(new Sprite( textures[ID(left_border)].get () ));
        Sprite_Handle handle_outline(new Sprite( textures[ID(Handle_Outline)].get () ));
        Sprite_Handle    handle_ridged(new Sprite( textures[ID(Handle_Ridged)].get () ));
        Sprite_Handle red_button(new Sprite( textures[ID(Red_Button)].get () ));

        background->set_anchor   (CENTER);
        background->set_position ({ canvas_width / 2.f, canvas_height / 2.f });
        top_bar->set_anchor   (CENTER);
        top_bar->set_position ({ canvas_width / 2.f, canvas_height / 2.f });
        bottom_bar->set_anchor   (CENTER);
        bottom_bar->set_position ({ canvas_width / 2.f, canvas_height / 2.f });
        right_bar->set_anchor   (CENTER);
        right_bar->set_position ({ canvas_width / 2.f, canvas_height / 2.f });
        left_bar->set_anchor   (CENTER);
        left_bar->set_position ({ canvas_width / 2.f, canvas_height / 2.f });
        handle_outline->set_anchor   (BOTTOM | RIGHT);
        handle_outline->set_position ({ 1250, 30 });
        handle_ridged->set_anchor   (BOTTOM | RIGHT);
        handle_ridged->set_position ({ 30, 30 });
        red_button->set_anchor   (BOTTOM | LEFT);
        red_button->set_position ({ 30, 30 });

        sprites.push_back (   background);
        sprites.push_back (   top_bar);
        sprites.push_back (bottom_bar);
        sprites.push_back (   right_bar);
        sprites.push_back (left_bar);
        sprites.push_back (handle_outline);
        sprites.push_back (handle_ridged);
        sprites.push_back (red_button);

        bg_image   =             background.get ();
        top_border    =             top_bar.get ();
        bottom_border =          bottom_bar.get ();
        right_border    =             right_bar.get ();
        left_border =          left_bar.get ();
        Handle_Outline =          handle_outline.get ();
        Handle_Ridged =          handle_ridged.get ();
        Red_Button =          red_button.get ();

        Sprite_Handle         bullet_handle(new Sprite( textures[ID(bullet)].get () ));
        Sprite_Handle  ship_handle(new Sprite( textures[ID(ship)].get () ));
        Sprite_Handle         asteroid_handle(new Sprite( textures[ID(asteroid)].get () ));

        sprites.push_back ( bullet_handle);
        sprites.push_back ( ship_handle);
        sprites.push_back ( asteroid_handle);

        bullet = bullet_handle.get ();
        ship     =     ship_handle.get ();
        asteroid = asteroid_handle.get ();
    }

    void Game_Scene::restart_game()
    {
        ship->set_position ({ canvas_width / 2.f, canvas_height / 2.f });
        ship->set_speed    ({ 0.f, 0.f });

        asteroid->set_position ({ 100, 100});
        asteroid->set_speed    ({ 0.1f, 0.1f });

        gameplay = WAITING_TO_START;
    }

    void Game_Scene::start_playing ()
    {
        Vector2f random_direction
        (
            float(rand () % int(canvas_width ) - int(canvas_width  / 2)),
            float(rand () % int(canvas_height) - int(canvas_height / 2))
        );

        asteroid->set_speed (random_direction.normalized () * asteroid_speed);

        gameplay = PLAYING;
    }

    void Game_Scene::run_simulation (float time)
    {
        for (auto & sprite : sprites)
        {
            sprite->update (time);
        }

        update_user ();
        //check_asteroid_collision ();
    }

    float delta_x;
    float delta_y;

    void Game_Scene::update_user ()
    {
        delta_x = finger_position_x - (Red_Button->get_position_x ());
        delta_y = finger_position_y - (Red_Button->get_position_y ());

        if(screen_touched == true)
        {
            if ((abs(delta_x) < 100) && (abs(delta_y) < 100))
            {
                if (delta_x < 0.f)
                {
                    //X_position = asteroid->get_position_x ();
                    Red_Button->set_speed_x (-Red_Button_speed);
                }
                else if (delta_x > 0.f)
                {
                    //X_position = asteroid->get_position_x ();
                    Red_Button->set_speed_x (+Red_Button_speed);
                }

                if (delta_y < 0.f)
                {
                    //Y_position = asteroid->get_position_y ();
                    Red_Button->set_speed_y (-Red_Button_speed);
                }
                else if (delta_y > 0.f)
                {
                    //Y_position = asteroid->get_position_y ();
                    Red_Button->set_speed_y (+Red_Button_speed);
                }
            }
            else
            {
                Red_Button->set_speed_x (0.f);
                Red_Button->set_speed_y (0.f);
            }
        }
        else if(screen_touched == false)
        {
            Red_Button->set_speed_x (0.f);
            Red_Button->set_speed_y (0.f);
        }
    }

    void Game_Scene::sapwn_bullets ()
    {
        float ship_position_x = ship->get_position_x ();
        float ship_position_y = ship->get_position_y ();

        bullet->set_position ({ ship_position_x, ship_position_y }); //donde spawnea
        bullet->set_speed    ({ 1.f, 1.f });

        //bullet->set_position ({ finger_position_x, finger_position_y }); //posicion si dispara
        //bullet->set_speed    ({ 0.5f, 0.5f });
    }

    /*
    void Game_Scene::update_facing_position ()
    {
        float pi = 3.14159f;

        float FaceAngle = atan2(delta_x, delta_y) * 180;
        FaceAngle = FaceAngle / pi;
    }
     */

    void Game_Scene::check_asteroid_collision ()
    {
        bool colisionaConDerecho = false;

        bool touchedX;
        bool touchedNegX;


        //float LeftBorderPosition = ship->get_position_y () - (left_border->get_position_x ()); Borrar

        if (ship->get_position_y () < canvas_height- 600)
        {
            ship->set_speed_y    ((-ship->get_speed_y ()));
        }
        if (ship->get_position_y () > canvas_height- 50)
        {
            ship->set_speed_y    ((+ship->get_speed_y ()));
        }
        /*
        if(ship->get_position_x () < canvas_width - left_border->get_width()) //Left
        {
            //dvd->set_position_x (left_border->get_right_x () + dvd->get_width() / 2.f);
            ship->set_speed_x    (+ship->get_speed_x ());
        }
        */
        if (ship->get_position_x () < canvas_width - 100) //Right 100%
        {
            ship->set_speed_x    (+ship->get_speed_x ());
        }
        /*
        if (asteroid->get_position_y () < canvas_height- 50)
        {
            //dvd->set_position_y (top_border->get_bottom_y () - dvd->get_height() / 2.f);
            asteroid->set_speed_y (+100.f);
        }

        if (asteroid->get_position_y () > 50)
        {
            //dvd->set_position_y (bottom_border->get_top_y () + dvd->get_height() / 2.f);
            asteroid->set_speed_y    (-50.f);
        }

        if(asteroid->get_position_x () < right_border->get_position_x())
        {
            //dvd->set_position_x (right_border->get_left_x () + dvd->get_width() / 2.f);
            asteroid->set_speed_x    (+100.f);
        }
        if(asteroid->get_position_x () > left_border->get_position_x())
        {
            //dvd->set_position_x (left_border->get_right_x () + dvd->get_width() / 2.f);
            asteroid->set_speed_x    (-100.f);
        }
         */

        /*
        if (asteroid->intersects (*ship))
        {
            restart_game();
        }
        if (asteroid->intersects (*bullet))
        {
            restart_game();
        }

        if (ship->intersects (*right_border))
        {
            ship->set_position_y (right_border->get_top_y () + ship->get_height() / 2.f);
        }

        if (asteroid->intersects (*top_border))
        {
            asteroid->set_position_y (top_border->get_top_y () + asteroid->get_height() / 2.f);
        }
        if (asteroid->intersects (*bottom_border))
        {
            asteroid->set_position_y (bottom_border->get_bottom_y () - asteroid->get_height() / 2.f);
        }
        if (asteroid->intersects (*right_border))
        {
            asteroid->set_position_x (right_border->get_left_x () + asteroid->get_width() / 2.f);
        }
        if (asteroid->intersects (*left_border))
        {
            asteroid->set_position_x (left_border->get_right_x () - asteroid->get_width() / 2.f);
        }
         */
    }

    void Game_Scene::render_loading (Canvas & canvas)
    {
        Texture_2D * loading_texture = textures[ID(loading)].get ();

        if (loading_texture)
        {
            canvas.fill_rectangle
            (
                { canvas_width * .5f, canvas_height * .5f },
                { loading_texture->get_width (), loading_texture->get_height () },
                  loading_texture
            );
        }
    }

    void Game_Scene::render_playfield (Canvas & canvas)
    {
        for (auto & sprite : sprites)
        {
            sprite->render (canvas);
        }
    }
}