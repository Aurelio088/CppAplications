//
// Created by David Burchill on 2022-09-27.
//

#ifndef GEOWARS_GAME_H
#define GEOWARS_GAME_H


#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

#include "Entity.h"
#include "EntityManager.h"

using uint = unsigned int;

// SR Shape radius, CR Collision radius, F(r,g,b) fill colour, O(r,g,b) outline colour
// V number of vertices, OT outline thickness, S speed, AS angular speed,
// VMIN minimum number of vertices, VMAX max number of vertices,
// L lifespan, SP, SI spawn interval, SMIN min speed, SMAX max speed

struct PlayerConfig { int  FR, FG, FB, OR, OG, OB, V; float  SR, CR, OT, S, AS; };
struct EnemyConfig { int  OR, OG, OB, VMIN, VMAX, L, SI; float  SR, CR, OT, SMIN, SMAX; };
struct BulletConfig { int  FR, FG, FB, OR, OG, OB, OT, V, L; float SR, CR, S; };

// Special Weapon
struct SpecialConfig { int  FR, FG, FB, OR, OG, OB, OT, V, L; float SR, CR, S; };


class Game {
private:
	const static sf::Time TIME_PER_FRAME;

	sf::Vector2u                m_windowSize{ 1280,768 };
	sf::RenderWindow            m_window;
	EntityManager               m_entityManager;
	sf::Font                    m_font;
	sPtrEntt                    m_player{ nullptr };
	int                         m_score{ 0 };

	PlayerConfig                m_playerConfig;
	EnemyConfig                 m_enemyConfig;
	BulletConfig                m_bulletConfig;

	// Special Weapon
	SpecialConfig			   m_specialConfig;
	int 					   m_specialWeaponCount{ 0 }; // number of special weapons


	bool                        m_isRunning{ true };
	bool                        m_isPaused{ false };
	bool                        m_drawBB{ false };

	// stats
	sf::Text                    m_statisticsText;
	sf::Time                    m_statisticsUpdateTime{ sf::Time::Zero };
	unsigned int                m_statisticsNumFrames{ 0 };


	// Systems
	void                        sMovement(sf::Time dt);
	void                        sUserInput();
	void                        sLifespan(sf::Time dt);
	void                        sRender();
	void                        sEnemySpawner(sf::Time dt);
	void                        sCollision();
	void                        sUpdate(sf::Time dt);


	// helpers
	void                        adjustPlayerPosition();
	void                        spawnPlayer();
	void                        spawnEnemy();
	void                        spawnSmallEnemies(sPtrEntt e);
	void                        spawnBullet(sf::Vector2f dir);
	void                        spawnSpecialWeapon(sf::Vector2f mPos2);
	void                        updateStatistics(sf::Time dt);
	void                        loadConfigFromFile(const std::string& path);
	sf::FloatRect               getViewBounds();
	void                        keepObjecsInBounds();
	void                        drawCR();

public:

	Game(const std::string& path);
	void run();


};

#endif //GEOWARS_GAME_H
