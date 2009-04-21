#include "rigid_body.h"

#define PI 3.14159265

void RigidBody::initialiseRigidBody() {
    // Try and run the physics at 100Hz
    this->timer = new FrameTimer(100);
    this->inertiaTensor = Matrix(3);
    this->inverseInertiaTensor = Matrix(3);
    this->linearVelocity = Vector(3);
    this->angularVelocity = Vector(3);
    this->transformationMatrix = Matrix(4);
    this->rotationMatrix = Matrix(4);

    float euler[] = { 0, PI / 16.0, 0 };
    this->orientation.fromEuler(euler);

    this->resetAccumulators();
    this->_updateMatrices();
}

void RigidBody::calculate() {
    double time = this->timer->getSeconds();

    // Calculate the current linear acceleration based on the accumulated forces
    Vector linearAcceleration = this->forceAccumulator / this->mass;
    this->accelerationAtUpdate = linearAcceleration;

    // Now we update the linear velocity, dampening the velocity to account for numerical
    // error
    this->linearVelocity = 
          this->linearVelocity * pow(LINEAR_DAMPER, time) + linearAcceleration * time;

    // Calculate a new angular moment
    Matrix inverseRotation(4);
    this->rotationMatrix.invert(inverseRotation);
    Matrix worldInverseInertiaTensor = 
        (this->rotationMatrix * this->inverseInertiaTensor) * inverseRotation;
    Vector angularAcceleration = worldInverseInertiaTensor * this->torqueAccumulator;

    // ... and apply it to the angular velocity
    this->angularVelocity = 
        this->angularVelocity * pow(ANGULAR_DAMPER, time) + angularAcceleration * time;

    // We update the position
    this->position += this->linearVelocity * time;

    // ... and update the orientation
    // ... first transforming the angular velocity into world coordinates
    this->orientation += 0.5 * ((time * this->angularVelocity) * this->orientation);
    this->orientation.normalise();

    // Clean up ready for the next calculation
    this->resetAccumulators();

    this->_updateMatrices();
}

void RigidBody::resetAccumulators() {
    this->forceAccumulator = Vector(0, 0, 0);
    this->torqueAccumulator = Vector(0, 0, 0);
}

void RigidBody::addForce(const Vector & force) {
    this->forceAccumulator += force;
}

void RigidBody::addForce(const Vector & force, const Vector & point) {
    // First we add the force effecting the linear movement
    this->addForce(force);

    // Now we deal with the rotational effect and calculate the torque
    this->torqueAccumulator += point ^ force;
}

void RigidBody::addLocalForce(const Vector & force, const Vector & point) {
    Vector globalForce = this->rotationMatrix * force;
    Vector globalPoint = this->rotationMatrix * point;
    this->addForce(globalForce, globalPoint);
}

void RigidBody::addForceLocalPoint(const Vector & force, const Vector & point) {
    Vector globalPoint = this->transformationMatrix * point;
    this->addForce(force, globalPoint);
}

void RigidBody::applyImpulse(const Vector & impulse, const Vector & point) {
    Matrix inverseRotation(4);
    this->rotationMatrix.invert(inverseRotation);
    Matrix worldInertiaTensor = 
        (this->rotationMatrix * this->inverseInertiaTensor) * inverseRotation;

    this->linearVelocity += impulse / this->mass;
    Vector rotationChange = worldInertiaTensor * (point ^ impulse);
    this->angularVelocity += rotationChange;
}

RigidBody::~RigidBody() {
    delete this->timer;
}

void RigidBody::setCenter(float * center) {
    this->cog = Vector(center, 3);
}

void RigidBody::setMass(float mass) {
    this->mass = mass;
}

void RigidBody::setInertia(float * inertia) {
    // At the moment we're not adding any extra elements to the car (wheels, gas tank, 
    // etc) so the inertia tensor is pretty simple.
    this->inertiaTensor[0] = inertia[0];
    this->inertiaTensor[1] = 0;
    this->inertiaTensor[2] = 0;

    this->inertiaTensor[3] = 0;
    this->inertiaTensor[4] = inertia[1];
    this->inertiaTensor[5] = 0;

    this->inertiaTensor[6] = 0;
    this->inertiaTensor[7] = 0;
    this->inertiaTensor[8] = inertia[2];

    // We're also going to need the inverse
    this->inertiaTensor.invert(this->inverseInertiaTensor);
}

void RigidBody::_updateMatrices() {
    this->_mutex.lock();
    // We'll need the rotation matrix
    this->orientation.toRotationMatrix(this->rotationMatrix);

    // Reset the matrix
    this->transformationMatrix.reset();

    // Move the car to its new position
    this->transformationMatrix.translate(
            this->position[0], this->position[1], this->position[2]);

    Vector center;

    center  = this->cog;

    // Translate away from center
    this->transformationMatrix.translate(center[0], center[1], center[2]);
    
    // Set the cars orientation using the quaternion
    this->transformationMatrix.multiplyMatrix(&(this->rotationMatrix));

    this->transformationMatrix.translate(-1 * center[0], -1 * center[1], -1 * center[2]);
    this->_mutex.unlock();
}
