//
// Created by David Burchill on 2022-09-27.
// Updated by Aurelio Rodrigues on 2023-10-18.
// 
// Functions that AURELIO RODRIGUES worked on:
// 
//  - Game::sUserInput()
//  - Game::sMovement()
//  - Game::sRender()
//	- Game::loadConfigFromFile()
//	- Game::sCollision()
//  - Game::keepObjecsInBounds()
//  - Game::adjustPlayerPosition()
//	- Game::spawnPlayer()
//  - Game::sLifespan()
//  - Game::spawnEnemy()
//  - Game::spawnSmallEnemies()
//  - Game::spawnBullet()
//  - Game::spawnSpecialWeapon()
// 
//

#include "Game.h"
#include <fstream>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "Utilities.h"
#include <random>

// Random number generator
namespace {
	std::random_device rd;
	std::mt19937 rng(rd());
}

const sf::Time Game::TIME_PER_FRAME = sf::seconds((1.f / 60.f));

Game::Game(const std::string& path) {

	// load the game configuration from file "path"
	loadConfigFromFile(path);

	// now that you have the config loaded you can create the RenderWindow
	m_window.create(sf::VideoMode(m_windowSize.x, m_windowSize.y), "GEX Engine");

	// set up stats text to display FPS
	m_statisticsText.setFont(m_font);
	m_statisticsText.setPosition(15.0f, 15.0f);
	m_statisticsText.setCharacterSize(15);

	// spawn the player
	spawnPlayer();
}

void Game::sUserInput() {

	auto& uInput = m_player->getComponent<CInput>();

	sf::Event event;
	while (m_window.pollEvent(event)) {
		if (event.type == sf::Event::Closed) {
			m_window.close();
			m_isRunning = false;
		}

		// Handle key press events
		if (event.type == sf::Event::KeyPressed) {
			switch (event.key.code) {

				// Set moviment up
			case sf::Keyboard::Up:
			case sf::Keyboard::W:
				uInput.up = true;
				break;

				// Set moviment down
			case sf::Keyboard::S:
			case sf::Keyboard::Down:
				uInput.down = true;
				break;

				// Set moviment left
			case sf::Keyboard::A:
			case sf::Keyboard::Left:
				uInput.left = true;
				break;

				// Set moviment right
			case sf::Keyboard::D:
			case sf::Keyboard::Right:
				uInput.right = true;
				break;

				// Pause the game
			case sf::Keyboard::Escape:
				m_isPaused = !m_isPaused;
				break;

				// Draw bounding boxes
			case sf::Keyboard::G:
				m_drawBB = !m_drawBB;
				break;

				// Quit the game
			case sf::Keyboard::Q:
				m_isRunning = false;
				break;

			default:
				break;
			}
		}

		// Handle key released events
		if (event.type == sf::Event::KeyReleased) {
			switch (event.key.code) {

			case sf::Keyboard::Up:
			case sf::Keyboard::W:
				uInput.up = false;
				break;

			case sf::Keyboard::S:
			case sf::Keyboard::Down:
				uInput.down = false;
				break;

			case sf::Keyboard::A:
			case sf::Keyboard::Left:
				uInput.left = false;
				break;

			case sf::Keyboard::D:
			case sf::Keyboard::Right:
				uInput.right = false;
				break;

			default:
				break;
			}

		}

		// If mouse is pressed, spawn bullet
		if (event.type == sf::Event::MouseButtonPressed) {
			if (event.mouseButton.button == sf::Mouse::Left) {

				// Spawn bullet at mouse position
				spawnBullet(sf::Vector2f(event.mouseButton.x, event.mouseButton.y));
			}
		}

		/***************
		* Special Weapon - Starts
		****************/

		// Right click to activate special weapon
		if (event.type == sf::Event::MouseButtonPressed) {
			if (event.mouseButton.button == sf::Mouse::Right) {
				spawnSpecialWeapon(sf::Vector2f(event.mouseButton.x, event.mouseButton.y));
			}
		}

		/***************
		* Special Weapon - Ends
		****************/
	}
}

void Game::sUpdate(sf::Time dt) {

	// (by AURELIO RODRIGUES) Pause the game if m_isPaused is true
	if (m_isPaused == true) {
		return;
	}

	// (by AURELIO RODRIGUES) Spawn a new player if player was destroyed
	if (m_player->isActive() == false) {
		spawnPlayer();
	}

	m_entityManager.update();

	if (m_player == nullptr)
		spawnPlayer();

	sEnemySpawner(dt);
	sLifespan(dt);
	sMovement(dt);
	sCollision();
}

