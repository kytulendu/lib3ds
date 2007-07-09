/*
 * The 3D Studio File Format Library
 * Copyright (C) 1996-2007 by Jan Eric Kyprianidis <www.kyprianidis.com>
 * All rights reserved.
 *
 * This program is  free  software;  you can redistribute it and/or modify it
 * under the terms of the  GNU Lesser General Public License  as published by
 * the  Free Software Foundation;  either version 2.1 of the License,  or (at
 * your option) any later version.
 *
 * This  program  is  distributed in  the  hope that it will  be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or  FITNESS FOR A  PARTICULAR PURPOSE.  See the  GNU Lesser General Public
 * License for more details.
 *
 * You should  have received  a copy of the GNU Lesser General Public License
 * along with  this program;  if not, write to the  Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: vector.c,v 1.12 2007/06/20 17:04:09 jeh Exp $
 */
#include "lib3ds_impl.h"


void
lib3ds_vector_zero(Lib3dsVector c) {
    int i;
    for (i = 0; i < 3; ++i) {
        c[i] = 0.0f;
    }
}


void
lib3ds_vector_copy(Lib3dsVector dst, Lib3dsVector src) {
    int i;
    for (i = 0; i < 3; ++i) {
        dst[i] = src[i];
    }
}


/*!
 * Negate a vector.
 *
 * \param c Vector to negate.
 */
void
lib3ds_vector_neg(Lib3dsVector c) {
    int i;
    for (i = 0; i < 3; ++i) {
        c[i] = -c[i];
    }
}


/*!
 * Add two vectors.
 *
 * \param c Result.
 * \param a First addend.
 * \param b Second addend.
 */
void
lib3ds_vector_add(Lib3dsVector c, Lib3dsVector a, Lib3dsVector b) {
    int i;
    for (i = 0; i < 3; ++i) {
        c[i] = a[i] + b[i];
    }
}


/*!
 * Subtract two vectors.
 *
 * \param c Result.
 * \param a Addend.
 * \param b Minuend.
 */
void
lib3ds_vector_sub(Lib3dsVector c, Lib3dsVector a, Lib3dsVector b) {
    int i;
    for (i = 0; i < 3; ++i) {
        c[i] = a[i] - b[i];
    }
}


/*!
 * Multiply a vector by a scalar.
 *
 * \param c Vector to be multiplied.
 * \param k Scalar.
 */
void
lib3ds_vector_scalar(Lib3dsVector c, float k) {
    int i;
    for (i = 0; i < 3; ++i) {
        c[i] *= k;
    }
}


/*!
 * Compute cross product.
 *
 * \param c Result.
 * \param a First vector.
 * \param b Second vector.
 */
void
lib3ds_vector_cross(Lib3dsVector c, Lib3dsVector a, Lib3dsVector b) {
    c[0] = a[1] * b[2] - a[2] * b[1];
    c[1] = a[2] * b[0] - a[0] * b[2];
    c[2] = a[0] * b[1] - a[1] * b[0];
}


/*!
 * Compute dot product.
 *
 * \param a First vector.
 * \param b Second vector.
 *
 * \return Dot product.
 */
float
lib3ds_vector_dot(Lib3dsVector a, Lib3dsVector b) {
    return(a[0]*b[0] + a[1]*b[1] + a[2]*b[2]);
}


/*!
 * Compute square of vector.
 *
 * Computes x*x + y*y + z*z.
 *
 * \param c Vector to square.
 *
 * \return Square of vector.
 */
float
lib3ds_vector_squared(Lib3dsVector c) {
    return(c[0]*c[0] + c[1]*c[1] + c[2]*c[2]);
}


/*!
 * Compute length of vector.
 *
 * Computes |c| = sqrt(x*x + y*y + z*z)
 *
 * \param c Vector to compute.
 *
 * \return Length of vector.
 */
float
lib3ds_vector_length(Lib3dsVector c) {
    return((float)sqrt(c[0]*c[0] + c[1]*c[1] + c[2]*c[2]));
}


/*!
 * Normalize a vector.
 *
 * Scales a vector so that its length is 1.0.
 *
 * \param c Vector to normalize.
 */
