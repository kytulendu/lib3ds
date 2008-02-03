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
#include "lib3ds_impl.h"


/*!
 * Create and return a new node object.
 *
 * The node is returned with an identity matrix. All other fields
 * are zero.
 *
 * \return Lib3dsNode
 */
Lib3dsNode*
lib3ds_node_new(Lib3dsNodeType type) {
    static size_t sizes[6] = {
        sizeof(Lib3dsAmbientNode),
        sizeof(Lib3dsObjectNode),
        sizeof(Lib3dsCameraNode),
        sizeof(Lib3dsTargetNode),
        sizeof(Lib3dsLightNode),
        sizeof(Lib3dsSpotNode)
    };
    Lib3dsNode *node = (Lib3dsNode*)calloc(sizes[type], 1);
    node->user_type = 'NODE';
    node->type = type;
    node->parent_id = LIB3DS_NO_PARENT;
    lib3ds_matrix_identity(node->matrix);
    return node;
}


static void
free_node_and_childs(Lib3dsNode *node) {
    assert(node);
    switch (node->type) {
        case LIB3DS_AMBIENT_NODE: {
            Lib3dsAmbientNode *n = (Lib3dsAmbientNode*)node;
            if (n->color_track)
                lib3ds_track_free(n->color_track);
            break;
        }

        case LIB3DS_OBJECT_NODE: {
            Lib3dsObjectNode *n = (Lib3dsObjectNode*)node;
            if (n->pos_track)
                lib3ds_track_free(n->pos_track);
            if (n->rot_track)
                lib3ds_track_free(n->rot_track);
            if (n->scl_track)
                lib3ds_track_free(n->scl_track);
            if (n->hide_track)
                lib3ds_track_free(n->hide_track);
            if (n->morph_track)
                lib3ds_track_free(n->morph_track);
            break;
        }

        case LIB3DS_CAMERA_NODE: {
            Lib3dsCameraNode *n = (Lib3dsCameraNode*)node;
            if (n->pos_track)
                lib3ds_track_free(n->pos_track);
            if (n->fov_track)
                lib3ds_track_free(n->fov_track);
            if (n->roll_track)
                lib3ds_track_free(n->roll_track);
            break;
        }

        case LIB3DS_TARGET_NODE: {
            Lib3dsCameraNode *n = (Lib3dsCameraNode*)node;
            if (n->pos_track)
                lib3ds_track_free(n->pos_track);
            break;
        }

        case LIB3DS_LIGHT_NODE: {
            Lib3dsLightNode *n = (Lib3dsLightNode*)node;
            if (n->pos_track)
                lib3ds_track_free(n->pos_track);
            if (n->color_track)
                lib3ds_track_free(n->color_track);
            if (n->hotspot_track)
                lib3ds_track_free(n->hotspot_track);
            if (n->falloff_track)
                lib3ds_track_free(n->falloff_track);
            if (n->roll_track)
                lib3ds_track_free(n->roll_track);
            break;
        }

        case LIB3DS_SPOT_NODE: {
            Lib3dsSpotNode *n = (Lib3dsSpotNode*)node;
            if (n->pos_track)
                lib3ds_track_free(n->pos_track);
            break;
        }
    } 
    {
        Lib3dsNode *p, *q;
        for (p = node->childs; p; p = q) {
            q = p->next;
            free_node_and_childs(p);
        }
    }
    free(node);
}


/*!
 * Free a node and all of its resources.
 *
 * \param node Lib3dsNode object to be freed.
 */
void
lib3ds_node_free(Lib3dsNode *node) {
    assert(node);
    free_node_and_childs(node);
}


/*!
 * Evaluate an animation node.
 *
 * Recursively sets node and its children to their appropriate values
 * for this point in the animation.
 *
 * \param node Node to be evaluated.
 * \param t time value, between 0. and file->frames
 */
