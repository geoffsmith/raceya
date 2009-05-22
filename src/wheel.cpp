#include "wheel.h"
#include "lib.h"
#include "track.h"

#include <math.h>

Wheel::Wheel(int position, Dof * dof, Car * car) {
    this->car = car;
    this->_dof = dof;
    this->_brakeDof = NULL;
    this->_rotation = 0;
    this->_position = position;
    this->_wheelAngle = 0;
    this->_steering = false;
    this->angularVelocity = 0;
    this->rotation = 0;
    this->braking = 0;

    this->isPowered = false;

    this->bodyId = dBodyCreate(Track::worldId);
    this->geomId = dCreateCylinder(car->spaceId, 1, 1);
    dGeomSetBody(this->geomId, this->bodyId);

    //this->suspensionJointId = dJointCreateSlider(Track::worldId, 0);
    this->suspensionJointId = dJointCreatePiston(Track::worldId, 0);
    dJointAttach(this->suspensionJointId, this->bodyId, car->bodyId);

    // Initialise the lateral pacejka constants with 15 0s
    this->_lateralPacejka = std::vector<float>(15, 0);
    // ... and longitudinal with 13 0s
    this->_longPacejka = std::vector<float>(13, 0);
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
    glRotatef(rad_2_deg(this->rotation), 1, 0, 0);

    this->_dof->render(true);

    glPopMatrix();
}

void Wheel::turn(float turn) {
    this->_rotation += turn;
    if (this->_rotation >= 360) this->_rotation -= 360;
}

void Wheel::setAngle(float angle) {
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
    this->radius = radius;
}

void Wheel::setMass(float mass, float inertia) {
    this->inertia = inertia;

    dMass newMass;

    dMassSetParameters(&newMass, mass, 0, 0, 0,
            inertia, inertia, inertia,
            0, 0, 0);

    dBodySetMass(this->bodyId, &newMass);
}

void Wheel::setMaxBrakeTorque(float torque) {
    this->maxBrakeTorque = torque;
}

void Wheel::setRollingCoefficient(float coefficient) {
    this->_rollingCoefficient = coefficient;
}

float Wheel::calculateRollingResitance() {
    // TODO: the weight on a wheel should actually be variable
    dMass mass;
    dBodyGetMass(this->car->bodyId, &mass);

    return this->_rollingCoefficient * (mass.mass / 4.0) * 9.8;
}

void Wheel::setLateralPacejka(float a0, float a1, float a2, float a3, float a4, float a5,
                float a6, float a7, float a8, float a9, float a10, float a11, float a12,
                float a13, float a14) {
    this->_lateralPacejka[0] = a0;
    this->_lateralPacejka[1] = a1;
    this->_lateralPacejka[2] = a2;
    this->_lateralPacejka[3] = a3;
    this->_lateralPacejka[4] = a4;
    this->_lateralPacejka[5] = a5;
    this->_lateralPacejka[6] = a6;
    this->_lateralPacejka[7] = a7;
    this->_lateralPacejka[8] = a8;
    this->_lateralPacejka[9] = a9;
    this->_lateralPacejka[10] = a10;
    this->_lateralPacejka[11] = a11;
    this->_lateralPacejka[12] = a12;
    this->_lateralPacejka[13] = a13;
    this->_lateralPacejka[14] = a14;
}

void Wheel::setLongPacejka(float b0, float b1, float b2, float b3, float b4, float b5,
        float b6, float b7, float b8, float b9, float b10, float b11, float b12) {
    this->_longPacejka[0] = b0;
    this->_longPacejka[1] = b1;
    this->_longPacejka[2] = b2;
    this->_longPacejka[3] = b3;
    this->_longPacejka[4] = b4;
    this->_longPacejka[5] = b5;
    this->_longPacejka[6] = b6;
    this->_longPacejka[7] = b7;
    this->_longPacejka[8] = b8;
    this->_longPacejka[9] = b9;
    this->_longPacejka[10] = b10;
    this->_longPacejka[11] = b11;
    this->_longPacejka[12] = b12;
}

float Wheel::calculateLateralPacejka() {
    // We'll need the car's mass for weight on wheels
    dMass mass;
    dBodyGetMass(this->car->bodyId, &mass);
    float fz = (mass.mass * 9.8 / 4.0) / 1000.0;

    // and we need the slip angle
    const dReal * worldVelocity = dBodyGetLinearVel(this->bodyId);
    dVector3 localVelocity;
    dBodyVectorFromWorld(this->bodyId, 
            worldVelocity[0], worldVelocity[1], worldVelocity[2], localVelocity);
    Vector localVelocityV(localVelocity[0], localVelocity[1], localVelocity[2]);
    Vector wheelDirection(0, 0, 1);

    // if the local velocity is too small, we ignore any lateral forces. This is a bit of
    // a hack but hopefully should hold up alright
    if (localVelocity[0] * localVelocity[0] 
            + localVelocity[2] * localVelocity[2] < 0.05) {
        return 0;
    }

    // If the localVelocity is 0, the slip calculation makes no sense and we set it to 0
    float slip = -atan(localVelocity[0] / localVelocity[2]);

    // The Pacejka formulae use degrees
    float slipDegrees = slip * 180.0 / PI;

    float camber = 0.33;

    float c = this->_lateralPacejka[0];

    // Peak lateral friction coefficient
    float uyp = this->_lateralPacejka[1] * fz + this->_lateralPacejka[2];

    // normal force times coefficient of friction
    float d = uyp * fz;

    float e = this->_lateralPacejka[6] * fz + this->_lateralPacejka[7];

    float lateralStiffness = this->_lateralPacejka[3] 
        * sin(2.0 * atan(fz / this->_lateralPacejka[4])) 
        * (1.0 - this->_lateralPacejka[5] * fabs(camber));
    float b = lateralStiffness / (c * d);

    float sh = this->_lateralPacejka[8] * camber + this->_lateralPacejka[9] * fz + 
        this->_lateralPacejka[10];

    // NOTE: 14 -> 112 - this is rubbish, though the alternative is variable per constant
    // which isn't very pretty either.
    float sv = (this->_lateralPacejka[11] * fz + this->_lateralPacejka[14]) * camber * fz
        + this->_lateralPacejka[12] * fz 
        + this->_lateralPacejka[13];

    float fy = d 
        * sin(c * atan(b * (1.0 - e) * (slipDegrees + sh) 
                    + e * atan(b * (slipDegrees + sh)))) + sv;

    //cout << "Slip: " << slipDegrees << ", fy: " << fy << endl;
    /*
    cout << "Fz: " << fz << endl;
    cout << "up: " << uyp << ", c: " << c << ", d: " << d << endl;
    cout << "e: " << e << ", b: " << b << ", sh: " << sh << ", sv: " << sv << endl;
    cout << "stiff: " << lateralStiffness << endl;
    */

    if (isnan(fy)) {
        return 0;
    } else {
        return fy ;
    }
}


