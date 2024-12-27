// useful primitives
#include "../defs.h"
# include <math.h>
///////////////////////////////////////////////////////////////////////////////////////////////
// vec2f functions: (vector of two floats)
//
float v2fLen(vec2f v) {
    float length = sqrt(v.x*v.x + v.y*v.y);
    if (v.x && v.y && length > 2)
        length -= 2;
    if (length != length)
        length = 0;
    return length;
}
float v2fDist(vec2f a, vec2f b) {
    //vec2f a = (vec2f) {x, y};
    return v2fLen(v2fSub(a,b));
}
float v2fDot(vec2f a, vec2f b) {
    return a.x*b.x + a.y*b.y;
}
void v2fNormalize(vec2f* v) {
    float length = sqrt(v->x*v->x + v->y*v->y);
    if (length == 0) return; // don't divide by zero!!!!
    v->x /= length;
    v->y /= length;
}
vec2f v2fNormalized(vec2f v) {
    vec2f new_vec = {v.x, v.y};
    float length = sqrt(v.x*v.x + v.y*v.y);
    if (length == 0) 
        return new_vec; // don't divide by zero!!!!
    new_vec.x /= length;
    new_vec.y /= length;
    return new_vec;
}
vec2f v2fFloor(vec2f v) { // Round down to the nearest whole number.
    return (vec2f) {floor(v.x), floor(v.y)};
}
vec2i v2fToI(vec2f v) { // Convert to vec2i
    return (vec2i) {floor(v.x), floor(v.y)};
}
vec2i v2fToIRoundUp(vec2f v) { // Convert to vec2i
    return (vec2i) {(int)floor(v.x + 0.5f), (int)floor(v.y + 0.5f)};
}
void v2fPrint(vec2f v) {
    printf("(%f, %f)", v.x, v.y);
}
vec2f v2fAdd(vec2f a, vec2f b) {
    return (vec2f){a.x + b.x, a.y + b.y};
}
vec2f v2fSub(vec2f a, vec2f b){
    return (vec2f){a.x - b.x, a.y - b.y};
}
vec2f v2fMult(vec2f a, vec2f b){
    return (vec2f){a.x * b.x, a.y * b.y};
}
vec2f v2fDiv(vec2f a, vec2f b){
    return (vec2f){a.x / b.x, a.y / b.y};
}
void v2fEq(vec2f a, vec2f b) {
    a.x = b.x;
    a.y = b.y;
}
bool v2fIsEq(vec2f a, vec2f b) {
    if (a.x == b.x && a.y == b.y)
        return true;
    else
        return false;
}
bool v2fNotIsEq(vec2f a, vec2f b) {
    if (a.x != b.x || a.y != b.y)
        return true;
    else
        return false;
}
#define v2fStringLen 64
char v2fString[v2fStringLen];
char* v2fToString(vec2f v) {
    snprintf(v2fString, v2fStringLen, "(%f, %f)", v.x, v.y);
    return v2fString;
}
// scale vector by an float
vec2f v2fScale(vec2f v, float scale){
    return (vec2f){v.x * scale, v.y * scale};
}
vec2f v2fScalarDiv(vec2f v, float scale){
    return (vec2f){v.x / scale, v.y / scale};
}

