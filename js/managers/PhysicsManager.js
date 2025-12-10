import GameConfig from '../utils/Constants.js';

/**
 * PhysicsManager - handles jump physics and collision detection
 * Encapsulates all physics calculations for the jumper
 */
class PhysicsManager {
  constructor() {
    this.config = GameConfig.physics;
    this.jumperConfig = GameConfig.jumper;
    this.platformConfig = GameConfig.platform;
    
    // Current velocity
    this.xSpeed = 0;
    this.ySpeed = 0;
  }

  /**
   * Charge jump (accumulate power)
   */
  charge() {
    if (this.xSpeed < this.config.maxXSpeed) {
      this.xSpeed += this.config.xAcceleration;
    }
    if (this.ySpeed < this.config.maxYSpeed) {
      this.ySpeed += this.config.yAcceleration;
    }
  }

  /**
   * Apply physics for jump movement
   * @param {Jumper} jumper - The jumper entity
   * @param {string} direction - 'x' or 'z'
   * @returns {boolean} Whether jumper is still in air
   */
  applyJumpPhysics(jumper, direction) {
    const groundLevel = this.jumperConfig.height / 2;
    
    if (jumper.position.y >= groundLevel) {
      // Apply movement
      if (direction === 'x') {
        jumper.position.x += this.xSpeed;
      } else {
        jumper.position.z -= this.xSpeed;
      }
      jumper.position.y += this.ySpeed;
      
      // Apply gravity
      this.ySpeed -= this.config.gravity;
      
      // Restore jumper shape
      jumper.restore();
      
      return true; // Still in air
    }
    
    return false; // Landed
  }

  /**
   * Check landing state using precise 2D collision detection
   * @param {Jumper} jumper - The jumper entity
   * @param {Platform} currentPlatform - Platform jumped from
   * @param {Platform} targetPlatform - Platform to land on
   * @param {string} direction - 'x' or 'z'
   * @returns {number} Landing state code
   */
  checkLanding(jumper, currentPlatform, targetPlatform, direction) {
    const jumperPos = jumper.position;
    const jumpR = this.jumperConfig.bottomRadius;
    const centerThreshold = GameConfig.scoring.centerThreshold;
    
    // Check if on current platform (using 2D bounds check)
    const onCurrent = this._isOnPlatform(jumperPos, currentPlatform, jumpR);
    if (onCurrent.fully) {
      return 1; // Fully on current platform
    }
    
    // Check if on target platform
    const onTarget = this._isOnPlatform(jumperPos, targetPlatform, jumpR);
    if (onTarget.fully) {
      // Check for center landing
      const distToCenter = this._getDistanceToCenter(jumperPos, targetPlatform);
      if (distToCenter <= centerThreshold) {
        return 3; // Center landing!
      }
      return 2; // Normal landing on target
    }
    
    // Check edge cases
    if (onCurrent.partial) {
      return -1; // On edge of current platform, will fall
    }
    
    if (onTarget.partial) {
      return -3; // On edge of target platform, will fall
    }
    
    // Completely missed
    return -2;
  }

  /**
   * Check if jumper is on a platform using 2D bounds
   * Improved detection for both cube and cylinder platforms
   * @returns {Object} { fully: boolean, partial: boolean }
   */
  _isOnPlatform(jumperPos, platform, jumperRadius) {
    const platPos = platform.position;
    const halfSize = platform.getHalfSize();
    
    // For cylinder platforms, use circular bounds check
    if (platform.type === 'cylinder') {
      const dx = jumperPos.x - platPos.x;
      const dz = jumperPos.z - platPos.z;
      const distFromCenter = Math.sqrt(dx * dx + dz * dz);
      const platformRadius = halfSize.x;
      
      // More generous tolerance for cylinder - if center is on platform, count as fully on
      const centerOnPlatform = distFromCenter <= platformRadius;
      const fullyOnCylinder = distFromCenter + jumperRadius * 0.5 <= platformRadius;
      const partiallyOnCylinder = distFromCenter < platformRadius + jumperRadius && 
                                   distFromCenter > platformRadius - jumperRadius;
      
      return {
        fully: fullyOnCylinder || centerOnPlatform,
        partial: partiallyOnCylinder && !centerOnPlatform
      };
    }
    
    // For cube platforms
    // Calculate platform bounds
    const pMinX = platPos.x - halfSize.x;
    const pMaxX = platPos.x + halfSize.x;
    const pMinZ = platPos.z - halfSize.z;
    const pMaxZ = platPos.z + halfSize.z;
    
    // Check if jumper center is within platform bounds
    const centerInX = jumperPos.x >= pMinX && jumperPos.x <= pMaxX;
    const centerInZ = jumperPos.z >= pMinZ && jumperPos.z <= pMaxZ;
    const centerInPlatform = centerInX && centerInZ;
    
    // If center is on platform, consider it fully on (more forgiving)
    if (centerInPlatform) {
      return { fully: true, partial: false };
    }
    
    // Calculate jumper bounds for edge detection
    const jMinX = jumperPos.x - jumperRadius;
    const jMaxX = jumperPos.x + jumperRadius;
    const jMinZ = jumperPos.z - jumperRadius;
    const jMaxZ = jumperPos.z + jumperRadius;
    
    // Check if jumper is partially on platform (edge case)
    const overlapX = jMaxX > pMinX && jMinX < pMaxX;
    const overlapZ = jMaxZ > pMinZ && jMinZ < pMaxZ;
    const partiallyOn = overlapX && overlapZ;
    
    return {
      fully: false,
      partial: partiallyOn
    };
  }

  /**
   * Get distance from jumper to platform center
   */
  _getDistanceToCenter(jumperPos, platform) {
    const platPos = platform.position;
    const dx = jumperPos.x - platPos.x;
    const dz = jumperPos.z - platPos.z;
    return Math.sqrt(dx * dx + dz * dz);
  }

  /**
   * Calculate distances for collision detection (legacy, kept for reference)
   */
  _calculateDistances(jumper, fromPlatform, toPlatform, direction) {
    const fromPos = fromPlatform.position;
    const toPos = toPlatform.position;
    const jumperPos = jumper.position;
    
    const fromSize = fromPlatform.getHalfSize();
    const toSize = toPlatform.getHalfSize();
    
    let d, d1, d2, d3, d4;
    
    if (direction === 'z') {
      // -Z direction
      d = Math.abs(jumperPos.z);
      d1 = Math.abs(fromPos.z - fromSize.z);
      d2 = Math.abs(toPos.z + toSize.z);
      d3 = Math.abs(toPos.z);
      d4 = Math.abs(toPos.z - toSize.z);
    } else {
      // +X direction
      d = Math.abs(jumperPos.x);
      d1 = Math.abs(fromPos.x + fromSize.x);
      d2 = Math.abs(toPos.x - toSize.x);
      d3 = Math.abs(toPos.x);
      d4 = Math.abs(toPos.x + toSize.x);
    }
    
    return { d, d1, d2, d3, d4 };
  }

  /**
   * Reset physics state
   */
  reset() {
    this.xSpeed = 0;
    this.ySpeed = 0;
  }

  /**
   * Get current speed values
   */
  getSpeed() {
    return { x: this.xSpeed, y: this.ySpeed };
  }
}

export default PhysicsManager;
