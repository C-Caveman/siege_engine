// useful primitives
#include "../defs.h"
# include <math.h>
///////////////////////////////////////////////////////////////////////////////////////////////
// Vector of 2 floats:
//

//vec2f::vec2f() : x(0), y(0) {}
//vec2f::vec2f(float new_x, float new_y) : x(new_x), y(new_y) {}
//vec2f::~vec2f() {}

float vec2f::get_x() {return x;}
float vec2f::get_y() {return y;}
void vec2f::set_x(float new_val) {x = new_val;}
void vec2f::set_y(float new_val) {y = new_val;}

float vec2f::is_cardinal() {
    return (this->x && this->y);
}
float vec2f::vlen() {
    float length = sqrt(x*x + y*y);
    if (this->x && this->y && length > 2)
        length -= 2;
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
ostream& operator << (ostream& os, const vec2f& v)
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


///////////////////////////////////////////////////////////////////////////////////////////////
// Vector of 2 ints:
//
//vec2i::vec2i() : x(0), y(0) {}
//vec2i::vec2i(int new_x, int new_y) : x(new_x), y(new_y) {}
//vec2i::~vec2i() {}

// operator overloading
vec2i vec2i::operator + (const vec2i& v) {
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
ostream& operator << (ostream& os, const vec2i& v)
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
