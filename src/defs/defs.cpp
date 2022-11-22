// useful primitives
#include "../defs.h"
# include <math.h>

vec2::vec2() : x(0), y(0) {}

vec2::vec2(float new_x, float new_y) : x(new_x), y(new_y) {}

vec2::~vec2() {}

float vec2::get_x() {return x;}

float vec2::get_y() {return y;}

void vec2::set_x(float new_val) {x = new_val;}

void vec2::set_y(float new_val) {y = new_val;}

float vec2::is_cardinal() {
    return (this->x && this->y);
}

float vec2::vlen() {
    float length = sqrt(x*x + y*y);
    if (this->x && this->y && length > 2)
        length -= 2;
    return length;
}

float vec2::dot(vec2& v) {
    return v.x*x + v.y*y;
}

void vec2::normalize() {
    float length = sqrt(x*x + y*y);
    if (length == 0) return; // don't divide by zero!!!!
    x /= length;
    y /= length;
}

void vec2::semi_normalize() {
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

vec2 vec2::normalized() {
    vec2 new_vec(x, y);
    float length = sqrt(x*x + y*y);
    if (length == 0) 
        return new_vec; // don't divide by zero!!!!
    new_vec.x /= length;
    new_vec.y /= length;
    return new_vec;
}

vec2 vec2::semi_normalized() {
    vec2 new_vec(x, y);
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

void vec2::scale_to(float new_length) {
    float old_length = this->vlen();
    float length_ratio = old_length / new_length;
    x *= length_ratio;
    y *= length_ratio;
}

vec2 vec2::scaled_to(float new_length) {
    vec2 new_vec(this->x, this->y);
    float old_length = this->vlen();
    if (old_length == 0)
        old_length = 1;
    float length_ratio = new_length / old_length;
    new_vec.x *= length_ratio;
    new_vec.y *= length_ratio;
    return new_vec;
    /*
    float old_length = this->vlen();
    if (old_length != 0)
        return vec2(x, y) * new_length / old_length;
    else
        return vec2(x, y) * new_length;
    */
}

void vec2::print() {
    printf("(%f, %f)", x, y);
}


// operator overloading
vec2 vec2::operator + (const vec2& v) {
    return vec2(x + v.x, y + v.y);
}

vec2 vec2::operator - (const vec2& v){
    return vec2(x - v.x, y - v.y);
}

vec2 vec2::operator * (const vec2& v){
    return vec2(x * v.x, y * v.y);
}

vec2 vec2::operator / (const vec2& v){
    return vec2(x / v.x, y / v.y);
}

void vec2::operator = (const vec2& v) {
    x = v.x;
    y = v.y;
}

bool vec2::operator == (const vec2& v) {
    if (x == v.x && y == v.y)
        return true;
    else
        return false;
}

ostream& operator << (ostream& os, const vec2& v)
{
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}

// scale vector by an float
vec2 vec2::operator * (const float& scale){
    return vec2(x * scale, y * scale);
}

vec2 vec2::operator / (const float& scale){
    return vec2(x / scale, y / scale);
}