void Game::sMovement(sf::Time dt) {
	// Keep the player and enemies in bounds
	adjustPlayerPosition();
	keepObjecsInBounds();

	// Player movement
	sf::Vector2f pv; // pv = player velocity
	auto& playerInput = m_player->getComponent<CInput>();

	// Following the player input, set the velocity
	if (playerInput.left) pv.x -= m_playerConfig.S;
	if (playerInput.right) pv.x += m_playerConfig.S;
	if (playerInput.up) pv.y -= m_playerConfig.S;
	if (playerInput.down) pv.y += m_playerConfig.S;

	// Normalize the vector to make it a unit vector
	pv = m_playerConfig.S * normalize(pv);
	m_player->getComponent<CTransform>().vel = pv;

	// Move all the entities and apply wall collision
	for (auto& e : m_entityManager.getEntities()) {

		auto& tfm = e->getComponent<CTransform>();
		tfm.pos += tfm.vel * dt.asSeconds();
		tfm.rot += tfm.rotSpeed * dt.asSeconds();

		// Apply wall collision
		float entitySize = 0.5f * e->getComponent<CShape>().circle.getRadius();
		if (tfm.pos.x - entitySize < 0) {
			tfm.vel.x = std::abs(tfm.vel.x);  // Bounce off the left wall
		}
		else if (tfm.pos.x + entitySize > m_windowSize.x) {
			tfm.vel.x = -std::abs(tfm.vel.x);  // Bounce off the right wall
		}

		if (tfm.pos.y - entitySize < 0) {
			tfm.vel.y = std::abs(tfm.vel.y);  // Bounce off the top wall
		}
		else if (tfm.pos.y + entitySize > m_windowSize.y) {
			tfm.vel.y = -std::abs(tfm.vel.y);  // Bounce off the bottom wall
		}
	}
}

void Game::sRender() {

	// (by AURELIO RODRIGUES) have a different colour background to indicate the game is paused (200,200,255)
	if (m_isPaused == true) {
		m_window.clear(sf::Color(200, 200, 255));
	}
	else {
		m_window.clear(sf::Color(100, 100, 255));
	}

	// (by AURELIO RODRIGUES) Handle lifespan of the entities
	for (auto& e : m_entityManager.getEntities()) {

		auto& tfm = e->getComponent<CTransform>();
		auto& shape = e->getComponent<CShape>().circle;
		shape.setPosition(tfm.pos);
		shape.setRotation(tfm.rot);

		if (e->hasComponent<CLifespan>()) {
			auto& lifespan = e->getComponent<CLifespan>();
			sf::Color color = shape.getFillColor();

			float alpha = lifespan.remaining / lifespan.total;

			// Static_cast is used to convert float to int
			// https://www.geeksforgeeks.org/static_cast-in-cpp/
			color.a = static_cast<int>(alpha * 255);

			shape.setFillColor(color);
		}
		m_window.draw(shape);
	}

	if (m_drawBB)
		drawCR();

	sf::Text score("Score: " + std::to_string(m_score), m_font);
	score.setPosition(5, 30);
	m_window.draw(score);
	m_window.draw(m_statisticsText);
	m_window.display();
}


void Game::drawCR() {
	for (auto e : m_entityManager.getEntities()) {
		if (e->hasComponent<CCollision>()) {
			auto cr = e->getComponent<CCollision>().radius;
			auto trf = e->getComponent<CTransform>();
			sf::CircleShape bCirc;
			bCirc.setRadius(cr);
			centerOrigin(bCirc);

			bCirc.setPosition(trf.pos);
			bCirc.setFillColor(sf::Color(0, 0, 0, 0));
			bCirc.setOutlineColor(sf::Color(0, 255, 0));
			bCirc.setOutlineThickness(1.f);
			m_window.draw(bCirc);
		}
	}
}

void Game::run() {
	sf::Clock clock;
	sf::Time timeSinceLastUpdate = sf::Time::Zero;

	while (m_isRunning) {

		sUserInput();

		sf::Time elapsedTime = clock.restart();
		timeSinceLastUpdate += elapsedTime;
		while (timeSinceLastUpdate > TIME_PER_FRAME) {
			timeSinceLastUpdate -= TIME_PER_FRAME;
			sUserInput();
			sUpdate(TIME_PER_FRAME);
		}
		updateStatistics(elapsedTime);  // times per second world is rendered
		sRender();
	}
}

