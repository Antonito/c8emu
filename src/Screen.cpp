#include "Screen.hpp"
#include <cmath>

namespace c8emu {
Screen::Screen(std::uint32_t const width, std::uint32_t const height,
               std::array<std::uint8_t, 64 * 32> const &gfx,
               std::array<bool, 16> &keys, std::uint8_t const scaleFactor)
    : m_width(width), m_height(height),
      m_win(sf::VideoMode(m_width * scaleFactor, m_height * scaleFactor),
            "Chip8 Emulator"),
      m_texture(), m_sprite(),
      m_pix(std::make_unique<sf::Uint8[]>(m_width * m_height * 4)), m_gfx(gfx),
      m_keys(keys), m_soundBuff(), m_beep() {
  if (!m_win.isOpen()) {
    throw std::runtime_error(
        "Cannot create SFML window"); // TODO: Real exception
  }
  m_texture.create(m_width, m_height);
  m_sprite.setTexture(m_texture);
  m_sprite.setScale(static_cast<float>(scaleFactor),
                    static_cast<float>(scaleFactor));

  loadBeep();
}

void Screen::loadBeep() {
  std::uint32_t const samples = 44100 / 8;
  std::uint32_t const rate = 44100;
  std::uint32_t const amplitude = 30000;
  sf::Int16 raw[samples];
  double const twoPI = 6.28318;
  double const increment = (440. * 1.5) / 44100;
  double x = 0;

  // Fill array
  for (std::uint32_t i = 0; i < samples; i++) {
    raw[i] = static_cast<sf::Int16>(amplitude * std::sin(x * twoPI));
    x += increment;
  }

  // Load sound
  if (!m_soundBuff.loadFromSamples(raw, samples, 1, rate)) {
    throw std::runtime_error("Cannot load sounds");
  }

  m_beep.setBuffer(m_soundBuff);
}

void Screen::beep() { m_beep.play(); }

void Screen::gpuExec() {
  m_win.clear();
  for (std::uint32_t y = 0; y < m_height; ++y) {
    for (std::uint32_t x = 0; x < m_width; ++x) {
      sf::Uint8 color = 0; // Black
      if (m_gfx[(y * m_width) + x] != 0) {
        color = 255; // White
      }
      m_pix[(y * m_width + x) * 4 + 0] = color; // R
      m_pix[(y * m_width + x) * 4 + 1] = color; // G
      m_pix[(y * m_width + x) * 4 + 2] = color; // B
      m_pix[(y * m_width + x) * 4 + 3] = 255;   // A
    }
  }
  m_texture.update(m_pix.get());
  m_win.draw(m_sprite);
  m_win.display();
}

void Screen::getInputs() {
  sf::Event event;
  while (m_win.pollEvent(event)) {

    // Stop the game
    if (event.type == sf::Event::Closed) {
      m_win.close();
    } else if (event.type == sf::Event::KeyPressed) {
      switch (event.key.code) {
      case sf::Keyboard::Escape:
        m_win.close();
        break;
      case sf::Keyboard::Num1:
        m_keys[0x1] = true;
        break;
      case sf::Keyboard::Num2:
        m_keys[0x2] = true;
        break;
      case sf::Keyboard::Num3:
        m_keys[0x3] = true;
        break;
      case sf::Keyboard::Num4:
        m_keys[0xC] = true;
        break;

      case sf::Keyboard::Q:
        m_keys[0x4] = true;
        break;
      case sf::Keyboard::W:
        m_keys[0x5] = true;
        break;
      case sf::Keyboard::E:
        m_keys[0x6] = true;
        break;
      case sf::Keyboard::R:
        m_keys[0xD] = true;
        break;

      case sf::Keyboard::A:
        m_keys[0x7] = true;
        break;
      case sf::Keyboard::S:
        m_keys[0x8] = true;
        break;
      case sf::Keyboard::D:
        m_keys[0x9] = true;
        break;
      case sf::Keyboard::F:
        m_keys[0xE] = true;
        break;

      case sf::Keyboard::Z:
        m_keys[0xA] = true;
        break;
      case sf::Keyboard::X:
        m_keys[0x0] = true;
        break;
      case sf::Keyboard::C:
        m_keys[0xB] = true;
        break;
      case sf::Keyboard::V:
        m_keys[0xF] = true;
        break;
      }
    } else if (event.type == sf::Event::KeyReleased) {
      switch (event.key.code) {
      case sf::Keyboard::Num1:
        m_keys[0x1] = false;
        break;
      case sf::Keyboard::Num2:
        m_keys[0x2] = false;
        break;
      case sf::Keyboard::Num3:
        m_keys[0x3] = false;
        break;
      case sf::Keyboard::Num4:
        m_keys[0xC] = false;
        break;

      case sf::Keyboard::Q:
        m_keys[0x4] = false;
        break;
      case sf::Keyboard::W:
        m_keys[0x5] = false;
        break;
      case sf::Keyboard::E:
        m_keys[0x6] = false;
        break;
      case sf::Keyboard::R:
        m_keys[0xD] = false;
        break;

      case sf::Keyboard::A:
        m_keys[0x7] = false;
        break;
      case sf::Keyboard::S:
        m_keys[0x8] = false;
        break;
      case sf::Keyboard::D:
        m_keys[0x9] = false;
        break;
      case sf::Keyboard::F:
        m_keys[0xE] = false;
        break;

      case sf::Keyboard::Z:
        m_keys[0xA] = false;
        break;
      case sf::Keyboard::X:
        m_keys[0x0] = false;
        break;
      case sf::Keyboard::C:
        m_keys[0xB] = false;
        break;
      case sf::Keyboard::V:
        m_keys[0xF] = false;
        break;
      }
    }
  }
}

} // namespace c8emu
