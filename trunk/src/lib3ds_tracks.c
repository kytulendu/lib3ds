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
 * $Id: tracks.c,v 1.20 2007/06/15 09:33:19 jeh Exp $
 */
#include "lib3ds_impl.h"


Lib3dsTrack* lib3ds_track_new(Lib3dsTrackType type, Lib3dsDword nkeys) 
{
    Lib3dsTrack *track = (Lib3dsTrack*)calloc(sizeof(Lib3dsTrack), 1);
    track->type = type;
    if (type != LIB3DS_TRACK_UNKNOWN) {
        lib3ds_track_resize(track, nkeys);
    }
    return(track);
}


void lib3ds_track_free(Lib3dsTrack *track)
{
    assert(track);
    lib3ds_track_resize(track, 0);
    memset(track, 0, sizeof(Lib3dsTrack));
    free(track);
}


void lib3ds_track_resize(Lib3dsTrack *track, Lib3dsDword nkeys)
{
    char *p;

    assert(track);
    if ((track->nkeys == nkeys) || (track->type == LIB3DS_TRACK_UNKNOWN))
        return;

    p = (char*)realloc(track->keys, sizeof(Lib3dsKey)*nkeys);
    if (nkeys > track->nkeys) {
        memset(p+(sizeof(Lib3dsKey)*track->nkeys), 0, sizeof(Lib3dsKey)*(nkeys-track->nkeys));
    }
    track->keys = (Lib3dsKey*)p;
    track->nkeys = nkeys;
}


static void float_key_setup(Lib3dsKey *pp, Lib3dsKey *pc, Lib3dsKey *pn)
{
    float tm,cm,cp,bm,bp,tmcm,tmcp,ksm,ksp,kdm,kdp,c;
    float dt,fp,fn;
    Lib3dsFloat delm, delp;

    assert(pc);
    fp = fn = 1.0f;
    if (pp && pn) {
        dt = 0.5f * (pn->frame - pp->frame);
        fp = (float)(pc->frame - pp->frame) / dt;
        fn = (float)(pn->frame - pc->frame) / dt;
        c  = (float)fabs(pc->tcb.cont);
        fp = fp + c - c * fp;
        fn = fn + c - c * fn;
    }

    cm = 1.0f - pc->tcb.cont;
    tm = 0.5f * (1.0f - pc->tcb.tens);
    cp = 2.0f - cm;
    bm = 1.0f - pc->tcb.bias;
    bp = 2.0f - bm;
    tmcm = tm * cm;   
    tmcp = tm * cp;
    ksm = tmcm * bp * fp;       
    ksp = tmcp * bm * fp;
    kdm = tmcp * bp * fn;       
    kdp = tmcm * bm * fn;

    delm = delp = 0;
    if (pp) delm = pc->data.f.value - pp->data.f.value;
    if (pn) delp = pn->data.f.value - pc->data.f.value;
    if (!pp) delm = delp;
    if (!pn) delp = delm;

    pc->data.f.ds = ksm * delm + ksp * delp;
    pc->data.f.dd = kdm * delm + kdp * delp;    
}


static void pos_key_setup(Lib3dsKey *pp, Lib3dsKey *pc, Lib3dsKey *pn)
{
    float tm,cm,cp,bm,bp,tmcm,tmcp,ksm,ksp,kdm,kdp,c;
    float dt,fp,fn;
    Lib3dsVector delm, delp;
    int i;

    assert(pc);
    fp = fn = 1.0f;
    if (pp && pn) {
        dt = 0.5f * (pn->frame - pp->frame);
        fp = (float)(pc->frame - pp->frame) / dt;
        fn = (float)(pn->frame - pc->frame) / dt;
        c  = (float)fabs(pc->tcb.cont);
        fp = fp + c - c * fp;
        fn = fn + c - c * fn;
    }

    cm = 1.0f - pc->tcb.cont;
    tm = 0.5f * (1.0f - pc->tcb.tens);
    cp = 2.0f - cm;
    bm = 1.0f - pc->tcb.bias;
    bp = 2.0f - bm;
    tmcm = tm * cm;   
    tmcp = tm * cp;
    ksm = tmcm * bp * fp;       
    ksp = tmcp * bm * fp;
    kdm = tmcp * bp * fn;       
    kdp = tmcm * bm * fn;

    lib3ds_vector_zero(delm);
    lib3ds_vector_zero(delp);
    if (pp) lib3ds_vector_sub(delm, pc->data.v.value, pp->data.v.value);
    if (pn) lib3ds_vector_sub(delp, pn->data.v.value, pc->data.v.value);
    if (!pp) lib3ds_vector_copy(delm, delp);
    if (!pn) lib3ds_vector_copy(delp, delm);

    for (i=0; i<3; ++i) {
        pc->data.v.ds[i] = ksm*delm[i] + ksp*delp[i];
        pc->data.v.dd[i] = kdm*delm[i] + kdp*delp[i];    
    }
}


