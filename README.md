# SWENU - Simple Wayland ENU

To build:
```sh
make
```

To install(have to be root):
```sh
make install
```

Usage: just like dmenu

## FAQ
Q: What is an ENU? 

## Configuration
Create your config file at `XDG_CONFIG_HOME/swenu/conf.ini` or `HOME/.config/swenu/conf.ini`. 
Example config: (hex colors also supported!)
```ini
fancy_scroll = false
exit_on_focus_lost = true
min_width = 500
font_size = 16

[colors]
text_color = (0.85, 0.85, 0.85, 1.0)
highlight_color = (0.1, 0.3, 0.7, 1.0)
background_color = (0.1, 0.1, 0.1, 1.0)
cursor_color = (0.9, 0.9, 0.9, 0.5)
```

## Todo -
- [x] "selection"
- [x] figure out whats going on with stderr and exit code
- [X] fancy highlights
- [X] scrolling
- [X] special mode if there is no stdin
- [x] sort filtering by match %
- [x] cursor
- [x] protocol clipboard
- [x] page scroll
- [x] emacs keybindings
- [x] "prompt"
- [x] fuzzy filter
- [x] config file
- [ ] border, top padding and set exact height
- [ ] unicode + text shaping + display unrenderable characters as something

Stretch:
- [ ] clean up the code (src folder, rename some files)
- [ ] input fluff (icons, descriptions, output names)
- [ ] arguments to override config such as fancy scroll?
- [ ] response history
