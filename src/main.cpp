/**
 * TODO:
 *  * Enable back face culling for speed enhancement. Need to sort out particle
 *    normals first though.
 */
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <SDL/SDL.h>

#include <list>
#include <sstream>
#include <iostream>
#include <time.h>
#include <string>


using namespace std;

void setupLighting();

void init(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glLineWidth(1.5);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);

    setupLighting();

    srand(clock());
}

void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    // Add two lights to the ship
    glEnable(GL_LIGHT2);
    //GLfloat ship_left_light[] = { 1, 0, 0 };
    //glLightfv(GL_LIGHT2, GL_DIFFUSE, ship_left_light);
    glEnable(GL_LIGHT3);

    // Set up the ambient light so we can see on the dark side of the planet
    GLfloat light[] = { 0, 0, 0 };
    GLfloat dark[] = { 1, 1, 1};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light);
    glLightfv(GL_LIGHT1, GL_AMBIENT, dark);
}

void init_objects() {
}

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

void display(void) {
    // display the planets
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);


    glColor3f(1.0, 1.0, 1.0);
    glLoadIdentity(); // clear the matrix

    glDisable(GL_TEXTURE_2D);
}

void handleKeyboard() {
    // Poll for events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        exit(0);
                    default:
                        break;
                }
                break;
            case SDL_QUIT:
                exit(0);
        }
    }
}

int main(int argc, char** argv) {
    int error = SDL_Init(SDL_INIT_EVERYTHING);

    if (error < 0) {
        cout << "Unable to init SDL: " << SDL_GetError();
        exit(1);
    }
    atexit(SDL_Quit);
    
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_Surface *drawContext;
    const SDL_VideoInfo *info = SDL_GetVideoInfo();
    int bpp = info->vfmt->BitsPerPixel;
    drawContext = SDL_SetVideoMode(500, 500, bpp, SDL_OPENGL);

    reshape(500, 500);

    if (drawContext == 0) {
        cout << "Failed to initialise video" << endl;
        exit(1);
    }

    init();
    init_objects();

    // Enter the main look
    while (1) {
        // Take input
        handleKeyboard();

        // Draw the scene
        display();

        // Swap the buffers
        SDL_GL_SwapBuffers();

        SDL_Delay(20);
    }
    return 0;
}
