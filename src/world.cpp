#include "world.h"
#include <OpenGL/gl.h>

void World::render() {
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
    glVertex3f(-100, 0, -100);
    glVertex3f(-100, 0, 100);
    glVertex3f(100, 0, 100);
    glVertex3f(100, 0, -100);
    glEnd();
}
