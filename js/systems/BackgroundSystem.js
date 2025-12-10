/**
 * BackgroundSystem - Animated background with particles and gradient sky
 * Creates dynamic, immersive atmosphere
 */
class BackgroundSystem {
  constructor(sceneManager) {
    this.sceneManager = sceneManager;
    this.particles = null;
    this.particleCount = 200;
    this.particleVelocities = [];
    this.skyMesh = null;
    this.groundMesh = null;
    this.time = 0;
    this.isAnimating = false;
    
    this._init();
  }

  _init() {
    this._createGradientSky();
    this._createGroundPlane();
    this._createParticles();
    this._startAnimation();
  }

  /**
   * Create gradient sky dome
   */
  _createGradientSky() {
    const canvas = document.createElement('canvas');
    canvas.width = 512;
    canvas.height = 512;
    const ctx = canvas.getContext('2d');

    // Create sky gradient
    const gradient = ctx.createLinearGradient(0, 0, 0, canvas.height);
    gradient.addColorStop(0, '#1a237e');    // Deep blue at top
    gradient.addColorStop(0.3, '#3949ab');  // Medium blue
    gradient.addColorStop(0.6, '#7986cb');  // Light blue
    gradient.addColorStop(0.8, '#c5cae9');  // Very light blue
    gradient.addColorStop(1, '#e8eaf6');    // Almost white at horizon
    
    ctx.fillStyle = gradient;
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    // Add subtle clouds
    ctx.fillStyle = 'rgba(255, 255, 255, 0.1)';
    for (let i = 0; i < 20; i++) {
      const x = Math.random() * canvas.width;
      const y = Math.random() * canvas.height * 0.6;
      const w = Math.random() * 100 + 50;
      const h = Math.random() * 30 + 10;
      ctx.beginPath();
      ctx.ellipse(x, y, w, h, 0, 0, Math.PI * 2);
      ctx.fill();
    }

    const texture = new THREE.CanvasTexture(canvas);
    
    // Create large sphere for sky
    const geometry = new THREE.SphereGeometry(500, 32, 32);
    const material = new THREE.MeshBasicMaterial({
      map: texture,
      side: THREE.BackSide,
      fog: false
    });
    
    this.skyMesh = new THREE.Mesh(geometry, material);
    this.sceneManager.add(this.skyMesh);
  }

  /**
   * Create ground plane with gradient
   */
  _createGroundPlane() {
    const canvas = document.createElement('canvas');
    canvas.width = 512;
    canvas.height = 512;
    const ctx = canvas.getContext('2d');

    // Create ground gradient
    const gradient = ctx.createRadialGradient(256, 256, 0, 256, 256, 360);
    gradient.addColorStop(0, '#e0e0e0');
    gradient.addColorStop(0.5, '#bdbdbd');
    gradient.addColorStop(1, '#9e9e9e');
    
    ctx.fillStyle = gradient;
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    // Add grid pattern
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.3)';
    ctx.lineWidth = 1;
    const gridSize = 32;
    for (let x = 0; x <= canvas.width; x += gridSize) {
      ctx.beginPath();
      ctx.moveTo(x, 0);
      ctx.lineTo(x, canvas.height);
      ctx.stroke();
    }
    for (let y = 0; y <= canvas.height; y += gridSize) {
      ctx.beginPath();
      ctx.moveTo(0, y);
      ctx.lineTo(canvas.width, y);
      ctx.stroke();
    }

    const texture = new THREE.CanvasTexture(canvas);
    texture.wrapS = THREE.RepeatWrapping;
    texture.wrapT = THREE.RepeatWrapping;
    texture.repeat.set(20, 20);

    const geometry = new THREE.PlaneGeometry(1000, 1000);
    const material = new THREE.MeshLambertMaterial({
      map: texture
    });
    