void
lib3ds_vector_normalize(Lib3dsVector c) {
    float l, m;

    l = (float)sqrt(c[0] * c[0] + c[1] * c[1] + c[2] * c[2]);
    if (fabs(l) < LIB3DS_EPSILON) {
        if ((c[0] >= c[1]) && (c[0] >= c[2])) {
            c[0] = 1.0f;
            c[1] = c[2] = 0.0f;
        } else
            if (c[1] >= c[2]) {
                c[1] = 1.0f;
                c[0] = c[2] = 0.0f;
            } else {
                c[2] = 1.0f;
                c[0] = c[1] = 0.0f;
            }
    } else {
        m = 1.0f / l;
        c[0] *= m;
        c[1] *= m;
        c[2] *= m;
    }
}


/*!
 * Compute a vector normal to two line segments.
 *
 * Computes the normal vector to the lines b-a and b-c.
 *
 * \param n Returned normal vector.
 * \param a Endpoint of first line.
 * \param b Base point of both lines.
 * \param c Endpoint of second line.
 */
void
lib3ds_vector_normal(Lib3dsVector n, Lib3dsVector a, Lib3dsVector b, Lib3dsVector c) {
    Lib3dsVector p, q;

    lib3ds_vector_sub(p, c, b);
    lib3ds_vector_sub(q, a, b);
    lib3ds_vector_cross(n, p, q);
    lib3ds_vector_normalize(n);
}


/*!
 * Multiply a point by a transformation matrix.
 *
 * Applies the given transformation matrix to the given point.  With some
 * transformation matrices, a vector may also be transformed.
 *
 * \param c Result.
 * \param m Transformation matrix.
 * \param a Input point.
 */
void
lib3ds_vector_transform(Lib3dsVector c, Lib3dsMatrix m, Lib3dsVector a) {
    c[0] = m[0][0] * a[0] + m[1][0] * a[1] + m[2][0] * a[2] + m[3][0];
    c[1] = m[0][1] * a[0] + m[1][1] * a[1] + m[2][1] * a[2] + m[3][1];
    c[2] = m[0][2] * a[0] + m[1][2] * a[1] + m[2][2] * a[2] + m[3][2];
}


/*!
 * Compute a point on a cubic spline.
 *
 * Computes a point on a parametric Bezier spline.
 *
 * \param c Result.
 * \param a First endpoint of the spline.
 * \param p First tangent vector of the spline.
 * \param q Second tangent vector of the spline.
 * \param b Second endpoint of the spline.
 * \param t Spline parameter [0. 1.]
 */
void
lib3ds_vector_cubic(Lib3dsVector c, Lib3dsVector a, Lib3dsVector p, Lib3dsVector q,
                    Lib3dsVector b, float t) {
    double x, y, z, w;

    x = 2 * t * t * t - 3 * t * t + 1;
    y = -2 * t * t * t + 3 * t * t;
    z = t * t * t - 2 * t * t + t;
    w = t * t * t - t * t;
    c[0] = (float)(x * a[0] + y * b[0] + z * p[0] + w * q[0]);
    c[1] = (float)(x * a[1] + y * b[1] + z * p[1] + w * q[1]);
    c[2] = (float)(x * a[2] + y * b[2] + z * p[2] + w * q[2]);
}


/*!
 * c[i] = min(c[i], a[i]);
 *
 * Computes minimum values of x,y,z independently.
 */
void
lib3ds_vector_min(Lib3dsVector c, Lib3dsVector a) {
    int i;
    for (i = 0; i < 3; ++i) {
        if (a[i] < c[i]) {
            c[i] = a[i];
        }
    }
}


/*!
 * c[i] = max(c[i], a[i]);
 *
 * Computes maximum values of x,y,z independently.
 */
void
lib3ds_vector_max(Lib3dsVector c, Lib3dsVector a) {
    int i;
    for (i = 0; i < 3; ++i) {
        if (a[i] > c[i]) {
            c[i] = a[i];
        }
    }
}


void
lib3ds_vector_dump(Lib3dsVector c) {
    fprintf(stderr, "%f %f %f\n", c[0], c[1], c[2]);
}

