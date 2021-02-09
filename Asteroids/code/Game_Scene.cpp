/*
 * GAME SCENE
 * Copyright © 2018+ Ángel Rodríguez Ballesteros
 *
 * Distributed under the Boost Software License, version  1.0
 * See documents/LICENSE.TXT or www.boost.org/LICENSE_1_0.txt
 *
 * angel.rodriguez@esne.edu
 */

#include "Game_Scene.hpp"

#include <cstdlib>
#include <basics/Canvas>
#include <basics/Director>

using namespace basics;
using namespace std;

namespace example
{
    Game_Scene::Texture_Data Game_Scene::textures_data[] =
    {
        { ID(loading),    "game-scene/loading_bar.png"         },
        { ID(background), "game-scene/background.png"          },
        { ID(h_bar),      "game-scene/borders/h_bar.png"       },
        { ID(v_bar),      "game-scene/borders/v_bar.png"       },
        { ID(r_arrow),    "game-scene/up_arrow.png"            },
        { ID(l_arrow),    "game-scene/up_arrow.png"            },
        { ID(up_arrow),   "game-scene/up_arrow.png"            },
        { ID(Red_Button), "game-scene/joysticks/red_button.png"},
        { ID(asteroid),   "game-scene/asteroid.png"            },
        { ID(mini_asteroid),   "game-scene/mini_asteroid.png"            },
        { ID(ship),       "game-scene/ship.png"                },
        { ID(bullet),     "game-scene/bullet.png"              },
        { ID(blue),       "game-scene/blue.png"                },
    };

    unsigned Game_Scene::textures_count = sizeof(textures_data) / sizeof(Texture_Data);

