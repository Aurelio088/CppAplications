////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
//  Author:			David Burchill
//  Year / Term:    Fall 2022
//  File name:      sfml.cpp
// 
//  Author 2:		Aurelio Rodrigues
//  Email:			aureliorodrigue20s@hotmail.com
// 
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "Game.h"

#include "Utilities.h"
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

int main() {

	Game game("../config.txt");
	game.run();
	return 0;
}
