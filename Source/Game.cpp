#include<SFML/Graphics.hpp>

std::string title = "Bad Asteroids";
unsigned int width = 1280, height = 720;

struct GameObject
{
    sf::ConvexShape polygon;
    sf::Color color;
    sf::Vector2f position;
    float angle;
};

void render(GameObject& gameObject, sf::RenderWindow& window)
{
    gameObject.polygon.setPosition(gameObject.position);
    window.draw(gameObject.polygon);
}

int main()
{   
    // create window
	sf::RenderWindow window;
    
    // set window size, title, and make window not resizable
	window.create(sf::VideoMode(width, height), title, sf::Style::Titlebar | sf::Style::Close);
    
    // cap framerate
    window.setFramerateLimit(500);

    // shape garbage
    sf::ConvexShape playerShape;
    playerShape.setPointCount(4);
    playerShape.setPoint(0, sf::Vector2f(0.0f, -12.0f));
    playerShape.setPoint(1, sf::Vector2f(6.0f, 12.0f));
    playerShape.setPoint(2, sf::Vector2f(0.0f, 7.0f));
    playerShape.setPoint(3, sf::Vector2f(-6.0f, 12.0f));
    playerShape.setOutlineThickness(1.2f);
    playerShape.setFillColor(sf::Color::Transparent);
    playerShape.setOutlineColor(sf::Color::White);
    playerShape.setPosition(width / 4, height / 4);

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

        // render and update game
        window.clear(sf::Color::Black);

        playerShape.move(deltaTime * 10.0f, 0.0f);
        window.draw(playerShape);

        window.display();
    }

	return 0;
}