    constexpr float Game_Scene:: asteroid_speed;
    constexpr float Game_Scene::     ship_speed;
    constexpr float Game_Scene::   bullet_speed;

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
                case ID(touch-moved):
                {
                    finger_position_x = *event[ID(x)].as< var::Float > ();
                    finger_position_y = *event[ID(y)].as< var::Float > ();
                    screen_touched = true;
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
                else
                {
                    state = ERROR;
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
        Sprite_Handle    background(new Sprite( textures[ID     (background)].get () ));
        Sprite_Handle            top_bar(new Sprite( textures[ID     (h_bar)].get () ));
        Sprite_Handle         bottom_bar(new Sprite( textures[ID     (h_bar)].get () ));
        Sprite_Handle          right_bar(new Sprite( textures[ID     (v_bar)].get () ));
        Sprite_Handle           left_bar(new Sprite( textures[ID     (v_bar)].get () ));
        Sprite_Handle            r_arrow(new Sprite( textures[ID   (r_arrow)].get () ));
        Sprite_Handle            l_arrow(new Sprite( textures[ID   (l_arrow)].get () ));
        Sprite_Handle           up_arrow(new Sprite( textures[ID  (up_arrow)].get () ));
        Sprite_Handle         red_button(new Sprite( textures[ID(Red_Button)].get () ));
        Sprite_Handle    asteroid_handle(new Sprite( textures[ID  (asteroid)].get () ));
        Sprite_Handle              ship_handle(new Sprite( textures[ID(ship)].get () ));
        Sprite_Handle          bullet_handle(new Sprite( textures[ID(bullet)].get () ));

        Sprite_Handle              blue_handle_4(new Sprite( textures[ID(blue)].get () ));
        Sprite_Handle              blue_handle_3(new Sprite( textures[ID(blue)].get () ));
        Sprite_Handle              blue_handle_2(new Sprite( textures[ID(blue)].get () ));
        Sprite_Handle              blue_handle_1(new Sprite( textures[ID(blue)].get () ));

        background->set_anchor                                                             (CENTER);
        background->set_position                         ({ canvas_width / 2, canvas_height / 2 });
        top_bar->set_anchor                                                          (CENTER | TOP);
        top_bar->set_position                                ({ canvas_width / 2, canvas_height });
        bottom_bar->set_anchor                                                    (CENTER | BOTTOM);
        bottom_bar->set_position                                         ({ canvas_width / 2, 0 });
        right_bar->set_anchor                                                      (CENTER | RIGHT);
        right_bar->set_position                              ({ canvas_width, canvas_height / 2 });
        left_bar->set_anchor                                                        (CENTER | LEFT);
        left_bar->set_position                                          ({ 0, canvas_height / 2 });
        r_arrow->set_anchor                                                         (BOTTOM | LEFT);
        r_arrow->set_position                                                         ({ 30, 30 });
        l_arrow->set_anchor                                                         (BOTTOM | LEFT);
        l_arrow->set_position                                                        ({ 200, 30 });
        up_arrow->set_anchor                                                       (BOTTOM | RIGHT);
        up_arrow->set_position                                                      ({ 1000, 30 });
        red_button->set_anchor                                                     (BOTTOM | RIGHT);
        red_button->set_position                                                     ({ 1250, 30 });

        sprites.push_back      (background);
        sprites.push_back         (top_bar);
        sprites.push_back      (bottom_bar);
        sprites.push_back       (right_bar);
        sprites.push_back        (left_bar);
        sprites.push_back         (r_arrow);
        sprites.push_back         (l_arrow);
        sprites.push_back        (up_arrow);
        sprites.push_back      (red_button);
        sprites.push_back (asteroid_handle);
        sprites.push_back     (ship_handle);
        sprites.push_back   (bullet_handle);
        sprites.push_back   (blue_handle_4);
        sprites.push_back   (blue_handle_3);
        sprites.push_back   (blue_handle_2);
        sprites.push_back   (blue_handle_1);

        top_border    =      background.get ();
        top_border    =         top_bar.get ();
        bottom_border =      bottom_bar.get ();
        right_border  =       right_bar.get ();
        left_border   =        left_bar.get ();
        right_arrow   =         r_arrow.get ();
        left_arrow    =         l_arrow.get ();
        Uparrow       =        up_arrow.get ();
        r_button      =      red_button.get ();
        asteroid      = asteroid_handle.get ();
        ship          =     ship_handle.get ();
        bullet        =   bullet_handle.get ();
        blue_4        =   blue_handle_4.get ();
        blue_3        =   blue_handle_3.get ();
        blue_2        =   blue_handle_2.get ();
        blue_1        =   blue_handle_1.get ();
    }

    void Game_Scene::restart_game()
    {
        blue_4->set_position ({ canvas_width - (r_button->get_height() / 2) - 27,  (r_button->get_width() / 2) + 27 });
        blue_3->set_position  ({ canvas_width - (Uparrow->get_height() / 2) - 280, (Uparrow->get_width() / 2) + 35 });
        blue_2->set_position  ({ canvas_width - (right_arrow->get_height() / 2) - 880, (right_arrow->get_width() / 2) + 35 });
        blue_1->set_position  ({ canvas_width - (left_arrow->get_height() / 2) - 1050, (left_arrow->get_width() / 2) + 35 });

        ship->set_position ({ canvas_width - (canvas_width / 4.f), canvas_height / 2 });

        bullet->set_position ({ -100 , 0  });

        asteroid->set_position ({ canvas_width / 4.f, canvas_height / 2.f });
        asteroid->set_speed    ({ 0.f, 0.f });

        screen_touched = false;

        gameplay = WAITING_TO_START;
    }

    void Game_Scene::start_playing ()
    {
        /*
        Vector2f random_direction
        (
            float(rand () % int(canvas_width ) - int(canvas_width  / 2)),
            float(rand () % int(canvas_height) - int(canvas_height / 2))
        );

        asteroid->set_speed (random_direction.normalized () * asteroid_speed);
         */

        ship->set_speed_x(-200);
        ship->set_speed_y( 50);

        gameplay = PLAYING;
    }

    void Game_Scene::run_simulation (float time)
    {
        for (auto & sprite : sprites)
        {
            sprite->update (time);
        }

        //update_movement_button ();
        update_red_button ();
        check_ship_collisions ();
        check_bullet_collisions ();
        //check_asteroid_collisions ();
    }

    void Game_Scene::update_movement_button ()
    {
        //float angleInDegrees;

        float thrust_x;
        float thrust_y;

        //ship->set_speed_x (+thrust_x);

        //ship->set_speed_y (+gravity);


        float RotateAngle = 360;
        float RadiansRotate = (RotateAngle * 3.14159265359f) / 180;

        delta_x_mov_button = finger_position_x - (canvas_width - (Uparrow->get_height() / 2) - 280);
        delta_y_mov_button = finger_position_y - ((Uparrow->get_width() / 2) + 35);

        delta_x_r_button = finger_position_x - (canvas_width - (right_arrow->get_height() / 2) - 880);
        delta_y_r_button = finger_position_y - ((right_arrow->get_width() / 2) + 35);

        delta_x_l_button = finger_position_x - (canvas_width - (left_arrow->get_height() / 2) - 1050);
        delta_y_l_button = finger_position_y - ((left_arrow->get_width() / 2) + 35);

        if(screen_touched == true)
        {
            if ((abs(delta_x_mov_button) < 50) && (abs(delta_y_mov_button) < 50))
            {
                //ship->set_speed_y (+ship_speed);

                //thrust_x = ship_speed * cos (ship->get_width() - (ship->get_width() / 2) - (ship_angle_right));
                //thrust_y = thrust_power * cos (ship_angle_left);

                //ship->set_speed_x (+thrust_x);

                //ship->set_speed_y (+gravity);

                //ship_angle_right -= rotation_speed;


                //int vector_x = ship->get_width() - (ship->get_width() / 2);
                //int vector_y = ship->get_height() - (ship->get_height() / 2);

                //Vector2f ship_direction ( vector_x, vector_y );

                //ship->set_speed (ship_direction.normalized () * ship_speed);


                //asteroid->set_position_y(ship->get_width() * cos(RadiansRotate));




                /*
                int x_toDouble = pow((double)vector_x, 2); //type casting from int to double
                int y_toDouble = pow((double)vector_y, 2);

                int d = abs( sqrt( (double)x_toDouble + (double)y_toDouble ) );

                int angleInRadian = atan2((double)vector_x,(double)vector_y); //angle in radian
                int angleInDegree = angleInRadian * 180 / 3.14159f; //angle in degree
                 */
            }

            if ((abs(delta_x_r_button) < 50) && (abs(delta_y_r_button) < 50))
            {
                //ship->set_speed_x (+ship_speed);
                ship_angle_right -= rotation_speed;

                //ship->get_position() = ship->get_position() - ship->get_width() * cos(RadiansRotate) + ship->get_height() * sin(RadiansRotate);
            }

            if ((abs(delta_x_l_button) < 50) && (abs(delta_y_l_button) < 50))
            {
                //ship->set_speed_x (-ship_speed);
                ship_angle_left += rotation_speed;
            }
        }
        else
        {
            ship->set_speed_x (0.f);
            ship->set_speed_y (0.f);
        }
    }

    void Game_Scene::update_red_button ()
    {
        delta_x_red_button = finger_position_x - (canvas_width - (r_button->get_height() / 2) - 27);
        delta_y_red_button = finger_position_y - ((r_button->get_width() / 2) + 27);

        if(screen_touched == true)
        {
            if ((abs(delta_x_red_button) < 50) && (abs(delta_y_red_button) < 50))
            {
                //ship->set_speed_y (-ship_speed);

                bullet->set_position ({ ship->get_position_x (), ship->get_position_y () }); //donde spawnea
                bullet->set_speed    ({ bullet_speed, 0.f });
            }
        }
    }

    void Game_Scene::check_ship_collisions ()
    {
        if (ship->intersects (*top_border))
        {
            ship->set_position_y (bottom_border->get_top_y() + 40);
        }
        else if (ship->intersects (*bottom_border))
        {
            ship->set_position_y (top_border->get_bottom_y () - 40);
        }

        if (ship->intersects (*right_border))
        {
            ship->set_position_x (left_border->get_right_x() + 40);
        }
        else if (ship->intersects (*left_border))
        {
            ship->set_position_x (right_border->get_left_x() - 40);
        }
    }

    void Game_Scene::check_bullet_collisions ()
    {
        if (bullet->intersects (*ship))
        {
            //spawn_mini_asteroids();
        }

        if (bullet->intersects (*top_border))
        {
            bullet->set_position_y (bottom_border->get_top_y() + 40);
        }
        else if (bullet->intersects (*bottom_border))
        {
            bullet->set_position_y (top_border->get_bottom_y () - 40);
        }

        if (bullet->intersects (*right_border))
        {
            bullet->set_position_x (left_border->get_right_x() + 40);
        }
        else if (bullet->intersects (*left_border))
        {
            bullet->set_position_x (right_border->get_left_x() - 40);
        }
    }

    void Game_Scene::check_asteroid_collisions ()
    {
        if (asteroid->intersects (*ship))
        {
            restart_game();
        }

        if (asteroid->intersects (*top_border))
        {
            asteroid->set_position_y (bottom_border->get_top_y() + 20);
        }
        else if (asteroid->intersects (*bottom_border))
        {
            asteroid->set_position_y (top_border->get_bottom_y () - 20);
        }

        if (asteroid->intersects (*right_border))
        {
            asteroid->set_position_x (left_border->get_right_x() + 20);
        }
        else if (asteroid->intersects (*left_border))
        {
            asteroid->set_position_x (right_border->get_left_x() - 20);
        }
    }

    void Game_Scene::spawn_mini_asteroids ()
    {
        /*
        int ast_width = rand () % int(asteroid->get_width() / 2);
        int ast_height = rand () % int(asteroid->get_width() / 2);

        Sprite_Handle          mini_asteroid(new Sprite( textures[ID(mini_asteroid)].get () ));

        mini_asteroid->set_position ({  asteroid->get_position_x (), asteroid->get_position_y () });

        Vector2f ast_direction (float(rand () % int(ast_width)), float(rand () % int(ast_height)));

        mini_asteroid->set_speed (ast_direction.normalized () * asteroid_speed);

        mini_asteroid->set_speed    ({ 0.f, 0.f });

        bullet->set_position ({ ship->get_position_x (), ship->get_position_y () }); //donde spawnea
        bullet->set_speed    ({ 100.f, 0.f });
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