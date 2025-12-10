import Platform from '../entities/Platform.js';
import GameConfig from '../utils/Constants.js';
import Helpers from '../utils/Helpers.js';

/**
 * PlatformManager - handles platform generation, positioning, and lifecycle
 * Prepared for music-based platform generation
 */
class PlatformManager {
  constructor(sceneManager) {
    this.sceneManager = sceneManager;
    this.platforms = [];
    this.config = GameConfig.platform;
  }

  /**
   * Create a new platform
   * @param {Object} options - Optional overrides for music-based generation
   * @returns {Platform} The created platform
   */
  createPlatform(options = {}) {
    // Determine platform type (can be overridden by music system)
    const type = options.type || (Helpers.getRandomBoolean() ? 'cube' : 'cylinder');
    
    // Determine direction (can be overridden by music system)
    const direction = options.direction || (Helpers.getRandomBoolean() ? 'xDir' : 'zDir');
    
    const platform = new Platform(type);
    
    // Position the platform
    if (this.platforms.length === 0) {
      platform.setPosition(0, 0, 0);
    } else {
      const position = this._calculateNextPosition(platform, direction, options);
      platform.setPosition(position.x, position.y, position.z);
    }
    
    // Validate position
    Helpers.validatePosition(platform.position);
    
    // Add to scene and track
    this.platforms.push(platform);
    this.sceneManager.add(platform.mesh);
    
    // Remove old platforms if exceeding max count
    this._pruneOldPlatforms();
    
    return platform;
  }

  /**
   * Calculate the maximum jumpable distance based on physics
   */
  _getMaxJumpableDistance() {
    const physics = GameConfig.physics;
    // Calculate max jump distance based on max speeds
    // Using projectile motion: distance = vx * t, where t = 2 * vy / g
    const maxTime = 2 * physics.maxYSpeed / physics.gravity;
    const maxDistance = physics.maxXSpeed * maxTime;
    return maxDistance * 0.85; // 85% of max for safety margin
  }

  /**
   * Calculate position for next platform
   */
  _calculateNextPosition(newPlatform, direction, options = {}) {
    const lastPlatform = this.platforms[this.platforms.length - 1];
    const lastPos = lastPlatform.position;
    
    const lastSize = lastPlatform.getHalfSize();
    const newSize = newPlatform.getHalfSize();
    
    // Calculate max jumpable gap (excluding platform sizes)
    const maxJumpable = this._getMaxJumpableDistance();
    const maxGap = Math.min(this.config.maxDistance, maxJumpable);
    
    // Distance can be overridden by music system
    const distance = options.distance || 
      Helpers.getRandomValue(this.config.minDistance, maxGap);
    
    let x = lastPos.x;
    let y = lastPos.y;
    let z = lastPos.z;
    
    if (direction === 'zDir') {
      // Moving in -Z direction
      z = lastPos.z - distance - lastSize.z - newSize.z;
    } else {
      // Moving in +X direction
      x = lastPos.x + distance + lastSize.x + newSize.x;
    }
    
    return { x, y, z };
  }

  /**
   * Remove old platforms to maintain max count
   */
  _pruneOldPlatforms() {
    while (this.platforms.length > this.config.maxCount) {
      const oldPlatform = this.platforms.shift();
      this.sceneManager.remove(oldPlatform.mesh);
      oldPlatform.dispose();
    }
  }

  /**
   * Get the current platform (second to last)
   */
  getCurrentPlatform() {
    if (this.platforms.length < 2) return null;
    return this.platforms[this.platforms.length - 2];
  }

  /**
   * Get the target platform (last)
   */
  getTargetPlatform() {
    if (this.platforms.length < 1) return null;
    return this.platforms[this.platforms.length - 1];
  }

  /**
   * Get direction between current and target platform
   */
  getDirection() {
    if (this.platforms.length < 2) return null;
    
    const from = this.getCurrentPlatform();
    const to = this.getTargetPlatform();
    
    if (from.position.z === to.position.z) return 'x';
    if (from.position.x === to.position.x) return 'z';
    
    return null;
  }

  /**
   * Get platform count
   */
  get count() {
    return this.platforms.length;
  }

  /**
   * Clear all platforms
   */
  clear() {
    this.platforms.forEach(platform => {
      this.sceneManager.remove(platform.mesh);
      platform.dispose();
    });
    this.platforms = [];
  }

  /**
   * Dispose of manager
   */
  dispose() {
    this.clear();
  }
}

export default PlatformManager;
