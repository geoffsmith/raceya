/**
 * Functions to calculate the closest point on a triangle to an abitrary point.
 * This algorithm is taken from "Real-Time Collision Detection" by Christer 
 * Ericson and is pretty optimised. Any improvement should probably come from
 * a partioning scheme.
 */
#pragma once

#include "dof.h"

void findClosestPoint(Dof ** dofs, unsigned int nDofs, float * point, float * closestPoint);
void findClosestPointInTriangle(float * a, float * b, float * c, float * point, float * result);