void
lib3ds_node_eval(Lib3dsNode *node, float t) {
    assert(node);
    switch (node->type) {
        case LIB3DS_AMBIENT_NODE: {
            Lib3dsAmbientNode *n = (Lib3dsAmbientNode*)node;
            if (node->parent) {
                lib3ds_matrix_copy(node->matrix, node->parent->matrix);
            } else {
                lib3ds_matrix_identity(node->matrix);
            }
            lib3ds_track_eval_vector(n->color_track, n->color, t);
            break;
        }

        case LIB3DS_OBJECT_NODE: {
            float M[4][4];
            Lib3dsObjectNode *n = (Lib3dsObjectNode*)node;

            lib3ds_track_eval_vector(n->pos_track, n->pos, t);
            lib3ds_track_eval_quat(n->rot_track, n->rot, t);
            if (n->scl_track) {
                lib3ds_track_eval_vector(n->scl_track, n->scl, t);
            } else {
                n->scl[0] = n->scl[1] = n->scl[2] = 1.0f;
            }
            lib3ds_track_eval_bool(n->hide_track, &n->hide, t);
            //lib3ds_track_eval_morph(n->morph_track, n->morph, t);

            lib3ds_matrix_identity(M);
            lib3ds_matrix_translate(M, n->pos[0], n->pos[1], n->pos[2]);
            lib3ds_matrix_rotate_quat(M, n->rot);
            lib3ds_matrix_scale(M, n->scl[0], n->scl[1], n->scl[2]);

            if (node->parent) {
                lib3ds_matrix_mult(node->matrix, node->parent->matrix, M);
            } else {
                lib3ds_matrix_copy(node->matrix, M);
            }
            break;
        }

        case LIB3DS_CAMERA_NODE: {
            Lib3dsCameraNode *n = (Lib3dsCameraNode*)node;
            lib3ds_track_eval_vector(n->pos_track, n->pos, t);
            lib3ds_track_eval_float(n->fov_track, &n->fov, t);
            lib3ds_track_eval_float(n->roll_track, &n->roll, t);
            if (node->parent) {
                lib3ds_matrix_copy(node->matrix, node->parent->matrix);
            } else {
                lib3ds_matrix_identity(node->matrix);
            }
            lib3ds_matrix_translate(node->matrix, n->pos[0], n->pos[1], n->pos[2]);
            break;
        }

        case LIB3DS_TARGET_NODE: {
            Lib3dsTargetNode *n = (Lib3dsTargetNode*)node;
            lib3ds_track_eval_vector(n->pos_track, n->pos, t);
            if (node->parent) {
                lib3ds_matrix_copy(node->matrix, node->parent->matrix);
            } else {
                lib3ds_matrix_identity(node->matrix);
            }
            lib3ds_matrix_translate(node->matrix, n->pos[0], n->pos[1], n->pos[2]);
            break;
        }

        case LIB3DS_LIGHT_NODE: {
            Lib3dsLightNode *n = (Lib3dsLightNode*)node;
            lib3ds_track_eval_vector(n->pos_track, n->pos, t);
            lib3ds_track_eval_vector(n->color_track, n->color, t);
            lib3ds_track_eval_float(n->hotspot_track, &n->hotspot, t);
            lib3ds_track_eval_float(n->falloff_track, &n->falloff, t);
            lib3ds_track_eval_float(n->roll_track, &n->roll, t);
            if (node->parent) {
                lib3ds_matrix_copy(node->matrix, node->parent->matrix);
            } else {
                lib3ds_matrix_identity(node->matrix);
            }
            lib3ds_matrix_translate(node->matrix, n->pos[0], n->pos[1], n->pos[2]);
            break;
        }

        case LIB3DS_SPOT_NODE: {
            Lib3dsSpotNode *n = (Lib3dsSpotNode*)node;
            lib3ds_track_eval_vector(n->pos_track, n->pos, t);
            if (node->parent) {
                lib3ds_matrix_copy(node->matrix, node->parent->matrix);
            } else {
                lib3ds_matrix_identity(node->matrix);
            }
            lib3ds_matrix_translate(node->matrix, n->pos[0], n->pos[1], n->pos[2]);
            break;
        }
    }
    {
        Lib3dsNode *p;
        for (p = node->childs; p != 0; p = p->next) {
            lib3ds_node_eval(p, t);
        }
    }
}


/*!
 * Return a node object by name and type.
 *
 * This function performs a recursive search for the specified node.
 * Both name and type must match.
 *
 * \param node The parent node for the search
 * \param name The target node name.
 * \param type The target node type
 *
 * \return A pointer to the first matching node, or NULL if not found.
 */
