#include "wheel.h"

Wheel::Wheel(int position, Dof * dof) {
    this->_dof = dof;
    this->_rotation = 0;
    this->_position = position;
    this->_wheelAngle = 0;

    // Get the central vertex for the wheel
    switch (this->_position) {
        case 0:
            this->_wheelCenter[0] = 0.953;
            this->_wheelCenter[1] = 1.434;
            this->_wheelCenter[2] = 0.342;
            break;
        case 1:
            this->_wheelCenter[0] = 0.953;
            this->_wheelCenter[1] = 1.434;
            this->_wheelCenter[2] = 0.342;
            break;
        case 2:
            this->_wheelCenter[0] = -0.934;
            this->_wheelCenter[1] = -1.217;
            this->_wheelCenter[2] = 0.342;
            break;
        case 3:
            this->_wheelCenter[0] = 0.939;
            this->_wheelCenter[1] = -1.217;
            this->_wheelCenter[2] = 0.342;
            break;
    }

}

void Wheel::render() {
    glPushMatrix();

    glTranslatef(this->_wheelCenter[0], this->_wheelCenter[1], this->_wheelCenter[2]);

    this->_dof->render(true);

    /*
    float normal[3];
    float scale = 0.235 / 2;

    // The left and right wheels have slightly different normals, which affects the rotations
    if (this->_position == 0 || this->_position == 2) {
        normal[0] = -1;
    } else {
        normal[0] = 1;
    }
    normal[1] = 0;
    normal[2] = 0.017;
    
    // Scale the normal down to be half the width of the tyre
    normal[0] *= -1 * scale;
    normal[1] *= scale;
    normal[2] *= scale;
    */


    /*
    glTranslatef(this->_wheelCenter[0] + normal[0], 
            this->_wheelCenter[1] + normal[1], 
            this->_wheelCenter[2] + normal[2]);

    // Rotate the wheel to the left or right
    glRotatef(this->_wheelAngle, 0, 0, -1);

    glTranslatef(-1 * (this->_wheelCenter[0] + normal[0]), 
            -1 * (this->_wheelCenter[1] + normal[1]), 
            -1 * (this->_wheelCenter[2] + normal[2]));

    // Render the caliper before we rotate the wheel but after we rotate for steering
    this->_dof->render();

    glTranslatef(this->_wheelCenter[0] + normal[0], 
            this->_wheelCenter[1] + normal[1], 
            this->_wheelCenter[2] + normal[2]);

    // Rotate the wheel around the axis
    if (this->_position == 0 || this->_position == 2) {
        glRotatef(this->_rotation, normal[0], normal[1], normal[2]);
    } else {
        glRotatef(this->_rotation, -1 * normal[0], normal[1], normal[2]);
    }

    glTranslatef(-1 * (this->_wheelCenter[0] + normal[0]), 
            -1 * (this->_wheelCenter[1] + normal[1]), 
            -1 * (this->_wheelCenter[2] + normal[2]));
            */

    /* 
    switch (this->_position) {
        case 0:
            this->_obj->renderGroup("RotorRR");
            this->_obj->renderGroup("TireRR");
            break;
        case 1:
            this->_obj->renderGroup("RotorRL");
            this->_obj->renderGroup("TireRL");
            break;
        case 2:
            this->_obj->renderGroup("RotorFR");
            this->_obj->renderGroup("TireFR");
            break;
        case 3:
            this->_obj->renderGroup("RotorFL");
            this->_obj->renderGroup("TireFL");
            break;
    }
    */

    glPopMatrix();
}

void Wheel::turn(float turn) {
    this->_rotation += turn;
    if (this->_rotation >= 360) this->_rotation -= 360;
}

void Wheel::setAngle(float angle) {
    this->_wheelAngle = angle;
}

void Wheel::getGroundContact(float * point) {
    // This is actually static, even though the wheel turns, just depends on
    // the wheel we want
    switch (this->_position) {
        case 0:
            point[0] = -0.928;
            point[1] = 1.47;
            point[2] = 0.0;
            break;
        case 1:
            point[0] = 0.928;
            point[1] = 1.399;
            point[2] = 0.0;
            break;
        case 2:
            point[0] = -0.914;
            point[1] = -1.181;
            point[2] = 0.0;
            break;
        case 3:
            point[0] = 0.914;
            point[1] = -1.252;
            point[2] = 0.0;
            break;
    }
}

void Wheel::setCenter(float * center) {
    this->_wheelCenter[0] = center[0];
    this->_wheelCenter[1] = center[1];
    this->_wheelCenter[2] = center[2];
}
