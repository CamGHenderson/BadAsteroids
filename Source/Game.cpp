// to make the g++ version work (don't worry about it)
#include<math.h>

// 2D graphics library
#include<SFML/Graphics.hpp>

// collision library because I'm to lazy to figure out how to do polygon collisions
#include"clipper.hpp"

// define window stuff
const std::string title = "Bad Asteroids";
const unsigned int width = 1280, height = 720;

// define pi
const float pi = 3.14159265358979323846f;

enum class GameState
{
    Title,
    Game
};

// struct for wireframe models
struct GameObject
{
    std::vector<sf::Vector2f> model;
    sf::Color color;
    sf::Vector2f position;
    float angle = 0.0f;
};

// stupid struct for asteroids
struct Asteroid : public GameObject
{
    float size = 0.0f;

    // I don't know why I didn't use a vector
    float directionX = 0.0f;
    float directionY = 0.0f;

    // speed that asteroid rotates at
    float rotationSpeed = 0.0f;

    // stupid bounds thing
    float longestDistance = 0.0f;
};

// gets where point should be when it is rotated
sf::Vector2f getRotatedModelPoint(GameObject& gameObject, unsigned int i)
{
    sf::Vector2f point;
    float angle = (gameObject.angle * pi / 180.0f);
    point.x = gameObject.model[i].x * cos(angle) - gameObject.model[i].y * sin(angle);
    point.y = gameObject.model[i].x * sin(angle) + gameObject.model[i].y * cos(angle);
    return point;
}

// method for if two game objects intersect
bool objectsIntersect(GameObject& gameObject0, float x, float y, GameObject& gameObject1)
{
    // I don't know how any of this magic works
    ClipperLib::Paths subject(1);
    for (unsigned int i = 0; i < gameObject0.model.size(); i++)
    {
        sf::Vector2f point = getRotatedModelPoint(gameObject0, i);
        subject[0] << ClipperLib::IntPoint((int)round(point.x + x), (int)round(point.y + y));
    }

    ClipperLib::Paths clip(1);
    for (unsigned int i = 0; i < gameObject1.model.size(); i++)
    {
        sf::Vector2f point = getRotatedModelPoint(gameObject1, i);
        clip[0] << ClipperLib::IntPoint((int)round(point.x + gameObject1.position.x), (int)round(point.y + gameObject1.position.y));
    }

    ClipperLib::Clipper clipper;
    clipper.AddPaths(subject, ClipperLib::ptSubject, true);
    clipper.AddPaths(clip, ClipperLib::ptClip, true);

    ClipperLib::Paths solution;
    clipper.Execute(ClipperLib::ctIntersection, solution, ClipperLib::pftNonZero, ClipperLib::pftNonZero);

    return solution.size() != 0;
}

// don't ask
bool objectsIntersect(GameObject& gameObject0, GameObject& gameObject1)
{   
    return objectsIntersect(gameObject0, gameObject0.position.x, gameObject0.position.y, gameObject1);
}

// generates random asteroid
Asteroid* generateAsteroid()
{
    // create new game object
    Asteroid* asteroid = new Asteroid();
    asteroid->size = 30.0f;

    // bounds garbage
    float leftX = 0.0f;
    float topY = 0.0f;
    float rightX = 0.0f;
    float bottomY = 0.0f;

    // epic asteroid point random thing mess
    unsigned int pointAmount = rand() % 10 + 5;
    for (unsigned int i = 0; i < pointAmount; i++)
    {
        // stupid random point thing
        float distance = (float)(rand() % (int)asteroid->size) + asteroid->size;
        float x = distance * cos((360 / pointAmount * i) * (pi / 180));
        float y = distance * sin((360 / pointAmount * i) * (pi / 180));
        asteroid->model.push_back(sf::Vector2f(x, y));

        // set stupid variables
        if (x < leftX)
            leftX = x;
        else if (x > rightX)
            rightX = x;
        
        if (y < topY)
            topY = y;
        else if (y > bottomY)
            bottomY = y;
        
        if (abs(distance) > asteroid->longestDistance)
            asteroid->longestDistance = distance;
    }

    // define direction to move in (don't ask about the minus 45)
    float moveDirection = (float)(rand() % 360);
    float speed = (float)(rand() % 100 + 150);
    asteroid->directionX = cos(moveDirection - 45.0f * (pi / 180)) * speed;
    asteroid->directionY = sin(moveDirection - 45.0f * (pi / 180)) * speed;
    asteroid->rotationSpeed = (float)(rand() % 360);

    // other asteroid setup stuff
    asteroid->color = sf::Color::White;
    asteroid->angle = 0.0f;
    
    // make asteroid spawn on right side of window based on movement direction (this is kind of stupid)
    if (moveDirection >= 0.0f && moveDirection < 90.0f)
        asteroid->position = sf::Vector2f(-rightX, (float)(rand() % height));
    else if (moveDirection >= 90.0f && moveDirection < 180.0f)
        asteroid->position = sf::Vector2f((float)(rand() % width), -bottomY);
    else if (moveDirection >= 180.0f && moveDirection < 270.0f)
        asteroid->position = sf::Vector2f(width - leftX, (float)(rand() % height));
    else if (moveDirection >= 270.0f && moveDirection <= 360.0f)
        asteroid->position = sf::Vector2f((float)(rand() % width), height - topY);

    return asteroid;
}

