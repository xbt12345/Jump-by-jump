import GameConfig from '../utils/Constants.js';

/**
 * CameraController - handles camera movement and transitions
 * Uses smooth lerp-based animation for fluid camera movement
 */
class CameraController {
  constructor(sceneManager) {
    this.sceneManager = sceneManager;
    this.config = GameConfig.camera;
    
    this.currentPosition = new THREE.Vector3(0, 0, 0);
    this.targetPosition = new THREE.Vector3(0, 0, 0);
    this.isAnimating = false;
  }

  /**
   * Linear interpolation helper
   */
  _lerp(start, end, factor) {
    return start + (end - start) * factor;
  }

  /**
   * Update camera target based on platform positions
   * @param {Platform} fromPlatform - Current platform
   * @param {Platform} toPlatform - Target platform
   */
  updateTarget(fromPlatform, toPlatform) {
    const fromPos = fromPlatform.position;
    const toPos = toPlatform.position;
    
    this.targetPosition.set(
      (fromPos.x + toPos.x) / 2,
      0,
      (fromPos.z + toPos.z) / 2
    );
    
    this._animateToTarget();
  }

  /**
   * Animate camera to target position using smooth lerp
   */
  _animateToTarget() {
    if (this.isAnimating) return;
    this.isAnimating = true;
    this._animate();
  }

  _animate() {
    const current = this.currentPosition;
    const target = this.targetPosition;
    const smoothFactor = this.config.smoothFactor;
    
    // Calculate distance to target
    const dx = Math.abs(target.x - current.x);
    const dz = Math.abs(target.z - current.z);
    const threshold = 0.01;
    
    // Check if we've reached the target
    if (dx < threshold && dz < threshold) {
      current.x = target.x;
      current.z = target.z;
      this.sceneManager.lookAt(new THREE.Vector3(current.x, 0, current.z));
      this.sceneManager.render();
      this.isAnimating = false;
      return;
    }
    
    // Smooth lerp towards target
    current.x = this._lerp(current.x, target.x, smoothFactor);
    current.z = this._lerp(current.z, target.z, smoothFactor);
    
    this.sceneManager.lookAt(new THREE.Vector3(current.x, 0, current.z));
    this.sceneManager.render();
    
    requestAnimationFrame(() => this._animate());
  }

  /**
   * Look at current position immediately
   */
  lookAtCurrent() {
    this.sceneManager.lookAt(this.currentPosition);
  }

  /**
   * Reset camera position
   */
  reset() {
    this.currentPosition.set(0, 0, 0);
    this.targetPosition.set(0, 0, 0);
    this.isAnimating = false;
    this.lookAtCurrent();
  }
}

export default CameraController;
