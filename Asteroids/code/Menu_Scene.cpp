/*
 * MENU SCENE
 * Copyright © 2021+ Marcelo López de lErma
 *
 * Distributed under the Boost Software License, version  1.0
 * See documents/LICENSE.TXT or www.boost.org/LICENSE_1_0.txt
 *
 * marcelolopezdelerma@gmail.com
 */

#include "Menu_Scene.hpp"
#include "Game_Scene.hpp"
#include "Help_Scene.hpp"
#include <basics/Canvas>
#include <basics/Director>
#include <basics/Transformation>

using namespace basics;
using namespace std;

namespace example
{
    Menu_Scene::Menu_Scene()
    {
        state         = LOADING;
        suspended     = true;
        canvas_width  = 1280;
        canvas_height =  720;
    }

    bool Menu_Scene::initialize ()
    {
        for (auto & option : options)
        {
            option.is_pressed = false;
        }

        return true;
    }

    void Menu_Scene::handle (basics::Event & event)
    {
        if (state == READY)
        {
            switch (event.id)
            {
                case ID(touch-started):
                case ID(touch-moved):
                {
                    Point2f touch_location = { *event[ID(x)].as< var::Float > (), *event[ID(y)].as< var::Float > () };
                    int     option_touched = option_at (touch_location);

                    for (int index = 0; index < number_of_options; ++index)
                    {
                        options[index].is_pressed = index == option_touched;
                    }

                    break;
                }

                case ID(touch-ended):
                {
                    for (auto & option : options) option.is_pressed = false;

                    Point2f touch_location = { *event[ID(x)].as< var::Float > (), *event[ID(y)].as< var::Float > () };

                    if (option_at (touch_location) == PLAY)
                    {
                        director.run_scene (shared_ptr< Scene >(new Game_Scene));
                    }
                    else if (option_at (touch_location) == HELP)
                    {
                        //director.run_scene (shared_ptr< Scene >(new Help_Scene));
                    }

                    break;
                }
            }
        }
    }

    void Menu_Scene::update (float time)
    {
        if (!suspended) if (state == LOADING)
        {
            Graphics_Context::Accessor context = director.lock_graphics_context ();

            if (context)
            {
                atlas.reset (new Atlas("menu-scene/main-menu.sprites", context));

                state = atlas->good () ? READY : ERROR;

                if (state == READY)
                {
                    configure_options ();
                }
            }
        }
    }

    void Menu_Scene::render (Graphics_Context::Accessor & context)
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

                if (state == READY)
                {
                    for (auto & option : options)
                    {
                        canvas->set_transform
                        (
                            scale_then_translate_2d
                            (
                                  option.is_pressed ? 1.f : 0.85f,
                                { option.position[0], option.position[1] }
                            )
                        );

                        canvas->fill_rectangle ({ 0.f, 0.f }, { option.slice->width, option.slice->height }, option.slice, CENTER | TOP);
                    }

                    canvas->set_transform (Transformation2f());
                }
            }
        }
    }

    void Menu_Scene::configure_options ()
    {
        options[PLAY   ].slice = atlas->get_slice (ID(play)   );
        options[SCORES ].slice = atlas->get_slice (ID(scores) );
        options[HELP   ].slice = atlas->get_slice (ID(help)   );
        options[CREDITS].slice = atlas->get_slice (ID(credits));

        float menu_height = 0;

        for (auto & option : options) menu_height += option.slice->height;

        float option_top = canvas_height / 2.f + menu_height / 2.f;

        for (unsigned index = 0; index < number_of_options; ++index)
        {
            options[index].position = Point2f{ canvas_width / 2.f, option_top };

            option_top -= options[index].slice->height;
        }

        initialize ();
    }

    int Menu_Scene::option_at (const Point2f & point)
    {
        for (int index = 0; index < number_of_options; ++index)
        {
            const Option & option = options[index];

            if
            (
                point[0] > option.position[0] - option.slice->width  &&
                point[0] < option.position[0] + option.slice->width  &&
                point[1] > option.position[1] - option.slice->height &&
                point[1] < option.position[1] + option.slice->height
            )
            {
                return index;
            }
        }

        return -1;
    }
}