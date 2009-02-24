#include "world.h"
#include <OpenGL/gl.h>

void World::render() {
    float points[4][4][3];

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            points[i][j][0] = -15 + 10 * i;
            points[i][j][1] = 0;
            points[i][j][2] = -15 + 10 * j;
        }
    }

    // Set up the evaluator
    glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &(points[0][0][0]));
    glEnable(GL_MAP2_VERTEX_3);
    glEnable(GL_AUTO_NORMAL);

    // Draw the grid
    glColor3f(1, 1, 1);
    glMapGrid2f(100, 0, 1, 100, 0, 1);
    glEvalMesh2(GL_LINE, 0, 100, 0, 100);
}
