#include "ent.h"

ent::ent() {
    id = 0;
    type = 0;
    anim = 0;
    frame_start = 0;
    think_frame = 0;
    anim_len = 0;
    pos = vec2f{0,0};
    dir = vec2f{0,0};
    vel = vec2f{0,0};
    next = 0;
    prev = 0;
}

ent::ent(int i, int ty, int an, int fs, int tf, int al, vec2f p, vec2f d, vec2f v) {
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

void ent::move_ent(vec2f p) {
    pos = p;
}

void ent::slide_ent(vec2f distance) {
    pos = pos + distance;
}

// box to box collision
void ent::collide_ent(ent* ent_b) {
    vec2f dpos = ent_b->get_pos() - pos;
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
        slide_ent(vec2f {dpx_sign*(RSIZE - dpx), 0});
    else
        slide_ent(vec2f{0, dpy_sign*(RSIZE - dpy)});
}

// circle to box collision
void ent::collide_ent_cs(ent* ent_b) {
    // box position
    vec2f bpos = ent_b->get_pos();
    // delta in position
    vec2f dpos = bpos - pos;
    // corner position (initialized to top left)
    vec2f cpos = bpos;
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
        vec2f truepos = pos + vec2f{CIRCLE_R, CIRCLE_R};
        vec2f cd = cpos-truepos;
        cd = cd.normalized() * abs(cd.vlen()-CIRCLE_R);
        // slide off the corner
        slide_ent(
            vec2f{dpx_sign, dpy_sign}*cd.vlen()
        );
    }
    // box on box collision
    else {
        if (dpx > dpy) {
            slide_ent(vec2f {dpx_sign*(RSIZE - dpx), 0});
            // contact friction
            vel.set_x(vel.get_x()*0.9);
        }
        else {
            slide_ent(vec2f {0, dpy_sign*(RSIZE - dpy)});
            // contact friction
            vel.set_y(vel.get_y()*0.9);
        }
    }
}

int ent::get_typ() {return type;}

vec2f ent::get_pos() {return pos;}
void ent::set_pos(vec2f p) {pos = p;}

vec2f ent::get_dir() {return dir;}
void ent::set_dir(vec2f d) {dir = d;}

vec2f ent::get_vel() {return vel;}
void ent::set_vel(vec2f v) {vel = v;}

int ent::get_id() {return id;}

int ent::get_anim() {return anim;}
void ent::set_anim(int animation) {anim = animation;}

ostream& operator << (ostream& os, const ent& e) {
    os << "ent type " << e.type << ", pos " << e.pos << ", vel " << e.vel << ", id " << e.id;
    return os;
}

ent* ent::get_next() {return next;}

ent* ent::get_prev() {return prev;}

void ent::set_next(ent* n) {next = n;}

void ent::set_prev(ent* p) {prev = p;}

void ent::set_state(int state_index, int new_state) {
    state_info[state_index] = (uint8_t) new_state;
}
int ent::get_state(int state_index) {return (int)state_info[state_index];}


/////////////////////////////////////////////////////////////////////////////////////////
// Initialize an entity (to the default values for its type).
//

/* Segments common to most entities.
enum ent_basics_segments {                  // basic entity (component of most entitites)
    head, chunkpos, pos, vel, dir, anim, health,
    basic_ent_size
};*/

/* enum player_segments {                      // player
    player_end_of_basics=basic_ent_size-1,
    weapon, name, whatever, ect,
    player_size
};*/
void ent_player::init() {
    data[head].head.type = PLAYER;
    data[head].head.flags = DRAWABLE | MOVABLE | ANIMATABLE | THINKABLE;
}
void ent_scenery::init() {}

/////////////////////////////////////////////////////////////////////////////////////////
// Spawn an entity (insert a given type of ent to an array of entitity segments).
//
// Return the size of the current entity type (in segments).
int get_ent_size(int type) {
    int size = -1;
    switch (type) {
    case EMPTY:
        size=0;
        break;
    case PLAYER:
        size=player_size;
        break;
    case SCENERY:
        size=scenery_size;
        break;
    default:
        printf("*** Unknown entity type in get_ent_size()\n");
        exit(-1);
    }
    return size;
}
// Make a new entity in the given segment array. Return its index. TODO init the entity TODO
segment* spawn_ent(int type, segment* array, int array_len) {
    int index = -1;
    int required_space = get_ent_size(type);
    int empty_space_len = 0;
    int i = 0;
    while (i<array_len) {
        // Empty slot?
        if (array[i].head.type == EMPTY) {
            printf("Found an open slot at %d.\n", i);
            empty_space_len += 1;
            i += 1;
        }
        // Slot occupied.
        else {
            printf("Slots [%d, %d] already taken.\n", i, i+array[i].head.size-1);
            empty_space_len = 0;
            i += array[i].head.size;
        }
        // Got enough space to store the ent.
        if (empty_space_len == required_space) {
            printf("Found enough space for ent in [%d, %d]\n", i-required_space, i-1);
            index = i-required_space;
            break;
        }
    }
    if (index == -1) {
        printf("***\n*** No space left in the entity array!!!\n***\n");
        return nullptr;
    }
    // Initialize the entity:
    array[index].head.type = type;
    array[index].head.size = required_space;
    return &array[index];
}
// Remove an entity from an entity segment array.
void despawn_ent(segment* ent_header) {
    int size = ent_header->head.size;
    printf("Despawning ent of size %d\n", size);
    memset(ent_header, 0, size*sizeof(segment));
}