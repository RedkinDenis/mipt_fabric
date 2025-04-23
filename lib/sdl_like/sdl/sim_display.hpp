#pragma once

#ifndef SIM
#error "Simulation only"
#endif

#include <sdl_types.hpp>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <cstring>

namespace sim {

constexpr size_t kDisplayScale = 4;

class Display {
 public:

    void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color);
    void drawFastHLine(int32_t x, int32_t y, int32_t w, uint32_t color);
    void drawFastVLine(int32_t x, int32_t y, int32_t h, uint32_t color);
    
    void fillCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color);
    void drawCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color);

    void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color); 
    void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);

    void fillScreen(uint32_t color) ;
    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data);

    void setRotation(uint8_t m);

    void setCursor(int16_t x, int16_t y);
    void setTextColor(uint16_t c);
    void setTextColor(uint16_t c, uint16_t b, bool bgfill);

    size_t print(const char* string);
    size_t println(const char* string);

    bool isOpen();
    bool pollEvent(sf::Event& e);
    void close();
    void flush();

  static Display *GetOrInit() {
    static Display local_tft(sf::VideoMode(sdl::kWidth * kDisplayScale,
                                           sdl::kHeight * kDisplayScale),
                             "esp tft");

    static int config = [&]() {
      sf::View scale(sf::Vector2f(sdl::kWidth / 2, sdl::kHeight / 2),
                     sf::Vector2f(sdl::kWidth, sdl::kHeight));

      local_tft.window_.setView(scale);
      return 0;
    }();

        return &local_tft;
    }
    
    void drawPixel(int x, int y, uint32_t c);
    
private:

    Display(sf::VideoMode vm, std::string name);

  static uint16_t ToBigEndian(uint16_t value) {
    return (value >> 8) | (value << 8);
  }

  // color - rgb big endian
  static sf::Color ConvertColor(uint16_t color) {
    uint8_t R8 = (((color & 0xF800) >> 11) * 255 + 15) / 31;
    uint8_t G8 = (((color & 0x07E0) >> 5) * 255 + 31) / 63;
    uint8_t B8 = ((color & 0x001F) * 255 + 15) / 31;
    return {R8, G8, B8};
  }

 private:
    sf::RenderWindow window_;
    
    sf::Font font_;
    sf::Text text_;
};
} // namespace sim