float Wheel::calculateLongPacejka() {
    // We'll need the car's mass for weight on wheels
    dMass mass;
    dBodyGetMass(this->car->bodyId, &mass);
    float fz = (mass.mass * 9.8 / 4.0) / 1000.0;

    // The slip is calculate as the difference between the wheel's angular velocity
    // and the speed of the car
    float carSpeed = this->car->getSpeed();

    float slip = 0;

    if (carSpeed != 0) {
        slip = (this->angularVelocity * this->radius - carSpeed) / fabs(carSpeed);
    } else {
        slip = 1.0;
    }

    //std::cout << "Longitudinal slip: " << slip << ", speed: " << carSpeed << endl;

    float c = this->_longPacejka[0];
    float up = this->_longPacejka[1] * fz + this->_longPacejka[2];
    float d = up * fz;
    float b = ((this->_longPacejka[3] * fz * fz + this->_longPacejka[4] * fz) * exp(-this->_longPacejka[5] * fz))
        /
        (c * d);
    float e = this->_longPacejka[6] * fz * fz + this->_longPacejka[7] * fz + this->_longPacejka[8];
    float sh = this->_longPacejka[9] * fz + this->_longPacejka[10];
    float sv = this->_longPacejka[11] * fz + this->_longPacejka[12];
    float fx = d * sin(c * atan(b * (1 - e) * (slip + sh) + e * atan(b * (slip + sh)))) + sv;

    if (isnan(fx)) {
        std::cout << "Fx is nan, c: " << c << ", d: " << d << std::endl;
        return 0;
    } else {
        return fx;
    }
}

void Wheel::updateRotation() {
    float time = this->car->timer->getTargetSeconds();

    // If the wheel is powered by the engine, we calculate the various torques on the 
    // wheel, if not we work out the angular velocity from the car's speed
    if (this->isPowered) {
        // The current gear ratio
        float gear = this->car->getGearbox()->getCurrentRatio();
        float differential = this->car->getEngine()->getDifferential();

        // First we find the RPM of the engine based on the previous angular velocity
        float rpm = this->angularVelocity * gear * differential * 60.0 / (2.0 * PI);
        this->car->getEngine()->setRpm(rpm);

        // Then we look up the torque for this RPM
        float engineTorque = this->car->getEngine()->calculateTorque(rpm);

        // Then we push the torque back through the drive system to get a new torque on the 
        // wheel
        float torque = engineTorque * gear * differential * 0.7;

        // Now we add the rolling coefficient using the load on the wheel, but only if the
        // wheel is actually turning
        if (fabs(this->angularVelocity) > 0) {
            dMass mass;
            dBodyGetMass(this->car->bodyId, &mass);
            float load = mass.mass * 9.8 / 4.0;

            // torque gets added in the opposite direction to the angular velocity
            float sign = -fabs(this->angularVelocity) / this->angularVelocity;

            // TODO We need something here to stop and oscillation at low speeds
            //torque += sign * load * this->_rollingCoefficient;
        }

        // Now we update the angular velocity with the acceleration due to torque in this 
        // timeframe. The timeframe is a 1/10th of a second at the moment. TODO we need to
        // link this time frame to a global
        this->angularVelocity += (torque / this->inertia) * time;
        //std::cout << "Angular velocity: " << this->angularVelocity << std::endl;
    } else {
        this->angularVelocity = this->car->getSpeed() / this->radius;
    }

    // Add brake torque
    // TODO this doesn't work at low speeds, needs tweaking (oscillation probably)
    this->angularVelocity += (this->calculateBrakeTorque() / this->inertia) * time;

    // Update the wheels rotation
    this->rotation += this->angularVelocity * time;

    // Keep the wheel between 0 and 2 * PI to prevent overflow. This assumes that there
    // isn't a whole rotation in a single time period NOTE fix this assumption
    if (this->rotation < 0) {
        this->rotation += 2 * PI;
    } else if (this->rotation > 2 * PI) {
        this->rotation -= 2 * PI;
    }
}

void Wheel::applyBrake(float amount) {
    this->braking = amount;
}

float Wheel::calculateBrakeTorque() {
    // If there is no angular velocity, there's no point in braking
    if (fabs(this->angularVelocity) < 0.0000001) return 0;

    // The braking effect will be in the opposite direction to the current velocity
    float sign = this->angularVelocity / fabs(this->angularVelocity);

    // The torque is calculated by amount of braking currently applied (0..1) and the
    // make braking torque
    return this->braking  * this->maxBrakeTorque * -sign;
}
