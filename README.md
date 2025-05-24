# Chaos Amen - LV2 Plugin

Chaotic Amen Break generator using logistic map algorithm for pattern variations.

## Repository Structure

```
chaos-amen-lv2/
├── chaos_amen.cpp     # Main plugin implementation
├── chaos_amen.ttl     # Plugin description 
├── manifest.ttl       # Plugin manifest
├── Makefile          # Build configuration
└── README.md         # This file
```

## Dependencies

**Ubuntu/Debian:**
```bash
sudo apt install build-essential pkg-config lv2-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install gcc-c++ pkgconfig lv2-devel
```

**Arch Linux:**
```bash
sudo pacman -S base-devel pkg-config lv2
```

## Build Instructions

1. **Clone and build:**
```bash
git clone <your-repo-url>
cd chaos-amen-lv2
make
```

2. **Install plugin:**
```bash
# User install (recommended)
make install

# System-wide install
make install-system
```

3. **Verify installation:**
```bash
lv2ls | grep chaos-amen
```

## Plugin Features

- **16-step sequencer** with classic Amen break pattern
- **4 drum voices:** Kick, Snare, Hi-hat, Cowbell
- **Chaotic variations** using logistic map: x[n+1] = k*x*(1-x)
- **Real-time controls:** BPM, Chaos K, Intensity, individual levels

## Parameters

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| BPM | 60-200 | 136 | Tempo |
| Chaos K | 1.0-4.0 | 3.8 | Chaos coefficient |
| Chaos Intensity | 0.0-1.0 | 0.3 | Variation amount |
| Kick Level | 0.0-1.0 | 0.8 | Kick volume |
| Snare Level | 0.0-1.0 | 0.7 | Snare volume |
| Hihat Level | 0.0-1.0 | 0.5 | Hi-hat volume |
| Cowbell Level | 0.0-1.0 | 0.6 | Cowbell volume |

## Usage in DAW

1. Load plugin in LV2-compatible host (Ardour, Qtractor, Carla, etc.)
2. Adjust BPM to match project tempo
3. Experiment with Chaos K values:
   - K < 3.0: Stable patterns
   - K = 3.8: Chaotic behavior
   - K → 4.0: Maximum chaos
4. Use Intensity to control variation amount

## Chaos Mathematics

The plugin uses the logistic map equation to generate variations:
- Values near K=3.8 produce complex, non-repeating patterns
- Higher intensity increases probability of pattern modifications
- Each drum voice has different chaos sensitivity

## Troubleshooting

**Plugin not found:**
- Verify LV2_PATH includes installation directory
- Check `~/.lv2/` or `/usr/lib/lv2/` for bundle

**Build errors:**
- Install missing dependencies
- Update pkg-config path for LV2

**No audio output:**
- Connect plugin output to DAW mixer
- Check individual drum levels

## Development

**Debug build:**
```bash
make debug
```

**Clean build:**
```bash
make clean && make
```

## License

MIT License - See source files for details.