///////////////////////////////////////////////////////////////////////////////////////////////
// vec2i functions: (vector of two ints)
//
float v2iLen(vec2i v) {
    float length = sqrt(v.x*v.x + v.y*v.y);
    if (v.x && v.y && length > 2)
        length -= 2;
    if (length != length)
        length = 0;
    return length;
}
float v2iDist(vec2i a, vec2i b) {
    //vec2i a = vec2i {x, y};
    return v2iLen(v2iSub(a,b));
}
float v2iDot(vec2i a, vec2i b) {
    return a.x*b.x + a.y*b.y;
}
void v2iNormalize(vec2i* v) {
    float length = sqrt(v->x*v->x + v->y*v->y);
    if (length == 0) return; // don't divide by zero!!!!
    v->x /= length;
    v->y /= length;
}
vec2i v2iNormalized(vec2i v) {
    vec2i new_vec = {v.x, v.y};
    float length = sqrt(v.x*v.x + v.y*v.y);
    if (length == 0) 
        return new_vec; // don't divide by zero!!!!
    new_vec.x /= length;
    new_vec.y /= length;
    return new_vec;
}
vec2f v2iToF(vec2i v) { // Convert to vec2i
    return (vec2f) {(float)v.x, (float)v.y};
}
void v2iPrint(vec2i v) {
    printf("(%d, %d)", v.x, v.y);
}
vec2i v2iAdd(vec2i a, vec2i b) {
    return (vec2i){a.x + b.x, a.y + b.y};
}
vec2i v2iSub(vec2i a, vec2i b){
    return (vec2i){a.x - b.x, a.y - b.y};
}
vec2i v2iMult(vec2i a, vec2i b){
    return (vec2i){a.x * b.x, a.y * b.y};
}
vec2i v2iDiv(vec2i a, vec2i b){
    return (vec2i){a.x / b.x, a.y / b.y};
}
void v2iEq(vec2i a, vec2i b) {
    a.x = b.x;
    a.y = b.y;
}
bool v2iIsEq(vec2i a, vec2i b) {
    if (a.x == b.x && a.y == b.y)
        return true;
    else
        return false;
}
bool v2iNotIsEq(vec2i a, vec2i b) {
    if (a.x != b.x || a.y != b.y)
        return true;
    else
        return false;
}
#define v2iStringLen 64
char v2iString[v2iStringLen];
char* v2iToString(vec2i v) {
    snprintf(v2iString, v2iStringLen, "(%d, %d)", v.x, v.y);
    return v2iString;
}
vec2i v2iScale(vec2i v, float scale){
    return (vec2i){v.x * scale, v.y * scale};
}
vec2i v2iScalarDiv(vec2i v, float scale){
    return (vec2i){v.x / scale, v.y / scale};
}
vec2i v2iModulo(vec2i v, int modulus){
    return (vec2i){v.x % modulus, v.y % modulus};
}
bool v2iInBounds(vec2i v, int min, int max) {
    return (v.x >= min && v.x < max && v.y >= min && v.y < max);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility primitives:
#define MAX_COUNTER_VALUE 255
void counterInc(struct counter* c) {
    int ms_since_last_count = anim_tick - c->prevTick + (anim_tick < c->prevTick)*256;
    if (!(c->flags & (uint16_t)PAUSED) && (ms_since_last_count > c->interval) && c->count < MAX_COUNTER_VALUE) {
        c->prevTick = anim_tick;
        c->count += 1;
    }
}
void counterDec(struct counter* c) {
    int ms_since_last_count = anim_tick - c->prevTick + (anim_tick < c->prevTick)*256;
    if (!(c->flags & (uint16_t)PAUSED) && (ms_since_last_count > c->interval) && c->count > 0) {
        c->prevTick = anim_tick;
        c->count -= 1;
    }
}
// struct timer functions
void timerStart(struct timer* t) {
    t->start = curFrameStart;
    t->count = 0;
}
void timerUpdate(struct timer* t, uint32_t intervalMillis) {
    uint32_t elapsedMillis = ( (curFrameStart < t->start)*((uint32_t)0xffff) + curFrameStart ) - t->start; // Account for overflow in SDL_GetTicks() value.
    t->count = elapsedMillis / intervalMillis;
}
void timerUpdate30FPS(struct timer* t) {
    uint32_t elapsedMillis = ( (curFrameStart < t->start)*((uint32_t)0xffff) + curFrameStart ) - t->start; // Account for overflow in SDL_GetTicks() value.
    t->count = elapsedMillis >> 5; // Divide by 32 via bit shifting.
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility functions:
float randf() {
    return (float)((float)rand() / RAND_MAX);
}
float randfs() {
    float n = (float)((float)rand() / RAND_MAX);
    return n*n;
}
float randfn() {
    return (float)((float)rand() / RAND_MAX)*2.f - 1.f;
}
float randfns() {
    float n = ((float)((float)rand() / RAND_MAX))*2.f - 1.f;
    return n*fabs(n);
}
vec2f angleToVector(float angle) {
    return (vec2f){
        cos(angle/180*(float)F_PI),
        sin(angle/180*(float)F_PI)
    };
}
float vectorToAngle(vec2f v) {
    return (float)atan2(v.y, v.x)*180/(float)F_PI;
}
float fclamp(float n, float min, float max) {
    if (n < min)
        n = min;
    if (n > max)
        n = max;
    return n;
}
int   iclamp(int n, int min, int max) {
    if (n < min)
        n = min;
    if (n > max)
        n = max;
    return n;
}
#define MAX_TIME_DELTA (0xffff / 2)
bool isAfter(uint32_t timeA, uint32_t timeB) {
    uint32_t delta = timeB - timeA;
    if (timeA > timeB || delta < MAX_TIME_DELTA)
        return true;
    else
        return false;
}
