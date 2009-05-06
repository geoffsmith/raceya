#include "wheel.h"
#include "lib.h"
#include "track.h"

Wheel::Wheel(int position, Dof * dof, Car * car) {
    this->_dof = dof;
    this->_brakeDof = NULL;
    this->_rotation = 0;
    this->_position = position;
    this->_wheelAngle = 0;
    this->_steering = false;

    this->isPowered = false;

    this->bodyId = dBodyCreate(Track::worldId);
    this->geomId = dCreateCylinder(car->spaceId, 1, 1);
    dGeomSetBody(this->geomId, this->bodyId);

    //this->suspensionJointId = dJointCreateSlider(Track::worldId, 0);
    this->suspensionJointId = dJointCreatePiston(Track::worldId, 0);
    dJointAttach(this->suspensionJointId, this->bodyId, car->bodyId);
}

Wheel::~Wheel() {
    dJointDestroy(this->suspensionJointId);
}

void Wheel::render() {
    glPushMatrix();

    const dReal * position = dBodyGetPosition(this->bodyId);
    glTranslatef(position[0], position[1], position[2]);

    // Get the car's rotation
    const dReal * rotation = dBodyGetRotation(this->bodyId);
    Matrix rotationMatrix(rotation, 3);
    glMultMatrixf(rotationMatrix.getMatrix());

    /*
    glTranslatef(this->_wheelCenter[0], this->_wheelCenter[1], this->_wheelCenter[2]);

    // Rotate the wheel left / right, if steering is enabled
    if (this->_steering) {
        glRotatef(this->_wheelAngle, 0, -1, 0);
    }
    */

    // Render the brake if there is one, this happens before we rotat the wheel but after
    // steering
    //if (this->_brakeDof != NULL) this->_brakeDof->render(true);

    // Rotate the wheel around the axis
    //glRotatef(this->_rotation, 1, 0, 0);

    this->_dof->render(true);

    glPopMatrix();
}

void Wheel::turn(float turn) {
    this->_rotation += turn;
    if (this->_rotation >= 360) this->_rotation -= 360;
}

void Wheel::setAngle(float angle) {

    float difference = angle - this->_wheelAngle;
    this->_wheelAngle = -angle * 0.0174532925;

    dJointSetPistonParam(this->suspensionJointId, dParamHiStop2, this->_wheelAngle);
    dJointSetPistonParam(this->suspensionJointId, dParamLoStop2, this->_wheelAngle);
}

float Wheel::getAngle() {
    return this->_wheelAngle;
}

void Wheel::getGroundContact(float * point) {
    // This is actually static, even though the wheel turns, just depends on
    // the wheel we want
    vertexCopy(this->_groundContact, point);
}

void Wheel::setCarPosition(const float * position) {
    dBodySetPosition(this->bodyId, 
            this->_wheelCenter[0] + position[0], 
            this->_wheelCenter[1] + position[1], 
            this->_wheelCenter[2] + position[2]);
    dJointSetPistonAnchor(this->suspensionJointId, 
            this->_wheelCenter[0] + position[0], 
            this->_wheelCenter[1] + position[1], 
            this->_wheelCenter[2] + position[2]);
    /*
    dJointSetSliderAxis(this->suspensionJointId, 0, 1, 0);
    dJointSetSliderParam(this->suspensionJointId, dParamHiStop, 0);
    dJointSetSliderParam(this->suspensionJointId, dParamLoStop, -0.15);
    dJointSetSliderParam(this->suspensionJointId, dParamCFM, 1);
    */
    dJointSetPistonAxis(this->suspensionJointId, 0, 1, 0);
    dJointSetPistonParam(this->suspensionJointId, dParamHiStop, 0);
    dJointSetPistonParam(this->suspensionJointId, dParamLoStop, -0.15);

    dJointSetPistonParam(this->suspensionJointId, dParamHiStop2, 0);
    dJointSetPistonParam(this->suspensionJointId, dParamLoStop2, 0);

    dJointSetPistonParam(this->suspensionJointId, dParamCFM, 1);
}

void Wheel::setCenter(float * center) {
    this->_wheelCenter[0] = center[0];
    this->_wheelCenter[1] = center[1];
    this->_wheelCenter[2] = center[2];

    dBodySetPosition(this->bodyId, center[0], center[1], center[2]);

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

bool Wheel::isSteering() {
    return this->_steering;
}

float * Wheel::getWheelCenter() {
    return this->_wheelCenter;
}

void Wheel::setRadius(float radius) {
    dGeomCylinderSetParams(this->geomId, radius, 0.1);
}

void Wheel::setMass(float mass, float inertia) {
    dMass newMass;

    dMassSetParameters(&newMass, mass, 0, 0, 0,
            inertia, inertia, inertia,
            0, 0, 0);

    dBodySetMass(this->bodyId, &newMass);
}
