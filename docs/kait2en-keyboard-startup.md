# KAIT2EN keyboard and startup validation

Set `keyboard_text_layout = us` or `keyboard_text_layout = it` in zenith.conf to match the currently active host layout, and restart. Default `unicode` uses application-dependent composition. The direct mapping assumes normal modifier and Caps Lock state; it does not automatically follow layout changes.

Test dummy text in GNOME Text Editor and a browser, including letters, capitals, digits, symbols and multi-character text. Only then test the lock screen. Do not share real passwords or historic debug input logs. In direct-key mode, unsupported characters are skipped rather than emitting a composition shortcut that could submit a password form. Use the GNOME on-screen keyboard for them, or choose the unicode mode for applications that support composition. Text control characters are ignored; ordinary Enter and Backspace key events remain unchanged.

The native systemd user unit excludes system users and GDM. Test reboot and login with one instance and zero restarts. On a controlled test system, occupy TCP 48010 before starting and verify nonzero exit status on RTSP bind failure. No pre-login access, automatic login, Foundation-specific keyboard toggle, adaptive GPU selection or HDMI/encoder changes are included.
