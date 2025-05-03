#ifdef SIM
#include "sim_display.hpp"

namespace sim {

 void Display::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color) {
    sf::Vertex line[] = {sf::Vertex(sf::Vector2f(x0, y0)),
                        sf::Vertex(sf::Vector2f(x1, y1))};

    window_.draw(line, 2, sf::Lines);
}

void Display::drawFastHLine(int32_t x, int32_t y, int32_t w, uint32_t color) {
    return drawLine(x, y, x + w, y, color);
    }

void Display::drawFastVLine(int32_t x, int32_t y, int32_t h, uint32_t color) {
    return drawLine(x, y, x, y + h, color);
    }

void Display::fillCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color) {
    sf::CircleShape circle(r, 100);

    circle.setFillColor(ConvertColor(color));
    circle.setPosition(x0, y0);
    
    window_.draw(circle);
}

void Display::drawCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color) {
    return fillCircle(x0, y0, r, color);
    }

void Display::fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color) {
    sf::RectangleShape rectangle(sf::Vector2f(w, h));
    rectangle.setPosition(x, y);
    rectangle.setFillColor(ConvertColor(color));
    
    window_.draw(rectangle);
}

void Display::drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color) { 
    return fillRect(x, y, w, h, color);
    }

void Display::fillScreen(uint32_t color) {
    return fillRect(0, 0, sdl::kWidth, sdl::kHeight, color);
}

void Display::setRotation(uint8_t m) {
}

void Display::pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data) {
    for (int x_coord = 0; x_coord < w; ++x_coord) {
        for (int y_coord = 0; y_coord < h; ++y_coord) {
            auto pixel = *(data + x_coord + y_coord * w);
            drawPixel(x_coord + x, y_coord + y, ToBigEndian(pixel));
        }
}
}

void Display::setCursor(int16_t x, int16_t y) {
    text_.setPosition(x, y);
}

void Display::setTextColor(uint16_t c) {
    text_.setFillColor(ConvertColor(c));
}

void Display::setTextColor(uint16_t c, uint16_t b, bool bgfill) {
    setTextColor(c);

    if (bgfill) {
        sf::FloatRect backgroundRect = text_.getLocalBounds();
        sf::RectangleShape background(
            sf::Vector2f(backgroundRect.width, backgroundRect.height * 4));
        background.setFillColor(ConvertColor(b));
        
        window_.draw(background, text_.getTransform());
    }
}

size_t Display::print(const char* string) {
    text_.setString(string);
    window_.draw(text_);
    return std::strlen(string);
}

size_t Display::println(const char* string) {
    return print(string);
    }

bool Display::isOpen() {
    return window_.isOpen();
    }

bool Display::pollEvent(sf::Event& e) {return window_.pollEvent(e);
}

void Display::close() {
    return window_.close();
    }

void Display::flush() {
    return window_.display();
}

void Display::drawPixel(int x, int y, uint32_t c) {
    fillRect(x, y, 1, 1, c);
}

Display::Display(sf::VideoMode vm, std::string name) : window_{vm, name} {
    if (!font_.loadFromFile("/home/daddy/Documents/mipt_fabric/assets/SpaceMono-Regular.ttf"))
        std::abort();

    text_.setFont(font_);
    text_.setCharacterSize(sdl::kFontHeight * 4);
    text_.setScale(0.25, 0.25); 
}

} // sim

#endif // SIM