void Game::loadConfigFromFile(const std::string& path) {
	std::ifstream config(path);
	if (config.fail()) {
		std::cerr << "Open file " << path << " failed\n";
		config.close();
		exit(1);
	}

	std::string token{ "" };
	config >> token;
	while (!config.eof()) {
		if (token == "Window") {
			config >> m_windowSize.x >> m_windowSize.y;
		}
		else if (token == "Font") {
			std::string fontPath;
			config >> fontPath;
			if (!m_font.loadFromFile(fontPath)) {
				std::cerr << "Failed to load font " << fontPath << "\n";
				exit(1);
			}
		}
		else if (token == "Player") {
			auto& pcf = m_playerConfig;

			// Order of variables for Player in config file
			config >> pcf.SR >> pcf.CR >> pcf.S
				>> pcf.AS >> pcf.FR >> pcf.FG
				>> pcf.FB >> pcf.OR >> pcf.OG
				>> pcf.OB >> pcf.OT >> pcf.V;

		}
		else if (token == "Enemy") {
			auto& ecf = m_enemyConfig;

			// Order of variables for Enemy in config file
			config >> ecf.SR >> ecf.CR >> ecf.SMIN
				>> ecf.SMAX >> ecf.OR >> ecf.OG
				>> ecf.OB >> ecf.OT >> ecf.VMIN
				>> ecf.VMAX >> ecf.L >> ecf.SI;

		}
		else if (token == "Bullet") {
			auto& bcf = m_bulletConfig;

			// Order of variables for Bullet in config file
			config >> bcf.SR >> bcf.CR >> bcf.S
				>> bcf.FR >> bcf.FG >> bcf.FB
				>> bcf.OR >> bcf.OG >> bcf.OB
				>> bcf.OT >> bcf.V >> bcf.L;
		}
		/***************
		* Special Weapon - START
		****************/
		else if (token == "SpecialWeapon") {
			auto& spcf = m_specialConfig;
			// Order of variables for Special in config file
			config >> spcf.SR >> spcf.CR >> spcf.S
				>> spcf.FR >> spcf.FG >> spcf.FB
				>> spcf.OR >> spcf.OG >> spcf.OB
				>> spcf.OT >> spcf.V >> spcf.L;
		}
		/***************
		* Special Weapon - END
		****************/

		else if (token[0] == '#') {
			std::string comment;
			std::getline(config, comment);
			std::cout << comment << "\n";
		}
		config >> token;
	}
	config.close();
}

void Game::updateStatistics(sf::Time dt) {
	m_statisticsUpdateTime += dt;
	m_statisticsNumFrames += 1;
	if (m_statisticsUpdateTime >= sf::seconds(1.0f)) {
		m_statisticsText.setString("FPS: " + std::to_string(m_statisticsNumFrames));
		m_statisticsUpdateTime -= sf::seconds(1.0f);
		m_statisticsNumFrames = 0;
	}
}

