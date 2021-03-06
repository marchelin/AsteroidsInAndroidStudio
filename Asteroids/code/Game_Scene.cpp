/*
 * GAME SCENE
 * Copyright © 2021+ Marcelo López de lErma
 *
 * Distributed under the Boost Software License, version  1.0
 * See documents/LICENSE.TXT or www.boost.org/LICENSE_1_0.txt
 *
 * marcelolopezdelerma@gmail.com
 */

#include "Game_Scene.hpp"
#include "Menu_Scene.hpp"

#include <cstdlib>
#include <basics/Canvas>
#include <basics/Director>
#include <cmath>

using namespace basics;
using namespace std;

namespace example
{
    // ---------------------------------------------------------------------------------------------
    // ID y ruta de las texturas que se deben cargar para esta escena. La textura con el mensaje de
    // carga está la primera para poder dibujarla cuanto antes:

    Game_Scene::Texture_Data Game_Scene::textures_data[] =
    {
        { ID(loading),      "game-scene/loading_bar.png"                },
        { ID(background),   "game-scene/background.png"                 },
        { ID(h_bar),        "game-scene/borders/h_bar.png"              },
        { ID(v_bar),        "game-scene/borders/v_bar.png"              },
        { ID(r_arrow),      "game-scene/arrows/left_arrow.png"          },
        { ID(l_arrow),      "game-scene/arrows/right_arrow.png"         },
        { ID(up_arrow),     "game-scene/arrows/up_arrow.png"            },
        { ID(Red_Button),   "game-scene/joysticks/red_button.png"       },

        { ID(asteroid_one),     "game-scene/asteroid.png"               },
        { ID(asteroid_two),     "game-scene/asteroid.png"               },

        { ID(mini_asteroid),"game-scene/mini_asteroid.png"              },
        { ID(ship),         "game-scene/ship.png"                       },
        { ID(bullet),       "game-scene/bullet.png"                     },

        { ID(heart_1),        "game-scene/heart.png"                    },
        { ID(heart_2),        "game-scene/heart.png"                    },
        { ID(heart_3),        "game-scene/heart.png"                    },

        { ID(pause_icon),   "game-scene/pause_icon.png"                 },
        { ID(pause_menu),   "game-scene/pause-menu/pause_menu.png"      },
        { ID(resume_button),   "game-scene/pause-menu/resume_button.png"},
        { ID(exit_button),   "game-scene/pause-menu/exit_button.png"    },
        { ID(blue),   "game-scene/sprites_pruebas/blue.png"             },
        { ID(yellow),   "game-scene/sprites_pruebas/yellow.png"         },
    };

    // Para determinar el número de items en el array textures_data, se divide el tamaño en bytes
    // del array completo entre el tamaaño±o en bytes de un item:

    unsigned Game_Scene::textures_count = sizeof(textures_data) / sizeof(Texture_Data);

    // ---------------------------------------------------------------------------------------------
    // Definiciones de los atributos estáticos de la clase:

    constexpr float Game_Scene:: asteroid_speed_1;
    constexpr float Game_Scene:: asteroid_speed_2;
    constexpr float Game_Scene::     ship_speed;
    constexpr float Game_Scene::   bullet_speed;

    Game_Scene::Game_Scene()
    {
        // Se establece la resolución virtual (independiente de la resolución virtual del dispositivo).
        // En este caso no se hace ajuste de aspect ratio, por lo que puede haber distorsión cuando
        // el aspect ratio real de la pantalla del dispositivo es distinto.

        canvas_width  = 1280;
        canvas_height =  720;

        // Se inicia la semilla del generador de números aleatorios:

        srand (unsigned(time(nullptr)));

        // Se inicializan otros atributos:

        initialize ();
    }

    // ---------------------------------------------------------------------------------------------
    // Algunos atributos se inicializan en este método en lugar de hacerlo en el constructor porque
    // este método puede ser llamado más veces para restablecer el estado de la escena y el constructor
    // solo se invoca una vez.

