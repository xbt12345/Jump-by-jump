import GameConfig from '../utils/Constants.js';

/**
 * CameraController - handles camera movement and transitions
 * Uses smooth easing-based animation for fluid camera movement
 */
class CameraController {
  constructor(sceneManager) {
    this.sceneManager = sceneManager;
    this.config = GameConfig.camera;
    
    this.currentPosition = new THREE.Vector3(0, 0, 0);
    this.targetPosition = new THREE.Vector3(0, 0, 0);
    this.startPosition = new THREE.Vector3(0, 0, 0);
    this.isAnimating = false;
    this.animationProgress = 0;
    this.animationDuration = 60; // frames (~1 second at 60fps)
  }

  /**
   * Smooth easing function (ease-out cubic)
   */
  _easeOutCubic(t) {
    return 1 - Math.pow(1 - t, 3);
  }

  /**
   * Smooth easing function (ease-in-out cubic)
   */
  _easeInOutCubic(t) {
    return t < 0.5 ? 4 * t * t * t : 1 - Math.pow(-2 * t + 2, 3) / 2;
  }

  /**
   * Linear interpolation helper
   */
  _lerp(start, end, factor) {
    return start + (end - start) * factor;
  }

  /**
   * Update camera target based on platform positions
   */
  updateTarget(fromPlatform, toPlatform) {
    const fromPos = fromPlatform.position;
    const toPos = toPlatform.position;
    
    // Store start position for easing
    this.startPosition.copy(this.currentPosition);
    
    this.targetPosition.set(
      (fromPos.x + toPos.x) / 2,
      0,
      (fromPos.z + toPos.z) / 2
    );
    
    // Reset animation progress
    this.animationProgress = 0;
    this._animateToTarget();
  }

  /**
   * Animate camera to target position using smooth easing
   */
  _animateToTarget() {
    if (this.isAnimating) return;
    this.isAnimating = true;
    this._animate();
  }

  _animate() {
    const current = this.currentPosition;
    const target = this.targetPosition;
    const start = this.startPosition;
    
    // Increment progress
    this.animationProgress += 1 / this.animationDuration;
    
    // Clamp progress to 1
    if (this.animationProgress >= 1) {
      this.animationProgress = 1;
      current.x = target.x;
      current.z = target.z;
      this.sceneManager.lookAt(new THREE.Vector3(current.x, 0, current.z));
      this.sceneManager.render();
      this.isAnimating = false;
      return;
    }
    
    // Apply easing
    const easedProgress = this._easeOutCubic(this.animationProgress);
    
    // Interpolate position with easing
    current.x = this._lerp(start.x, target.x, easedProgress);
    current.z = this._lerp(start.z, target.z, easedProgress);
    
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
    this.startPosition.set(0, 0, 0);
    this.isAnimating = false;
    this.animationProgress = 0;
    this.lookAtCurrent();
  }
}

export default CameraController;
