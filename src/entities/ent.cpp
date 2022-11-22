#include "../ent.h"

ent::ent() {
    id = 0;
    type = 0;
    anim = 0;
    frame_start = 0;
    think_frame = 0;
    anim_len = 0;
    pos = vec2(0, 0);
    dir = vec2(0, 0);
    vel = vec2(0, 0);
    next = 0;
    prev = 0;
}

ent::ent(int i, int ty, int an, int fs, int tf, int al, vec2 p, vec2 d, vec2 v) {
    id = i;
    type = ty;
    anim = an;
    frame_start = fs;
    think_frame = tf;
    anim_len = al;
    pos = p;
    dir = d;
    vel = v;
    next = 0;
    prev = 0;
}
ent::~ent() {
    //TODO deal with the sprites
}

void ent::move_ent(vec2 p) {
    pos = p;
}

void ent::slide_ent(vec2 distance) {
    pos = pos + distance;
}

// box to box collision
void ent::collide_ent(ent* ent_b) {
    vec2 dpos = ent_b->get_pos() - pos;
    float dpx = dpos.get_x();
    float dpy = dpos.get_y();
    float dpx_sign = 1;
    float dpy_sign = 1;
    if (dpx > 0)
        dpx_sign = -1;
    if (dpy > 0)
        dpy_sign = -1;
    dpx = abs(dpx);
    dpy = abs(dpy);
    if (!(dpx < RSIZE && dpy < RSIZE))
        return; // no collision, our work here is done
    if (dpx > dpy)
        slide_ent(vec2( dpx_sign*(RSIZE - dpx), 0));
    else
        slide_ent(vec2(0, dpy_sign*(RSIZE - dpy)));
}

// circle to box collision
void ent::collide_ent_cs(ent* ent_b) {
    // box position
    vec2 bpos = ent_b->get_pos();
    // delta in position
    vec2 dpos = bpos - pos;
    // corner position (initialized to top left)
    vec2 cpos = bpos;
    float dpx = dpos.get_x();
    float dpy = dpos.get_y();
    float dpx_sign = 1;
    float dpy_sign = 1;
    if (dpx > 0)
        dpx_sign = -1;
    if (dpy > 0)
        dpy_sign = -1;
    dpx = abs(dpx);
    dpy = abs(dpy);
    // are the bounding boxes AND bounding circles NOT touching?
    if (!(dpx < RSIZE && dpy < RSIZE && dpos.vlen() < CIRCLE_R+RSIZE/2*sqrt(2.0))) {
        return;
        // no collision, our work here is done
    }
    // circle on corner point collision
    else if (abs(dpx - dpy) < CIRCLE_R) {
        // get the vec btwn circle and corner
        if (dpx_sign == 1) {
            cpos.set_x(bpos.get_x() + RSIZE);
            vel.set_x(vel.get_x()*0.9);
        }
        if (dpy_sign == 1) {
            cpos.set_y(bpos.get_y() + RSIZE);
            vel.set_y(vel.get_y()*0.9);
        }
        vec2 truepos = pos + vec2(CIRCLE_R, CIRCLE_R);
        vec2 cd = cpos-truepos;
        cd = cd.normalized() * abs(cd.vlen()-CIRCLE_R);
        // slide off the corner
        slide_ent(
            vec2(dpx_sign, dpy_sign)*cd.vlen()
        );
    }
    // box on box collision
    else {
        if (dpx > dpy) {
            slide_ent(vec2(dpx_sign*(RSIZE - dpx), 0));
            // contact friction
            vel.set_x(vel.get_x()*0.9);
        }
        else {
            slide_ent(vec2(0, dpy_sign*(RSIZE - dpy)));
            // contact friction
            vel.set_y(vel.get_y()*0.9);
        }
    }
}

int ent::get_typ() {return type;}

vec2 ent::get_pos() {return pos;}

void ent::set_pos(vec2 p) {pos = p;}

vec2 ent::get_dir() {return dir;}

void ent::set_dir(vec2 d) {dir = d;}

vec2 ent::get_vel() {return vel;}

void ent::set_vel(vec2 v) {vel = v;}

int ent::get_id() {return id;}

int ent::get_anim() {return anim;}

ostream& operator << (ostream& os, const ent& e) {
    os << "ent type " << e.type << ", pos " << e.pos << ", vel " << e.vel << ", id " << e.id;
    return os;
}

ent* ent::get_next() {return next;}

ent* ent::get_prev() {return prev;}

void ent::set_next(ent* n) {next = n;}

void ent::set_prev(ent* p) {prev = p;}

void ent::set_state(int state_index, int new_state) {
    state_info[state_index] = new_state;
}
int ent::get_state(int state_index) {return (int)state_info[state_index];}