void Game::sCollision() {

	EntityVec bulletsToRemove; // To store bullets that should be removed
	EntityVec smallEnemiesToRemove; // To store small enemies that should be removed

	for (auto bullet : m_entityManager.getEntities("bullet")) {
		auto& bulletTransform = bullet->getComponent<CTransform>();
		auto& bulletCollision = bullet->getComponent<CCollision>();

		// Check for collisions with walls
		if (bulletTransform.pos.x - bulletCollision.radius < 0 ||
			bulletTransform.pos.x + bulletCollision.radius > m_windowSize.x) {
			// Horizontal wall collision, reverse the x-velocity of the bullet
			bulletTransform.vel.x = -bulletTransform.vel.x;
		}

		if (bulletTransform.pos.y - bulletCollision.radius < 0 ||
			bulletTransform.pos.y + bulletCollision.radius > m_windowSize.y) {
			// Vertical wall collision, reverse the y-velocity of the bullet
			bulletTransform.vel.y = -bulletTransform.vel.y;
		}

		for (auto enemy : m_entityManager.getEntities("largeEnemy")) {
			auto& enemyTransform = enemy->getComponent<CTransform>();
			auto& enemyCollision = enemy->getComponent<CCollision>();

			// Check for collisions between bullets and large enemies
			float distance = length(bulletTransform.pos - enemyTransform.pos);
			if (distance <= bulletCollision.radius + enemyCollision.radius) {
				// Collision occurred, remove the bullet
				bulletsToRemove.push_back(bullet);
				// Add to the score based on the number of vertices
				int scoreToAdd = enemy->getComponent<CScore>().score;
				m_score += scoreToAdd;
				// Spawn small enemies
				spawnSmallEnemies(enemy);
				// Destroy the large enemy
				enemy->destroy();
			}
		}

		for (auto smallEnemy : m_entityManager.getEntities("smallEnemy")) {
			auto& smallEnemyTransform = smallEnemy->getComponent<CTransform>();
			auto& smallEnemyCollision = smallEnemy->getComponent<CCollision>();

			// Check for collisions between bullets and small enemies
			float distance = length(bulletTransform.pos - smallEnemyTransform.pos);
			if (distance <= bulletCollision.radius + smallEnemyCollision.radius) {
				bulletsToRemove.push_back(bullet);
				int scoreToAdd = smallEnemy->getComponent<CScore>().score;
				m_score += scoreToAdd;
				smallEnemiesToRemove.push_back(smallEnemy);
			}
		}
	}

	// Remove bullets that collided or went out of bounds
	for (auto bullet : bulletsToRemove) {
		bullet->destroy();
	}

	/***************
	* Special Weapon - START
	****************/

	// Collision after Special Weapon is activated
	for (auto specialWeapon : m_entityManager.getEntities("specialWeapon")) {

		auto& specialWeaponTransform = specialWeapon->getComponent<CTransform>(); // Special Weapon Transform
		auto& specialWeaponCollision = specialWeapon->getComponent<CCollision>(); // Special Weapon Collision

		for (auto enemy : m_entityManager.getEntities("largeEnemy")) {
			auto& enemyTransform = enemy->getComponent<CTransform>();
			auto& enemyCollision = enemy->getComponent<CCollision>();

			// Check for collisions between the Special Weapon and large enemies
			float distance = length(specialWeaponTransform.pos - enemyTransform.pos);
			if (distance <= specialWeaponCollision.radius + enemyCollision.radius) {
				// ATTENTION: special weapon is not destroyed when colliding with large enemy

				// Destroy the large enemy
				enemy->destroy();

				// Add to the score based on the number of vertices
				int scoreToAdd = enemy->getComponent<CScore>().score;
				m_score += scoreToAdd;

			}
		}

		// Check for collisions between the Special Weapon and small enemies
		for (auto smallEnemy : m_entityManager.getEntities("smallEnemy")) {

			auto& smallEnemyTransform = smallEnemy->getComponent<CTransform>(); // Get the transform of the small enemy
			auto& smallEnemyCollision = smallEnemy->getComponent<CCollision>(); // Get the collision of the small enemy

			float distance = length(specialWeaponTransform.pos - smallEnemyTransform.pos);
			if (distance <= specialWeaponCollision.radius + smallEnemyCollision.radius) {
				// ATTENTION: special weapon is not destroyed when colliding with small enemy

				// Destroy the small enemy
				smallEnemy->destroy();

				// Add score for destroying the small enemy
				int scoreToAdd = smallEnemy->getComponent<CScore>().score;
				m_score += scoreToAdd;
			}
		}
	}

	/***************
	* Special Weapon - END
	****************/

	// Remove large enemies that collided with the player
	for (auto largeEnemy : m_entityManager.getEntities("largeEnemy")) {

		if (largeEnemy->hasComponent<CCollision>()) {

			auto& largeEnemyTransform = largeEnemy->getComponent<CTransform>(); // Get the transform of the large enemy
			auto& largeEnemyCollision = largeEnemy->getComponent<CCollision>(); // Get the collision of the large enemy
			float distance = length(m_player->getComponent<CTransform>().pos - largeEnemyTransform.pos);

			if (distance <= m_player->getComponent<CCollision>().radius + largeEnemyCollision.radius) {
				// Loose 500 points for colliding with a large enemy
				m_score -= 500;

				// Destroy the large enemy
				largeEnemy->destroy();
				// Destroy the player and respawn it, call the respawn function
				m_player->destroy();
			}
		}
	}

	// Remove small enemies that collided with bullets
	for (auto smallEnemy : smallEnemiesToRemove) {
		smallEnemy->destroy();
	}
}

