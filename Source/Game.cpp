#include<iostream>
#include<SFML/Graphics.hpp>

// define window stuff
const std::string title = "Bad Asteroids";
const unsigned int width = 1280, height = 720;

// define pi
const float pi = 3.14159265358979323846f;

// struct for wireframe models
struct GameObject
{
    std::vector<sf::Vector2f> model;
    sf::Color color;
    sf::Vector2f position;
    float angle = 0.0f;
    bool wrap = false;
};

// struct for lines
struct Line
{
    sf::Vector2f point0;
    sf::Vector2f point1;
};

// method for if objects intersect
bool objectsIntersect(GameObject& gameObject0, GameObject& gameObject1)
{
    std::vector<Line> lines;
}

// method for rendering game objects to the window
void render(GameObject& gameObject, sf::RenderWindow& window)
{
    sf::ConvexShape polygon = sf::ConvexShape();
    polygon.setPointCount(gameObject.model.size());
    for (unsigned int i = 0; i < gameObject.model.size(); i++)
        polygon.setPoint(i, gameObject.model[i]);

    if (gameObject.wrap)
    {
        if (gameObject.position.x > width)
        {
            gameObject.position.x = 0.0f;
        }
        else if (gameObject.position.x < 0)
        {
            gameObject.position.x = (float)width;
        }

        if (gameObject.position.y > height)
        {
            gameObject.position.y = 0.0f;
        }
        else if (gameObject.position.y < 0)
        {
            gameObject.position.y = (float)height;
        }
    }
    polygon.setOutlineThickness(1.0f);
    polygon.setOutlineColor(gameObject.color);
    polygon.setFillColor(sf::Color::Transparent);
    polygon.setPosition(gameObject.position);
    polygon.setRotation(gameObject.angle);
    window.draw(polygon);
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
    player.position = sf::Vector2f((float)width / 2, (float)height / 2);
    player.angle = 0.0f;
    player.wrap = true;

    // variables for player transformation
    const float playerAccelerationSpeed = 150.0f;
    const float playerRotationSpeed = 180.0f;

    // for player acceleration and drift
    float playerDirectionX = 0.0f;
    float playerDirectionY = 0.0f;

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
                window.close();
                break;
            }
        }

        // get delta time from deltaClock
        float deltaTime = deltaClock.restart().asSeconds();

        // handle player rotation
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            player.angle -= playerRotationSpeed * deltaTime;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            player.angle += playerRotationSpeed * deltaTime;
        }

        // handle player move and player drift
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            // add to acceleration
            playerDirectionX += cosf((player.angle - 90.0f) * pi / 180) * playerAccelerationSpeed * deltaTime;
            playerDirectionY += sinf((player.angle - 90.0f) * pi / 180) * playerAccelerationSpeed * deltaTime;
        }

        // actually move the player
        player.position.x += playerDirectionX * deltaTime;
        player.position.y += playerDirectionY * deltaTime; 

        // render and update game
        window.clear(sf::Color::Black);

        render(player, window);

        window.display();
    }

	return 0;
}