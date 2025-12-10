import GameConfig from '../utils/Constants.js';

/**
 * Jumper entity - the player character
 * Handles jumper creation, scaling, and state with textured materials
 */
class Jumper {
  static textureSystem = null;
  
  constructor() {
    this.mesh = this._createMesh();
    this.config = GameConfig.jumper;
    this.physicsConfig = GameConfig.physics;
    this.mesh.castShadow = true;
    this.mesh.receiveShadow = true;
  }

  static setTextureSystem(system) {
    Jumper.textureSystem = system;
  }

  _createMesh() {
    const { topRadius, bottomRadius, height, segments } = GameConfig.jumper;
    
    const geometry = new THREE.CylinderGeometry(topRadius, bottomRadius, height, segments);
    const material = this._createMaterial();
    const mesh = new THREE.Mesh(geometry, material);
    
    // Translate geometry so bottom is at y=0
    geometry.translate(0, height / 2, 0);
    mesh.position.set(0, height / 2, 0);
    
    return mesh;
  }

  _createMaterial() {
    // Use textured material if texture system is available
    if (Jumper.textureSystem) {
      return Jumper.textureSystem.createJumperMaterial();
    }
    // Fallback to basic material
    const { color } = GameConfig.jumper;
    return new THREE.MeshLambertMaterial({ color });
  }

  /**
   * Get jumper position
   */
  get position() {
    return this.mesh.position;
  }

  /**
   * Get jumper rotation
   */
  get rotation() {
    return this.mesh.rotation;
  }

  /**
   * Get jumper scale
   */
  get scale() {
    return this.mesh.scale;
  }

  /**
   * Get the bottom radius for collision detection
   */
  get bottomRadius() {
    return this.config.bottomRadius;
  }

  /**
   * Get the height
   */
  get height() {
    return this.config.height;
  }

  /**
   * Compress the jumper (during charge)
   * @returns {boolean} Whether compression is still possible
   */
  compress() {
    if (this.mesh.scale.y > this.physicsConfig.minScale) {
      this.mesh.scale.y -= this.physicsConfig.scaleDecrement;
      return true;
    }
    return false;
  }

  /**
   * Restore jumper to normal scale
   * @returns {boolean} Whether restoration is complete
   */
  restore() {
    if (this.mesh.scale.y < 1) {
      this.mesh.scale.y += this.physicsConfig.scaleIncrement;
      return false;
    }
    this.mesh.scale.y = 1;
    return true;
  }

  /**
   * Reset jumper to initial state
   */
  reset() {
    this.mesh.scale.y = 1;
    this.mesh.rotation.set(0, 0, 0);
  }

  /**
   * Set position
   */
  setPosition(x, y, z) {
    this.mesh.position.set(x, y, z);
  }

  /**
   * Check if jumper is on ground level
   */
  isOnGround() {
    return this.mesh.position.y >= this.config.height / 2;
  }

  /**
   * Dispose of jumper resources
   */
  dispose() {
    if (this.mesh) {
      this.mesh.geometry.dispose();
      this.mesh.material.dispose();
    }
  }
}

export default Jumper;
