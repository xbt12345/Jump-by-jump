# Jump Jump - 3D Rhythm Game

A Three.js-based 3D rhythm game inspired by WeChat's "Jump Jump".

## How to Play

1. **Hold SPACE** (or click/tap) to charge jump power
2. **Release** to jump - longer hold = further jump
3. Land on platforms to score points
4. **Center landing** = more points + combo multiplier
5. Fall off = game over

## Running

```bash
python -m http.server 8080
# or
npx serve .
```

Open `http://localhost:8080`

## Project Structure

```
src/
├── core/               # Core systems
│   ├── EventBus.js         # Pub/sub event system
│   ├── GameConfig.js       # Centralized configuration
│   ├── GameState.js        # Game state management
│   └── Timer.js            # High-precision timer
├── gameplay/           # Game logic
│   ├── Player.js           # Player with kinematic physics
│   ├── Platform.js         # Platform entity
│   ├── PlatformGenerator.js # Seeded RNG platform generation
│   ├── ScoreManager.js     # Score calculation
│   └── Collision.js        # Landing detection
├── systems/            # Engine systems
│   ├── SceneManager.js     # Three.js rendering
│   ├── InputSystem.js      # Input handling
│   └── UISystem.js         # UI updates
├── effects/            # Visual effects
│   └── ParticleSystem.js   # Particle effects
├── entities/           # Scene entities
│   └── Environment.js      # Background elements
├── utils/              # Utilities
│   ├── AnimationManager.js # Tween animations
│   ├── Easing.js           # Easing functions
│   └── RNG.js              # Seeded random number generator
├── Game.js             # Main game orchestrator
└── main.js             # Entry point
```

## Scoring System

- **Center landing**: `baseScore * 1.0 * comboMultiplier`
- **Good landing**: `baseScore * 0.7 * comboMultiplier`
- **Edge landing**: `baseScore * 0.3-0.5 * comboMultiplier`
- **Combo multiplier**: `1 + (combo * 0.1)`, max 3.0

## Architecture Features

- **Seeded RNG**: Deterministic platform generation for fair gameplay
- **Event-driven**: Decoupled systems via EventBus
- **Kinematic physics**: Predictable parabolic jump arcs
- **Modular design**: Easy to extend with audio/beat sync