void Game::keepObjecsInBounds() {

	auto vb = getViewBounds();

	// (by AURELIO RODRIGUES) - Keep the entities in bounds
	for (auto& entity : m_entityManager.getEntities()) {

		// Check if the entity is not the special weapon
		// Special weapon is not affected by the bounds
		if (entity->hasComponent<CTransform>() && entity->hasComponent<CCollision>()) {

			auto& tfm = entity->getComponent<CTransform>(); // Get the transform of the entity
			auto& collision = entity->getComponent<CCollision>(); // Get the collision of the entity

			// Ensure the entity remains in bounds
			if (tfm.pos.x - collision.radius <= vb.left || tfm.pos.x + collision.radius >= vb.left + vb.width) {
				tfm.vel.x *= -1; // Reverse the x-velocity for bouncing
			}

			if (tfm.pos.y - collision.radius <= vb.top || tfm.pos.y + collision.radius >= vb.top + vb.height) {
				tfm.vel.y *= -1; // Reverse the y-velocity for bouncing
			}

			// Correct the entity's position if it goes out of bounds
			tfm.pos.x = std::max(tfm.pos.x, vb.left + collision.radius);
			tfm.pos.x = std::min(tfm.pos.x, vb.left + vb.width - collision.radius);
			tfm.pos.y = std::max(tfm.pos.y, vb.top + collision.radius);
			tfm.pos.y = std::min(tfm.pos.y, vb.top + vb.height - collision.radius);
		}
	}
}

void Game::adjustPlayerPosition() {
	auto vb = getViewBounds();

	// (by AURELIO RODRIGUES) - Keep the player in bounds
	auto& player_pos = m_player->getComponent<CTransform>().pos; // Get the position of the player
	auto player_cr = m_player->getComponent<CCollision>().radius; // Get the collision radius of the player

	// Keep player in bounds
	player_pos.x = std::max(player_pos.x, vb.left + player_cr);
	player_pos.x = std::min(player_pos.x, vb.left + vb.width - player_cr);
	player_pos.y = std::max(player_pos.y, vb.top + player_cr);
	player_pos.y = std::min(player_pos.y, vb.top + vb.height - player_cr);
}

void Game::spawnPlayer() {

	// We will always spawn the player in the middle
	auto spawnPoint = sf::Vector2f(m_windowSize.x / 2.f, m_windowSize.y / 2.f);

	// Following the same pattern as Blackout demo
	// Steps: 1. Create new player entity
	// 		  2. Add components to the player entity

	// Create new player entity, using the addEntity function from the EntityManager class
	m_player = m_entityManager.addEntity("player");

	// Component for position and movement
	m_player->addComponent<CTransform>(spawnPoint, m_playerConfig.S * sf::Vector2f{ 1.f, 1.f });

	// Component for rendering
	m_player->addComponent<CShape>(
		m_playerConfig.SR,														// Shape radius
		m_playerConfig.V,														// Number of vertices
		sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), 	// Fill color
		sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), 	// Outline color
		m_playerConfig.OT);														// Outline thickness

	// Component for collision detection
	m_player->addComponent<CCollision>(m_playerConfig.SR);

	// Component for Input
	m_player->addComponent<CInput>();
}

void Game::sLifespan(sf::Time dt) {

	// For all entities that have a CLifespan compnent
	// reduce the remaining life by dt time.
	// if the lifespan has run out destroy the entity

	auto entits = m_entityManager.getEntities();

	for (auto& e : entits) {

		if (e->hasComponent<CLifespan>()) {
			auto& cl = e->getComponent<CLifespan>();
			cl.remaining -= dt;

			if (cl.remaining <= sf::Time::Zero) {
				e->destroy();
			}
		}
	}
}

void Game::sEnemySpawner(sf::Time dt) {
	//
	// exponential distribution models random arrival times with an average
	// arrival interval of SI.
	// when the timer expires spawn a new enemy and re-set the timer for the
	// next enemy arrival time.
	std::exponential_distribution<float> exp(1.f / m_enemyConfig.SI);

	static sf::Time countDownTimer{ sf::Time::Zero };
	countDownTimer -= dt;
	if (countDownTimer < sf::Time::Zero) {
		countDownTimer = sf::seconds(exp(rng));
		spawnEnemy();
	}
}

