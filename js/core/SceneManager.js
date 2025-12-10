import GameConfig from '../utils/Constants.js';

/**
 * SceneManager - handles Three.js scene, camera, renderer, and lighting
 * Prepared for soft shadow implementation
 */
class SceneManager {
  constructor() {
    this.scene = null;
    this.camera = null;
    this.renderer = null;
    this.lights = {};
    
    this._initScene();
    this._initCamera();
    this._initRenderer();
    this._initLights();
  }

  _initScene() {
    this.scene = new THREE.Scene();
    // Future: Add fog, background, etc.
  }

  _initCamera() {
    const { frustumSize, near, far, position } = GameConfig.camera;
    const aspect = window.innerWidth / window.innerHeight;
    
    this.camera = new THREE.OrthographicCamera(
      window.innerWidth / -frustumSize,
      window.innerWidth / frustumSize,
      window.innerHeight / frustumSize,
      window.innerHeight / -frustumSize,
      near,
      far
    );
    
    this.camera.position.set(position.x, position.y, position.z);
  }

  _initRenderer() {
    this.renderer = new THREE.WebGLRenderer({ antialias: true });
    this.renderer.setSize(window.innerWidth, window.innerHeight);
    this.renderer.setPixelRatio(window.devicePixelRatio);
    
    // Future: Enable shadows
    // this.renderer.shadowMap.enabled = true;
    // this.renderer.shadowMap.type = THREE.PCFSoftShadowMap;
    
    document.body.appendChild(this.renderer.domElement);
  }

  _initLights() {
    const { directional, ambient } = GameConfig.lighting;
    
    // Directional light
    this.lights.directional = new THREE.DirectionalLight(
      directional.color,
      directional.intensity
    );
    this.lights.directional.position.set(
      directional.position.x,
      directional.position.y,
      directional.position.z
    );
    
    // Future: Configure shadows
    // this.lights.directional.castShadow = true;
    // this.lights.directional.shadow.mapSize.width = 2048;
    // this.lights.directional.shadow.mapSize.height = 2048;
    
    this.scene.add(this.lights.directional);
    
    // Ambient light
    this.lights.ambient = new THREE.AmbientLight(
      ambient.color,
      ambient.intensity
    );
    this.scene.add(this.lights.ambient);
  }

  /**
   * Get the canvas element
   */
  get canvas() {
    return this.renderer.domElement;
  }

  /**
   * Add object to scene
   */
  add(object) {
    this.scene.add(object);
  }

  /**
   * Remove object from scene
   */
  remove(object) {
    this.scene.remove(object);
  }

  /**
   * Render the scene
   */
  render() {
    this.renderer.render(this.scene, this.camera);
  }

  /**
   * Update camera to look at target
   */
  lookAt(target) {
    this.camera.lookAt(target);
  }

  /**
   * Handle window resize
   */
  onResize() {
    const { frustumSize } = GameConfig.camera;
    
    this.camera.left = window.innerWidth / -frustumSize;
    this.camera.right = window.innerWidth / frustumSize;
    this.camera.top = window.innerHeight / frustumSize;
    this.camera.bottom = window.innerHeight / -frustumSize;
    this.camera.updateProjectionMatrix();
    
    this.renderer.setSize(window.innerWidth, window.innerHeight);
  }

  /**
   * Dispose of all resources
   */
  dispose() {
    this.renderer.dispose();
    // Dispose lights, etc.
  }
}

export default SceneManager;