Lib3dsNode*
lib3ds_node_by_name(Lib3dsNode *node, const char* name, Lib3dsNodeType type) {
    Lib3dsNode *p, *q;

    for (p = node->childs; p != 0; p = p->next) {
        if ((p->type == type) && (strcmp(p->name, name) == 0)) {
            return(p);
        }
        q = lib3ds_node_by_name(p, name, type);
        if (q) {
            return(q);
        }
    }
    return(0);
}


/*!
 * Return a node object by id.
 *
 * This function performs a recursive search for the specified node.
 *
 * \param node The parent node for the search
 * \param node_id The target node id.
 *
 * \return A pointer to the first matching node, or NULL if not found.
 */
Lib3dsNode*
lib3ds_node_by_id(Lib3dsNode *node, uint16_t node_id) {
    Lib3dsNode *p, *q;

    for (p = node->childs; p != 0; p = p->next) {
        if (p->node_id == node_id) {
            return(p);
        }
        q = lib3ds_node_by_id(p, node_id);
        if (q) {
            return(q);
        }
    }
    return(0);
}


void
lib3ds_node_read(Lib3dsNode *node, Lib3dsFile *file, Lib3dsIo *io) {
    Lib3dsChunk c;
    uint16_t chunk;

    assert(node);
    lib3ds_chunk_read_start(&c, 0, io);

    switch (c.chunk) {
        case LIB3DS_AMBIENT_NODE_TAG:
        case LIB3DS_OBJECT_NODE_TAG:
        case LIB3DS_CAMERA_NODE_TAG:
        case LIB3DS_TARGET_NODE_TAG:
        case LIB3DS_LIGHT_NODE_TAG:
        case LIB3DS_SPOTLIGHT_NODE_TAG:
        case LIB3DS_L_TARGET_NODE_TAG:
            break;
        
        default:
            return;
    }

    while ((chunk = lib3ds_chunk_read_next(&c, io)) != 0)  {
        switch (chunk) {
            case LIB3DS_NODE_ID: {
                node->node_id = lib3ds_io_read_word(io);
                lib3ds_io_log_indent(io, 1);
                lib3ds_io_log(io, LIB3DS_LOG_INFO, "ID=%d", (short)node->node_id);
                lib3ds_io_log_indent(io, -1);
                break;
            }

            case LIB3DS_NODE_HDR: {
                lib3ds_io_read_string(io, node->name, 64);
                node->flags1 = lib3ds_io_read_word(io);
                node->flags2 = lib3ds_io_read_word(io);
                node->parent_id = lib3ds_io_read_word(io);

                lib3ds_io_log_indent(io, 1);
                lib3ds_io_log(io, LIB3DS_LOG_INFO, "NAME=%s", node->name);
                lib3ds_io_log(io, LIB3DS_LOG_INFO, "PARENT=%d", (short)node->parent_id);
                lib3ds_io_log_indent(io, -1);
                break;
            }

            case LIB3DS_PIVOT: {
                if (node->type == LIB3DS_OBJECT_NODE) {
                    Lib3dsObjectNode *n = (Lib3dsObjectNode*)node;
                    lib3ds_io_read_vector(io, n->pivot);
                } else {
                    lib3ds_chunk_unknown(chunk, io);
                }
                break;
            }

            case LIB3DS_INSTANCE_NAME: {
                if (node->type == LIB3DS_OBJECT_NODE) {
                    Lib3dsObjectNode *n = (Lib3dsObjectNode*)node;
                    lib3ds_io_read_string(io, n->instance, 64);
                } else {
                    lib3ds_chunk_unknown(chunk, io);
                }
                break;
            }

            case LIB3DS_BOUNDBOX: {
                if (node->type == LIB3DS_OBJECT_NODE) {
                    Lib3dsObjectNode *n = (Lib3dsObjectNode*)node;
                    lib3ds_io_read_vector(io, n->bbox_min);
                    lib3ds_io_read_vector(io, n->bbox_max);
                } else {
                    lib3ds_chunk_unknown(chunk, io);
                }
                break;
            }

            case LIB3DS_COL_TRACK_TAG: {
                Lib3dsTrack **track = 0;
                switch (node->type) {
                    case LIB3DS_AMBIENT_NODE: {
                        Lib3dsAmbientNode *n = (Lib3dsAmbientNode*)node;
                        track = &n->color_track;
                        break;
                    }              
                    case LIB3DS_LIGHT_NODE: {
                        Lib3dsLightNode *n = (Lib3dsLightNode*)node;
                        track = &n->color_track;
                        break;
                    }
                    default:
                        lib3ds_chunk_unknown(chunk, io);
                }
                if (track) {
                    *track = lib3ds_track_new(node, LIB3DS_TRACK_VECTOR, 0);
                    lib3ds_track_read(*track, io);
                }
                break;
            }

            case LIB3DS_POS_TRACK_TAG: {
                Lib3dsTrack **track = 0;
                switch (node->type) {
                    case LIB3DS_OBJECT_NODE: {
                        Lib3dsObjectNode *n = (Lib3dsObjectNode*)node;
                        track = &n->pos_track;
                        break;
                    }
                    case LIB3DS_CAMERA_NODE: {
                        Lib3dsCameraNode *n = (Lib3dsCameraNode*)node;
                        track = &n->pos_track;
                        break;
                    }
                    case LIB3DS_TARGET_NODE: {
                        Lib3dsTargetNode *n = (Lib3dsTargetNode*)node;
                        track = &n->pos_track;
                        break;
                    }
                    case LIB3DS_LIGHT_NODE: {
                        Lib3dsLightNode *n = (Lib3dsLightNode*)node;
                        track = &n->pos_track;
                        break;
                    }
                    case LIB3DS_SPOT_NODE: {
                        Lib3dsSpotNode *n = (Lib3dsSpotNode*)node;
                        track = &n->pos_track;
                        break;
                    }
                    default:
                        lib3ds_chunk_unknown(chunk, io);
                }
                if (track) {
                    *track = lib3ds_track_new(node, LIB3DS_TRACK_VECTOR, 0);
                    lib3ds_track_read(*track, io);
                }
                break;
            }

            case LIB3DS_ROT_TRACK_TAG: {
                if (node->type == LIB3DS_OBJECT_NODE) {
                    Lib3dsObjectNode *n = (Lib3dsObjectNode*)node;
                    n->rot_track = lib3ds_track_new(node, LIB3DS_TRACK_QUAT, 0);
                    lib3ds_track_read(n->rot_track, io);
                } else {
                    lib3ds_chunk_unknown(chunk, io);
                }
                break;
            }

            case LIB3DS_SCL_TRACK_TAG: {
                if (node->type == LIB3DS_OBJECT_NODE) {
                    Lib3dsObjectNode *n = (Lib3dsObjectNode*)node;
                    n->scl_track = lib3ds_track_new(node, LIB3DS_TRACK_VECTOR, 0);
                    lib3ds_track_read(n->scl_track, io);
                } else {
                    lib3ds_chunk_unknown(chunk, io);
                }
                break;
            }

            case LIB3DS_FOV_TRACK_TAG: {
                if (node->type == LIB3DS_CAMERA_NODE) {
                    Lib3dsCameraNode *n = (Lib3dsCameraNode*)node;
                    n->fov_track = lib3ds_track_new(node, LIB3DS_TRACK_FLOAT, 0);
                    lib3ds_track_read(n->fov_track, io);
                } else {
                    lib3ds_chunk_unknown(chunk, io);
                }
                break;
            }

            case LIB3DS_HOT_TRACK_TAG: {
                if (node->type == LIB3DS_LIGHT_NODE) {
                    Lib3dsLightNode *n = (Lib3dsLightNode*)node;
                    n->hotspot_track = lib3ds_track_new(node, LIB3DS_TRACK_FLOAT, 0);
                    lib3ds_track_read(n->hotspot_track, io);
                } else {
                    lib3ds_chunk_unknown(chunk, io);
                }
                break;
            }

            case LIB3DS_FALL_TRACK_TAG: {
                if (node->type == LIB3DS_LIGHT_NODE) {
                    Lib3dsLightNode *n = (Lib3dsLightNode*)node;
                    n->falloff_track = lib3ds_track_new(node, LIB3DS_TRACK_FLOAT, 0);
                    lib3ds_track_read(n->falloff_track, io);
                } else {
                    lib3ds_chunk_unknown(chunk, io);
                }
                break;
            }

            case LIB3DS_ROLL_TRACK_TAG: {
                switch (node->type) {
                    case LIB3DS_CAMERA_NODE: {
                        Lib3dsCameraNode *n = (Lib3dsCameraNode*)node;
                        n->roll_track = lib3ds_track_new(node, LIB3DS_TRACK_FLOAT, 0);
                        lib3ds_track_read(n->roll_track, io);
                        break;
                    }
                    case LIB3DS_LIGHT_NODE: {
                        Lib3dsLightNode *n = (Lib3dsLightNode*)node;
                        n->roll_track = lib3ds_track_new(node, LIB3DS_TRACK_FLOAT, 0);
                        lib3ds_track_read(n->roll_track, io);
                        break;
                    }
                    default:
                        lib3ds_chunk_unknown(chunk, io);
                }
                break;
            }

            case LIB3DS_HIDE_TRACK_TAG: {
                if (node->type == LIB3DS_OBJECT_NODE) {
                    Lib3dsObjectNode *n = (Lib3dsObjectNode*)node;
                    n->hide_track = lib3ds_track_new(node, LIB3DS_TRACK_BOOL, 0);
                    lib3ds_track_read(n->hide_track, io);
                } else {
                    lib3ds_chunk_unknown(chunk, io);
                }
                break;
            }

            case LIB3DS_MORPH_SMOOTH: {
                if (node->type == LIB3DS_OBJECT_NODE) {
                    Lib3dsObjectNode *n = (Lib3dsObjectNode*)node;
                    n->morph_smooth = lib3ds_io_read_float(io);
                } else {
                    lib3ds_chunk_unknown(chunk, io);
                }
            }
            break;

            /*
            case LIB3DS_MORPH_TRACK_TAG: {
                if (node->type == LIB3DS_OBJECT_NODE) {
                    Lib3dsObjectNode *n = (Lib3dsObjectNode*)node;
                    n->morph_track = lib3ds_track_new(node, LIB3DS_TRACK_MORPH, 0);
                    lib3ds_track_read(n->morph_track, io);
                } else {
                    lib3ds_chunk_unknown(chunk, io);
                }
            }
            break;
            */

            default:
                lib3ds_chunk_unknown(chunk, io);
        }
    }

    lib3ds_chunk_read_end(&c, io);
}