static void rot_key_setup(Lib3dsKey *prev, Lib3dsKey *cur, Lib3dsKey *next)
{
    Lib3dsFloat tm,cm,cp,bm,bp,tmcm,tmcp,ksm,ksp,kdm,kdp,c;
    Lib3dsFloat dt,fp,fn;
    Lib3dsQuat q,qm,qp,qa,qb;
    int i;

    assert(cur);
    if (prev) {
        if (cur->data.q.angle > LIB3DS_TWOPI-LIB3DS_EPSILON) {
            lib3ds_quat_axis_angle(qm, cur->data.q.axis, 0.0f);
            lib3ds_quat_ln(qm);
        }
        else {
            lib3ds_quat_copy(q, prev->data.q.quad);
            if (lib3ds_quat_dot(q, cur->data.q.quad) < 0) lib3ds_quat_neg(q);
            lib3ds_quat_ln_dif(qm, q, cur->data.q.quad);
        }
    }
    if (next) {
        if (next->data.q.angle > LIB3DS_TWOPI-LIB3DS_EPSILON) {
            lib3ds_quat_axis_angle(qp, next->data.q.axis, 0.0f);
            lib3ds_quat_ln(qp);
        }
        else {
            lib3ds_quat_copy(q, next->data.q.quad);
            if (lib3ds_quat_dot(q, cur->data.q.quad) < 0) lib3ds_quat_neg(q);
            lib3ds_quat_ln_dif(qp, cur->data.q.quad, q);
        }
    }
    if (!prev) lib3ds_quat_copy(qm, qp);
    if (!next) lib3ds_quat_copy(qp, qm);

    fp = fn = 1.0f;
    cm = 1.0f - cur->tcb.cont;
    if (prev && next) {
        dt = 0.5f*(next->frame - prev->frame);
        fp = (float)(cur->frame - prev->frame) / dt;
        fn = (float)(next->frame - cur->frame) / dt;
        c  = (float)fabs( cur->tcb.cont );
        fp = fp + c - c * fp;
        fn = fn + c - c * fn;
    }

    tm = 0.5f * (1.0f - cur->tcb.tens);
    cp = 2.0f - cm;
    bm = 1.0f - cur->tcb.bias;
    bp = 2.0f - bm;
    tmcm = tm * cm;   
    tmcp = tm * cp;
    ksm = 1.0f - tmcm * bp * fp;       
    ksp = -tmcp * bm * fp;
    kdm = tmcp * bp * fn;       
    kdp = tmcm * bm * fn - 1.0f;
        
    for (i=0; i<4; i++) {
        qa[i] = 0.5f * (kdm * qm[i] + kdp * qp[i]);
        qb[i] = 0.5f * (ksm * qm[i] + ksp * qp[i]);
    }
    lib3ds_quat_exp(qa);
    lib3ds_quat_exp(qb);

    lib3ds_quat_mul(cur->data.q.a, cur->data.q.quad, qa);
    lib3ds_quat_mul(cur->data.q.b, cur->data.q.quad, qb);
}


void lib3ds_track_setup(Lib3dsTrack *track)
{
    unsigned i;
    Lib3dsKey keyp, keyn;
    Lib3dsKey *pp, *pn;

    assert(track);
    if (track->type == LIB3DS_TRACK_QUAT) {
        Lib3dsQuat q;
        for (i=0; i<track->nkeys; ++i) {
            lib3ds_quat_axis_angle(q, track->keys[i].data.q.axis, track->keys[i].data.q.angle);
            if (i > 0) {
                lib3ds_quat_mul(track->keys[i].data.q.quad, q, track->keys[i-1].data.q.quad);
            } else {
                lib3ds_quat_copy(track->keys[i].data.q.quad, q);
            }
        }
    }

    if (track->nkeys <= 1)
        return;

    for (i=0; i<track->nkeys; ++i) {
        pp = pn = 0;
        if (i > 0) {
            pp = &track->keys[i-1];
        } else {
            if (track->flags & LIB3DS_SMOOTH) {
                keyp = track->keys[track->nkeys-2];
                keyp.frame = track->keys[track->nkeys-2].frame - (track->keys[track->nkeys-1].frame - track->keys[0].frame);
                pp = &keyp;
            }
        }
        if (i < track->nkeys-1) {
            pn = &track->keys[i+1];
        } else {
            if (track->flags & LIB3DS_SMOOTH) {
                keyn = track->keys[1];
                keyn.frame = track->keys[1].frame + (track->keys[track->nkeys-1].frame - track->keys[0].frame);
                pn = &keyn;
            }
        }

        switch (track->type) {
            case LIB3DS_TRACK_FLOAT:
                float_key_setup(pp, &track->keys[i], pn);
                break;

            case LIB3DS_TRACK_VECTOR:
                pos_key_setup(pp, &track->keys[i], pn);
                break;

            case LIB3DS_TRACK_QUAT:
                rot_key_setup(pp, &track->keys[i], pn);
                break;

            default:
                break;
        }
    }
}


