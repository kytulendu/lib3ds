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
#include <math.h>


#ifndef MPI
#define M_PI 3.14159265358979323846
#endif


static float g_vertices[8][3] = {
    { -10.0, -10.0,  15.0 },
    {  10.0, -10.0,  15.0 },
    {  10.0,  10.0,  15.0 },
    { -10.0,  10.0,  15.0 },
    { -10.0, -10.0, -15.0 },
    {  10.0, -10.0, -15.0 },
    {  10.0,  10.0, -15.0 },
    { -10.0,  10.0, -15.0 }
};


// Texture coodinate origin (0,0) is in bottom-left corner!
static float g_texcoords[8][3] = {
    {  0.00, 1.0 },
    {  0.25, 1.0 },
    {  0.50, 1.0 },
    {  0.75, 1.0 },
    {  0.00, 0.0 },
    {  0.25, 0.0 },
    {  0.50, 0.0 },
    {  0.75, 0.0 }
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
    file->frames = 360;
    
    {
        Lib3dsMaterial *mat = lib3ds_material_new("c_tex");
        lib3ds_file_insert_material(file, mat, -1);
        strcpy(mat->texture1_map.name, "cube.tga");
        mat->texture1_map.percent = 1.0;

        mat = lib3ds_material_new("c_red");
        lib3ds_file_insert_material(file, mat, -1);
        mat->diffuse[0] = 1.0;
        mat->diffuse[1] = 0.0;
        mat->diffuse[2] = 0.0;

        mat = lib3ds_material_new("c_blue");
        lib3ds_file_insert_material(file, mat, -1);
        mat->diffuse[0] = 0.0;
        mat->diffuse[1] = 0.0;
        mat->diffuse[2] = 1.0;
    }

    {
        int i, j;
        Lib3dsMesh *mesh = lib3ds_mesh_new("cube");
        lib3ds_file_insert_mesh(file, mesh, -1);

        lib3ds_mesh_resize_vertices(mesh, 8);
        for (i = 0; i < 8; ++i) {
            lib3ds_vector_copy(mesh->vertices[i].pos, g_vertices[i]);
            mesh->vertices[i].tex[0] = g_texcoords[i][0];
            mesh->vertices[i].tex[1] = g_texcoords[i][1];
        }

        lib3ds_mesh_resize_faces(mesh, 12);
        for (i = 0; i < 12; ++i) {
            for (j = 0; j < 3; ++j) {
                mesh->faces[i].index[j] = g_indices[i][j];
            }
        }

        for (i = 0; i < 8; ++i) {
            mesh->faces[i].material = 0;
        }
        for (i = 0; i < 2; ++i) {
            mesh->faces[8+i].material = 1;
        }
        for (i = 0; i < 2; ++i) {
            mesh->faces[10+i].material = 2;
        }

        lib3ds_file_new_mesh_node(file, mesh, NULL, NULL, NULL, NULL);
    }

    {
        Lib3dsCamera *camera;
        Lib3dsCameraNode *n;
        int i;

        camera = lib3ds_camera_new("camera01");
        lib3ds_file_insert_camera(file, camera, -1);
        lib3ds_vector_make(camera->position, 0.0, -100, 0.0);
        lib3ds_vector_make(camera->target, 0.0, 0.0, 0.0);

        n = lib3ds_file_new_camera_node(file, camera, NULL);
        lib3ds_file_new_target_node(file, camera, NULL);

        lib3ds_track_resize(&n->pos_track, 37);
        for (i = 0; i <= 36; i++) {
            n->pos_track.keys[i].frame = 10 * i;
            lib3ds_vector_make(n->pos_track.keys[i].value, (float)100.0 * cos(M_PI * i / 36.0), (float)100.0 * sin(M_PI * i / 36.0), 50.0);
        }
    }

    if (!lib3ds_file_save(file, "cube.3ds")) {
        fprintf(stderr, "ERROR: Saving 3ds file failed!\n");
    }
    lib3ds_file_free(file);
}
