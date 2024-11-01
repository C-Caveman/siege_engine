// useful primitives
#include "../defs.h"
# include <math.h>
///////////////////////////////////////////////////////////////////////////////////////////////
// Vector of 2 floats:
//

float vec2f::vlen() {
    float length = sqrt(x*x + y*y);
    if (this->x && this->y && length > 2)
        length -= 2;
    if (length != length)
        length = 0;
    return length;
}
float vec2f::dist(vec2f &b) {
    //vec2f a = vec2f {x, y};
    return (float)(*this - b).vlen();
}
float vec2f::dot(vec2f& v) {
    return v.x*x + v.y*y;
}
void vec2f::normalize() {
    float length = sqrt(x*x + y*y);
    if (length == 0) return; // don't divide by zero!!!!
    x /= length;
    y /= length;
}
void vec2f::semi_normalize() {
    bool x_pos = (x > 0);
    bool y_pos = (y > 0);
    bool x_neg = (x < 0);
    bool y_neg = (y < 0);
    float length = sqrt(x*x + y*y);
    if (length == 0) return; // don't divide by zero!!!!
    x /= length;
    y /= length;
    //
    // make sure no rounding down to zero takes place
    //
    if (x_pos && x == 0)
        x = 1;
    else if (x_neg && x == 0)
        x = -1;
    if (y_pos && y == 0)
        y = 1;
    else if (y_neg && y == 0)
        y = -1;
}
vec2f vec2f::normalized() {
    vec2f new_vec = vec2f{x, y};
    float length = sqrt(x*x + y*y);
    if (length == 0) 
        return new_vec; // don't divide by zero!!!!
    new_vec.x /= length;
    new_vec.y /= length;
    return new_vec;
}
vec2f vec2f::semi_normalized() {
    vec2f new_vec = vec2f{x, y};
    float length = sqrt(x*x + y*y);
    if (length == 0) 
        return new_vec; // don't divide by zero!!!!
    new_vec.x /= length;
    new_vec.y /= length;
    //
    // make sure no rounding down to zero takes place
    //
    if (x > 0 && new_vec.x == 0)
        new_vec.x = 1;
    else if (x < 0 && new_vec.x == 0)
        new_vec.x = -1;
    if (y > 0 && new_vec.y == 0)
        new_vec.y = 1;
    else if (y < 0 && new_vec.y == 0)
        new_vec.y = -1;
    return new_vec;
}
vec2f vec2f::floor() { // Round down to the nearest whole number.
    return vec2f {std::floor(x), std::floor(y)};
}
vec2i vec2f::to_int() { // Convert to vec2i
    return vec2i {(int)std::floor(x), (int)std::floor(y)};
}
vec2i vec2f::to_int_round_up() { // Convert to vec2i
    return vec2i {(int)std::floor(x + 0.5f), (int)std::floor(y + 0.5f)};
}

void vec2f::print() {
    printf("(%f, %f)", x, y);
}

// operator overloading
vec2f vec2f::operator + (const vec2f& v) {
    return vec2f{x + v.x, y + v.y};
}
vec2f vec2f::operator - (const vec2f& v){
    return vec2f{x - v.x, y - v.y};
}
vec2f vec2f::operator * (const vec2f& v){
    return vec2f{x * v.x, y * v.y};
}
vec2f vec2f::operator / (const vec2f& v){
    return vec2f{x / v.x, y / v.y};
}
void vec2f::operator = (const vec2f& v) {
    x = v.x;
    y = v.y;
}
bool vec2f::operator == (const vec2f& v) {
    if (x == v.x && y == v.y)
        return true;
    else
        return false;
}
bool vec2f::operator != (const vec2f& v) {
    if (x != v.x || y != v.y)
        return true;
    else
        return false;
}
std::ostream& operator << (std::ostream& os, const vec2f& v)
{
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}

// scale vector by an float
vec2f vec2f::operator * (const float& scale){
    return vec2f{x * scale, y * scale};
}
vec2f vec2f::operator / (const float& scale){
    return vec2f{x / scale, y / scale};
}


//==============================================================================// Vec2i Functions //
vec2f vec2i::to_float() { //----------------- Convert to vec2f
    return vec2f {(float)x, (float)y};
}
bool vec2i::in_bounds(int min, int max) { //-------- Check if within [min,max]
    if (x >= min && x <= max && y >= min && y <= max)
        { return true; }
    else
        { return false; }
}

vec2i vec2i::operator + (const vec2i& v) { //----------------- Operator overloading.
    return vec2i{x + v.x, y + v.y};
}
vec2i vec2i::operator - (const vec2i& v){
    return vec2i{x - v.x, y - v.y};
}
vec2i vec2i::operator * (const vec2i& v){
    return vec2i{x * v.x, y * v.y};
}
vec2i vec2i::operator / (const vec2i& v){
    return vec2i{x / v.x, y / v.y};
}
vec2i vec2i::operator % (const int modulo){
    return vec2i{x % modulo, y % modulo};
}
void vec2i::operator = (const vec2i& v) {
    x = v.x;
    y = v.y;
}
bool vec2i::operator == (const vec2i& v) {
    if (x == v.x && y == v.y)
        return true;
    else
        return false;
}
bool vec2i::operator != (const vec2i& v) {
    if (x != v.x || y != v.y)
        return true;
    else
        return false;
}
std::ostream& operator << (std::ostream& os, const vec2i& v)
{
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}
// scale vector
vec2i vec2i::operator * (const float& scale){
    return vec2i{(int)(x * scale), (int)(y * scale)};
}
vec2i vec2i::operator / (const int& scale){
    return vec2i{x / scale, y / scale};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility functions:
float randf() {
    return (float)((float)rand() / RAND_MAX);
}
float randfn() {
    return (float)((float)rand() / RAND_MAX) - 0.5;
}
vec2f angleToVector(float angle) {
    return vec2f{
        cos(angle/180*(float)M_PI),
        sin(angle/180*(float)M_PI)
    };
}
