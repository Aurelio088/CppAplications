//
// Created by David Burchill on 2022-09-27.
//

#include "Utilities.h"
#include <cmath>
#include <numbers>

static const float PI = std::numbers::pi_v<float>;


float radToDeg(float r) {
    return r * 180.f / PI ;
}


float degToRad(float d) {
    return d * PI / 180.f;
}


sf::Vector2f normalize(sf::Vector2f v)
{
    static const float epsi = 0.00001f;
    float d = length(v);
    if (d > epsi)
        v = v/d;
    return v;
}

float length(const sf::Vector2f& v)
{
    return std::sqrtf(v.x*v.x + v.y*v.y);
}

float dist(const sf::Vector2f& u, const sf::Vector2f& v)
{
    return length(v - u);
}


float bearing(const sf::Vector2f& v)
{
    return radToDeg(std::atan2(v.y, v.x));
}


sf::Vector2f uVecBearing(float b)
{
    return sf::Vector2f(std::cos(degToRad(b)), std::sin(degToRad(b)));
}


std::ostream& operator<<(std::ostream& os, sf::Vector2f v) {
    os << "(" << (abs(v.x) < 0.0001 ? 0 : v.x )
    << ", " << (abs(v.y) < 0.0001 ? 0 : v.y )  << ")";
    return os;
}
