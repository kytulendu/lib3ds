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
 * $Id$
 */
#include "lib3ds_impl.h"


void lib3ds_util_reserve_array(void ***ptr, Lib3dsIntd *n, Lib3dsIntd *size, 
                               Lib3dsIntd new_size, Lib3dsBool force, Lib3dsFreeFunc free_func)
{
    assert(ptr && n && size);
    if ((*size < new_size) || force) {
        if (force && free_func) {
            int i;
            for (i=new_size; i<*n; ++i) {
                free_func((*ptr)[i]);
                (*ptr)[i] = 0;
            }
        }
        *ptr = (void*)realloc(*ptr, sizeof(void*) * new_size);
        *size = new_size;
        if (*n > new_size) {
            *n = new_size;
        }
    }
}


void lib3ds_util_insert_array(void ***ptr, Lib3dsIntd *n, Lib3dsIntd *size, 
                              void *element, Lib3dsIntd index)
{
    int i;
    assert(ptr && n && size && element);
    i = ((index >= 0) && (index < *n))? index : *n;
    if (i >= *size) {
        int new_size = 2*(*size);
        #ifdef _DEBUG
        if (new_size < 1) {
            new_size = 1;
        }
        #else
        if (new_size < 32) {
            new_size = 32;
        }
        #endif
        lib3ds_util_reserve_array(ptr, n, size, new_size, FALSE, NULL);
    }
    assert(*ptr);
    if (i < *n) {
        memmove(&(*ptr)[i+1], &(*ptr)[i], sizeof(void*) * (*n - i));
    }
    (*ptr)[i] = element;
    *n = *n + 1;
}


void lib3ds_util_remove_array(void ***ptr, Lib3dsIntd *n, Lib3dsIntd index, Lib3dsFreeFunc free_func)
{
    assert(ptr && n);
    if ((index >= 0) && (index < *n)) {
        assert(*ptr);
        free_func((*ptr)[index]);
        if (index < *n - 1) {
            memmove(&(*ptr)[index], &(*ptr)[index+1], sizeof(void*) * (*n - index - 1));
        }
        *n = *n - 1;
    }
}
