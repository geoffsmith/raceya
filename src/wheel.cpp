#include "wheel.h"

Wheel::Wheel(int position, Obj * obj) {
    this->_obj = obj;
    this->_rotation = 0;
    this->_position = position;

    // Get the central vertex for the wheel
    vector<GLfloat> vector;
    switch (this->_position) {
        case 0:
            vector = this->_obj->getVertex(13197);
            this->_wheelCenter[0] = vector[0];
            this->_wheelCenter[1] = vector[1];
            this->_wheelCenter[2] = vector[2];
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
    glTranslatef(this->_wheelCenter[0], 
            this->_wheelCenter[1], 
            this->_wheelCenter[2]);
    if (this->_position == 0 || this->_position == 2) {
        glRotatef(this->_rotation, -1, 0, 0.017);
    } else {
        glRotatef(this->_rotation, 1, 0, 0.017);
    }
    glTranslatef(-1 * this->_wheelCenter[0], 
            -1 * this->_wheelCenter[1], 
            -1 * this->_wheelCenter[2]);
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
    glPopMatrix();
}

void Wheel::turn(float turn) {
    this->_rotation += turn;
    if (this->_rotation >= 360) this->_rotation -= 360;
}
