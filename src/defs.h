#ifndef DEFS
#define DEFS

// some useful primitives and constants

#include <iostream>
using namespace std;

class vec2 {
private:
    float x;
    float y;
public:
    vec2();
    vec2(float x, float y);
    ~vec2();
    float get_x();
    float get_y();
    void set_x(float new_val);
    void set_y(float new_val);
    float is_cardinal();
    float vlen();
    float dot(vec2& v);
    void normalize();
    void semi_normalize(); // don't round x or y down to zero
    vec2 normalized();
    vec2 semi_normalized();
    void scale_to(float new_length);
    vec2 scaled_to(float new_length);
    void print();
    // operator overloading
    vec2 operator + (const vec2& v);
    vec2 operator - (const vec2& v);
    vec2 operator * (const vec2& v);
    vec2 operator / (const vec2& v);
    void operator = (const vec2& v);
    bool operator == (const vec2& v);
    friend ostream& operator << (ostream& os, const vec2& v);
    // scale vector by an float
    vec2 operator * (const float& scale);
    vec2 operator / (const float& scale);
};

#endif
