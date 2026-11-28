/**
 * @file src/platform/linux/input/text_keymap.h
 * @brief Direct text-to-key mapping for explicitly selected US and Italian layouts.
 */
#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace platf::keyboard::text {
  /** @brief A virtual key and its required modifiers. */
  struct stroke {
    std::uint16_t key;  ///< Windows virtual key used by Inputtino.
    bool shift = false;  ///< Hold left Shift.
    bool altgr = false;  ///< Hold right Alt.
  };

  /**
   * @brief Look up a character on the explicitly configured keyboard layout.
   * @param cp Unicode code point.
   * @param layout XKB layout name (us or it).
   * @return A direct stroke, or no value when composition is required.
   */
  inline std::optional<stroke> lookup(char32_t cp, std::string_view layout) {
    if (layout != "us" && layout != "it") {
      return std::nullopt;
    }
    if (cp >= U'a' && cp <= U'z') {
      return stroke {static_cast<std::uint16_t>(0x41 + cp - U'a')};
    }
    if (cp >= U'A' && cp <= U'Z') {
      return stroke {static_cast<std::uint16_t>(0x41 + cp - U'A'), true};
    }
    if (cp >= U'0' && cp <= U'9') {
      return stroke {static_cast<std::uint16_t>(0x30 + cp - U'0')};
    }
    if (cp == U' ') {
      return stroke {0x20};
    }
    if (cp < 0x20 || cp == 0x7f) {
      return std::nullopt;
    }
    auto digits = layout == "us" ? U")!@#$%^&*(" : U"=!\"£$%&/()";
    for (std::uint16_t i = 0; i < 10; ++i) {
      if (digits[i] == cp) {
        return stroke {static_cast<std::uint16_t>(0x30 + i), true};
      }
    }
    constexpr std::uint16_t keys[] = {
      0xC0, 0xBD, 0xBB, 0xDB, 0xDD, 0xDC, 0xBA, 0xDE, 0xBC, 0xBE, 0xBF, 0xE2
    };
    const std::u32string_view plain = layout == "us" ? U"\x60-=[]\\;',./" : U"\\'ìè+ùòà,.-<";
    const std::u32string_view shifted = layout == "us" ? U"~_+{}|:\"<>?" : U"|?^é*§ç°;:_>";
    for (std::size_t i = 0; i < plain.size(); ++i) {
      if (cp == plain[i]) {
        return stroke {keys[i]};
      }
      if (cp == shifted[i]) {
        return stroke {keys[i], true};
      }
    }
    if (layout == "it") {
      switch (cp) {
        case U'[': return stroke {0xDB, false, true};
        case U']': return stroke {0xDD, false, true};
        case U'{': return stroke {0xDB, true, true};
        case U'}': return stroke {0xDD, true, true};
        case U'@': return stroke {0xBA, false, true};
        case U'#': return stroke {0xDE, false, true};
        case U'€': return stroke {0x45, false, true};
        default: break;
      }
    }
    return std::nullopt;
  }
}  // namespace platf::keyboard::text
