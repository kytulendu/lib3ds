/*
    Copyright (C) 1996-2008 by Jan Eric Kyprianidis <www.kyprianidis.com>
    All rights reserved.
    
    This program is free  software: you can redistribute it and/or modify 
    it under the terms of the GNU Lesser General Public License as published 
    by the Free Software Foundation, either version 2.1 of the License, or 
    (at your option) any later version.

    Thisprogram  is  distributed in the hope that it will be useful, 
    but WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
    GNU Lesser General Public License for more details.
    
    You should  have received a copy of the GNU Lesser General Public License
    along with  this program; If not, see <http://www.gnu.org/licenses/>. 
*/
#include <lib3ds.h>
#include <stdio.h>
#include <string.h>


static float g_vertices[8][3] = {
    { -10.0, -10.0,  10.0 },
    {  10.0, -10.0,  10.0 },
    {  10.0,  10.0,  10.0 },
    { -10.0,  10.0,  10.0 },
    { -10.0, -10.0, -10.0 },
    {  10.0, -10.0, -10.0 },
    {  10.0,  10.0, -10.0 },
    { -10.0,  10.0, -10.0 }
};


// CCW
static unsigned short g_indices[12][3] = {
    { 0, 5, 1 },
    { 0, 4, 5 },
    { 1, 6, 2 },
    { 1, 5, 6 },
    { 2, 6, 7 },
    { 2, 7, 3 },
    { 0, 3, 7 },
    { 0, 7, 4 },
    { 0, 1, 2 },
    { 0, 2, 3 },
    { 4, 7, 6 },
    { 4, 6, 5 }
};


int main(int argc, char **argv) {
    Lib3dsFile *file = lib3ds_file_new();
    {
        int i, j;
        Lib3dsMesh *mesh = lib3ds_mesh_new("cube");
        lib3ds_file_insert_mesh(file, mesh, -1);

        lib3ds_mesh_resize_vertices(mesh, 8);
        for (i = 0; i < 8; ++i) {
            lib3ds_vector_copy(mesh->vertices[i].pos, g_vertices[i]);
        }

        lib3ds_mesh_resize_faces(mesh, 12);
        for (i = 0; i < 12; ++i) {
            for (j = 0; j < 3; ++j) {
                mesh->faces[i].index[j] = g_indices[i][j];
            }
        }
    }

    {
        Lib3dsNode *node;
        Lib3dsObjectNode *onode;
            
        node = lib3ds_node_new(LIB3DS_OBJECT_NODE);
        lib3ds_file_insert_node(file, node);

        onode = (Lib3dsObjectNode*)node;
        strcpy(onode->base.name, "cube");
        strcpy(onode->instance, "01");

        onode->pos_track =  lib3ds_track_new(node, LIB3DS_TRACK_VECTOR, 2);
        onode->pos_track->keys[0].value[0] = 0.0;
        onode->pos_track->keys[0].value[1] = 0.0;
        onode->pos_track->keys[0].value[2] = 0.0;

        onode->scl_track =  lib3ds_track_new(node, LIB3DS_TRACK_VECTOR, 1);
        onode->scl_track->keys[0].value[0] = 1.0;
        onode->scl_track->keys[0].value[1] = 1.0;
        onode->scl_track->keys[0].value[2] = 1.0;

        onode->rot_track = lib3ds_track_new(node, LIB3DS_TRACK_QUAT, 1);
        onode->rot_track->keys[0].value[0] = 0.0;
        onode->rot_track->keys[0].value[1] = 0.0;
        onode->rot_track->keys[0].value[2] = 0.0;
        onode->rot_track->keys[0].value[3] = 0.0;
    }

    if (!lib3ds_file_save(file, "cube.3ds")) {
        fprintf(stderr, "ERROR: Saving 3ds file failed!\n");
    }
    lib3ds_file_free(file);
}
