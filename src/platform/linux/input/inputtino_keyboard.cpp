/**
 * @file src/platform/linux/input/inputtino_keyboard.cpp
 * @brief Definitions for inputtino keyboard input handling.
 */
// standard includes
#include <charconv>

// lib includes
#include <boost/locale.hpp>
#include <inputtino/input.hpp>
#include <libevdev/libevdev.h>

// local includes
#include "inputtino_common.h"
#include "inputtino_keyboard.h"
#include "text_keymap.h"
#include "src/config.h"
#include "src/logging.h"
#include "src/platform/common.h"
#include "src/utility.h"

using namespace std::literals;

namespace platf::keyboard {

  /**
   * @brief Apply the supplied state update to the platform backend.
   */
  void update(input_raw_t *raw, uint16_t modcode, bool release, uint8_t flags) {
    if (raw->keyboard) {
      if (release) {
        (*raw->keyboard).release(modcode);
      } else {
        (*raw->keyboard).press(modcode);
      }
    }
  }

  /**
   * @brief Submit UTF-8 text input to the keyboard backend.
   */
  void unicode(input_raw_t *raw, char *utf8, int size) {
    if (!raw->keyboard || !utf8 || size <= 0) {
      return;
    }
    std::u32string codepoints;
    try {
      codepoints = boost::locale::conv::utf_to_utf<char32_t>(utf8, utf8 + size, boost::locale::conv::stop);
    } catch (const boost::locale::conv::conversion_error &) {
      BOOST_LOG(warning) << "Ignoring invalid UTF-8 keyboard input"sv;
      return;
    }
    auto &keyboard = *raw->keyboard;
    auto tap = [&keyboard](std::uint16_t key) {
      keyboard.press(key);
      keyboard.release(key);
    };
    for (auto cp : codepoints) {
      if (cp < 0x20 || cp == 0x7f) {
        continue;
      }
      if (auto direct = text::lookup(cp, config::input.keyboard_text_layout)) {
        if (direct->shift) {
          keyboard.press(0xA0);
        }
        if (direct->altgr) {
          keyboard.press(0xA5);
        }
        tap(direct->key);
        if (direct->altgr) {
          keyboard.release(0xA5);
        }
        if (direct->shift) {
          keyboard.release(0xA0);
        }
        continue;
      }
      // A direct-key layout is also suitable for authentication fields.
      // Do not fall back to a composition shortcut there: an unsupported
      // application could interpret its final Enter as form submission.
      if (config::input.keyboard_text_layout != "unicode") {
        BOOST_LOG(debug) << "Text character unavailable in configured direct-key layout"sv;
        continue;
      }
      // Application-dependent composition, one code point at a time.
      char hex[8];
      auto result = std::to_chars(hex, hex + sizeof(hex), static_cast<std::uint32_t>(cp), 16);
      if (result.ec != std::errc {}) {
        continue;
      }
      keyboard.press(0xA2);
      keyboard.press(0xA0);
      tap(0x55);
      keyboard.release(0xA0);
      keyboard.release(0xA2);
      for (auto digit = hex; digit != result.ptr; ++digit) {
        tap(static_cast<std::uint16_t>(*digit <= '9' ? 0x30 + *digit - '0' : 0x41 + *digit - 'a'));
      }
      tap(0x0D);
    }
  }
}  // namespace platf::keyboard
