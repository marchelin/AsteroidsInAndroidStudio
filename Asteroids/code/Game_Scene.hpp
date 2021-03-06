/*
 * GAME SCENE
 * Copyright © 2021+ Marcelo López de lErma
 *
 * Distributed under the Boost Software License, version  1.0
 * See documents/LICENSE.TXT or www.boost.org/LICENSE_1_0.txt
 *
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
            // Estos typedefs pueden ayudar a hacer el código más compacto y claro:

            typedef std::shared_ptr < Sprite     >     Sprite_Handle;
            typedef std::list< Sprite_Handle     >     Sprite_List;
            typedef std::shared_ptr< Texture_2D  >     Texture_Handle;
            typedef std::map< Id, Texture_Handle >     Texture_Map;
            typedef basics::Graphics_Context::Accessor Context;

            /**
             * Representa el estado de la escena en su conjunto.
             */
            enum State
            {
                LOADING,
                RUNNING,
                GAME_PAUSED,
                ERROR
            };

            /**
             * Representa el estado del juego cuando el estado de la escena es RUNNING.
             */
            enum Gameplay_State
            {
                UNINITIALIZED,
                WAITING_TO_START,
                PLAYING,
                GAME_OVER,
            };

        private:

            /**
             * Array de estructuras con la información de las texturas (Id y ruta) que hay que cargar.
             */
            static struct   Texture_Data { Id id; const char * path; } textures_data[];

            /**
             * Número de items que hay en el array textures_data.
             */
            static unsigned textures_count;

        private:

            static constexpr float asteroid_speed_1 =  200.f;      ///< Velocidad a la que se mueve el asteroide_1 (en unideades virtuales por segundo).
            static constexpr float asteroid_speed_2 =  100.f;      ///< Velocidad a la que se mueve el asteroide_2 (en unideades virtuales por segundo).
            static constexpr float     ship_speed   =  500.f;      ///< Velocidad a la que se mueve la nave (en unideades virtuales por segundo).
            static constexpr float   bullet_speed   = 2000.f;      ///< Velocidad a la que se mueve la bala (en unideades virtuales por segundo).

            static constexpr float     gravity = 20.f;
            static constexpr float     rotation_speed = 10.f;
            static constexpr float     thrust_power = 100.f;

        private:

            State          state;                               ///< Estado de la escena.
            Gameplay_State gameplay;                            ///< Estado del juego cuando la escena está RUNNING.
            bool           suspended;                           ///< true cuando la escena está en segundo plano y viceversa.

            unsigned       canvas_width;                        ///< Ancho de la resolución virtual usada para dibujar.
            unsigned       canvas_height;                       ///< Alto  de la resolución virtual usada para dibujar.

            Texture_Map    textures;                            ///< Mapa  en el que se guardan shared_ptr a las texturas cargadas.
            Sprite_List    sprites;                             ///< Lista en la que se guardan shared_ptr a los sprites creados.

            Sprite       *    background;                       ///< Puntero al sprite de la lista de sprites que representa fondo de la ezcena.
            Sprite       *    top_border;                       ///< Puntero al sprite de la lista de sprites que representa el borde superior.
            Sprite       * bottom_border;                       ///< Puntero al sprite de la lista de sprites que representa el borde inferior.
            Sprite       *   left_border;                       ///< Puntero al sprite de la lista de sprites que representa el borde izquierdo.
            Sprite       *  right_border;                       ///< Puntero al sprite de la lista de sprites que representa el borde derecho.
            Sprite       *    asteroid_1;                       ///< Puntero al sprite de la lista de sprites que representa el asteroide_1.
            Sprite       *    asteroid_2;                       ///< Puntero al sprite de la lista de sprites que representa el asteroide_1.
            Sprite       *   right_arrow;                       ///< Puntero al sprite de la lista de sprites que representa el botón de girar derecha.
            Sprite       *    left_arrow;                       ///< Puntero al sprite de la lista de sprites que representa el botón de girar izquierda.
            Sprite       *       Uparrow;                       ///< Puntero al sprite de la lista de sprites que representa el botón de moverse.
            Sprite       *      r_button;                       ///< Puntero al sprite de la lista de sprites que representa el botón de disparar.
            Sprite       *          ship;                       ///< Puntero al sprite de la lista de sprites que representa la nave.
            Sprite       *        bullet;                       ///< Puntero al sprite de la lista de sprites que representa la bala.
            Sprite       *      h_life_1;                       ///< Puntero al sprite de la lista de sprites que representa la vida nº 1.
            Sprite       *      h_life_2;                       ///< Puntero al sprite de la lista de sprites que representa la vida nº 2.
            Sprite       *      h_life_3;                       ///< Puntero al sprite de la lista de sprites que representa la vida nº 3.
            Sprite       *     blue_ball;                       ///< Puntero al sprite de la lista de sprites que representa la bala.
            Sprite       *     yellow_ball;                       ///< Puntero al sprite de la lista de sprites que representa la bala.

            Sprite       *        p_icon;                       ///< Puntero al sprite de la lista de sprites que representa el icono de pausa.
            Sprite       *        p_menu;                       ///< Puntero al sprite de la lista de sprites que representa fondo del menú de pausa.
            Sprite       *    res_button;                       ///< Puntero al sprite de la lista de sprites que representa el botón de resume.
            Sprite       *    ext_button;                       ///< Puntero al sprite de la lista de sprites que representa el botón de exit.

            int               vidas = 3;                        ///< Número de vidas de jugador inicialmente.
            bool    ast_ColisionaUnaVez = false;                ///< false hasta que el asteroide no colisione 1 vez con la nave.
            bool  ast_ColisionaDosVeces = false;                ///< false hasta que el asteroide no colisione 2 vez con la nave.
            bool ast_ColisionaTresVeces = false;                ///< false hasta que el asteroide no colisione 3 vez con la nave.
            bool        ast_Seguridad_1 = false;                ///< nivel de seguridad 1 es false hasta que no se reinicie el juego.
            bool        ast_Seguridad_2 = false;                ///< nivel de seguridad 2 es false hasta que no se reinicie el juego.
            bool        ast_Seguridad_3 = false;                ///< nivel de seguridad 3 es false hasta que no se reinicie el juego.

            bool   screen_touched_mov;                          ///< true si el usuario está tocando mientras se mueve por la pantalla.
            bool       screen_touched;                          ///< true si el usuario está tocando la pantalla.
            float   finger_position_x;                          ///< Coordenada X hacia donde toca la pantalla el usuario.
            float   finger_position_y;                          ///< Coordenada Y hacia donde toca la pantalla el usuario.

            float ship_angle_right;                             ///< Ángulo de giro hacia la derecha de la nave.
            float  ship_angle_left;                             ///< Ángulo de giro hacia la izquierda de la nave.

            float  vector_x_point_ref;                           ///< Vector x desde el punto de referencia (100, 0).
            float  vector_y_point_ref;                           ///< Vector x desde el punto de referencia (100, 0).
            float delta_x_ship_vector;                           ///< Distancia entre la punta de la nave y la base en el eje X.
            float delta_y_ship_vector;                           ///< Distancia entre la punta de la nave y la base en el eje Y.
            float  delta_x_pause_icon;                           ///< Distancia entre el dedo del usuario y la posición X del icono de pausa.
            float  delta_y_pause_icon;                           ///< Distancia entre el dedo del usuario y la posición Y del icono de pausa.
            float  delta_x_res_button;                           ///< Distancia entre el dedo del usuario y la posición X del botón de resume.
            float  delta_y_res_button;                           ///< Distancia entre el dedo del usuario y la posición Y del botón de resume.
            float  delta_x_ext_button;                           ///< Distancia entre el dedo del usuario y la posición X del botón de exit.
            float  delta_y_ext_button;                           ///< Distancia entre el dedo del usuario y la posición Y del botón de exit.
            float  delta_x_red_button;                           ///< Distancia entre el dedo del usuario y la posición X del botón rojo.
            float  delta_y_red_button;                           ///< Distancia entre el dedo del usuario y la posición Y del botón rojo.
            float  delta_x_mov_button;                           ///< Distancia entre el dedo del usuario y la posición X del botón de moverse.
            float  delta_y_mov_button;                           ///< Distancia entre el dedo del usuario y la posición Y del botón de moverse.
            float    delta_x_r_button;                           ///< Distancia entre el dedo del usuario y la posición X del botón de girar derecha.
            float    delta_y_r_button;                           ///< Distancia entre el dedo del usuario y la posición Y del botón de girar derecha.
            float    delta_x_l_button;                           ///< Distancia entre el dedo del usuario y la posición X del botón de girar izquierda.
            float    delta_y_l_button;                           ///< Distancia entre el dedo del usuario y la posición Y del botón de girar izquierda.

            Timer          timer;                               ///< Cronómetro usado para medir intervalos de tiempo

        public:

            /**
             * Solo inicializa los atributos que deben estar inicializados la primera vez, cuando se
             * crea la escena desde cero.
             */
            Game_Scene();

            /**
             * Este método lo llama Director para conocer la resolución virtual con la que está
             * trabajando la escena.
             * @return Tamaño en coordenadas virtuales que está usando la escena.
             */
            basics::Size2u get_view_size () override
            {
                return { canvas_width, canvas_height };
            }

            /**
             * Aquí se inicializan los atributos que deben restablecerse cada vez que se inicia la escena.
             * @return
             */
            bool initialize () override;

            /**
             * Este método lo invoca Director automáticamente cuando el juego pasa a segundo plano.
             */
            void suspend () override;

            /**
             * Este método lo invoca Director automáticamente cuando el juego pasa a primer plano.
             */
            void resume () override;

            /**
             * Este método se invoca automáticamente una vez por fotograma cuando se acumulan
             * eventos dirigidos a la escena.
             */
            void handle (basics::Event & event) override;

            /**
             * Este método se invoca automáticamente una vez por fotograma para que la escena
             * actualize su estado.
             */
            void update (float time) override;

            /**
             * Este método se invoca automáticamente una vez por fotograma para que la escena
             * dibuje su contenido.
             */
            void render (Context & context) override;

        private:

            /**
             * En este método se cargan las texturas (una cada fotograma para facilitar que la
             * propia carga se pueda pausar cuando la aplicación pasa a segundo plano).
             */
            void load_textures ();

            /**
             * En este método se crean los sprites cuando termina la carga de texturas.
             */
            void create_sprites ();

            /**
             * Se llama cada vez que se debe reiniciar el juego. En concreto la primera vez y cada
             * vez que un jugador pierde.
             */
            void restart_game ();

            /**
             * Cuando se ha reiniciado el juego y el usuario toca la pantalla por primera vez se
             * pone la bola en movimiento en una dirección al azar.
             */
            void start_playing ();

            /**
             * Actualiza el estado del juego cuando el estado de la escena es RUNNING.
             */
            void run_simulation (float time);

            /**
             * Spawnea el menú de pausa si se pulsa sobre su icono.
             */
            void update_pause_icon ();

            /**
             * Se checkea mediante el input de botones si el usuario quiere moverse.
             */
            void update_movement_button ();

            /**
             * Se calcula el vector forward de la nave.
             */
            void update_ship_vector ();

            /**
             * Ejecuta la función de disparar de la nave.
             */
            void update_red_button ();

            /**
             * Comprueba las colisiones del asteroide con el escenario.
             */
            void check_ship_collisions ();

            /**
             * Comprueba las colisiones del asteroide con el escenario y con los asteroides.
             */
            void check_bullet_collisions ();

            /**
             * Comprueba las colisiones del asteroide con el escenario y con la nave.
             */
            void check_asteroid_collisions ();

            /**
             * Comprueba la nave tiene que perder vidas cuando choca con asteroides.
             */
            void check_ship_life ();

            /**
             * Se spawnean los pequeños sprites de los asteroids si una bala colisiona previamente.
             */
            void spawn_mini_asteroids ();

            /**
             * Dibuja la textura con el mensaje de carga mientras el estado de la escena es LOADING.
             * La textura con el mensaje se carga la primera para mostrar el mensaje cuanto antes.
             * @param canvas Referencia al Canvas con el que dibujar la textura.
             */
            void render_loading (Canvas & canvas);

            /**
             * Dibuja la escena de juego cuando el estado de la escena es RUNNING.
             * @param canvas Referencia al Canvas con el que dibujar.
             */
            void render_playfield (Canvas & canvas);

            /**
             * Dibuja la escena del menu de pausa cuando el estado de la escena es GAME_PAUSED.
             * @param canvas Referencia al Canvas con el que dibujar.
             */
            void render_pause_menu ();
        };
    }

#endif