    this.groundMesh = new THREE.Mesh(geometry, material);
    this.groundMesh.rotation.x = -Math.PI / 2;
    this.groundMesh.position.y = -1.5;
    this.groundMesh.receiveShadow = true;
    this.sceneManager.add(this.groundMesh);
  }

  /**
   * Create floating particles for atmosphere
   */
  _createParticles() {
    const geometry = new THREE.BufferGeometry();
    const positions = new Float32Array(this.particleCount * 3);
    const colors = new Float32Array(this.particleCount * 3);

    for (let i = 0; i < this.particleCount; i++) {
      // Position particles in a large area around the game
      positions[i * 3] = (Math.random() - 0.5) * 200;
      positions[i * 3 + 1] = Math.random() * 50 + 5;
      positions[i * 3 + 2] = (Math.random() - 0.5) * 200;

      // Soft white/blue colors
      colors[i * 3] = 0.8 + Math.random() * 0.2;
      colors[i * 3 + 1] = 0.8 + Math.random() * 0.2;
      colors[i * 3 + 2] = 0.9 + Math.random() * 0.1;

      // Store velocities for animation
      this.particleVelocities.push({
        x: (Math.random() - 0.5) * 0.02,
        y: (Math.random() - 0.5) * 0.01,
        z: (Math.random() - 0.5) * 0.02
      });
    }

    geometry.addAttribute('position', new THREE.BufferAttribute(positions, 3));
    geometry.addAttribute('color', new THREE.BufferAttribute(colors, 3));

    // Create particle material (Three.js r89 compatible)
    const material = new THREE.PointsMaterial({
      size: 0.5,
      vertexColors: THREE.VertexColors,
      transparent: true,
      opacity: 0.6,
      sizeAttenuation: true
    });

    this.particles = new THREE.Points(geometry, material);
    this.sceneManager.add(this.particles);
  }

  /**
   * Start background animation loop
   */
  _startAnimation() {
    this.isAnimating = true;
    this._animate();
  }

  _animate() {
    if (!this.isAnimating) return;

    this.time += 0.01;

    // Animate particles
    if (this.particles) {
      const positions = this.particles.geometry.attributes.position.array;
      
      for (let i = 0; i < this.particleCount; i++) {
        const vel = this.particleVelocities[i];
        
        positions[i * 3] += vel.x;
        positions[i * 3 + 1] += vel.y + Math.sin(this.time + i) * 0.005;
        positions[i * 3 + 2] += vel.z;

        // Wrap particles around
        if (positions[i * 3] > 100) positions[i * 3] = -100;
        if (positions[i * 3] < -100) positions[i * 3] = 100;
        if (positions[i * 3 + 2] > 100) positions[i * 3 + 2] = -100;
        if (positions[i * 3 + 2] < -100) positions[i * 3 + 2] = 100;
        
        // Keep particles in vertical range
        if (positions[i * 3 + 1] > 55) positions[i * 3 + 1] = 5;
        if (positions[i * 3 + 1] < 5) positions[i * 3 + 1] = 55;
      }
      
      this.particles.geometry.attributes.position.needsUpdate = true;
    }

    // Slowly rotate sky for subtle movement
    if (this.skyMesh) {
      this.skyMesh.rotation.y += 0.0001;
    }

    requestAnimationFrame(() => this._animate());
  }

  /**
   * Update ground position to follow camera
   */
  updatePosition(x, z) {
    if (this.groundMesh) {
      this.groundMesh.position.x = x;
      this.groundMesh.position.z = z;
    }
    if (this.skyMesh) {
      this.skyMesh.position.x = x;
      this.skyMesh.position.z = z;
    }
  }

  dispose() {
    this.isAnimating = false;
    
    if (this.particles) {
      this.particles.geometry.dispose();
      this.particles.material.dispose();
      this.sceneManager.remove(this.particles);
    }
    
    if (this.skyMesh) {
      this.skyMesh.geometry.dispose();
      this.skyMesh.material.dispose();
      this.sceneManager.remove(this.skyMesh);
    }
    
    if (this.groundMesh) {
      this.groundMesh.geometry.dispose();
      this.groundMesh.material.dispose();
      this.sceneManager.remove(this.groundMesh);
    }
  }
}

export default BackgroundSystem;
