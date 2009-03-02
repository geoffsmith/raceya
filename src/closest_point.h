/**
 * Functions to calculate the closest point on a triangle to an abitrary point
 */
#pragma once

#include "dof.h"

void findClosestPoint(Dof ** dofs, unsigned int nDofs, float * point, float * closestPoint);
void findClosestPointInTriangle(float * a, float * b, float * c, float * point, float * result);