static Lib3dsFloat
lib3ds_float_cubic(Lib3dsFloat a, Lib3dsFloat p, Lib3dsFloat q, Lib3dsFloat b, Lib3dsFloat t)
{
    Lib3dsDouble x,y,z,w;   

    x=2*t*t*t - 3*t*t + 1;
    y=-2*t*t*t + 3*t*t;
    z=t*t*t - 2*t*t + t;
    w=t*t*t - t*t;
    return((Lib3dsFloat)(x*a + y*b + z*p + w*q));
}


static int find_index(Lib3dsTrack *track, Lib3dsFloat t, Lib3dsFloat *u)
{
    unsigned i;
    Lib3dsFloat nt;
    Lib3dsIntd t0, t1;

    assert(track);
    assert(track->nkeys > 0);

    t0 = track->keys[0].frame;
    t1 = track->keys[track->nkeys-1].frame;
    if (track->flags & LIB3DS_REPEAT) {
        nt = (Lib3dsFloat)fmod(t - t0, t1 - t0) + t0;
    } else {
        nt = t;
    }

    if (nt <= t0) {
        return -1;
    }
    if (nt >= t1) {
        return track->nkeys;
    }

    for (i=1; i<track->nkeys; ++i) {
        if (nt < track->keys[i].frame)
            break;
    }

    *u = nt - (Lib3dsFloat)track->keys[i-1].frame;
    *u /= (Lib3dsFloat)(track->keys[i].frame - track->keys[i-1].frame);

    assert((*u >= 0.0f) && (*u <= 1.0f));
    return i;
}


void lib3ds_track_eval_bool(Lib3dsTrack *track, Lib3dsBool *b, Lib3dsFloat t)
{
    unsigned index;
    Lib3dsFloat u;

    *b = LIB3DS_FALSE;
    if (!track) return;

    index = find_index(track, t, &u);
    if (index < 0) {
        *b = LIB3DS_FALSE;
        return;
    }
    if (index >= track->nkeys ) {
        *b = !(track->nkeys & 1);
        return;
    }
    *b = !(index & 1);
}


void lib3ds_track_eval_float(Lib3dsTrack *track, Lib3dsFloat *f, Lib3dsFloat t)
{
    unsigned index;
    Lib3dsFloat u;

    *f = 0;
    if (!track) return;

    index = find_index(track, t, &u);
    if (index < 0) {
        *f = track->keys[0].data.f.value;
        return;
    }
    if (index >= track->nkeys ) {
        *f = track->keys[track->nkeys-1].data.f.value;
        return;
    }
    *f = lib3ds_float_cubic(
        track->keys[index-1].data.f.value,
        track->keys[index-1].data.f.dd,
        track->keys[index].data.f.ds,
        track->keys[index].data.f.value,
        u
    );
}


void lib3ds_track_eval_vector(Lib3dsTrack *track, Lib3dsVector p, Lib3dsFloat t)
{
    unsigned index;
    Lib3dsFloat u;

    lib3ds_vector_zero(p);
    if (!track) return;

    index = find_index(track, t, &u);
    if (index < 0) {
        lib3ds_vector_copy(p, track->keys[0].data.v.value);
        return;
    }
    if (index >= track->nkeys ) {
        lib3ds_vector_copy(p, track->keys[track->nkeys-1].data.v.value);
        return;
    }
    lib3ds_vector_cubic(
        p,
        track->keys[index-1].data.v.value,
        track->keys[index-1].data.v.dd,
        track->keys[index].data.v.ds,
        track->keys[index].data.v.value,
        u
    );
}


