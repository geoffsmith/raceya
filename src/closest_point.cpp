#include "closest_point.h"
#include "lib.h"
#include <iostream>
#include <string>

using namespace std;

static string winner;

void findClosestPoint(Dof ** dofs, unsigned int nDofs, float * point, float * closestPoint) {
    Geob * geob;
    float tmpPoint[3];
    float tmpDistance;
    float closestDistance;
    int burstCount, burstStart, stop;

    bool first = true;

    // To find the closest point, we iterate over all the triangles in all the
    // dofs
    // TODO: only check dofs which are part of the track
    for (unsigned int dofIndex = 0; dofIndex < nDofs; ++dofIndex) {
        for (int geobIndex = 0; 
                geobIndex < dofs[dofIndex]->getNGeobs(); 
                ++geobIndex) {
            geob = dofs[dofIndex]->getGeob(geobIndex);

            for (int burst = 0; burst < geob->nBursts; burst++) {
                burstCount = geob->burstsCount[burst] / 3;
                burstStart = geob->burstStarts[burst] / 3;

                stop = min(burstStart + burstCount, geob->nIndices);

                // Check each triangle
                for (int triangleIndex = burstStart; 
                        triangleIndex < stop;
                        triangleIndex += 3) {
                    findClosestPointInTriangle(
                            geob->vertices[geob->indices[triangleIndex]],
                            geob->vertices[geob->indices[triangleIndex + 1]],
                            geob->vertices[geob->indices[triangleIndex + 2]],
                            point,
                            tmpPoint);


                    // Now see if the tmp point is the closest so far
                    tmpDistance = vertexSquareDistance(point, tmpPoint);
                    if (first || tmpDistance < closestDistance) {
                        vertexCopy(tmpPoint, closestPoint);
                        closestDistance = tmpDistance;
                        first = false;
                    }
                }
            }
        }
    }
}

void findClosestPointInTriangle(float * a, float * b, float * c, float * point, float * result) {
    float ab[3];
    float ac[3];
    float ap[3];
    float tmp[3];

    // Check if vertex in region outside A
    vertexSub(b, a, ab);
    vertexSub(c, a, ac);
    vertexSub(point, a, ap);

    float d1 = dotProduct(ab, ap);
    float d2 = dotProduct(ac, ap);
    if (d1 <= 0.0 && d2 <= 0.0) {
        vertexCopy(a, result);
        winner = "a";
        return;
    }

    // ... and outside B
    float bp[3];

    vertexSub(point, b, bp);
    float d3 = dotProduct(ab, bp);
    float d4 = dotProduct(ac, bp);
    if (d3 >= 0.0 && d4 <= d3) {
        vertexCopy(b, result);
        winner = "b";
        return;
    }

    // Edge of AB
    float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0) {
        vertexMultiply(d1 / (d1 - d3), ab, tmp);
        vertexAdd(a, tmp, result);
        winner = "ab";
        return;
    }

    // region outside C
    float cp[3];
    vertexSub(point, c, cp);
    float d5 = dotProduct(ab, cp);
    float d6 = dotProduct(ac, cp);
    if (d6 >= 0.0 && d5 <= d6) {
        vertexCopy(c, result);
        winner = "c";
        return;
    }

    // edge region of AC
    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0) {
        vertexMultiply(d2 / (d2 - d6), ac, tmp);
        vertexAdd(a, tmp, result);
        winner = "ac";
        return;
    }

    // edge region BC
    float va = d3 * d6 - d5 * d4;
    float bc[3];
    vertexSub(c, b, bc);
    if (va <= 0.0 && (d4 - d3) >= 0.0 && (d5 - d6) >= 0.0) {
        vertexMultiply((d4 - d3) / ((d4 - d3) + (d5 - d6)), bc, tmp);
        vertexAdd(b, tmp, result);
        winner = "bc";
        return;
    }

    // inside face region
    float d = 1.0 / (va + vb + vc);
    float v = vb * d;
    float w = vc * d;

    winner = "face";

    vertexCopy(a, result);
    vertexMultiply(v, ab, tmp);
    vertexAdd(result, tmp, result);
    vertexMultiply(w, ac, tmp);
    vertexAdd(result, tmp, result);
}

/*void findClosestPointInTriangle(float * a, float * b, float * c, float * point, float * result) {
    // TODO: see if we can reduce the number of floats on the stack here with
    // reuse
    float ap[3];
    float ab[3];
    float bp[3];
    float ba[3];
    float ac[3];
    float cp[3];
    float ca[3];
    float bc[3];
    float n[3];
    float pa[3];
    float pb[3];
    float pc[3];
    float tmp[3];


    vertexSub(point, a, ap);
    vertexSub(b, a, ab);
    vertexSub(point, b, bp);
    vertexSub(a, b, ab);
    vertexSub(c, a, ac);
    vertexSub(point, a, ac);
    vertexSub(a, c, ca);

    // First check the vertex voronoi regions

    // Calculate the parametric position of the projection of point onto AB
    float s = dotProduct(ap, ab);
    float is = dotProduct(bp, ba);
    // .. and for AC
    float t = dotProduct(ap, ac);
    float it = dotProduct(cp, ca);

    // If s and u are negative, the point is in the voronoi region of vertex A
    // and so A is the closest point
    if (s <= 0.0 && t <= 0.0) {
        vertexCopy(a, result);
        return;
    }

    // ... calculate the paramtetric position of projection onto BC
    vertexSub(c, b, bc);
    float u = dotProduct(bp, bc);
    float iu = dotProduct(cp, ca);

    // Check if the point is in the voronoi region of B
    if (is <= 0.0 && u <= 0.0) {
        vertexCopy(b, result);
        return;
    }
    // .. and for C
    if (it <= 0.0 && iu <= 0.0) {
        vertexCopy(c, result);
        return;
        
    }

    // It was not in the vertex regions, so we check the edge regions
    // the point is outside (or on) AB if the triple scalar product [N PA PB] is
    // <= 0
    vertexSub(a, point, pa);
    vertexSub(b, point, pb);
    crossProduct(ab, ac, n);
    crossProduct(pa, pb, tmp);
    float vc = dotProduct(n, tmp);

    // If point is outside AB and within feature region of AB return
    // projection of point onto AB
    if (vc <= 0.0 && s >= 0.0 && is >= 0.0) {
        vertexMultiply(s / (s + is), ab, tmp);
        vertexAdd(a, tmp, result);
        return;
    }

    // .. and the same for BC
    crossProduct(pb, pc, tmp);
    float va = dotProduct(n, tmp);
    if (va <= 0.0 && u >= 0.0 && iu >= 0.0) {
        vertexMultiply(u / (u + iu), bc, tmp);
        vertexAdd(b, tmp, result);
        return;
    }

    // ... and finally for CA
    crossProduct(pc, pa, tmp);
    float vb = dotProduct(n, tmp);
    if (vb <= 0.0 && t >= 0.0 && it >= 0.0) {
        vertexMultiply(t / (t + it), ac, tmp);
        vertexAdd(a, tmp, result);
        return;
    }

    // P must project inside the face region, compute Q using barycentric coords
    s = va / (va + vb + vc);
    t = vb / (va + vb + vc);
    u = vc / (va + vb + vc);
    vertexMultiply(s, a, result);
    vertexMultiply(t, b, tmp);
    vertexAdd(result, tmp, result);
    vertexMultiply(u, c, tmp);
    vertexAdd(result, tmp, result);
}
*/
