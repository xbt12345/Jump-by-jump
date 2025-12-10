import GameConfig from '../utils/Constants.js';

/**
 * Platform entity - represents a single platform (cube or cylinder)
 * Encapsulates platform creation and type detection
 */
class Platform {
  constructor(type = 'cube', size = null) {
    this.type = type;
    this.size = size || this._generateRandomSize();
    this.mesh = this._createMesh();
  }

  _generateRandomSize() {
    if (this.type === 'cube') {
      const { minWidth, maxWidth, minDepth, maxDepth } = GameConfig.platform.cube;
      return {
        width: minWidth + Math.random() * (maxWidth - minWidth),
        depth: minDepth + Math.random() * (maxDepth - minDepth)
      };
    } else {
      const { minRadius, maxRadius } = GameConfig.platform.cylinder;
      return {
        radius: minRadius + Math.random() * (maxRadius - minRadius)
      };
    }
  }

  _createMesh() {
    const geometry = this._createGeometry();
    const material = this._createMaterial();
    return new THREE.Mesh(geometry, material);
  }

  _createGeometry() {
    if (this.type === 'cube') {
      const { height } = GameConfig.platform.cube;
      return new THREE.CubeGeometry(this.size.width, height, this.size.depth);
    } else {
      const { height, segments } = GameConfig.platform.cylinder;
      return new THREE.CylinderGeometry(this.size.radius, this.size.radius, height, segments);
    }
  }

  _createMaterial() {
    const color = this.type === 'cube' 
      ? GameConfig.platform.cube.color 
      : GameConfig.platform.cylinder.color;
    return new THREE.MeshLambertMaterial({ color });
  }

  /**
   * Get the platform's effective radius/half-width for collision detection
   */
  getHalfSize() {
    if (this.type === 'cube') {
      return {
        x: this.size.width / 2,
        z: this.size.depth / 2
      };
    } else {
      return {
        x: this.size.radius,
        z: this.size.radius
      };
    }
  }

  /**
   * Set position of the platform
   */
  setPosition(x, y, z) {
    this.mesh.position.set(x, y, z);
  }

  /**
   * Get position of the platform
   */
  get position() {
    return this.mesh.position;
  }

  /**
   * Check if this platform is a cube
   */
  isCube() {
    return this.type === 'cube';
  }

  /**
   * Check if this platform is a cylinder
   */
  isCylinder() {
    return this.type === 'cylinder';
  }

  /**
   * Dispose of platform resources
   */
  dispose() {
    if (this.mesh) {
      this.mesh.geometry.dispose();
      this.mesh.material.dispose();
    }
  }
}

export default Platform;