    bool Game_Scene::initialize ()
    {
        state     = LOADING;
        suspended = true;
        gameplay  = UNINITIALIZED;

        return true;
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::suspend ()
    {
        suspended = true; // Se marca que la escena ha pasado a primer plano
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::resume ()
    {
        suspended = false; // Se marca que la escena ha pasado a segundo plano
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::handle (Event & event)
    {
        if (state == RUNNING) // Se descartan los eventos cuando la escena está LOADING
        {
            if (gameplay == WAITING_TO_START)
            {
                start_playing (); // Se empieza a jugar cuando el usuario toca la pantalla por primera vez
            }
            else switch (event.id)
            {
                case ID(touch-started): // El usuario toca la pantalla
                case ID(touch-moved): // El usuario mueve se dedo
                {
                    finger_position_x = *event[ID(x)].as< var::Float > (); // Posición X donde pulsa
                    finger_position_y = *event[ID(y)].as< var::Float > (); // Posición Y donde pulsa
                    screen_touched = true; // La pantalla ha sido tocada
                    break;
                }

                case ID(touch-ended): // El usuario deja de tocar la pantalla
                {
                    screen_touched = false; // La pantalla deja de ser tocada
                    break;
                }
            }
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::update (float time)
    {
        if (!suspended) switch (state)
        {
            case LOADING:        load_textures  (); break;
            case RUNNING:    run_simulation (time); break;
            case ERROR:   break;
        }

        if(state == GAME_PAUSED)
        {
            //state = RUNNING;
            //suspended = true;
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::render (Context & context)
    {
        if (!suspended)
        {
            // El canvas se puede haber creado previamente, en cuyo caso solo hay que pedirlo:

            Canvas * canvas = context->get_renderer< Canvas > (ID(canvas));

            // Si no se ha creado previamente, hay que crearlo una vez:

            if (!canvas)
            {
                canvas = Canvas::create (ID(canvas), context, {{ canvas_width, canvas_height }});
            }

            // Si el canvas se ha podido obtener o crear, se puede dibujar con él:

            if (canvas)
            {
                canvas->clear ();

                switch (state)
                {
                    case LOADING:      render_loading   (*canvas); break;
                    case RUNNING:      render_playfield (*canvas); break;
                    case ERROR:   break;
                }
            }
        }
    }

    // ---------------------------------------------------------------------------------------------
    // En este método solo se carga una textura por fotograma para poder pausar la carga si el
    // juego pasa a segundo plano inesperadamente. Otro aspecto interesante es que la carga no
    // comienza hasta que la escena se inicia para así tener la posibilidad de mostrar al usuario
    // que la carga está en curso en lugar de tener una pantalla en negro que no responde durante
    // un tiempo.

    void Game_Scene::load_textures ()
    {
        if (textures.size () < textures_count) // Si quedan texturas por cargar...
        {
            // Las texturas se cargan y se suben al contexto gráfico, por lo que es necesario
            // disponer de uno:

            Graphics_Context::Accessor context = director.lock_graphics_context ();

            if (context)
            {
                // Se carga la siguiente textura (textures.size() indica cuántas llevamos cargadas):

                Texture_Data   & texture_data = textures_data[textures.size ()];
                Texture_Handle & texture      = textures[texture_data.id] = Texture_2D::create (texture_data.id, context, texture_data.path);

                // Se comprueba si la textura se ha podido cargar correctamente:

                if (texture)
                {
                    context->add (texture);
                }
                else
                {
                    state = ERROR;
                }

                // Cuando se han terminado de cargar todas las texturas se pueden crear los sprites que
                // las usarán e iniciar el juego:
            }
        }
        else if (timer.get_elapsed_seconds () > 1.f)    // Si las texturas se han cargado muy rápido
        {                                               // se espera un segundo desde el inicio de
            create_sprites ();                          // la carga antes de pasar al juego para que
            restart_game   ();                          // el mensaje de carga no aparezca y desaparezca
            // demasiado rápido.
            state = RUNNING;
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::create_sprites ()
    {
        // Se crean y configuran los sprites del fondo:

        Sprite_Handle         background(new Sprite( textures[ID(background)].get () ));
        Sprite_Handle            top_bar(new Sprite( textures[ID     (h_bar)].get () ));
        Sprite_Handle         bottom_bar(new Sprite( textures[ID     (h_bar)].get () ));
        Sprite_Handle          right_bar(new Sprite( textures[ID     (v_bar)].get () ));
        Sprite_Handle           left_bar(new Sprite( textures[ID     (v_bar)].get () ));
        Sprite_Handle            r_arrow(new Sprite( textures[ID   (r_arrow)].get () ));
        Sprite_Handle            l_arrow(new Sprite( textures[ID   (l_arrow)].get () ));
        Sprite_Handle           up_arrow(new Sprite( textures[ID  (up_arrow)].get () ));
        Sprite_Handle         red_button(new Sprite( textures[ID(Red_Button)].get () ));
        Sprite_Handle         pause_icon(new Sprite( textures[ID(pause_icon)].get () ));
        Sprite_Handle              heart_1(new Sprite( textures[ID (heart_1)].get () ));
        Sprite_Handle              heart_2(new Sprite( textures[ID (heart_2)].get () ));
        Sprite_Handle              heart_3(new Sprite( textures[ID (heart_3)].get () ));
        Sprite_Handle         blue(new Sprite( textures[ID(blue)].get () ));
        Sprite_Handle         yellow(new Sprite( textures[ID(yellow)].get () ));

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
        red_button->set_position                                                    ({ 1250, 30 });

        sprites.push_back      (background);
        sprites.push_back         (top_bar);
        sprites.push_back      (bottom_bar);
        sprites.push_back       (right_bar);
        sprites.push_back        (left_bar);
        sprites.push_back         (r_arrow);
        sprites.push_back         (l_arrow);
        sprites.push_back        (up_arrow);
        sprites.push_back      (red_button);
        sprites.push_back      (pause_icon);
        sprites.push_back         (heart_1);
        sprites.push_back         (heart_2);
        sprites.push_back         (heart_3);
        sprites.push_back            (blue);
        sprites.push_back          (yellow);

        // Se crea la nave, el asteroide y la bala:

        Sprite_Handle    asteroid_handle(new Sprite( textures[ID  (asteroid_one)].get () ));
        Sprite_Handle    asteroid_handle_2(new Sprite( textures[ID  (asteroid_two)].get () ));
        Sprite_Handle              ship_handle(new Sprite( textures[ID(ship)].get () ));
        Sprite_Handle          bullet_handle(new Sprite( textures[ID(bullet)].get () ));

        sprites.push_back (asteroid_handle);
        sprites.push_back (asteroid_handle_2);
        sprites.push_back     (ship_handle);
        sprites.push_back   (bullet_handle);

        // Se guardan punteros a los sprites que se van a usar frecuentemente:

        top_border    =      background.get ();
        top_border    =         top_bar.get ();
        bottom_border =      bottom_bar.get ();
        right_border  =       right_bar.get ();
        left_border   =        left_bar.get ();
        right_arrow   =         r_arrow.get ();
        left_arrow    =         l_arrow.get ();
        Uparrow       =        up_arrow.get ();
        r_button      =      red_button.get ();
        asteroid_1      = asteroid_handle.get ();
        asteroid_2      = asteroid_handle_2.get ();
        ship          =     ship_handle.get ();
        bullet        =   bullet_handle.get ();
        p_icon        =      pause_icon.get ();
        h_life_1        =       heart_1.get ();
        h_life_2        =       heart_2.get ();
        h_life_3        =       heart_3.get ();
        blue_ball        =         blue.get ();
        yellow_ball        =         yellow.get ();
    }

    // ---------------------------------------------------------------------------------------------
    // Juando el juego se inicia por primera vez o cuando se reinicia porque el jugador pierde, se
    // llama a este método para restablecer la posición o velocidad de los sprites necesarios:

    void Game_Scene::restart_game()
    {
        vidas = 3;
        ast_ColisionaUnaVez = false; ast_ColisionaDosVeces = false; ast_ColisionaTresVeces  = false;
        ast_Seguridad_1 = false; ast_Seguridad_2 = false; ast_Seguridad_3  = false;

        p_icon->set_position ({ (left_border->get_width())  + 50, canvas_height  - (top_border->get_height())  - 50 });

        h_life_1->set_position ({ right_border->get_position_x() - 50, canvas_height  - (top_border->get_height())  - 50 });
        h_life_2->set_position ({ right_border->get_position_x() - 100, canvas_height  - (top_border->get_height())  - 50 });
        h_life_3->set_position ({ right_border->get_position_x() - 150, canvas_height  - (top_border->get_height())  - 50 });

        ship->set_position ({ canvas_width - (canvas_width / 4.f), canvas_height / 2 });

        bullet->set_position ({ -100 , 0  });

        asteroid_1->set_position ({ canvas_width / 4.f, canvas_height / 2.f }); // posicion de inicio
        asteroid_2->set_position ({ canvas_width / 3.f, canvas_height / 8.f }); // posicion de inicio
        //asteroid->set_speed    ({ 0.f, 0.f });

        screen_touched = false;

        gameplay = WAITING_TO_START;
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::start_playing ()
    {
        // Se genera un vector de dirección al azar:

        Vector2f random_direction_1
        (
            float(rand () % int(canvas_width ) - int(canvas_width  / 2)),
            float(rand () % int(canvas_height) - int(canvas_height / 2))
        );
        Vector2f random_direction_2
                (
                        float(rand () % int(canvas_width ) - int(canvas_width  / 2)),
                        float(rand () % int(canvas_height) - int(canvas_height / 2))
                );

        // Se hace unitario el vector y se multiplica un el valor de velocidad para que el vector
        // resultante tenga exactamente esa longitud:

        asteroid_1->set_speed (random_direction_1.normalized () * asteroid_speed_1);
        asteroid_2->set_speed (random_direction_2.normalized () * asteroid_speed_2);

        //asteroid->set_speed    ({ 200.f, 0.f });

        gameplay = PLAYING;
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::run_simulation (float time)
    {
        // Se actualiza el estado de todos los sprites:

        for (auto & sprite : sprites)
        {
            sprite->update (time);
        }

        update_pause_icon ();
        update_movement_button ();
        update_red_button ();

        // Se comprueban las posibles colisiones de la nave y la bala con los bordes y asteroides:

        check_ship_collisions ();
        check_bullet_collisions ();
        check_asteroid_collisions ();
        check_ship_life ();
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::update_pause_icon ()
    {
        // Se calcula la posisión entre la posición del dedo el icono de pausa, spawneando así una
        // el menu de pausa.

        delta_x_pause_icon = finger_position_x - ((left_border->get_width()) + 50);
        delta_y_pause_icon = finger_position_y - ((canvas_height  - (top_border->get_height()) - 50));

        if(screen_touched == true)
        {
            if ((abs(delta_x_pause_icon) < 50) && (abs(delta_y_pause_icon) < 50))
            {
                screen_touched = false;
                render_pause_menu ();
            }
        }
    }

    // ---------------------------------------------------------------------------------------------
    // Se hace que el jugador se mueva hacia arriba, derecha o izquierda según el usuario esté
    // tocando los botones correspondientes respecto de su centro. Cuando el usuario no toca la
    // pantalla se deja al player quieto.


    void Game_Scene::update_movement_button ()
    {
        // Se calcula la posisión entre la posición del dedo y los distintos botones de movimiento,
        // dejando un area de interacción sobre ellos para que se puedan detectar correctamente

        delta_x_mov_button = finger_position_x - (canvas_width - (Uparrow->get_height() / 2) - 280);
        delta_y_mov_button = finger_position_y - ((Uparrow->get_width() / 2) + 35);

        delta_x_r_button = finger_position_x - (canvas_width - (right_arrow->get_height() / 2) - 880);
        delta_y_r_button = finger_position_y - ((right_arrow->get_width() / 2) + 35);

        delta_x_l_button = finger_position_x - (canvas_width - (left_arrow->get_height() / 2) - 1050);
        delta_y_l_button = finger_position_y - ((left_arrow->get_width() / 2) + 35);

        // -----------------------------------------------------------------------------------------

        vector_x_point_ref = (100 - ship->get_position_x() - ship->get_width() / 2);
        vector_y_point_ref = (0 - ship->get_position_y());

        Vector2f vector_ref (vector_x_point_ref, vector_y_point_ref);

        float ProductoVectorial_x = (vector_x_point_ref) * (delta_x_ship_vector);
        float ProductoVectorial_y = (vector_y_point_ref) * (delta_y_ship_vector);
        float ProductoVectorial_total = (ProductoVectorial_x) + (ProductoVectorial_y);

        float raiz_vector_1 = (abs(sqrt(pow((vector_x_point_ref),2.0) + pow((vector_y_point_ref),2.0))));
        float raiz_vector_2 = (abs(sqrt(pow((delta_x_ship_vector),2.0) + pow((delta_y_ship_vector),2.0))));
        float multipicacion_raices_vectores = raiz_vector_1 * raiz_vector_2;

        float cos_alfa = ProductoVectorial_total / multipicacion_raices_vectores;
        float angulo_nave = acos(cos_alfa);

        // -----------------------------------------------------------------------------------------

        if(screen_touched == true)
        {
            if ((abs(delta_x_mov_button) < 50) && (abs(delta_y_mov_button) < 50))
            {
                update_ship_vector ();
            }

            if ((abs(delta_x_r_button) < 50) && (abs(delta_y_r_button) < 50))
            {
                ship->set_speed_y({ -ship_speed / 3});

                //Rotate Ship right
                /*
                if(angulo_nave >= 0 && angulo_nave < 45 )
                {
                    blue_ball->set_position ({ 4000, 2000 });
                }
                else if(angulo_nave >= 45 && angulo_nave < 90 )
                {
                    blue_ball->set_position ({ 2000, 4000 });
                }

                if(angulo_nave < 0 && angulo_nave >= 315 )
                {
                    blue_ball->set_position ({ 4000, -2000 });
                }
                else if(angulo_nave >= 270 && angulo_nave < 315 )
                {
                    blue_ball->set_position ({ 2000, -4000 });
                }
                 */
            }

            if ((abs(delta_x_l_button) < 50) && (abs(delta_y_l_button) < 50))
            {
                ship->set_speed_y({ ship_speed / 3});

                //Rotate Ship left
                /*
                if(angulo_nave >= 90 && angulo_nave < 135 )
                {
                    blue_ball->set_position ({ -2000, 4000 });
                }
                else if(angulo_nave >= 135 && angulo_nave < 180 )
                {
                    blue_ball->set_position ({ -4000, 2000 });
                }

                if(angulo_nave >= 180 && angulo_nave < 225 )
                {
                    blue_ball->set_position ({ -4000, -2000 });
                }
                else if(angulo_nave >= 225 && angulo_nave < 270 )
                {
                    blue_ball->set_position ({ -2000, -4000 });
                }
                 */
            }
        }
        else
        {
            if (delta_x_ship_vector > 0)
            {
                ship->set_speed_x({ ship_speed / 3});
            }
            else if (delta_x_ship_vector < 0)
            {
                ship->set_speed_x({ -ship_speed / 3});
            }

            if (delta_y_ship_vector > 0)
            {
                ship->set_speed_y({ ship_speed / 3});
            }
            else if (delta_x_ship_vector < 0)
            {
                ship->set_speed_y({ -ship_speed / 3});
            }
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::update_ship_vector ()
    {
        blue_ball->hide();   // Punto utilizados para realizar pruebas
        yellow_ball->hide(); // Punto utilizados para realizar pruebas

        blue_ball->set_scale(0.5);
        yellow_ball->set_scale(0.5);

        blue_ball->set_position ({ ship->get_position_x() + ship->get_width() / 2, ship->get_position_y() });
        yellow_ball->set_position ({ ship->get_position_x() - ship->get_width() / 2, ship->get_position_y() });

        delta_x_ship_vector = (ship->get_position_x() + ship->get_width() / 2) - (ship->get_position_x() - ship->get_width() / 2);
        delta_y_ship_vector = (ship->get_position_y()) - (ship->get_position_y());

        Vector2f ship_vector (delta_x_ship_vector, delta_y_ship_vector);

        ship->set_speed (ship_vector.normalized () * ship_speed); // El vector se hace unitario

        if (delta_x_ship_vector > 0)
        {
            ship->set_speed_x({ ship_speed });
        }
        else if (delta_x_ship_vector < 0)
        {
            ship->set_speed_x({ -ship_speed });
        }

        if (delta_y_ship_vector > 0)
        {
            ship->set_speed_y({ ship_speed });
        }
        else if (delta_x_ship_vector < 0)
        {
            ship->set_speed_y({ -ship_speed });
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::update_red_button ()
    {
        // Se calcula la posisión entre la posición del dedo el botón de disparo, creando así una
        // bala en la posición de la nave cada vez que se pulse este.

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

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::check_ship_collisions ()
    {
        // Se comprueba si la nave coliciona con el borde superior, inferior, derecho e izquierdo,
        // en cuyo caso se ajusta su posición para que lo atraviese y aparezca en su borde opuesto.

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

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::check_bullet_collisions ()
    {
        // Se comprueba si la bala coliciona con algún asteroide, entonces se reinicia la partida.

        if (bullet->intersects (*asteroid_1))
        {
            //spawn_mini_asteroids();
            asteroid_1->hide();
            bullet->set_position ({ -100 , 0  }); //Destroy(bullet);
        }
        else if (bullet->intersects (*asteroid_2))
        {
            //spawn_mini_asteroids();
            asteroid_2->hide();
            bullet->set_position ({ -100 , 0  }); //Destroy(bullet);
        }

        // Se comprueba si la bala coliciona con el borde superior, inferior, derecho e izquierdo,
        // en cuyo caso se ajusta su posición para que lo atraviese y aparezca en su borde opuesto.

        if (bullet->intersects (*top_border))
        {
            //bullet->set_position_y (bottom_border->get_top_y() + 40);
            bullet->set_position ({ -100 , 0  }); //Destroy(bullet);
        }
        else if (bullet->intersects (*bottom_border))
        {
            //bullet->set_position_y (top_border->get_bottom_y () - 40);
            bullet->set_position ({ -100 , 0  }); //Destroy(bullet);
        }

        if (bullet->intersects (*right_border))
        {
            //bullet->set_position_x (left_border->get_right_x() + 40);
            bullet->set_position ({ -100 , 0  }); //Destroy(bullet);
        }
        else if (bullet->intersects (*left_border))
        {
            //bullet->set_position_x (right_border->get_left_x() - 40);
            bullet->set_position ({ -100 , 0  }); //Destroy(bullet);
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::check_asteroid_collisions ()
    {
        // Se comprueba si algún asteroide coliciona con la nave.

        if ((asteroid_1->intersects (*ship) || asteroid_2->intersects (*ship)) && ast_Seguridad_3 == false && vidas == 3)
        {
            ast_ColisionaUnaVez = true;
        }
        else if (asteroid_1->intersects (*ship) && ast_Seguridad_2 == true && vidas == 2)
        {
            ast_ColisionaDosVeces = true;
        }
        else if ((asteroid_1->intersects (*ship) || asteroid_2->intersects (*ship)) && ast_Seguridad_1 == true && vidas == 1)
        {
            ast_ColisionaTresVeces = true;
        }

        // Se comprueba si algún asteroide coliciona con algún borde, en cuyo caso se ajusta su
        //  posición para que lo atraviese y aparezca en su borde opuesto.

        if (asteroid_1->intersects (*top_border))
        {
            asteroid_1->set_position_y (bottom_border->get_top_y() + 40);
        }
        else if (asteroid_1->intersects (*bottom_border))
        {
            asteroid_1->set_position_y (top_border->get_bottom_y () - 40);
        }

        if (asteroid_1->intersects (*right_border))
        {
            asteroid_1->set_position_x (left_border->get_right_x() + 40);
        }
        else if (asteroid_1->intersects (*left_border))
        {
            asteroid_1->set_position_x (right_border->get_left_x() - 40);
        }



        if (asteroid_2->intersects (*top_border))
        {
            asteroid_2->set_position_y (bottom_border->get_top_y() + 40);
        }
        else if (asteroid_2->intersects (*bottom_border))
        {
            asteroid_2->set_position_y (top_border->get_bottom_y () - 40);
        }

        if (asteroid_2->intersects (*right_border))
        {
            asteroid_2->set_position_x (left_border->get_right_x() + 40);
        }
        else if (asteroid_2->intersects (*left_border))
        {
            asteroid_2->set_position_x (right_border->get_left_x() - 40);
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::check_ship_life () {
        // Se comprueba si la nave debe perder sus vidas hasta reinicia la partida.

        if (ast_ColisionaUnaVez)
        {
            vidas = 2;
            h_life_3->hide();

            ast_Seguridad_3 = true; // impide volver a entrar en el mismo
            ast_ColisionaUnaVez = false;

            ast_Seguridad_2 = true;
        }
        else if (ast_ColisionaDosVeces)
        {
            vidas = 1;
            h_life_2->hide();

            ast_Seguridad_2 = false; // impide volver a entrar en el mismo
            ast_ColisionaDosVeces = false;

            ast_Seguridad_1 = true; // permite avanzar una posición
        }
        else if (ast_ColisionaTresVeces)
        {
            vidas = 0;
            h_life_1->hide();

            ast_Seguridad_1 = false; // impide volver a entrar en el mismo
            ast_ColisionaTresVeces = false; // permite avanzar una posición

            restart_game();
        }
    }

    // ---------------------------------------------------------------------------------------------

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

    // ---------------------------------------------------------------------------------------------

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

    // ---------------------------------------------------------------------------------------------
    // Simplemente se dibujan todos los sprites que conforman la escena.

    void Game_Scene::render_playfield (Canvas & canvas)
    {
        for (auto & sprite : sprites)
        {
            sprite->render (canvas);
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::render_pause_menu ()
    {
        // Se crean los Sprites necesarios para el menu de pausa:

        Sprite_Handle         pause_menu(new Sprite( textures[ID(pause_menu)].get () ));
        Sprite_Handle         resume_button(new Sprite( textures[ID(resume_button)].get () ));
        Sprite_Handle         exit_button(new Sprite( textures[ID(exit_button)].get () ));

        sprites.push_back      (pause_menu);
        sprites.push_back      (resume_button);
        sprites.push_back      (exit_button);

        p_menu = pause_menu.get ();
        res_button = resume_button.get ();
        ext_button = exit_button.get ();

        p_menu->set_position ({ canvas_width / 2, canvas_height  / 2 });
        res_button->set_position ({ canvas_width / 2, canvas_height  / 2 });
        ext_button->set_position ({ canvas_width / 2, canvas_height  / 4 });

        // Se comprueba si el usuario quiere seguir jugando o salir:

        if(screen_touched == false)
        {
            //state = GAME_PAUSED;

            delta_x_res_button = finger_position_x - (canvas_width / 2);
            delta_y_res_button = finger_position_y - (canvas_height  / 2);

            if ((abs(delta_x_res_button) < 150) && (abs(delta_y_res_button) < 50))
            {
                //resume();
                //state = RUNNING;
                director.run_scene (shared_ptr< Scene >(new Game_Scene));
            }

            delta_x_ext_button = finger_position_x - (canvas_width / 2);
            delta_y_ext_button = finger_position_y - (canvas_height  / 2);

            if ((abs(delta_x_ext_button) < 150) && (abs(delta_y_ext_button) < 50))
            {
                //suspend ();
                director.run_scene (shared_ptr< Scene >(new Menu_Scene));
            }
        }
    }
}