void lib3ds_track_eval_quat(Lib3dsTrack *track, Lib3dsQuat q, Lib3dsFloat t)
{
    int index;
    Lib3dsFloat u;

    lib3ds_quat_identity(q);
    if (!track) return;

    index = find_index(track, t, &u);
    if (index < 0) {
        lib3ds_quat_copy(q, track->keys[0].data.q.quad);
        return;
    }
    if (index >= (int)track->nkeys) {
        lib3ds_quat_copy(q, track->keys[track->nkeys-1].data.q.quad);
        return;
    }
    lib3ds_quat_squad(
        q,
        track->keys[index-1].data.q.quad,
        track->keys[index-1].data.q.a,
        track->keys[index].data.q.b,
        track->keys[index].data.q.quad,
        u
    );
}


static void tcb_read(Lib3dsTcb *tcb, Lib3dsIo *io)
{
    tcb->flags = lib3ds_io_read_word(io);
    if (tcb->flags & LIB3DS_USE_TENSION) {
        tcb->tens = lib3ds_io_read_float(io);
    }
    if (tcb->flags & LIB3DS_USE_CONTINUITY) {
        tcb->cont = lib3ds_io_read_float(io);
    }
    if (tcb->flags & LIB3DS_USE_BIAS) {
        tcb->bias = lib3ds_io_read_float(io);
    }
    if (tcb->flags & LIB3DS_USE_EASE_TO) {
        tcb->ease_to = lib3ds_io_read_float(io);
    }
    if (tcb->flags & LIB3DS_USE_EASE_FROM) {
        tcb->ease_from = lib3ds_io_read_float(io);
    }
}


Lib3dsBool lib3ds_track_read(Lib3dsTrack *track, Lib3dsIo *io)
{
    int nkeys;
    int i;

    track->flags = lib3ds_io_read_word(io);
    lib3ds_io_read_dword(io);
    lib3ds_io_read_dword(io);
    nkeys = lib3ds_io_read_intd(io);
    lib3ds_track_resize(track, nkeys);

    switch (track->type) {
        case LIB3DS_TRACK_BOOL:
            for (i=0; i<nkeys; ++i) {
                track->keys[i].frame = lib3ds_io_read_intd(io);
                tcb_read(&track->keys[i].tcb, io);
            }
            break;

        case LIB3DS_TRACK_FLOAT:
            for (i=0; i<nkeys; ++i) {
                track->keys[i].frame = lib3ds_io_read_intd(io);
                tcb_read(&track->keys[i].tcb, io);
                track->keys[i].data.f.value = lib3ds_io_read_float(io);
            }
            break;

        case LIB3DS_TRACK_VECTOR:
            for (i=0; i<nkeys; ++i) {
                track->keys[i].frame = lib3ds_io_read_intd(io);
                tcb_read(&track->keys[i].tcb, io);
                lib3ds_io_read_vector(io, track->keys[i].data.v.value);
            }
            break;

        case LIB3DS_TRACK_QUAT:
            for (i=0; i<nkeys; ++i) {
                track->keys[i].frame = lib3ds_io_read_intd(io);
                tcb_read(&track->keys[i].tcb, io);
                track->keys[i].data.q.angle = lib3ds_io_read_float(io);
                lib3ds_io_read_vector(io, track->keys[i].data.q.axis);
            }
            break;

        default:
            break;
    }

    lib3ds_track_setup(track);
    return LIB3DS_TRUE;
}


static Lib3dsBool
lib3ds_tcb_write(Lib3dsTcb *tcb, Lib3dsIo *io)
{
    lib3ds_io_write_word(io, tcb->flags);
    if (tcb->flags&LIB3DS_USE_TENSION) {
        lib3ds_io_write_float(io, tcb->tens);
    }
    if (tcb->flags&LIB3DS_USE_CONTINUITY) {
        lib3ds_io_write_float(io, tcb->cont);
    }
    if (tcb->flags&LIB3DS_USE_BIAS) {
        lib3ds_io_write_float(io, tcb->bias);
    }
    if (tcb->flags&LIB3DS_USE_EASE_TO) {
        lib3ds_io_write_float(io, tcb->ease_to);
    }
    if (tcb->flags&LIB3DS_USE_EASE_FROM) {
        lib3ds_io_write_float(io, tcb->ease_from);
    }
    if (lib3ds_io_error(io)) {
        return(LIB3DS_FALSE);
    }
    return(LIB3DS_TRUE);
}


Lib3dsBool lib3ds_track_write(Lib3dsTrack *track, Lib3dsIo *io)
{
    return LIB3DS_TRUE;
}


#if 0

