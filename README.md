# picky: a wayland color picker

picky is a tiny color picker that allows you to choose a color in the same vein that [dmenu](https://tools.suckless.org/dmenu/) 
(or [bemenu](https://github.com/Cloudef/bemenu)) allows you to choose text.
Its intended use is in shell scripts and related endeavours.

There may be some confusion regarding calling this program a "color picker". It's not in the same category as, for instance,
[hyprpicker](https://github.com/hyprwm/hyprpicker), since instead of letting you pick a color from your screen, it display its own color palette.

## Usage/Examples
Picking a color immediately closes the window and prints its hex representation on stdout.

```
usage: picky [OPTIONS]
Options:
  --help                    show this message
  -w --width  [PIXELS]      set window width (default: 800)
  -h --height [PIXELS]      set window height (default: 600)
  -z --zoom-region [PIXELS] set the zoom region size (default: 75)
  -b --zoom-border [PIXELS] set the zoom border size (default: 4)
  -u --hue [DEG]            set hue (in HSV degrees) (default: 0)
  -l --lock-hue             lock the hue (disable scroll)
  -f --format [hex|rgb|hsv] set the output format (CSS-ish style)
```

| Keybind        | Description                              |
|----------------|------------------------------------------|
| Control        | "Zoom in" on the currently hovered color |
| Scroll         | Change hue by a small increment          |
| Shift + Scroll | Change hue by a larger increment         |
| Left Click     | Pick the currently hovered color         |

#### Copy your color selection with [wl-clipboard](https://github.com/bugaevc/wl-clipboard)
```sh
wl-copy $(picky)
```

#### Change your desktop background color with [swaybg](https://github.com/swaywm/swaybg) (works on any wlroots compositor)
```sh
swaybg --color $(picky) 
```

## Building
`make build` or `make debug` to build with ASAN.

### Dependencies
 - `libwayland` (wayland-client, wayland-scanner)

a basic `shell.nix` is provided.