void Game::spawnEnemy() {
	auto bounds = getViewBounds();
	std::uniform_real_distribution<float>   d_width(m_enemyConfig.CR, bounds.width - m_enemyConfig.CR);
	std::uniform_real_distribution<float>   d_height(m_enemyConfig.CR, bounds.height - m_enemyConfig.CR);
	std::uniform_int_distribution<>         d_points(m_enemyConfig.VMIN, m_enemyConfig.VMAX);
	std::uniform_real_distribution<float>   d_speed(m_enemyConfig.SMIN, m_enemyConfig.SMAX);
	std::uniform_int_distribution<>         d_color(0, 255);
	std::uniform_real_distribution<float>   d_dir(-1, 1);

	sf::Vector2f  pos(d_width(rng), d_height(rng));
	sf::Vector2f  vel = sf::Vector2f(d_dir(rng), d_dir(rng));
	vel = normalize(vel);
	vel = d_speed(rng) * vel;

	// Spawn a new enemy with random settings according to m_enemyConfig
	// the CScore component will be the number of points the player gets for destroying this
	// enenmy. It should be set to the number of Verticies the enemy has
	// tag is largeEnemy

	// (by AURELIO RODRIGUES) - Spawn a new enemy

	// Following the same pattern as Blackout demo and spawnPlayer function
	// Steps: 1. Create new enemy entity
	//		  2. Add components to the enemy entity

	// TODO: Spawn a new enemy with random settings according to m_enemyConfig
	auto enemy = m_entityManager.addEntity("largeEnemy");

	// Component for position and movement
	enemy->addComponent<CTransform>(pos, vel);

	// Before component for rendering, I need to initialize a variable for random number of vertices
	int numVertices = d_points(rng);

	// Component for rendering
	enemy->addComponent<CShape>(
		m_enemyConfig.SR,                                                     // Shape radius
		d_points(rng),                                                        // Number of vertices (random number)
		sf::Color(d_color(rng), d_color(rng), d_color(rng)),                  // Fill color (random color)
		sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB),      // Fill color (random color)
		m_enemyConfig.OT);                                                    // Outline thickness

	// Component for collision detection
	enemy->addComponent<CCollision>(m_enemyConfig.CR);

	// Component for score (points for destroying the enemy)
	enemy->addComponent<CScore>(numVertices);
}

void Game::spawnSmallEnemies(sPtrEntt e) {

	// Definitions:
	// 
	// Enemy entity e just collided with a bullet.
	// spawn small enemies according to game rules.
	//    * the radius of the small enemies is half the radius of the enemy that was hit
	//    * There are v small enemies where v is the number of vertices that the large enemy had
	//    * each small enemy has the same number of vertices and is the same colour as the large
	//          enemy, e, that was blown up.
	//    * the small enemies should all be moving away from the e.pos spread out equaly
	//           ie, if there e had 5 vertices there should be 5 small enemies each with 5 vertices
	//               with angle 360/5 degrees between each of them.
	//    *  Each small enemy has a CLifetime component with lifetime as specified in enemyConfig
	//    *  Each small enemy should have it's CScore component set to 10 times the number of
	//           vertices it has.
	//   tag is smallEnemy

	// (by AURELIO RODRIGUES) - Spawn small enemies
	auto& circle = e->getComponent<CShape>().circle; // Get the circle component of the entity

	// Calculate the angle between each small enemy after the collision
	float angle = 360.0f / circle.getPointCount();

	// I need to do a for loop to spawn all the small enemies after the collision
	for (int i = 0; i < circle.getPointCount(); i++) {

		auto smallEnemy = m_entityManager.addEntity("smallEnemy");

		// Get the position and velocity of the large enemy that was hit
		auto& tfm = e->getComponent<CTransform>();
		sf::Vector2f dir = uVecBearing(i * angle);

		// For small enemies, I need to add the radius of the large enemy that was hit
		// to the position of the large enemy that was hit
		// The first property of the CTransform component is the position
		// The second property of the CTransform component is the velocity
		smallEnemy->addComponent<CTransform>
			(
				tfm.pos + dir * (circle.getRadius() + circle.getRadius() / 2),
				m_enemyConfig.SMAX * dir
			);

		// Add components to the small enemy entity
		// I need to follow the order of the components in the constructor
		smallEnemy->addComponent<CShape>(
			circle.getRadius() / 2, // half the radius of the enemy that was hit
			circle.getPointCount(),
			circle.getFillColor(),
			circle.getOutlineColor(),
			circle.getOutlineThickness()
		);

		// Add the collision component to the small enemy entity 
		// with half the radius of the enemy that was hit
		smallEnemy->addComponent<CCollision>(e->getComponent<CCollision>().radius / 2);

		// Add the lifespan component to the small enemy entity
		smallEnemy->addComponent<CLifespan>(m_enemyConfig.L);

		// Add the score component to the small enemy entity
		smallEnemy->addComponent<CScore>(e->getComponent<CScore>().score * 10);
	}
}