Lib3dsBool
lib3ds_bool_track_write(Lib3dsBoolTrack *track, Lib3dsIo *io)
{
  Lib3dsBoolKey *k;
  Lib3dsDword num=0;
  for (k=track->keyL; k; k=k->next) {
    ++num;
  }
  lib3ds_io_write_word(io, (Lib3dsWord)track->flags);
  lib3ds_io_write_dword(io, 0);
  lib3ds_io_write_dword(io, 0);
  lib3ds_io_write_dword(io, num);

  for (k=track->keyL; k; k=k->next) {
    if (!lib3ds_tcb_write(&k->tcb,io)) {
      return(LIB3DS_FALSE);
    }
  }
  return(LIB3DS_TRUE);
}


Lib3dsBool
lib3ds_lin1_track_write(Lib3dsLin1Track *track, Lib3dsIo *io)
{
  Lib3dsLin1Key *k;
  Lib3dsDword num=0;
  for (k=track->keyL; k; k=k->next) {
    ++num;
  }
  lib3ds_io_write_word(io, (Lib3dsWord)track->flags);
  lib3ds_io_write_dword(io, 0);
  lib3ds_io_write_dword(io, 0);
  lib3ds_io_write_dword(io, num);

  for (k=track->keyL; k; k=k->next) {
    if (!lib3ds_tcb_write(&k->tcb,io)) {
      return(LIB3DS_FALSE);
    }
    lib3ds_io_write_float(io, k->value);
  }
  return(LIB3DS_TRUE);
}


Lib3dsBool
lib3ds_lin3_track_write(Lib3dsLin3Track *track, Lib3dsIo *io)
{
  Lib3dsLin3Key *k;
  Lib3dsDword num=0;
  for (k=track->keyL; k; k=k->next) {
    ++num;
  }
  lib3ds_io_write_word(io, (Lib3dsWord)track->flags);
  lib3ds_io_write_dword(io, 0);
  lib3ds_io_write_dword(io, 0);
  lib3ds_io_write_dword(io, num);

  for (k=track->keyL; k; k=k->next) {
    if (!lib3ds_tcb_write(&k->tcb,io)) {
      return(LIB3DS_FALSE);
    }
    lib3ds_io_write_vector(io, k->value);
  }
  return(LIB3DS_TRUE);
}


Lib3dsBool
lib3ds_quat_track_write(Lib3dsQuatTrack *track, Lib3dsIo *io)
{
  Lib3dsQuatKey *k;
  Lib3dsDword num=0;
  for (k=track->keyL; k; k=k->next) {
    ++num;
  }
  lib3ds_io_write_word(io, (Lib3dsWord)track->flags);
  lib3ds_io_write_dword(io, 0);
  lib3ds_io_write_dword(io, 0);
  lib3ds_io_write_dword(io, num);

  for (k=track->keyL; k; k=k->next) {
    if (!lib3ds_tcb_write(&k->tcb,io)) {
      return(LIB3DS_FALSE);
    }
    lib3ds_io_write_float(io, k->angle);
    lib3ds_io_write_vector(io, k->axis);
  }
  return(LIB3DS_TRUE);
}

void
lib3ds_morph_track_eval(Lib3dsMorphTrack *track, char *p, Lib3dsFloat t)
{
  Lib3dsMorphKey *k;
  char* result;

  ASSERT(p);
  if (!track->keyL) {
    strcpy(p,"");
    return;
  }
  if (!track->keyL->next) {
    strcpy(p,track->keyL->name);
    return;
  }


  /* TODO: this function finds the mesh frame that corresponds to this
   * timeframe.  It would be better to actually interpolate the mesh.
   */

  result=0;

  for(k = track->keyL;
      k->next != NULL && t >= k->next->tcb.frame;
      k = k->next);

  result=k->name;

  if (result) {
    strcpy(p,result);
  }
  else {
    strcpy(p,"");
  }
}


/*!
 * \ingroup tracks
 */
Lib3dsBool
lib3ds_morph_track_read(Lib3dsMorphTrack *track, Lib3dsIo *io)
{
  /* This function was written by Stephane Denis on 5-18-04 */
    int i;
    Lib3dsMorphKey *k, *pk;
    int keys;
    track->flags=lib3ds_io_read_word(io);
    lib3ds_io_read_dword(io);
    lib3ds_io_read_dword(io);
    keys=lib3ds_io_read_intd(io);

    for (i=0; i<keys; ++i) {
        k=lib3ds_morph_key_new();
        if (!lib3ds_tcb_read(&k->tcb, io)) {
            return(LIB3DS_FALSE);
        }
        if (!lib3ds_io_read_string(io, k->name, 11)) {
            return(LIB3DS_FALSE);
        }
        if (!track->keyL)
            track->keyL = k;
        else
            pk->next = k;
        pk = k;
    }
  return(LIB3DS_TRUE);
}

#endif
