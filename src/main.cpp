/**
 * TODO:
 *  * Enable back face culling for speed enhancement. Need to sort out particle
 *    normals first though.
 */
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <list>
#include <sstream>
#include <iostream>
#include <time.h>
#include <string>

#include "car.h"
#include "camera.h"
#include "frame_timer.h"
#include "world.h"
#include "hud.h"
#include "track.h"
#include "frustrum_culler.h"

using namespace std;

static Car * car;
static Camera * camera;
static World * world;
static Hud * hud;
static SDL_Surface * drawContext;
static Track * track;

void setupLighting();

void init(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_NORMALIZE);

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glLineWidth(1.5);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    setupLighting();

    srand(clock());
}

void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void initObjects() {
    // Load a track

    // Load up a car obj
    car = new Car();

    //track = new Track("resources/tracks/adelaide/");
    track = new Track("resources/tracks/broussailles/");

    // Create the camera, pointing at the player's car
    camera = new Camera(car);

    // Create the world model
    world = new World();
    
    // Create the HUD
    hud = new Hud(car);

}

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-2.0, 2.0, -1.0, 1.0, 1.5, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

void display(void) {
    // Render the scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);

    glColor3f(1.0, 1.0, 1.0);
    glLoadIdentity(); // clear the matrix

    // Render the HUD
    //hud->render();

    // Position the light at the camera
    float lightPosition[] = { 0, 0, 1, 0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    camera->viewTransform();

    ViewFrustrumCulling::culler->refreshMatrices();

    // Draw the car
    car->render();

    track->render();


    // Draw the world
    //world->render();

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
        camera->handleKeyPress(event);
        car->handleKeyPress(event);
    }
}

int main(int argc, char** argv) {
    int error = SDL_Init(SDL_INIT_EVERYTHING);

    // Initialise the TTF library
    TTF_Init();

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

    const SDL_VideoInfo *info = SDL_GetVideoInfo();
    int bpp = info->vfmt->BitsPerPixel;
    drawContext = SDL_SetVideoMode(1000, 500, bpp, SDL_OPENGL);

    reshape(1000, 500);

    if (drawContext == 0) {
        cout << "Failed to initialise video" << endl;
        exit(1);
    }

    init();
    initObjects();

    // Enter the main look
    while (1) {
        // Update the frame timer
        FrameTimer::timer.newFrame();

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
