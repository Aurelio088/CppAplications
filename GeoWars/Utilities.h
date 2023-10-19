//
// Created by David Burchill on 2022-09-27.
//

#ifndef GEOWARS_UTILITIES_H
#define GEOWARS_UTILITIES_H

#include <SFML/System.hpp>
#include <iostream>

sf::Vector2f    normalize(sf::Vector2f v);
float           bearing(const sf::Vector2f& v);
sf::Vector2f    uVecBearing(float b);
float           length(const sf::Vector2f& v);
float           dist(const sf::Vector2f& u, const sf::Vector2f& v);


float radToDeg(float r);
float degToRad(float d);


// center sprite origin
template <typename T>
inline void centerOrigin(T& s) {
    auto bounds = s.getLocalBounds();
    s.setOrigin(sf::Vector2f(bounds.width / 2.f, bounds.height / 2.f));
}


template <typename T>
inline void centerOrigin(T* s) {
    auto bounds = s->getLocalBounds();
    s->setOrigin(sf::Vector2f(bounds.width / 2.f, bounds.height / 2.f));
}

std::ostream& operator<<(std::ostream& os, sf::Vector2f v);

#endif //GEOWARS_UTILITIES_H