void Game::spawnBullet(sf::Vector2f mPos) {
	// Create a Bullet object
	// the bullet is spawned at the players location
	// the bullets velocity is in the direction of the mouse click location
	// the bullets config is according to m_bulletConfig
	// tag is "bullet"

	// (by AURELIO RODRIGUES) - Spawn a new bullet

	// Following the same pattern as Blackout demo and spawnPlayer function
	// Steps: 1. Create new bullet entity
	//		  2. Add components to the bullet entity
	auto entity = m_entityManager.addEntity("bullet");

	// Add the necessary components to the bullet entity (CTransform, CShape, CCollision, CLifespan)

	// Player position
	auto playerPosition = m_player->getComponent<CTransform>().pos;

	// Mouse position = mPos
	mPos -= playerPosition;

	// Component position to mouse position
	entity->addComponent<CTransform>(playerPosition, m_bulletConfig.S * normalize(mPos));

	// Component for rendering
	entity->addComponent<CShape>(
		m_bulletConfig.SR,														// Shape radius
		m_bulletConfig.V, 														// Number of vertices
		sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB), 	// Fill color
		sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), 	// Outline color
		m_bulletConfig.OT); 													// Outline thickness

	entity->addComponent<CCollision>(m_bulletConfig.CR); // CR = Collision radius

	entity->addComponent<CLifespan>(m_bulletConfig.L); // L = Lifespan time
}

void Game::spawnSpecialWeapon(sf::Vector2f mPos2) {

	// Special weapon will be spawned when the player clicks the right mouse button
	// It will be available only 3x per game
	// Special weapon will be a gold circle with outline colour red
	// It will move slower than bullets, but it will much larger
	// Special weapon will have a lifespan

	/********** Special Weapon Config **********

		# Special Weapon config
		#      			SR   CR   S     F(r,g,b),   O(r,g,b),   OT,  V   L
		SpecialWeapon	250 250  800    255 215 0   255 0 0   	8    40  1

	********************************************/

	// TODO of create a SpecialWeapon object
	// the special weapon is spawned at the players location
	// the special weapons velocity is in the direction of the mouse click location
	// the special weapons config is according to m_specialWeaponConfig
	if (m_specialWeaponCount < 3) {
		auto specialWeapon = m_entityManager.addEntity("specialWeapon");

		// Add the necessary components to the Special Weapon (CTransform, CShape, CCollision, CLifespan)

		// Player position
		auto playerPosition = m_player->getComponent<CTransform>().pos;

		// Mouse position = mPos2
		mPos2 -= playerPosition;

		// Component position to mouse position
		specialWeapon->addComponent<CTransform>(playerPosition, m_specialConfig.S * normalize(mPos2));

		// Component for rendering
		specialWeapon->addComponent<CShape>(
			m_specialConfig.SR,														// Shape radius
			m_specialConfig.V, 														// Number of vertices
			sf::Color(m_specialConfig.FR, m_specialConfig.FG, m_specialConfig.FB), 	// Fill color
			sf::Color(m_specialConfig.OR, m_specialConfig.OG, m_specialConfig.OB), 	// Outline color
			m_specialConfig.OT); 													// Outline thickness

		// Collision radius
		specialWeapon->addComponent<CCollision>(m_specialConfig.CR); // CR = Collision radius

		// Lifespan
		specialWeapon->addComponent<CLifespan>(m_specialConfig.L); // L = Lifespan time

		// Increment special weapon count
		m_specialWeaponCount++;
	}
}

// convenience function to return the view bounds as a FloatRect
sf::FloatRect Game::getViewBounds() {
	auto view = m_window.getView();
	return sf::FloatRect(
		(view.getCenter().x - view.getSize().x / 2.f), (view.getCenter().y - view.getSize().y / 2.f),
		view.getSize().x, view.getSize().y);
}