void
lib3ds_node_write(Lib3dsNode *node, Lib3dsFile *file, Lib3dsIo *io) {
    Lib3dsChunk c;

    switch (node->type) {
        case LIB3DS_AMBIENT_NODE:
            c.chunk = LIB3DS_AMBIENT_NODE_TAG;
            break;

        case LIB3DS_OBJECT_NODE:
            c.chunk = LIB3DS_OBJECT_NODE_TAG;
            break;

        case LIB3DS_CAMERA_NODE:
            c.chunk = LIB3DS_CAMERA_NODE_TAG;
            break;

        case LIB3DS_TARGET_NODE:
            c.chunk = LIB3DS_TARGET_NODE_TAG;
            break;

        case LIB3DS_LIGHT_NODE:
            if (lib3ds_file_node_by_name(file, node->name, LIB3DS_SPOT_NODE)) {
                c.chunk = LIB3DS_SPOTLIGHT_NODE_TAG;
            } else {
                c.chunk = LIB3DS_LIGHT_NODE_TAG;
            }
            break;

        case LIB3DS_SPOT_NODE:
            c.chunk = LIB3DS_L_TARGET_NODE_TAG;
            break;

        default:
            assert(0);
            return;
    }
    
    lib3ds_chunk_write_start(&c, io);

    { /*---- LIB3DS_NODE_ID ----*/
        Lib3dsChunk c;
        c.chunk = LIB3DS_NODE_ID;
        c.size = 8;
        lib3ds_chunk_write(&c, io);
        lib3ds_io_write_intw(io, node->node_id);
    }

    { /*---- LIB3DS_NODE_HDR ----*/
        Lib3dsChunk c;
        c.chunk = LIB3DS_NODE_HDR;
        c.size = 6 + 1 + (uint32_t)strlen(node->name) + 2 + 2 + 2;
        lib3ds_chunk_write(&c, io);
        lib3ds_io_write_string(io, node->name);
        lib3ds_io_write_word(io, node->flags1);
        lib3ds_io_write_word(io, node->flags2);
        lib3ds_io_write_word(io, node->parent_id);
    }

    switch (c.chunk) {
        case LIB3DS_AMBIENT_NODE_TAG: { /*---- LIB3DS_COL_TRACK_TAG ----*/
            Lib3dsChunk c;
            Lib3dsAmbientNode *n = (Lib3dsAmbientNode*)node;
            c.chunk = LIB3DS_COL_TRACK_TAG;
            lib3ds_chunk_write_start(&c, io);
            lib3ds_track_write(n->color_track, io);
            lib3ds_chunk_write_end(&c, io);
            break;
        }

        case LIB3DS_OBJECT_NODE_TAG: { /*---- LIB3DS_PIVOT ----*/
            Lib3dsChunk c;
            Lib3dsObjectNode *n = (Lib3dsObjectNode*)node;
            c.chunk = LIB3DS_PIVOT;
            c.size = 18;
            lib3ds_chunk_write(&c, io);
            lib3ds_io_write_vector(io, n->pivot);

            { /*---- LIB3DS_INSTANCE_NAME ----*/
                Lib3dsChunk c;
                const char *name;
                if (strlen(n->instance)) {
                    name = n->instance;

                    c.chunk = LIB3DS_INSTANCE_NAME;
                    c.size = 6 + 1 + (uint32_t)strlen(name);
                    lib3ds_chunk_write(&c, io);
                    lib3ds_io_write_string(io, name);
                }
            }
            {
                int i;
                for (i = 0; i < 3; ++i) {
                    if ((fabs(n->bbox_min[i]) > LIB3DS_EPSILON) ||
                        (fabs(n->bbox_max[i]) > LIB3DS_EPSILON)) {
                        break;
                    }
                }

                if (i < 3) { /*---- LIB3DS_BOUNDBOX ----*/
                    Lib3dsChunk c;
                    c.chunk = LIB3DS_BOUNDBOX;
                    c.size = 30;
                    lib3ds_chunk_write(&c, io);
                    lib3ds_io_write_vector(io, n->bbox_min);
                    lib3ds_io_write_vector(io, n->bbox_max);
                }
            }
            { /*---- LIB3DS_POS_TRACK_TAG ----*/
                Lib3dsChunk c;
                c.chunk = LIB3DS_POS_TRACK_TAG;
                lib3ds_chunk_write_start(&c, io);
                lib3ds_track_write(n->pos_track, io);
                lib3ds_chunk_write_end(&c, io);
            }
            { /*---- LIB3DS_ROT_TRACK_TAG ----*/
                Lib3dsChunk c;
                c.chunk = LIB3DS_ROT_TRACK_TAG;
                lib3ds_chunk_write_start(&c, io);
                lib3ds_track_write(n->rot_track, io);
                lib3ds_chunk_write_end(&c, io);
            }
            { /*---- LIB3DS_SCL_TRACK_TAG ----*/
                Lib3dsChunk c;
                c.chunk = LIB3DS_SCL_TRACK_TAG;
                lib3ds_chunk_write_start(&c, io);
                lib3ds_track_write(n->scl_track, io);
                lib3ds_chunk_write_end(&c, io);
            }
            if (n->hide_track) { /*---- LIB3DS_HIDE_TRACK_TAG ----*/
                Lib3dsChunk c;
                c.chunk = LIB3DS_HIDE_TRACK_TAG;
                lib3ds_chunk_write_start(&c, io);
                lib3ds_track_write(n->hide_track, io);
                lib3ds_chunk_write_end(&c, io);
            }
            if (fabs(n->morph_smooth) > LIB3DS_EPSILON) { /*---- LIB3DS_MORPH_SMOOTH ----*/
                Lib3dsChunk c;
                c.chunk = LIB3DS_MORPH_SMOOTH;
                c.size = 10;
                lib3ds_chunk_write(&c, io);
                lib3ds_io_write_float(io, n->morph_smooth);
            }
            break;
        }

        case LIB3DS_CAMERA_NODE_TAG: { /*---- LIB3DS_POS_TRACK_TAG ----*/
            Lib3dsChunk c;
            Lib3dsCameraNode *n = (Lib3dsCameraNode*)node;
            c.chunk = LIB3DS_POS_TRACK_TAG;
            lib3ds_chunk_write_start(&c, io);
            lib3ds_track_write(n->pos_track, io);
            lib3ds_chunk_write_end(&c, io);
            
            { /*---- LIB3DS_FOV_TRACK_TAG ----*/
                Lib3dsChunk c;
                c.chunk = LIB3DS_FOV_TRACK_TAG;
                lib3ds_chunk_write_start(&c, io);
                lib3ds_track_write(n->fov_track, io);
                lib3ds_chunk_write_end(&c, io);
            }
            { /*---- LIB3DS_ROLL_TRACK_TAG ----*/
                Lib3dsChunk c;
                c.chunk = LIB3DS_ROLL_TRACK_TAG;
                lib3ds_chunk_write_start(&c, io);
                lib3ds_track_write(n->roll_track, io);
                lib3ds_chunk_write_end(&c, io);
            }
            break;
        }

        case LIB3DS_TARGET_NODE_TAG: { /*---- LIB3DS_POS_TRACK_TAG ----*/
            Lib3dsChunk c;
            Lib3dsTargetNode *n = (Lib3dsTargetNode*)node;
            c.chunk = LIB3DS_POS_TRACK_TAG;
            lib3ds_chunk_write_start(&c, io);
            lib3ds_track_write(n->pos_track, io);
            lib3ds_chunk_write_end(&c, io);
            break;
        }

        case LIB3DS_LIGHT_NODE_TAG: { /*---- LIB3DS_POS_TRACK_TAG ----*/
            Lib3dsChunk c;
            Lib3dsLightNode *n = (Lib3dsLightNode*)node;
            c.chunk = LIB3DS_POS_TRACK_TAG;
            lib3ds_chunk_write_start(&c, io);
            lib3ds_track_write(n->pos_track, io);
            lib3ds_chunk_write_end(&c, io);
            
            { /*---- LIB3DS_COL_TRACK_TAG ----*/
                Lib3dsChunk c;
                c.chunk = LIB3DS_COL_TRACK_TAG;
                lib3ds_chunk_write_start(&c, io);
                lib3ds_track_write(n->color_track, io);
                lib3ds_chunk_write_end(&c, io);
            }
            break;
        }

        case LIB3DS_SPOTLIGHT_NODE_TAG: { /*---- LIB3DS_POS_TRACK_TAG ----*/
            Lib3dsChunk c;
            Lib3dsLightNode *n = (Lib3dsLightNode*)node;
            c.chunk = LIB3DS_POS_TRACK_TAG;
            lib3ds_chunk_write_start(&c, io);
            lib3ds_track_write(n->pos_track, io);
            lib3ds_chunk_write_end(&c, io);

            { /*---- LIB3DS_COL_TRACK_TAG ----*/
                Lib3dsChunk c;
                c.chunk = LIB3DS_COL_TRACK_TAG;
                lib3ds_chunk_write_start(&c, io);
                lib3ds_track_write(n->color_track, io);
                lib3ds_chunk_write_end(&c, io);
            }
            { /*---- LIB3DS_HOT_TRACK_TAG ----*/
                Lib3dsChunk c;
                c.chunk = LIB3DS_HOT_TRACK_TAG;
                lib3ds_chunk_write_start(&c, io);
                lib3ds_track_write(n->hotspot_track, io);
                lib3ds_chunk_write_end(&c, io);
            }
            { /*---- LIB3DS_FALL_TRACK_TAG ----*/
                Lib3dsChunk c;
                c.chunk = LIB3DS_FALL_TRACK_TAG;
                lib3ds_chunk_write_start(&c, io);
                lib3ds_track_write(n->falloff_track, io);
                lib3ds_chunk_write_end(&c, io);
            }
            { /*---- LIB3DS_ROLL_TRACK_TAG ----*/
                Lib3dsChunk c;
                c.chunk = LIB3DS_ROLL_TRACK_TAG;
                lib3ds_chunk_write_start(&c, io);
                lib3ds_track_write(n->roll_track, io);
                lib3ds_chunk_write_end(&c, io);
            }
            break; 
        }

        case LIB3DS_L_TARGET_NODE_TAG: { /*---- LIB3DS_POS_TRACK_TAG ----*/
            Lib3dsChunk c;
            Lib3dsSpotNode *n = (Lib3dsSpotNode*)node;
            c.chunk = LIB3DS_POS_TRACK_TAG;
            lib3ds_chunk_write_start(&c, io);
            lib3ds_track_write(n->pos_track, io);
            lib3ds_chunk_write_end(&c, io);
            break;
        }

        default:
            break;
    }

    lib3ds_chunk_write_end(&c, io);
}