// generates random asteroid with specified position and size
Asteroid* generateAsteroid(float size, sf::Vector2f position, float angle)
{
    // create new game object
    Asteroid* asteroid = new Asteroid();
    asteroid->size = size;

    // epic asteroid point random thing mess
    unsigned int pointAmount = rand() % 10 + 5;
    for (unsigned int i = 0; i < pointAmount; i++)
    {
        // stupid random point thing
        float distance = (float)(rand() % (int)asteroid->size) + asteroid->size;
        float x = distance * cos((360 / pointAmount * i) * (pi / 180));
        float y = distance * sin((360 / pointAmount * i) * (pi / 180));
        asteroid->model.push_back(sf::Vector2f(x, y));

        if (abs(distance) > asteroid->longestDistance)
            asteroid->longestDistance = distance;
    }

    // define direction to move in (don't ask about the minus 45)
    float moveDirection = angle;
    float speed = (float)(rand() % 100 + 150);
    asteroid->directionX = cos(moveDirection - 45.0f * (pi / 180)) * speed;
    asteroid->directionY = sin(moveDirection - 45.0f * (pi / 180)) * speed;
    asteroid->rotationSpeed = (float)(rand() % 360);

    // other asteroid setup stuff
    asteroid->color = sf::Color::White;
    asteroid->angle = 0.0f;
    asteroid->position = position;

    return asteroid;
}

// removes element from game object vector
void removeObject(std::vector<GameObject*>& objects, unsigned int i)
{
    GameObject* object = objects[i];
    objects.erase(objects.begin() + i);
    delete object;
}

// same thing for asteroids
void removeAsteroid(std::vector<Asteroid*>& asteroids, unsigned int i)
{
    GameObject* asteroid = asteroids[i];
    asteroids.erase(asteroids.begin() + i);
    delete asteroid;
}

// result of asteroid being shot or colliding with another asteroid
void splitAsteroid(std::vector<Asteroid*>& asteroids, unsigned int i)
{
    sf::Vector2f position = asteroids[i]->position;
    float size = asteroids[i]->size;
    removeAsteroid(asteroids, i);
    if (size > 5)
    {
        unsigned int amountOfAsteroids = (rand() % 4) + 2;
        for (unsigned int i = 0; i < amountOfAsteroids; i++)
        {
            // why am I the way I am
            asteroids.push_back(generateAsteroid((float)(size / amountOfAsteroids), sf::Vector2f(position.x + (cos(360.0f / amountOfAsteroids * i * pi / 180) * size),
                                position.y + (sin(360.0f / amountOfAsteroids * i * pi / 180) * size)), (float)(360.0f / amountOfAsteroids * i)));
        }
    }
}

// method for rendering game objects with independent x and y (bad way of doing this)
void render(GameObject& gameObject, float x, float y, sf::RenderWindow& window)
{
    // this not a very good way of doing this but it also runs at a decent speed on my garbage laptop too so I don't care
    sf::ConvexShape polygon = sf::ConvexShape();
    polygon.setPointCount(gameObject.model.size());
    for (unsigned int i = 0; i < gameObject.model.size(); i++)
        polygon.setPoint(i, gameObject.model[i]);
    polygon.setOutlineThickness(1.0f);
    polygon.setOutlineColor(gameObject.color);
    polygon.setFillColor(sf::Color::Transparent);
    polygon.setPosition(sf::Vector2f(x, y));
    polygon.setRotation(gameObject.angle);
    window.draw(polygon);
}

