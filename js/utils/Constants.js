/**
 * Game configuration constants
 * Centralized configuration for easy tuning and future music-based modifications
 */
const GameConfig = {
  // Jumper settings
  jumper: {
    topRadius: 0.3,
    bottomRadius: 0.5,
    height: 2,
    color: 0xffff00,
    segments: 100
  },

  // Platform settings
  platform: {
    cube: {
      minWidth: 2,
      maxWidth: 4,
      width: 4,
      height: 2,
      minDepth: 2,
      maxDepth: 4,
      depth: 4,
      color: 0x00ff00
    },
    cylinder: {
      minRadius: 1,
      maxRadius: 2,
      radius: 2,
      height: 2,
      color: 0x00ff00,
      segments: 100
    },
    maxCount: 6,
    minDistance: 1,
    maxDistance: 3
  },

  // Physics settings (optimized for faster charging and smoother animation)
  physics: {
    xAcceleration: 0.0016,    // 2x faster charging
    yAcceleration: 0.0032,    // 2x faster charging
    gravity: 0.012,           // Slightly higher for snappier jumps
    scaleDecrement: 0.004,    // 2x faster compression
    scaleIncrement: 0.04,     // 2x faster restoration
    minScale: 0.02,
    fallSpeed: 0.08,          // Faster fall animation
    maxXSpeed: 0.25,
    maxYSpeed: 0.5
  },

  // Camera settings (smoother movement)
  camera: {
    frustumSize: 80,
    near: 0.1,
    far: 5000,
    position: { x: 100, y: 100, z: 100 },
    moveSpeed: 0.05,
    smoothFactor: 0.04       // Lower = smoother (was 0.08)
  },

  // Lighting settings (prepared for soft shadows)
  lighting: {
    directional: {
      color: 0xffffff,
      intensity: 1.1,
      position: { x: 3, y: 10, z: 15 }
    },
    ambient: {
      color: 0xffffff,
      intensity: 0.3
    }
  },

  // Scoring settings
  scoring: {
    baseScore: 1,
    centerScore: 2,
    centerThreshold: 0.5  // Distance from center to count as center landing
  }
};

export default GameConfig;
