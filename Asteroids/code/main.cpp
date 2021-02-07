/*
 * MAIN
 * Copyright © 2018+ Ángel Rodríguez Ballesteros
 *
 * Distributed under the Boost Software License, version  1.0
 * See documents/LICENSE.TXT or www.boost.org/LICENSE_1_0.txt
 *
 * angel.rodriguez@esne.edu
 */

#include <basics/Director>
#include <basics/enable>
#include <basics/Graphics_Resource_Cache>
#include <basics/opengles/Context>
#include <basics/Window>
#include "Intro_Scene.hpp"
#include <basics/opengles/Canvas_ES2>
#include <basics/opengles/OpenGL_ES2>

using namespace basics;
using namespace example;
using namespace std;

int main ()
{
    enable< basics::OpenGL_ES2 > ();

    director.run_scene (shared_ptr< Scene >(new Intro_Scene));

    return 0;
}

void keep_links ()
{
    const bool &c = Window::can_be_instantiated;
    Window::Accessor window;
    Graphics_Resource_Cache cache;
    opengles::Context::create(window, &cache);
    Canvas::Factory f = opengles::Canvas_ES2::create;
}