// method for rendering game objects to the window
void render(GameObject& gameObject, sf::RenderWindow& window)
{
	// this is quite stupid
    render(gameObject, gameObject.position.x, gameObject.position.y, window);
}

int main()
{
    // create window
	sf::RenderWindow window;
        
    // create context with antialiasing
    sf::ContextSettings contextSettings;
    contextSettings.antialiasingLevel = 8;

    // set window size, title, disable resizing, and enable antialiasing 
	window.create(sf::VideoMode(width, height), title, sf::Style::Titlebar | sf::Style::Close, contextSettings);
    
    // cap framerate
    window.setFramerateLimit(500);

    // make keys work properly
    window.setKeyRepeatEnabled(true);

    // construct player
    GameObject player;

    // define player shape
    player.model.push_back(sf::Vector2f(0.0f, -12.0f));
    player.model.push_back(sf::Vector2f(6.0f, 12.0f));
    player.model.push_back(sf::Vector2f(0.0f, 7.0f));
    player.model.push_back(sf::Vector2f(-6.0f, 12.0f));

    // other player setup stuff
    player.color = sf::Color::White;
    player.position;
    player.angle;

    bool playerAlive;

    // variables for player transformation
    const float maxPlayerSpeed = 500.0f;
    const float playerAccelerationSpeed = 200.0f;
    const float playerRotationSpeed = 180.0f;

    // for player acceleration and drift
    float playerDirectionX;
    float playerDirectionY;

    bool spaceHold;

    // vector for bullets in game (who could of guessed)
    std::vector<GameObject*> bullets;

    // stupid speed of bullets
    const float bulletSpeed = 2.0f;

    // vector of asteroids that are in the game
    std::vector<Asteroid*> asteroids;

    // initialize rand method for asteroids
    srand((unsigned int)time(NULL));

    // clock for spawning asteroids on a interval
    sf::Clock asteroidSpawnClock;

    // asteroid spawn stuff
    const float fastestAsteroidSpawnTime = 0.1f;
    const float asteroidSpawnTimeChange = 0.1f;
    const unsigned int asteroidSpawnAmount = 10;
    float asteroidSpawnTime;
    unsigned int asteroidSpawnCount;

    // stupid font
    sf::Font font;
    font.loadFromFile("Resources/Fonts/stupid_font.ttf");

    // score stuff
    unsigned int score;
    sf::Text scoreLabel;
    scoreLabel.setFont(font);
    scoreLabel.setCharacterSize(14);
    scoreLabel.setFillColor(sf::Color::White);
    scoreLabel.setPosition(25.0f, 25.0f);

    // title screen garbage
    sf::Text titleLabel;
    titleLabel.setFont(font);
    titleLabel.setCharacterSize(50);
    titleLabel.setFillColor(sf::Color::White);
    titleLabel.setString("Bad Asteroids");
    titleLabel.setPosition((float)(width / 2.0f) - (float)(titleLabel.getLocalBounds().width / 2.0f), 100.0f);

    sf::Text buttonLabel;
    buttonLabel.setFont(font);
    buttonLabel.setCharacterSize(20);
    buttonLabel.setFillColor(sf::Color::White);
    buttonLabel.setString("Play");
    buttonLabel.setPosition((float)(width / 2.0f) - (float)(buttonLabel.getLocalBounds().width / 2.0f), 480.0f);

    // text is always stupid to work with
    sf::RectangleShape buttonRect;
    buttonRect.setPosition(buttonLabel.getPosition().x - 10, buttonLabel.getPosition().y - 2);
    buttonRect.setSize(sf::Vector2f(buttonLabel.getLocalBounds().width + 20, buttonLabel.getLocalBounds().height + 10));
    buttonRect.setFillColor(sf::Color::Transparent);
    buttonRect.setOutlineThickness(1.0f);
    buttonRect.setOutlineColor(sf::Color::White);

    // color button should be if mouse is hovering over button
    sf::Color hoverColor = sf::Color(120, 120, 120);

    // game state garbage
    GameState gameState = GameState::Title;

    // gives some time between player death and going to title screen
    sf::Clock deathClock;

    // clock for getting delta time
    sf::Clock deltaClock;

    // game loop
    while (window.isOpen())
    {
        // handle window events
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                // close the stupid window when x button is pressed
                window.close();
                break;
            }
        }

        // clear window
        window.clear(sf::Color::Black);

        switch (gameState)
        {
        case GameState::Title:
        {
            // stupid button press thing
            if (sf::Mouse::getPosition(window).x >= buttonRect.getPosition().x && sf::Mouse::getPosition(window).x <= buttonRect.getPosition().x + buttonRect.getLocalBounds().width && 
                sf::Mouse::getPosition(window).y >= buttonRect.getPosition().y && sf::Mouse::getPosition(window).y <= buttonRect.getPosition().y + buttonRect.getLocalBounds().height)
            {
                buttonRect.setFillColor(hoverColor);
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    // reset all of the stupid variables
                    player.position = sf::Vector2f((float)width / 2, (float)height / 2);
                    player.angle = 0.0f;

                    playerDirectionX = 0.0f;
                    playerDirectionY = 0.0f;

                    playerAlive = true;
                    
                    spaceHold = false;

                    bullets.clear();
                    bullets.shrink_to_fit();

                    asteroids.clear();
                    asteroids.shrink_to_fit();

                    asteroidSpawnClock.restart();

                    asteroidSpawnTime = 1.0f;
                    asteroidSpawnCount = 0;

                    score = 0;
                    scoreLabel.setString("Score: " + std::to_string(score));

                    deltaClock.restart();

                    // set stupid game state
                    gameState = GameState::Game;
                }
            }
            else
                buttonRect.setFillColor(sf::Color::Transparent);

            window.draw(titleLabel);
            window.draw(buttonRect);
            window.draw(buttonLabel);
            break;
        }
        case GameState::Game:
        {
            // get delta time from deltaClock and reset deltaClock
            float deltaTime = deltaClock.restart().asSeconds();
            if (deltaTime < 0.25)
            {
                if (playerAlive)
                {
                    // handle player rotation
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                        player.angle -= playerRotationSpeed * deltaTime;

                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                        player.angle += playerRotationSpeed * deltaTime;

                    // handle player movement and player drift
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                    {
                        // add to acceleration
                        playerDirectionX += cos((player.angle - 90.0f) * pi / 180) * playerAccelerationSpeed * deltaTime;
                        playerDirectionY += sin((player.angle - 90.0f) * pi / 180) * playerAccelerationSpeed * deltaTime;

                        // cap speed
                        if (playerDirectionX >= maxPlayerSpeed)
                            playerDirectionX = maxPlayerSpeed;
                        else if (playerDirectionX <= -maxPlayerSpeed)
                            playerDirectionX = -maxPlayerSpeed;
                        if (playerDirectionY >= maxPlayerSpeed)
                            playerDirectionY = maxPlayerSpeed;
                        else if (playerDirectionY <= -maxPlayerSpeed)
                            playerDirectionY = -maxPlayerSpeed;
                    }

                    // stupid shooting thing
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
                    {
                        if (!spaceHold)
                        {
                            GameObject* bullet = new GameObject();

                            // construct bullet model
                            bullet->model.push_back(sf::Vector2f(-1.0f, -1.0f));
                            bullet->model.push_back(sf::Vector2f(-1.0f, 1.0f));
                            bullet->model.push_back(sf::Vector2f(1.0f, 1.0f));
                            bullet->model.push_back(sf::Vector2f(1.0f, -1.0f));


                            // other bullet stuff
                            bullet->position = player.position;
                            bullet->position.x += cos((player.angle - 90.0f) * pi / 180) * 12.0f;
                            bullet->position.y += sin((player.angle - 90.0f) * pi / 180) * 12.0f;
                            bullet->angle = player.angle;
                            bullet->color = sf::Color::White;

                            bullets.push_back(bullet);
                            spaceHold = true;
                        }
                    }
                    else
                        spaceHold = false;

                    // actually move the player
                    player.position.x += playerDirectionX * deltaTime;
                    player.position.y += playerDirectionY * deltaTime;

                    // handle player wrapping (for coords)
                    if (player.position.x > width)
                        player.position.x = 0.0f;
                    else if (player.position.x < 0)
                        player.position.x = (float)width;
                    if (player.position.y > height)
                        player.position.y = 0.0f;
                    else if (player.position.y < 0)
                        player.position.y = (float)height;
                }

                // update asteroids positions
                for (unsigned int i = 0; i < asteroids.size(); i++)
                {
                    asteroids[i]->position.x += asteroids[i]->directionX * deltaTime;
                    asteroids[i]->position.y += asteroids[i]->directionY * deltaTime;
                    asteroids[i]->angle += asteroids[i]->rotationSpeed * deltaTime;

                    // delete garbage asteroids (not perfect but will do)
                    if (asteroids[i]->position.x + asteroids[i]->longestDistance <= 0 || asteroids[i]->position.x - asteroids[i]->longestDistance >= width ||
                        asteroids[i]->position.y + asteroids[i]->longestDistance <= 0 || asteroids[i]->position.y - asteroids[i]->longestDistance >= height)
                        removeAsteroid(asteroids, i);
                }

                // update position of bullets
                for (unsigned int i = 0; i < bullets.size(); i++)
                {
                    bullets[i]->position.x += cos((bullets[i]->angle - 90.0f) * (pi / 180)) * bulletSpeed;
                    bullets[i]->position.y += sin((bullets[i]->angle - 90.0f) * (pi / 180)) * bulletSpeed;

                    // bullet clean up
                    if (bullets[i]->position.x <= -10.0f || bullets[i]->position.x >= width + 10 || bullets[i]->position.y <= -10.0f || bullets[i]->position.y >= height + 10.0f)
                        removeObject(bullets, i);
                }

                // collision stuff (not perfect but probably will be fine)
                for (unsigned int i = 0; i < asteroids.size(); i++)
                {
                    if (objectsIntersect(player, *asteroids[i]))
                    {
                        playerAlive = false;
                        deathClock.restart();
                    }

                    for (unsigned int j = 0; j < bullets.size(); j++)
                        if (objectsIntersect(*asteroids[i], *bullets[j]))
                        {
                            splitAsteroid(asteroids, i);
                            removeObject(bullets, j);

                            if (playerAlive)
                            {
                                // add to score
                                score += 10;
                                scoreLabel.setString("Score: " + std::to_string(score));
                            }
                        }
                }

                // resize vectors capacities
                asteroids.shrink_to_fit();
                bullets.shrink_to_fit();

                // spawn asteroids on a second interval
                if (asteroidSpawnClock.getElapsedTime().asSeconds() >= asteroidSpawnTime)
                {
                    // add stupid asteroid to game
                    asteroids.push_back(generateAsteroid());

                    // reset the clock
                    asteroidSpawnClock.restart();

                    // update spawn speed stuff
                    asteroidSpawnCount++;
                    if (asteroidSpawnCount >= asteroidSpawnAmount)
                    {
                        if (asteroidSpawnTime - asteroidSpawnTimeChange >= fastestAsteroidSpawnTime)
                            asteroidSpawnTime -= asteroidSpawnTimeChange;
                        asteroidSpawnCount = 0;
                    }

                    // add to score for survival
                    if (playerAlive)
                    {
                        score++;
                        scoreLabel.setString("Score: " + std::to_string(score));
                    }
                }

                if (playerAlive)
                {
                    // render player
                    render(player, window);

                    // handle wrap effect (for graphics)
                    bool hasWrapped = false;
                    sf::Vector2f position;

                    // I hate this terrible if statement
                    if (player.position.x + 12 >= width)
                    {
                        render(player, player.position.x - width, player.position.y, window);
                        hasWrapped = true;
                        position = sf::Vector2f(player.position.x - width, player.position.y);
                    }
                    else if (player.position.x - 12 <= 0)
                    {
                        render(player, width + player.position.x, player.position.y, window);
                        hasWrapped = true;
                        position = sf::Vector2f(width + player.position.x, player.position.y);
                    }
                    else if (player.position.y + 12 >= height)
                    {
                        render(player, player.position.x, player.position.y - height, window);
                        hasWrapped = true;
                        position = sf::Vector2f(player.position.x, player.position.y - height);
                    }
                    else if (player.position.y - 12 <= 0)
                    {
                        render(player, player.position.x, height + player.position.y, window);
                        hasWrapped = true;
                        position = sf::Vector2f(player.position.x, height + player.position.y);
                    }

                    // check collisions for wrapped model
                    if (hasWrapped)
                        for (Asteroid* asteroid : asteroids)
                            if (objectsIntersect(player, position.x, position.y, *asteroid))
                            {
                                playerAlive = false;
                                deathClock.restart();
                            }
                }
                else
                    if (deathClock.getElapsedTime().asSeconds() >= 3.0f)
                        gameState = GameState::Title;

                // render all of the bullets
                for (GameObject* bullet : bullets)
                    render(*bullet, window);

                // render all of the asteroids
                for (Asteroid* asteroid : asteroids)
                    render(*asteroid, window);

                // render score text
                window.draw(scoreLabel);
            }
            break;
        }
        }

        // update window when all of the garbage is rendered
        window.display();
    }

	return 0;
}
