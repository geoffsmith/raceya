/**
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <list>
#include <sstream>
#include <iostream>
#include <time.h>
#include <string>

#include "lib.h"
#include "car.h"
#include "car_parser.h"
#include "camera.h"
#include "frame_timer.h"
#include "hud.h"
#include "track.h"
#include "frustum_culler.h"
#include "logger.h"

using namespace std;

static Car * car;
static Camera * camera;
static Hud * hud;
static SDL_Surface * drawContext;
static Track * track;

static int screenWidth = 800;
static int screenHeight = 600;

void setupLighting();

void init(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);
    glShadeModel(GL_FLAT);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glDisable(GL_DITHER);

    //glEnable(GL_LINE_SMOOTH);
    //glEnable(GL_POINT_SMOOTH);
    //glEnable(GL_POLYGON_SMOOTH);
    //glLineWidth(1.5);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
    glHint(GL_TEXTURE_COMPRESSION_HINT, GL_FASTEST);
    glHint(GL_GENERATE_MIPMAP_HINT, GL_FASTEST);

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    setupLighting();

    srand(clock());

}

void checkExtensions() {
    char * extensions = (char *)glGetString(GL_EXTENSIONS);

    // Not using any extensions at the moment
    
    // Check for GL_ARB_point_sprite
    //if (strstr(extensions, "GL_ARB_pixel_buffer_object") == NULL) {
        //cout << "GL_ARB_pixel_buffer_object not available" << endl;
        //exit(1);
    //}
}

void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void initObjects() {
    // Load a track
    //track = new Track("resources/tracks/Monaco_AM/");
    track = new Track("resources/tracks/broussailles/");

    // Load up a car obj
    //car = new Car(track);
    //car = parseCar("resources/cars/Mitsubishi_Lancer_EVO_IX/");
    car = parseCar("resources/cars/Alfa_Romeo_GT_Junior/");
    if (car == NULL) {
        cout << "Car was not loaded" << endl;
        exit(1);
    }
    car->setTrack(track);

    // Create the camera, pointing at the player's car
    camera = new Camera(car);

    // Create the HUD
    hud = new Hud(car, screenWidth, screenHeight);

}

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Calculate the screen ratio
    float height = 1.0;
    float width = (float)w / (float)h;
    glFrustum(-1.0 * width, width, -1.0 * height, height, 1.5, 150.0);
    glMatrixMode(GL_MODELVIEW);
}

void display(void) {
    // Render the scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(1.0, 1.0, 1.0);
    glLoadIdentity(); // clear the matrix

    camera->viewTransform();

    ViewFrustumCulling::culler->refreshMatrices();

    // Reset the openGL state
    OpenGLState::global.reset();

    track->render();

    // Draw the car, this needs to be after track, because we can sometime see through
    // the window
    car->render();

    // Render the HUD
    // Alpha blending is still messed up here. Not sure why, 
    hud->render();

    // Position the light at the camera
    float lightPosition[] = { 0, 0, 1, 0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    printError();
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
    //drawContext = SDL_SetVideoMode(screenWidth, screenHeight, bpp, SDL_OPENGL | SDL_HWSURFACE | SDL_DOUBLEBUF);
    drawContext = SDL_SetVideoMode(screenWidth, screenHeight, bpp, SDL_OPENGL);

    reshape(screenWidth, screenHeight);

    if (drawContext == 0) {
        cout << "Failed to initialise video" << endl;
        exit(1);
    }

    // Check that we have all the extensions we need
    checkExtensions();

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

        // Clear log
        Logger::maintain();

        SDL_Delay(FrameTimer::timer.getTimeTillNextDraw());
    }
    return 0;
}
