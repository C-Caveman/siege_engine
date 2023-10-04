#ifndef DEFS
#define DEFS

// some useful primitives and constants

#include <iostream>
using namespace std;

struct vec2f {
    float x;
    float y;
    
    //vec2f();
    //vec2f(float x, float y);
    //~vec2f();
    float get_x();
    float get_y();
    void set_x(float new_val);
    void set_y(float new_val);
    float is_cardinal();
    float vlen();
    float dot(vec2f& v);
    void normalize();
    void semi_normalize(); // don't round x or y down to zero
    vec2f normalized();
    vec2f semi_normalized();
    void print();
    // operator overloading
    vec2f operator + (const vec2f& v);
    vec2f operator - (const vec2f& v);
    vec2f operator * (const vec2f& v);
    vec2f operator / (const vec2f& v);
    void operator = (const vec2f& v);
    bool operator == (const vec2f& v);
    friend ostream& operator << (ostream& os, const vec2f& v);
    // scale vector by an float
    vec2f operator * (const float& scale);
    vec2f operator / (const float& scale);
};

struct vec2i {
    int x;
    int y;
    
    //vec2i();
    //vec2i(int x, int y);
    //~vec2i();
    // operator overloading
    vec2i operator + (const vec2i& v);
    vec2i operator - (const vec2i& v);
    vec2i operator * (const vec2i& v);
    vec2i operator / (const vec2i& v);
    void operator = (const vec2i& v);
    bool operator == (const vec2i& v);
    friend ostream& operator << (ostream& os, const vec2i& v);
    // scale vector by an float
    vec2i operator * (const float& scale);
    vec2i operator / (const int& scale);
};

#endif
