import Platform from '../entities/Platform.js';
import GameConfig from '../utils/Constants.js';
import Helpers from '../utils/Helpers.js';

/**
 * PlatformManager - handles platform generation, positioning, and lifecycle
 * Uses jumper landing position and consistent direction to ensure reachable platforms
 */
class PlatformManager {
  constructor(sceneManager) {
    this.sceneManager = sceneManager;
    this.platforms = [];
    this.config = GameConfig.platform;
    this.lastJumperPosition = null; // Track where jumper landed
    this.lastDirection = null; // Track last platform direction for consistency
  }

  /**
   * Update the last jumper landing position
   * Called after successful landing to ensure next platform is reachable
   */
  setJumperLandingPosition(position) {
    this.lastJumperPosition = { x: position.x, z: position.z };
  }

  /**
   * Determine the next platform direction
   * Ensures direction is reachable from current jumper position
   */
  _determineDirection() {
    // First platform after start - random direction
    if (this.platforms.length <= 1) {
      this.lastDirection = Helpers.getRandomBoolean() ? 'xDir' : 'zDir';
      return this.lastDirection;
    }
    
    // Get current direction (how we got to current platform)
    const currentDir = this.getDirection();
    
    // 70% chance to continue same direction, 30% to change
    // This creates a more natural path while allowing turns
    if (Math.random() < 0.7) {
      this.lastDirection = currentDir === 'x' ? 'xDir' : 'zDir';
    } else {
      this.lastDirection = currentDir === 'x' ? 'zDir' : 'xDir';
    }
    
    return this.lastDirection;
  }

  /**
   * Create a new platform
   * @param {Object} options - Optional overrides for music-based generation
   * @returns {Platform} The created platform
   */
  createPlatform(options = {}) {
    // Determine platform type (can be overridden by music system)
    const type = options.type || (Helpers.getRandomBoolean() ? 'cube' : 'cylinder');
    
    // Determine direction - use smart direction selection
    const direction = options.direction || this._determineDirection();
    
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
    return maxDistance * 0.8; // 80% of max for safety margin
  }

  /**
   * Calculate position for next platform based on jumper's landing position
   */
  _calculateNextPosition(newPlatform, direction, options = {}) {
    const lastPlatform = this.platforms[this.platforms.length - 1];
    const lastPlatformPos = lastPlatform.position;
    const lastSize = lastPlatform.getHalfSize();
    const newSize = newPlatform.getHalfSize();
    
    // Use jumper's actual landing position if available, otherwise use platform center
    const jumpFromPos = this.lastJumperPosition || { x: lastPlatformPos.x, z: lastPlatformPos.z };
    
    // Calculate max jumpable gap from jumper's position
    const maxJumpable = this._getMaxJumpableDistance();
    const maxGap = Math.min(this.config.maxDistance, maxJumpable);
    
    // Distance can be overridden by music system
    const distance = options.distance || 
      Helpers.getRandomValue(this.config.minDistance, maxGap);
    
    let x, y, z;
    y = lastPlatformPos.y;
    
    if (direction === 'zDir') {
      // Moving in -Z direction - calculate from jumper position
      // New platform center should be reachable from jumper's position
      x = jumpFromPos.x; // Keep X aligned with jumper
      z = jumpFromPos.z - distance - newSize.z;
      
      // Ensure minimum gap from platform edge
      const minZ = lastPlatformPos.z - lastSize.z - this.config.minDistance - newSize.z;
      z = Math.min(z, minZ);
    } else {
      // Moving in +X direction - calculate from jumper position
      z = jumpFromPos.z; // Keep Z aligned with jumper
      x = jumpFromPos.x + distance + newSize.x;
      
      // Ensure minimum gap from platform edge
      const minX = lastPlatformPos.x + lastSize.x + this.config.minDistance + newSize.x;
      x = Math.max(x, minX);
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
   * Uses position difference to determine actual jump direction needed
   */
  getDirection() {
    if (this.platforms.length < 2) return null;
    
    const from = this.getCurrentPlatform();
    const to = this.getTargetPlatform();
    
    // Calculate the actual displacement
    const dx = Math.abs(to.position.x - from.position.x);
    const dz = Math.abs(to.position.z - from.position.z);
    
    // Direction is determined by which axis has greater displacement
    // This ensures jumper always jumps toward the target platform
    if (dx > dz) {
      return 'x'; // Jump in X direction
    } else {
      return 'z'; // Jump in Z direction
    }
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
    this.lastJumperPosition = null;
    this.lastDirection = null;
  }

  /**
   * Dispose of manager
   */
  dispose() {
    this.clear();
  }
}

export default PlatformManager;
