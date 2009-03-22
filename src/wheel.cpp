#include "wheel.h"
#include "lib.h"

Wheel::Wheel(int position, Dof * dof) {
    this->_dof = dof;
    this->_brakeDof = NULL;
    this->_rotation = 0;
    this->_position = position;
    this->_wheelAngle = 0;
    this->_steering = false;

    this->isPowered = false;

}

void Wheel::render() {
    glPushMatrix();

    glTranslatef(this->_wheelCenter[0], this->_wheelCenter[1], this->_wheelCenter[2]);

    // Rotate the wheel left / right, if steering is enabled
    if (this->_steering) {
        glRotatef(this->_wheelAngle, 0, -1, 0);
    }

    // Render the brake if there is one, this happens before we rotat the wheel but after
    // steering
    if (this->_brakeDof != NULL) this->_brakeDof->render(true);

    // Rotate the wheel around the axis
    glRotatef(this->_rotation, 1, 0, 0);

    this->_dof->render(true);

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
    vertexCopy(this->_groundContact, point);
}

void Wheel::setCenter(float * center) {
    this->_wheelCenter[0] = center[0];
    this->_wheelCenter[1] = center[1];
    this->_wheelCenter[2] = center[2];

    // Calculate the lowest point for the wheel, now that we have a position
    Geob * geob;
    for (int geobIndex = 0; geobIndex < this->_dof->getNGeobs(); ++geobIndex) {
        geob = this->_dof->getGeob(geobIndex);
        // Get the lowest vertex
        for (unsigned int i = 0; i < geob->nVertices; ++i) {
            // If this is the first vertex, we set it regardless
            if (geobIndex == 0 && i == 0) {
                vertexCopy(geob->vertices[i], this->_groundContact);
                continue;
            }

            // Otherwise we check to see if it is lower
            if (geob->vertices[i][1] < this->_groundContact[1]) {
                vertexCopy(geob->vertices[i], this->_groundContact);
            }
        }
    }

    // Transform to wheel position
    this->_groundContact[0] += this->_wheelCenter[0];
    this->_groundContact[1] += this->_wheelCenter[1];
    this->_groundContact[2] += this->_wheelCenter[2];
}

void Wheel::setBrakeDof(Dof * dof) {
    this->_brakeDof = dof;
}

void Wheel::enableSteering() {
    this->_steering = true;
}
