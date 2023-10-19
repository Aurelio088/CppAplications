//
// Created by David Burchill on 2022-09-27.
//

#ifndef GEOWARS_COMPONENTS_H
#define GEOWARS_COMPONENTS_H

#include <memory>
#include <SFML/Graphics.hpp>
#include "Utilities.h"

struct Component
{
    bool		has{ false };
    Component() = default;

};


struct CShape : public Component
{
    sf::CircleShape circle;

    CShape() = default;


    CShape(float r, size_t points, const sf::Color& fill, const sf::Color& outline=sf::Color::Black, float thickness = 5.f)
            : circle(r, points)
    {
        circle.setFillColor(fill);
        circle.setOutlineColor(outline);
        circle.setOutlineThickness(thickness);
        circle.setOrigin(r, r);
    }
};


struct CInput : public Component
{
    bool up{ false };
    bool left{ false };
    bool right{ false };
    bool down{ false };

    CInput() = default;
};


struct CCollision : public Component
{
    float radius{0.f};

    CCollision() = default;


    CCollision(float r) : radius(r) {}

};

struct CScore : public Component
{
    int score{0};

    CScore() = default;
    CScore(int p) : score(p) {}
};


struct CTransform : public Component
{
    sf::Vector2f        pos{0.f, 0.f};
    sf::Vector2f		vel{ 0.f, 0.f };

    float               rot{ 0 };   // degrees
    float	            rotSpeed{ 0.f };  // degrees per second

    CTransform() = default;

    CTransform(sf::Vector2f p, sf::Vector2f v,  float rs = 60.f )
            : pos(p), vel(v),  rotSpeed(rs) {}
};


struct CLifespan : public Component
{
    sf::Time total{ sf::Time::Zero};
    sf::Time remaining{ sf::Time::Zero };

    CLifespan() = default;
    CLifespan(float t) : total(sf::seconds(t)), remaining{ sf::seconds(t)} {}

};




#endif //GEOWARS_COMPONENTS_H
