/**
 * ShadowSystem - Soft shadow implementation using area lights and shadow mapping
 * Creates realistic soft shadows for game objects
 */
class ShadowSystem {
  constructor(sceneManager) {
    this.sceneManager = sceneManager;
    this.enabled = false;
    this.quality = 'high';
    this.lights = [];
    this.shadowPlanes = [];
  }

  /**
   * Enable soft shadows with area lighting
   */
  enable(quality = 'high') {
    this.quality = quality;
    this.enabled = true;
    
    const renderer = this.sceneManager.renderer;
    
    // Enable shadow mapping (Three.js r89 compatible)
    renderer.shadowMap.enabled = true;
    renderer.shadowMap.type = THREE.PCFSoftShadowMap;
    
    // Setup lights for soft shadows
    this._setupLighting();
  }

  /**
   * Setup area-like lighting using multiple soft lights
   */
  _setupLighting() {
    const scene = this.sceneManager.scene;
    
    // Remove existing directional light
    if (this.sceneManager.lights.directional) {
      scene.remove(this.sceneManager.lights.directional);
    }
    if (this.sceneManager.lights.ambient) {
      scene.remove(this.sceneManager.lights.ambient);
    }

    // Quality settings
    const shadowMapSize = { low: 512, medium: 1024, high: 2048 }[this.quality];
    
    // Main directional light (sun-like) with soft shadows
    const mainLight = new THREE.DirectionalLight(0xffffff, 0.8);
    mainLight.position.set(30, 50, 30);
    mainLight.castShadow = true;
    mainLight.shadow.mapSize.width = shadowMapSize;
    mainLight.shadow.mapSize.height = shadowMapSize;
    mainLight.shadow.camera.near = 0.5;
    mainLight.shadow.camera.far = 200;
    mainLight.shadow.camera.left = -50;
    mainLight.shadow.camera.right = 50;
    mainLight.shadow.camera.top = 50;
    mainLight.shadow.camera.bottom = -50;
    mainLight.shadow.bias = -0.0005;
    scene.add(mainLight);
    this.lights.push(mainLight);
    this.sceneManager.lights.main = mainLight;

    // Secondary fill light (no shadow for performance)
    const fillLight = new THREE.DirectionalLight(0x87ceeb, 0.3);
    fillLight.position.set(-20, 30, -20);
    scene.add(fillLight);
    this.lights.push(fillLight);
    this.sceneManager.lights.fill = fillLight;

    // Hemisphere light for ambient sky/ground color
    const hemiLight = new THREE.HemisphereLight(0x87ceeb, 0x8b7355, 0.4);
    hemiLight.position.set(0, 50, 0);
    scene.add(hemiLight);
    this.lights.push(hemiLight);
    this.sceneManager.lights.hemisphere = hemiLight;

    // Soft ambient light
    const ambientLight = new THREE.AmbientLight(0xffffff, 0.3);
    scene.add(ambientLight);
    this.lights.push(ambientLight);
    this.sceneManager.lights.ambient = ambientLight;

    // Add subtle point lights for local illumination
    this._addAccentLights();
  }

  /**
   * Add accent point lights for extra depth
   */
  _addAccentLights() {
    const scene = this.sceneManager.scene;
    
    // Warm accent light
    const warmLight = new THREE.PointLight(0xffaa44, 0.3, 100);
    warmLight.position.set(10, 20, 10);
    scene.add(warmLight);
    this.lights.push(warmLight);
  }

  /**
   * Configure shadow for a mesh
   */
  configureMesh(mesh, cast = true, receive = true) {
    if (!this.enabled) return;
    
    mesh.castShadow = cast;
    mesh.receiveShadow = receive;
    
    // If mesh has children, configure them too
    mesh.traverse((child) => {
      if (child.isMesh) {
        child.castShadow = cast;
        child.receiveShadow = receive;
      }
    });
  }

  /**
   * Create contact shadow (blob shadow under objects)
   */
  createContactShadow(object, size = 1) {
    const canvas = document.createElement('canvas');
    canvas.width = 64;
    canvas.height = 64;
    const ctx = canvas.getContext('2d');
    
    // Create soft circular gradient
    const gradient = ctx.createRadialGradient(32, 32, 0, 32, 32, 32);
    gradient.addColorStop(0, 'rgba(0, 0, 0, 0.4)');
    gradient.addColorStop(0.5, 'rgba(0, 0, 0, 0.2)');
    gradient.addColorStop(1, 'rgba(0, 0, 0, 0)');
    
    ctx.fillStyle = gradient;
    ctx.fillRect(0, 0, 64, 64);
    
    const texture = new THREE.CanvasTexture(canvas);
    const geometry = new THREE.PlaneGeometry(size * 2, size * 2);
    const material = new THREE.MeshBasicMaterial({
      map: texture,
      transparent: true,
      depthWrite: false
    });
    
    const shadow = new THREE.Mesh(geometry, material);
    shadow.rotation.x = -Math.PI / 2;
    shadow.position.y = -0.99; // Just above ground
    
    this.shadowPlanes.push(shadow);
    return shadow;
  }

  /**
   * Update shadow camera to follow game action
   */
  updateShadowCamera(target) {
    if (!this.enabled) return;
    
    const mainLight = this.sceneManager.lights.main;
    const fillLight = this.sceneManager.lights.fill;
    
    if (mainLight) {
      mainLight.position.x = target.x + 30;
      mainLight.position.z = target.z + 30;
      mainLight.target.position.copy(target);
      mainLight.target.updateMatrixWorld();
    }
    
    if (fillLight) {
      fillLight.position.x = target.x - 20;
      fillLight.position.z = target.z - 20;
      fillLight.target.position.copy(target);
      fillLight.target.updateMatrixWorld();
    }
  }

  /**
   * Disable shadows
   */
  disable() {
    this.enabled = false;
    this.sceneManager.renderer.shadowMap.enabled = false;
    
    // Remove added lights
    this.lights.forEach(light => {
      this.sceneManager.scene.remove(light);
    });
    this.lights = [];
  }

  dispose() {
    this.disable();
    
    this.shadowPlanes.forEach(plane => {
      plane.geometry.dispose();
      plane.material.dispose();
    });
    this.shadowPlanes = [];
  }
}

export default ShadowSystem;
