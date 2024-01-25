#ifndef DEFS
#define DEFS
// Some useful primitives and constants.

// Debug flags:
#define DEBUG_ENTS 0
#define DEBUG_ENT_SPAWNING 0
#define DEBUG_ENT_HANDLES 0
#define DEBUG_GRAPHICS 0
#define DEBUG_GRAPHICS_LOADING 0

#include <iostream>
using namespace std;

//====================================================================// Vectors //
struct vec2f {
    float x;
    float y;
    // ---------------------------------------------------- vec2f functions
    float vlen();
    float dist(vec2f &b);
    float dot(vec2f& v);
    void normalize();
    void semi_normalize(); // don't round x or y down to zero
    vec2f normalized();
    vec2f semi_normalized();
    void print();
    //---------------------------------------------------- operator overloading
    vec2f operator + (const vec2f& v);
    vec2f operator - (const vec2f& v);
    vec2f operator * (const vec2f& v);
    vec2f operator / (const vec2f& v);
    void operator = (const vec2f& v);
    bool operator == (const vec2f& v);
    bool operator != (const vec2f& v);
    friend ostream& operator << (ostream& os, const vec2f& v);
    // scale vector by an float
    vec2f operator * (const float& scale);
    vec2f operator / (const float& scale);
};
struct vec2i {
    int x;
    int y;
    //---------------------------------------------------- operator overloading
    vec2i operator + (const vec2i& v);
    vec2i operator - (const vec2i& v);
    vec2i operator * (const vec2i& v);
    vec2i operator / (const vec2i& v);
    void operator = (const vec2i& v);
    bool operator == (const vec2i& v);
    bool operator != (const vec2i& v);
    friend ostream& operator << (ostream& os, const vec2i& v);
    // scale vector by an float
    vec2i operator * (const float& scale);
    vec2i operator / (const int& scale);
};//===============================================================================//

#endif
