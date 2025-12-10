/**
 * Jump Jump Game - Bundled Version
 * Enhanced with textures, soft shadows, and animated backgrounds
 */

(function() {
  'use strict';

  // ============================================
  // Utils: Helpers
  // ============================================
  const Helpers = {
    getRandomValue(min, max) {
      return min + Math.random() * (max - min);
    },

    getRandomBoolean(probability = 0.5) {
      return Math.random() > probability;
    },

    isPC() {
      const userAgentInfo = navigator.userAgent;
      const mobileAgents = ['Android', 'iPhone', 'SymbianOS', 'Windows Phone', 'iPad', 'iPod'];
      return !mobileAgents.some(agent => userAgentInfo.includes(agent));
    },

    getInputEvents() {
      const isPC = this.isPC();
      return {
        down: isPC ? 'mousedown' : 'touchstart',
        up: isPC ? 'mouseup' : 'touchend'
      };
    },

    validatePosition(position) {
      if (isNaN(position.x) || isNaN(position.y) || isNaN(position.z)) {
        console.warn('Invalid position detected:', position);
        return false;
      }
      return true;
    }
  };

  // ============================================
  // Utils: Constants
  // ============================================
  const GameConfig = {
    jumper: {
      topRadius: 0.3,
      bottomRadius: 0.5,
      height: 2,
      color: 0xffff00,
      segments: 100
    },
    platform: {
      cube: {
        minWidth: 2,
        maxWidth: 4,
        width: 4,
        height: 2,
        minDepth: 2,
        maxDepth: 4,
        depth: 4,
        color: 0x00ff00
      },
      cylinder: {
        minRadius: 1,
        maxRadius: 2,
        radius: 2,
        height: 2,
        color: 0x00ff00,
        segments: 100
      },
      maxCount: 6,
      minDistance: 1,
      maxDistance: 3
    },
    physics: {
      xAcceleration: 0.0016,
      yAcceleration: 0.0032,
      gravity: 0.012,
      scaleDecrement: 0.004,
      scaleIncrement: 0.04,
      minScale: 0.02,
      fallSpeed: 0.08,
      maxXSpeed: 0.25,
      maxYSpeed: 0.5
    },
    camera: {
      frustumSize: 80,
      near: 0.1,
      far: 5000,
      position: { x: 100, y: 100, z: 100 },
      moveSpeed: 0.05,
      smoothFactor: 0.04
    },
    lighting: {
      directional: { color: 0xffffff, intensity: 1.1, position: { x: 3, y: 10, z: 15 } },
      ambient: { color: 0xffffff, intensity: 0.3 }
    },
    scoring: {
      baseScore: 1,
      centerScore: 2,
      centerThreshold: 0.5
    }
  };

  // ============================================
  // Systems: TextureSystem (Matte Plastic Textures)
  // ============================================
  class TextureSystem {
    constructor() {
      this.textureCache = new Map();
      this.platformColors = [
        { base: '#4CAF50', accent: '#388E3C', highlight: '#81C784' },
        { base: '#2196F3', accent: '#1976D2', highlight: '#64B5F6' },
        { base: '#9C27B0', accent: '#7B1FA2', highlight: '#BA68C8' },
        { base: '#F44336', accent: '#D32F2F', highlight: '#E57373' },
        { base: '#FF9800', accent: '#F57C00', highlight: '#FFB74D' },
        { base: '#00BCD4', accent: '#0097A7', highlight: '#4DD0E1' },
        { base: '#795548', accent: '#5D4037', highlight: '#A1887F' },
        { base: '#607D8B', accent: '#455A64', highlight: '#90A4AE' },
      ];
      this.colorIndex = 0;
    }

    _createCanvasTexture(width, height, drawFunc) {
      const canvas = document.createElement('canvas');
      canvas.width = width;
      canvas.height = height;
      const ctx = canvas.getContext('2d');
      drawFunc(ctx, width, height);
      const texture = new THREE.CanvasTexture(canvas);
      texture.wrapS = THREE.RepeatWrapping;
      texture.wrapT = THREE.RepeatWrapping;
      return texture;
    }

    _addPlasticGrain(ctx, w, h, intensity) {
      const imageData = ctx.getImageData(0, 0, w, h);
      const data = imageData.data;
      for (let i = 0; i < data.length; i += 4) {
        const noise = (Math.random() - 0.5) * intensity;
        data[i] = Math.max(0, Math.min(255, data[i] + noise));
        data[i + 1] = Math.max(0, Math.min(255, data[i + 1] + noise));
        data[i + 2] = Math.max(0, Math.min(255, data[i + 2] + noise));
      }
      ctx.putImageData(imageData, 0, 0);
    }

    _addSurfaceImperfections(ctx, w, h) {
      ctx.strokeStyle = 'rgba(255, 255, 255, 0.02)';
      ctx.lineWidth = 0.5;
      for (let i = 0; i < 15; i++) {
        const x1 = Math.random() * w;
        const y1 = Math.random() * h;
        const len = Math.random() * 30 + 10;
        const angle = Math.random() * Math.PI;
        ctx.beginPath();
        ctx.moveTo(x1, y1);
        ctx.lineTo(x1 + Math.cos(angle) * len, y1 + Math.sin(angle) * len);
        ctx.stroke();
      }
      ctx.fillStyle = 'rgba(0, 0, 0, 0.015)';
      for (let i = 0; i < 30; i++) {
        ctx.beginPath();
        ctx.arc(Math.random() * w, Math.random() * h, Math.random() * 1.5 + 0.5, 0, Math.PI * 2);
        ctx.fill();
      }
    }

    createPlatformTexture(type = 'cube') {
      const colors = this.platformColors[this.colorIndex % this.platformColors.length];
      this.colorIndex++;

      return this._createCanvasTexture(512, 512, (ctx, w, h) => {
        // Solid matte base
        ctx.fillStyle = colors.base;
        ctx.fillRect(0, 0, w, h);

        // Subtle gradient (very subtle for matte)
        const baseGrad = ctx.createLinearGradient(0, 0, w * 0.7, h * 0.7);
        baseGrad.addColorStop(0, colors.highlight + '20');
        baseGrad.addColorStop(0.5, 'transparent');
        baseGrad.addColorStop(1, colors.accent + '30');
        ctx.fillStyle = baseGrad;
        ctx.fillRect(0, 0, w, h);

        // Soft top highlight
        const topHL = ctx.createLinearGradient(0, 0, 0, h * 0.25);
        topHL.addColorStop(0, 'rgba(255, 255, 255, 0.15)');
        topHL.addColorStop(1, 'rgba(255, 255, 255, 0)');
        ctx.fillStyle = topHL;
        ctx.fillRect(0, 0, w, h * 0.25);

        // Left edge highlight
        const leftHL = ctx.createLinearGradient(0, 0, w * 0.15, 0);
        leftHL.addColorStop(0, 'rgba(255, 255, 255, 0.08)');
        leftHL.addColorStop(1, 'rgba(255, 255, 255, 0)');
        ctx.fillStyle = leftHL;
        ctx.fillRect(0, 0, w * 0.15, h);

        // Bottom shadow
        const bottomSH = ctx.createLinearGradient(0, h * 0.8, 0, h);
        bottomSH.addColorStop(0, 'rgba(0, 0, 0, 0)');
        bottomSH.addColorStop(1, 'rgba(0, 0, 0, 0.15)');
        ctx.fillStyle = bottomSH;
        ctx.fillRect(0, h * 0.8, w, h * 0.2);

        // Right shadow
        const rightSH = ctx.createLinearGradient(w * 0.85, 0, w, 0);
        rightSH.addColorStop(0, 'rgba(0, 0, 0, 0)');
        rightSH.addColorStop(1, 'rgba(0, 0, 0, 0.1)');
        ctx.fillStyle = rightSH;
        ctx.fillRect(w * 0.85, 0, w * 0.15, h);

        // Subtle embossed grid
        ctx.strokeStyle = 'rgba(0, 0, 0, 0.03)';
        ctx.lineWidth = 1;
        for (let x = 64; x < w; x += 64) {
          ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, h); ctx.stroke();
        }
        for (let y = 64; y < h; y += 64) {
          ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(w, y); ctx.stroke();
        }
        ctx.strokeStyle = 'rgba(255, 255, 255, 0.02)';
        for (let x = 63; x < w; x += 64) {
          ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, h); ctx.stroke();
        }
        for (let y = 63; y < h; y += 64) {
          ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(w, y); ctx.stroke();
        }

        this._addPlasticGrain(ctx, w, h, 6);
        this._addSurfaceImperfections(ctx, w, h);
      });
    }

    createJumperTexture() {
      return this._createCanvasTexture(512, 1024, (ctx, w, h) => {
        // Matte golden plastic
        ctx.fillStyle = '#FFC107';
        ctx.fillRect(0, 0, w, h);

        // Cylindrical shading
        const cylGrad = ctx.createLinearGradient(0, 0, w, 0);
        cylGrad.addColorStop(0, '#FF8F0060');
        cylGrad.addColorStop(0.2, 'transparent');
        cylGrad.addColorStop(0.35, '#FFE08230');
        cylGrad.addColorStop(0.5, '#FFE08220');
        cylGrad.addColorStop(0.65, 'transparent');
        cylGrad.addColorStop(1, '#FF8F0050');
        ctx.fillStyle = cylGrad;
        ctx.fillRect(0, 0, w, h);

        // Horizontal bands
        for (let y = 0; y < h; y += 200) {
          ctx.fillStyle = 'rgba(0, 0, 0, 0.08)';
          ctx.fillRect(0, y, w, 6);
          ctx.fillStyle = 'rgba(255, 255, 255, 0.1)';
          ctx.fillRect(0, y + 6, w, 2);
        }

        // Top dome highlight
        const topHL = ctx.createLinearGradient(0, 0, 0, h * 0.2);
        topHL.addColorStop(0, 'rgba(255, 255, 255, 0.25)');
        topHL.addColorStop(0.5, 'rgba(255, 255, 255, 0.1)');
        topHL.addColorStop(1, 'rgba(255, 255, 255, 0)');
        ctx.fillStyle = topHL;
        ctx.fillRect(0, 0, w, h * 0.2);

        // Bottom shadow
        const bottomSH = ctx.createLinearGradient(0, h * 0.85, 0, h);
        bottomSH.addColorStop(0, 'rgba(0, 0, 0, 0)');
        bottomSH.addColorStop(1, 'rgba(0, 0, 0, 0.2)');
        ctx.fillStyle = bottomSH;
        ctx.fillRect(0, h * 0.85, w, h * 0.15);

        this._addPlasticGrain(ctx, w, h, 5);
        this._addSurfaceImperfections(ctx, w, h);
      });
    }

    createBumpMap() {
      return this._createCanvasTexture(256, 256, (ctx, w, h) => {
        ctx.fillStyle = '#808080';
        ctx.fillRect(0, 0, w, h);
        for (let i = 0; i < 100; i++) {
          const x = Math.random() * w;
          const y = Math.random() * h;
          const r = Math.random() * 10 + 5;
          const grad = ctx.createRadialGradient(x, y, 0, x, y, r);
          grad.addColorStop(0, '#c0c0c0');
          grad.addColorStop(1, '#808080');
          ctx.fillStyle = grad;
          ctx.beginPath();
          ctx.arc(x, y, r, 0, Math.PI * 2);
          ctx.fill();
        }
      });
    }

    createPlatformMaterial(type = 'cube') {
      const texture = this.createPlatformTexture(type);
      return new THREE.MeshLambertMaterial({
        map: texture
      });
    }

    createJumperMaterial() {
      const texture = this.createJumperTexture();
      return new THREE.MeshLambertMaterial({
        map: texture
      });
    }

    reset() { this.colorIndex = 0; }
    dispose() {
      this.textureCache.forEach(texture => texture.dispose());
      this.textureCache.clear();
    }
  }

  // ============================================
  // Systems: ShadowSystem
  // ============================================
  class ShadowSystem {
    constructor(sceneManager) {
      this.sceneManager = sceneManager;
      this.enabled = false;
      this.quality = 'high';
      this.lights = [];
    }

    enable(quality = 'high') {
      this.quality = quality;
      this.enabled = true;
      
      const renderer = this.sceneManager.renderer;
      renderer.shadowMap.enabled = true;
      renderer.shadowMap.type = THREE.PCFSoftShadowMap;
      
      this._setupLighting();
    }

    _setupLighting() {
      const scene = this.sceneManager.scene;
      
      if (this.sceneManager.lights.directional) {
        scene.remove(this.sceneManager.lights.directional);
      }
      if (this.sceneManager.lights.ambient) {
        scene.remove(this.sceneManager.lights.ambient);
      }

      const shadowMapSize = { low: 512, medium: 1024, high: 2048 }[this.quality];
      
      // Main directional light with soft shadows
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

      // Fill light (no shadow for performance)
      const fillLight = new THREE.DirectionalLight(0x87ceeb, 0.3);
      fillLight.position.set(-20, 30, -20);
      scene.add(fillLight);
      this.lights.push(fillLight);
      this.sceneManager.lights.fill = fillLight;

      // Hemisphere light for natural ambient
      const hemiLight = new THREE.HemisphereLight(0x87ceeb, 0x8b7355, 0.4);
      hemiLight.position.set(0, 50, 0);
      scene.add(hemiLight);
      this.lights.push(hemiLight);

      // Ambient light
      const ambientLight = new THREE.AmbientLight(0xffffff, 0.3);
      scene.add(ambientLight);
      this.lights.push(ambientLight);

      // Warm accent light
      const warmLight = new THREE.PointLight(0xffaa44, 0.3, 100);
      warmLight.position.set(10, 20, 10);
      scene.add(warmLight);
      this.lights.push(warmLight);
    }

    configureMesh(mesh, cast = true, receive = true) {
      if (!this.enabled) return;
      mesh.castShadow = cast;
      mesh.receiveShadow = receive;
    }

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

    dispose() {
      this.lights.forEach(light => this.sceneManager.scene.remove(light));
      this.lights = [];
    }
  }

  // ============================================
  // Systems: BackgroundSystem
  // ============================================
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

    _createGradientSky() {
      const canvas = document.createElement('canvas');
      canvas.width = 512;
      canvas.height = 512;
      const ctx = canvas.getContext('2d');

      const gradient = ctx.createLinearGradient(0, 0, 0, canvas.height);
      gradient.addColorStop(0, '#1a237e');
      gradient.addColorStop(0.3, '#3949ab');
      gradient.addColorStop(0.6, '#7986cb');
      gradient.addColorStop(0.8, '#c5cae9');
      gradient.addColorStop(1, '#e8eaf6');
      ctx.fillStyle = gradient;
      ctx.fillRect(0, 0, canvas.width, canvas.height);

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
      const geometry = new THREE.SphereGeometry(500, 32, 32);
      const material = new THREE.MeshBasicMaterial({
        map: texture,
        side: THREE.BackSide,
        fog: false
      });
      this.skyMesh = new THREE.Mesh(geometry, material);
      this.sceneManager.add(this.skyMesh);
    }

    _createGroundPlane() {
      const canvas = document.createElement('canvas');
      canvas.width = 512;
      canvas.height = 512;
      const ctx = canvas.getContext('2d');

      const gradient = ctx.createRadialGradient(256, 256, 0, 256, 256, 360);
      gradient.addColorStop(0, '#e0e0e0');
      gradient.addColorStop(0.5, '#bdbdbd');
      gradient.addColorStop(1, '#9e9e9e');
      ctx.fillStyle = gradient;
      ctx.fillRect(0, 0, canvas.width, canvas.height);

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

    _createParticles() {
      const geometry = new THREE.BufferGeometry();
      const positions = new Float32Array(this.particleCount * 3);
      const colors = new Float32Array(this.particleCount * 3);

      for (let i = 0; i < this.particleCount; i++) {
        positions[i * 3] = (Math.random() - 0.5) * 200;
        positions[i * 3 + 1] = Math.random() * 50 + 5;
        positions[i * 3 + 2] = (Math.random() - 0.5) * 200;

        colors[i * 3] = 0.8 + Math.random() * 0.2;
        colors[i * 3 + 1] = 0.8 + Math.random() * 0.2;
        colors[i * 3 + 2] = 0.9 + Math.random() * 0.1;

        this.particleVelocities.push({
          x: (Math.random() - 0.5) * 0.02,
          y: (Math.random() - 0.5) * 0.01,
          z: (Math.random() - 0.5) * 0.02
        });
      }

      geometry.addAttribute('position', new THREE.BufferAttribute(positions, 3));
      geometry.addAttribute('color', new THREE.BufferAttribute(colors, 3));

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

    _startAnimation() {
      this.isAnimating = true;
      this._animate();
    }

    _animate() {
      if (!this.isAnimating) return;
      this.time += 0.01;

      if (this.particles) {
        const positions = this.particles.geometry.attributes.position.array;
        for (let i = 0; i < this.particleCount; i++) {
          const vel = this.particleVelocities[i];
          positions[i * 3] += vel.x;
          positions[i * 3 + 1] += vel.y + Math.sin(this.time + i) * 0.005;
          positions[i * 3 + 2] += vel.z;

          if (positions[i * 3] > 100) positions[i * 3] = -100;
          if (positions[i * 3] < -100) positions[i * 3] = 100;
          if (positions[i * 3 + 2] > 100) positions[i * 3 + 2] = -100;
          if (positions[i * 3 + 2] < -100) positions[i * 3 + 2] = 100;
          if (positions[i * 3 + 1] > 55) positions[i * 3 + 1] = 5;
          if (positions[i * 3 + 1] < 5) positions[i * 3 + 1] = 55;
        }
        this.particles.geometry.attributes.position.needsUpdate = true;
      }

      if (this.skyMesh) {
        this.skyMesh.rotation.y += 0.0001;
      }

      requestAnimationFrame(() => this._animate());
    }

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


  // ============================================
  // Entities: Platform (with textures)
  // ============================================
  class Platform {
    static textureSystem = null;

    constructor(type = 'cube', size = null) {
      this.type = type;
      this.size = size || this._generateRandomSize();
      this.mesh = this._createMesh();
      this.mesh.castShadow = true;
      this.mesh.receiveShadow = true;
    }

    static setTextureSystem(system) {
      Platform.textureSystem = system;
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
        return { radius: minRadius + Math.random() * (maxRadius - minRadius) };
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
      if (Platform.textureSystem) {
        return Platform.textureSystem.createPlatformMaterial(this.type);
      }
      const color = this.type === 'cube' 
        ? GameConfig.platform.cube.color 
        : GameConfig.platform.cylinder.color;
      return new THREE.MeshLambertMaterial({ color });
    }

    getHalfSize() {
      if (this.type === 'cube') {
        return { x: this.size.width / 2, z: this.size.depth / 2 };
      } else {
        return { x: this.size.radius, z: this.size.radius };
      }
    }

    setPosition(x, y, z) { this.mesh.position.set(x, y, z); }
    get position() { return this.mesh.position; }

    dispose() {
      if (this.mesh) {
        this.mesh.geometry.dispose();
        this.mesh.material.dispose();
        if (this.mesh.material.map) this.mesh.material.map.dispose();
        if (this.mesh.material.bumpMap) this.mesh.material.bumpMap.dispose();
      }
    }
  }

  // ============================================
  // Entities: Jumper (with textures)
  // ============================================
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
      geometry.translate(0, height / 2, 0);
      mesh.position.set(0, height / 2, 0);
      return mesh;
    }

    _createMaterial() {
      if (Jumper.textureSystem) {
        return Jumper.textureSystem.createJumperMaterial();
      }
      const { color } = GameConfig.jumper;
      return new THREE.MeshLambertMaterial({ color });
    }

    get position() { return this.mesh.position; }
    get rotation() { return this.mesh.rotation; }
    get scale() { return this.mesh.scale; }
    get bottomRadius() { return this.config.bottomRadius; }
    get height() { return this.config.height; }

    compress() {
      if (this.mesh.scale.y > this.physicsConfig.minScale) {
        this.mesh.scale.y -= this.physicsConfig.scaleDecrement;
        return true;
      }
      return false;
    }

    restore() {
      if (this.mesh.scale.y < 1) {
        this.mesh.scale.y += this.physicsConfig.scaleIncrement;
        return false;
      }
      this.mesh.scale.y = 1;
      return true;
    }

    reset() {
      this.mesh.scale.y = 1;
      this.mesh.rotation.set(0, 0, 0);
    }

    setPosition(x, y, z) { this.mesh.position.set(x, y, z); }

    dispose() {
      if (this.mesh) {
        this.mesh.geometry.dispose();
        this.mesh.material.dispose();
        if (this.mesh.material.map) this.mesh.material.map.dispose();
      }
    }
  }

  // ============================================
  // Core: SceneManager
  // ============================================
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

    _initScene() { this.scene = new THREE.Scene(); }

    _initCamera() {
      const { frustumSize, near, far, position } = GameConfig.camera;
      this.camera = new THREE.OrthographicCamera(
        window.innerWidth / -frustumSize,
        window.innerWidth / frustumSize,
        window.innerHeight / frustumSize,
        window.innerHeight / -frustumSize,
        near, far
      );
      this.camera.position.set(position.x, position.y, position.z);
    }

    _initRenderer() {
      this.renderer = new THREE.WebGLRenderer({ antialias: true });
      this.renderer.setSize(window.innerWidth, window.innerHeight);
      this.renderer.setPixelRatio(window.devicePixelRatio);
      document.body.appendChild(this.renderer.domElement);
    }

    _initLights() {
      const { directional, ambient } = GameConfig.lighting;
      this.lights.directional = new THREE.DirectionalLight(directional.color, directional.intensity);
      this.lights.directional.position.set(directional.position.x, directional.position.y, directional.position.z);
      this.scene.add(this.lights.directional);
      this.lights.ambient = new THREE.AmbientLight(ambient.color, ambient.intensity);
      this.scene.add(this.lights.ambient);
    }

    get canvas() { return this.renderer.domElement; }
    add(object) { this.scene.add(object); }
    remove(object) { this.scene.remove(object); }
    render() { this.renderer.render(this.scene, this.camera); }
    lookAt(target) { this.camera.lookAt(target); }

    onResize() {
      const { frustumSize } = GameConfig.camera;
      this.camera.left = window.innerWidth / -frustumSize;
      this.camera.right = window.innerWidth / frustumSize;
      this.camera.top = window.innerHeight / frustumSize;
      this.camera.bottom = window.innerHeight / -frustumSize;
      this.camera.updateProjectionMatrix();
      this.renderer.setSize(window.innerWidth, window.innerHeight);
    }

    dispose() { this.renderer.dispose(); }
  }

  // ============================================
  // Core: EventManager
  // ============================================
  class EventManager {
    constructor(canvas) {
      this.canvas = canvas;
      this.inputEvents = Helpers.getInputEvents();
      this.listeners = new Map();
      this.boundHandlers = new Map();
    }

    on(eventType, callback) {
      if (!this.listeners.has(eventType)) this.listeners.set(eventType, []);
      this.listeners.get(eventType).push(callback);
    }

    emit(eventType, event) {
      if (this.listeners.has(eventType)) {
        this.listeners.get(eventType).forEach(callback => callback(event));
      }
    }

    startListening() {
      const downHandler = (e) => this.emit('down', e);
      this.boundHandlers.set('down', downHandler);
      this.canvas.addEventListener(this.inputEvents.down, downHandler);

      const upHandler = (e) => this.emit('up', e);
      this.boundHandlers.set('up', upHandler);
      this.canvas.addEventListener(this.inputEvents.up, upHandler);

      const resizeHandler = () => this.emit('resize');
      this.boundHandlers.set('resize', resizeHandler);
      window.addEventListener('resize', resizeHandler, false);
    }

    stopListening() {
      if (this.boundHandlers.has('down')) {
        this.canvas.removeEventListener(this.inputEvents.down, this.boundHandlers.get('down'));
      }
      if (this.boundHandlers.has('up')) {
        this.canvas.removeEventListener(this.inputEvents.up, this.boundHandlers.get('up'));
      }
      if (this.boundHandlers.has('resize')) {
        window.removeEventListener('resize', this.boundHandlers.get('resize'));
      }
      this.boundHandlers.clear();
    }

    dispose() {
      this.stopListening();
      this.listeners.clear();
    }
  }

  // ============================================
  // Managers: PlatformManager (with smart direction)
  // ============================================
  class PlatformManager {
    constructor(sceneManager) {
      this.sceneManager = sceneManager;
      this.platforms = [];
      this.config = GameConfig.platform;
      this.lastJumperPosition = null;
      this.lastDirection = null;
    }

    setJumperLandingPosition(position) {
      this.lastJumperPosition = { x: position.x, z: position.z };
    }

    _determineDirection() {
      if (this.platforms.length <= 1) {
        this.lastDirection = Helpers.getRandomBoolean() ? 'xDir' : 'zDir';
        return this.lastDirection;
      }
      const currentDir = this.getDirection();
      // 70% continue same direction, 30% change
      if (Math.random() < 0.7) {
        this.lastDirection = currentDir === 'x' ? 'xDir' : 'zDir';
      } else {
        this.lastDirection = currentDir === 'x' ? 'zDir' : 'xDir';
      }
      return this.lastDirection;
    }

    createPlatform(options = {}) {
      const type = options.type || (Helpers.getRandomBoolean() ? 'cube' : 'cylinder');
      const direction = options.direction || this._determineDirection();
      const platform = new Platform(type);

      if (this.platforms.length === 0) {
        platform.setPosition(0, 0, 0);
      } else {
        const position = this._calculateNextPosition(platform, direction, options);
        platform.setPosition(position.x, position.y, position.z);
      }

      Helpers.validatePosition(platform.position);
      this.platforms.push(platform);
      this.sceneManager.add(platform.mesh);
      this._pruneOldPlatforms();
      return platform;
    }

    _getMaxJumpableDistance() {
      const physics = GameConfig.physics;
      const maxTime = 2 * physics.maxYSpeed / physics.gravity;
      const maxDistance = physics.maxXSpeed * maxTime;
      return maxDistance * 0.8;
    }

    _calculateNextPosition(newPlatform, direction, options = {}) {
      const lastPlatform = this.platforms[this.platforms.length - 1];
      const lastPlatformPos = lastPlatform.position;
      const lastSize = lastPlatform.getHalfSize();
      const newSize = newPlatform.getHalfSize();

      // Use jumper's actual landing position if available
      const jumpFromPos = this.lastJumperPosition || { x: lastPlatformPos.x, z: lastPlatformPos.z };

      const maxJumpable = this._getMaxJumpableDistance();
      const maxGap = Math.min(this.config.maxDistance, maxJumpable);
      const distance = options.distance || Helpers.getRandomValue(this.config.minDistance, maxGap);

      let x, y, z;
      y = lastPlatformPos.y;

      if (direction === 'zDir') {
        x = jumpFromPos.x;
        z = jumpFromPos.z - distance - newSize.z;
        const minZ = lastPlatformPos.z - lastSize.z - this.config.minDistance - newSize.z;
        z = Math.min(z, minZ);
      } else {
        z = jumpFromPos.z;
        x = jumpFromPos.x + distance + newSize.x;
        const minX = lastPlatformPos.x + lastSize.x + this.config.minDistance + newSize.x;
        x = Math.max(x, minX);
      }

      return { x, y, z };
    }

    _pruneOldPlatforms() {
      while (this.platforms.length > this.config.maxCount) {
        const oldPlatform = this.platforms.shift();
        this.sceneManager.remove(oldPlatform.mesh);
        oldPlatform.dispose();
      }
    }

    getCurrentPlatform() {
      if (this.platforms.length < 2) return null;
      return this.platforms[this.platforms.length - 2];
    }

    getTargetPlatform() {
      if (this.platforms.length < 1) return null;
      return this.platforms[this.platforms.length - 1];
    }

    getDirection() {
      if (this.platforms.length < 2) return null;
      const from = this.getCurrentPlatform();
      const to = this.getTargetPlatform();
      // Calculate actual displacement to determine jump direction
      const dx = Math.abs(to.position.x - from.position.x);
      const dz = Math.abs(to.position.z - from.position.z);
      // Direction is determined by which axis has greater displacement
      return dx > dz ? 'x' : 'z';
    }

    get count() { return this.platforms.length; }

    clear() {
      this.platforms.forEach(platform => {
        this.sceneManager.remove(platform.mesh);
        platform.dispose();
      });
      this.platforms = [];
      this.lastJumperPosition = null;
      this.lastDirection = null;
    }

    dispose() { this.clear(); }
  }


  // ============================================
  // Managers: CameraController (Smooth Easing)
  // ============================================
  class CameraController {
    constructor(sceneManager) {
      this.sceneManager = sceneManager;
      this.config = GameConfig.camera;
      this.currentPosition = new THREE.Vector3(0, 0, 0);
      this.targetPosition = new THREE.Vector3(0, 0, 0);
      this.startPosition = new THREE.Vector3(0, 0, 0);
      this.isAnimating = false;
      this.animationProgress = 0;
      this.animationDuration = 60;
    }

    _easeOutCubic(t) {
      return 1 - Math.pow(1 - t, 3);
    }

    _lerp(start, end, factor) {
      return start + (end - start) * factor;
    }

    updateTarget(fromPlatform, toPlatform) {
      const fromPos = fromPlatform.position;
      const toPos = toPlatform.position;
      this.startPosition.copy(this.currentPosition);
      this.targetPosition.set((fromPos.x + toPos.x) / 2, 0, (fromPos.z + toPos.z) / 2);
      this.animationProgress = 0;
      this._animateToTarget();
    }

    _animateToTarget() {
      if (this.isAnimating) return;
      this.isAnimating = true;
      this._animate();
    }

    _animate() {
      const current = this.currentPosition;
      const target = this.targetPosition;
      const start = this.startPosition;

      this.animationProgress += 1 / this.animationDuration;

      if (this.animationProgress >= 1) {
        this.animationProgress = 1;
        current.x = target.x;
        current.z = target.z;
        this.sceneManager.lookAt(new THREE.Vector3(current.x, 0, current.z));
        this.sceneManager.render();
        this.isAnimating = false;
        return;
      }

      const easedProgress = this._easeOutCubic(this.animationProgress);
      current.x = this._lerp(start.x, target.x, easedProgress);
      current.z = this._lerp(start.z, target.z, easedProgress);

      this.sceneManager.lookAt(new THREE.Vector3(current.x, 0, current.z));
      this.sceneManager.render();

      requestAnimationFrame(() => this._animate());
    }

    lookAtCurrent() { this.sceneManager.lookAt(this.currentPosition); }

    reset() {
      this.currentPosition.set(0, 0, 0);
      this.targetPosition.set(0, 0, 0);
      this.startPosition.set(0, 0, 0);
      this.isAnimating = false;
      this.animationProgress = 0;
      this.lookAtCurrent();
    }
  }

  // ============================================
  // Managers: PhysicsManager (Improved Landing Detection)
  // ============================================
  class PhysicsManager {
    constructor() {
      this.config = GameConfig.physics;
      this.jumperConfig = GameConfig.jumper;
      this.xSpeed = 0;
      this.ySpeed = 0;
    }

    charge() {
      if (this.xSpeed < this.config.maxXSpeed) {
        this.xSpeed += this.config.xAcceleration;
      }
      if (this.ySpeed < this.config.maxYSpeed) {
        this.ySpeed += this.config.yAcceleration;
      }
    }

    applyJumpPhysics(jumper, direction) {
      const groundLevel = this.jumperConfig.height / 2;

      if (jumper.position.y >= groundLevel) {
        if (direction === 'x') {
          jumper.position.x += this.xSpeed;
        } else {
          jumper.position.z -= this.xSpeed;
        }
        jumper.position.y += this.ySpeed;
        this.ySpeed -= this.config.gravity;
        jumper.restore();
        return true;
      }
      return false;
    }

    checkLanding(jumper, currentPlatform, targetPlatform, direction) {
      const jumperPos = jumper.position;
      const jumpR = this.jumperConfig.bottomRadius;
      const centerThreshold = GameConfig.scoring.centerThreshold;

      const onCurrent = this._isOnPlatform(jumperPos, currentPlatform, jumpR);
      if (onCurrent.fully) return 1;

      const onTarget = this._isOnPlatform(jumperPos, targetPlatform, jumpR);
      if (onTarget.fully) {
        const distToCenter = this._getDistanceToCenter(jumperPos, targetPlatform);
        if (distToCenter <= centerThreshold) return 3;
        return 2;
      }

      if (onCurrent.partial) return -1;
      if (onTarget.partial) return -3;

      return -2;
    }

    _isOnPlatform(jumperPos, platform, jumperRadius) {
      const platPos = platform.position;
      const halfSize = platform.getHalfSize();

      // For cylinder platforms, use circular bounds check
      if (platform.type === 'cylinder') {
        const dx = jumperPos.x - platPos.x;
        const dz = jumperPos.z - platPos.z;
        const distFromCenter = Math.sqrt(dx * dx + dz * dz);
        const platformRadius = halfSize.x;

        // More generous - if center is on platform, count as fully on
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

      return { fully: false, partial: partiallyOn };
    }

    _getDistanceToCenter(jumperPos, platform) {
      const platPos = platform.position;
      const dx = jumperPos.x - platPos.x;
      const dz = jumperPos.z - platPos.z;
      return Math.sqrt(dx * dx + dz * dz);
    }

    reset() {
      this.xSpeed = 0;
      this.ySpeed = 0;
    }

    getSpeed() {
      return { x: this.xSpeed, y: this.ySpeed };
    }
  }

  // ============================================
  // Managers: ScoreManager
  // ============================================
  class ScoreManager {
    constructor() {
      this.score = 0;
      this.centerCombo = 0;
      this.scoreElement = null;
      this.bonusElement = null;
      this.config = GameConfig.scoring;
      this._initUI();
    }

    _initUI() {
      this.scoreElement = document.createElement('div');
      this.scoreElement.id = 'score';
      this.scoreElement.innerHTML = '0';
      document.body.appendChild(this.scoreElement);

      this.bonusElement = document.createElement('div');
      this.bonusElement.id = 'bonus-popup';
      this.bonusElement.style.cssText = `
        position: fixed;
        top: 50%;
        left: 50%;
        transform: translate(-50%, -50%);
        font-size: 48px;
        font-weight: bold;
        color: #FFD700;
        text-shadow: 2px 2px 4px rgba(0,0,0,0.5);
        opacity: 0;
        pointer-events: none;
        z-index: 1000;
        transition: all 0.3s ease-out;
      `;
      document.body.appendChild(this.bonusElement);
    }

    addScore(landingType = 'normal') {
      let points = 0;

      if (landingType === 'center') {
        this.centerCombo++;
        points = this.config.centerScore * this.centerCombo;
        this._showCenterAnimation(points);
      } else {
        this.centerCombo = 0;
        points = this.config.baseScore;
      }

      this.score += points;
      this._updateUI();
      return points;
    }

    _showCenterAnimation(points) {
      if (!this.bonusElement) return;

      let text = `+${points}`;
      if (this.centerCombo > 1) {
        text += ` x${this.centerCombo} COMBO!`;
      } else {
        text += ' CENTER!';
      }

      this.bonusElement.innerHTML = text;
      this.bonusElement.style.opacity = '1';
      this.bonusElement.style.transform = 'translate(-50%, -50%) scale(1.2)';

      setTimeout(() => {
        this.bonusElement.style.opacity = '0';
        this.bonusElement.style.transform = 'translate(-50%, -80%) scale(0.8)';
      }, 800);
    }

    getScore() { return this.score; }
    getCombo() { return this.centerCombo; }

    reset() {
      this.score = 0;
      this.centerCombo = 0;
      this._updateUI();
    }

    _updateUI() {
      if (this.scoreElement) this.scoreElement.innerHTML = this.score;
    }

    dispose() {
      if (this.scoreElement && this.scoreElement.parentNode) {
        this.scoreElement.parentNode.removeChild(this.scoreElement);
      }
      if (this.bonusElement && this.bonusElement.parentNode) {
        this.bonusElement.parentNode.removeChild(this.bonusElement);
      }
    }
  }

  // ============================================
  // Managers: PowerBarManager
  // ============================================
  class PowerBarManager {
    constructor() {
      this.config = GameConfig.physics;
      this.container = null;
      this.bar = null;
      this.isVisible = false;
      this._initUI();
    }

    _initUI() {
      this.container = document.createElement('div');
      this.container.id = 'power-bar-container';
      this.container.style.cssText = `
        position: fixed;
        bottom: 80px;
        left: 50%;
        transform: translateX(-50%);
        width: 200px;
        height: 20px;
        background: rgba(0, 0, 0, 0.5);
        border-radius: 10px;
        border: 2px solid #fff;
        overflow: hidden;
        opacity: 0;
        transition: opacity 0.2s ease;
        z-index: 100;
      `;

      this.bar = document.createElement('div');
      this.bar.id = 'power-bar';
      this.bar.style.cssText = `
        width: 0%;
        height: 100%;
        background: linear-gradient(90deg, #4CAF50, #FFEB3B, #FF5722);
        border-radius: 8px;
        transition: width 0.05s linear;
      `;

      this.container.appendChild(this.bar);
      document.body.appendChild(this.container);
    }

    show() {
      if (!this.isVisible) {
        this.isVisible = true;
        this.container.style.opacity = '1';
        this.bar.style.width = '0%';
      }
    }

    hide() {
      if (this.isVisible) {
        this.isVisible = false;
        this.container.style.opacity = '0';
        setTimeout(() => { this.bar.style.width = '0%'; }, 200);
      }
    }

    update(currentSpeed) {
      const maxSpeed = this.config.maxXSpeed;
      const percentage = Math.min((currentSpeed / maxSpeed) * 100, 100);
      this.bar.style.width = `${percentage}%`;

      if (percentage < 33) {
        this.bar.style.background = 'linear-gradient(90deg, #4CAF50, #4CAF50)';
      } else if (percentage < 66) {
        this.bar.style.background = 'linear-gradient(90deg, #4CAF50, #FFEB3B)';
      } else {
        this.bar.style.background = 'linear-gradient(90deg, #4CAF50, #FFEB3B, #FF5722)';
      }
    }

    dispose() {
      if (this.container && this.container.parentNode) {
        this.container.parentNode.removeChild(this.container);
      }
    }
  }


  // ============================================
  // Core: Game (Main Orchestrator with Enhanced Rendering)
  // ============================================
  class Game {
    constructor() {
      // Core systems
      this.sceneManager = new SceneManager();
      this.eventManager = new EventManager(this.sceneManager.canvas);
      
      // Rendering systems
      this.textureSystem = new TextureSystem();
      this.shadowSystem = new ShadowSystem(this.sceneManager);
      this.backgroundSystem = new BackgroundSystem(this.sceneManager);
      
      // Setup texture system for entities
      Platform.setTextureSystem(this.textureSystem);
      Jumper.setTextureSystem(this.textureSystem);
      
      // Enable soft shadows
      this.shadowSystem.enable('high');
      
      // Managers
      this.platformManager = new PlatformManager(this.sceneManager);
      this.cameraController = new CameraController(this.sceneManager);
      this.physicsManager = new PhysicsManager();
      this.scoreManager = new ScoreManager();
      this.powerBarManager = new PowerBarManager();
      
      this.jumper = null;

      this.state = {
        mouseDown: false,
        isJumping: false,
        isGameOver: false
      };

      this.failCallback = null;
      this._bindEvents();
    }

    _bindEvents() {
      this.eventManager.on('down', () => this._onInputDown());
      this.eventManager.on('up', () => this._onInputUp());
      this.eventManager.on('resize', () => this._onResize());
    }

    start() {
      this.platformManager.createPlatform();
      this.platformManager.createPlatform();
      this._createJumper();
      this._initializeCamera();
      this.eventManager.startListening();
      this.sceneManager.render();
    }

    _initializeCamera() {
      const currentPlatform = this.platformManager.getCurrentPlatform();
      const targetPlatform = this.platformManager.getTargetPlatform();

      if (currentPlatform && targetPlatform) {
        const centerX = (currentPlatform.position.x + targetPlatform.position.x) / 2;
        const centerZ = (currentPlatform.position.z + targetPlatform.position.z) / 2;
        this.sceneManager.lookAt(new THREE.Vector3(centerX, 0, centerZ));
      } else {
        this.sceneManager.lookAt(new THREE.Vector3(0, 0, 0));
      }
    }

    restart() {
      this.platformManager.clear();
      if (this.jumper) {
        this.sceneManager.remove(this.jumper.mesh);
        this.jumper.dispose();
      }

      this.state.mouseDown = false;
      this.state.isJumping = false;
      this.state.isGameOver = false;

      this.physicsManager.reset();
      this.scoreManager.reset();
      this.cameraController.reset();
      this.textureSystem.reset();
      this.backgroundSystem.updatePosition(0, 0);

      this.platformManager.createPlatform();
      this.platformManager.createPlatform();
      this._createJumper();
      this._initializeCamera();
      this.sceneManager.render();
    }

    _createJumper() {
      this.jumper = new Jumper();
      this.sceneManager.add(this.jumper.mesh);
    }

    _onInputDown() {
      if (this.state.isGameOver || this.state.isJumping) return;
      this.state.mouseDown = true;
      this.powerBarManager.show();
      this._chargeJump();
    }

    _chargeJump() {
      if (!this.state.mouseDown) return;
      if (this.jumper.compress()) {
        this.physicsManager.charge();
        const speed = this.physicsManager.getSpeed();
        this.powerBarManager.update(speed.x);
        this.sceneManager.render();
        requestAnimationFrame(() => this._chargeJump());
      }
    }

    _onInputUp() {
      if (this.state.isGameOver) return;
      this.state.mouseDown = false;
      this.powerBarManager.hide();
      this.state.isJumping = true;
      this._executeJump();
    }

    _executeJump() {
      const direction = this.platformManager.getDirection();
      if (this.physicsManager.applyJumpPhysics(this.jumper, direction)) {
        this.sceneManager.render();
        requestAnimationFrame(() => this._executeJump());
      } else {
        this._handleLanding();
      }
    }

    _handleLanding() {
      const currentPlatform = this.platformManager.getCurrentPlatform();
      const targetPlatform = this.platformManager.getTargetPlatform();
      const direction = this.platformManager.getDirection();

      const landingState = this.physicsManager.checkLanding(
        this.jumper, currentPlatform, targetPlatform, direction
      );

      console.log('Landing state:', landingState);

      switch (landingState) {
        case 1:
          this._resetJumperState();
          break;
        case 3:
          this.scoreManager.addScore('center');
          this._resetJumperState();
          this.platformManager.setJumperLandingPosition(this.jumper.position);
          this.platformManager.createPlatform();
          this._updateCamera();
          break;
        case 2:
          this.scoreManager.addScore('normal');
          this._resetJumperState();
          this.platformManager.setJumperLandingPosition(this.jumper.position);
          this.platformManager.createPlatform();
          this._updateCamera();
          break;
        case -2:
          // Fell off current platform - add slight horizontal movement
          const fallVel = { x: 0, z: 0 };
          const currentPlat = this.platformManager.getCurrentPlatform();
          if (currentPlat) {
            // Move away from platform center
            const dx = this.jumper.position.x - currentPlat.position.x;
            const dz = this.jumper.position.z - currentPlat.position.z;
            const dist = Math.sqrt(dx * dx + dz * dz) || 1;
            fallVel.x = (dx / dist) * 0.1;
            fallVel.z = (dz / dist) * 0.1;
          }
          this._animateFallDown(fallVel, () => this._triggerGameOver());
          break;
        case -1:
        case -3:
          this._animateFallOff(landingState);
          break;
      }
    }

    _resetJumperState() {
      this.physicsManager.reset();
      this.jumper.reset();
      this.jumper.position.y = GameConfig.jumper.height / 2;
      this.state.isJumping = false;
    }

    _updateCamera() {
      const currentPlatform = this.platformManager.getCurrentPlatform();
      const targetPlatform = this.platformManager.getTargetPlatform();

      if (currentPlatform && targetPlatform) {
        this.cameraController.updateTarget(currentPlatform, targetPlatform);
        
        const centerX = (currentPlatform.position.x + targetPlatform.position.x) / 2;
        const centerZ = (currentPlatform.position.z + targetPlatform.position.z) / 2;
        
        this.shadowSystem.updateShadowCamera(new THREE.Vector3(centerX, 0, centerZ));
        this.backgroundSystem.updatePosition(centerX, centerZ);
      }
    }

    _animateFallDown(horizontalVelocity = { x: 0, z: 0 }, onComplete = null) {
      const groundLevel = -GameConfig.jumper.height / 2;
      let velocityY = 0;
      const gravity = 0.015;
      const friction = 0.98;

      const animate = () => {
        if (this.jumper.position.y >= groundLevel) {
          // Apply gravity
          velocityY += gravity;
          this.jumper.position.y -= velocityY;
          
          // Apply horizontal movement with friction
          this.jumper.position.x += horizontalVelocity.x;
          this.jumper.position.z += horizontalVelocity.z;
          horizontalVelocity.x *= friction;
          horizontalVelocity.z *= friction;
          
          // Add tumbling rotation during fall
          this.jumper.rotation.x += 0.05;
          this.jumper.rotation.z += 0.03;
          
          this.sceneManager.render();
          requestAnimationFrame(animate);
        } else {
          // Animation complete - trigger callback
          if (onComplete) onComplete();
        }
      };

      animate();
    }

    _animateFallOff(state) {
      const direction = this.platformManager.getDirection();
      const rotateAxis = direction === 'z' ? 'x' : 'z';
      const moveAxis = direction === 'z' ? 'z' : 'x';
      const rotateDirection = state === -1 ? -1 : 1;
      const targetRotation = rotateDirection * Math.PI / 2;
      
      // Calculate horizontal velocity away from platform edge
      const currentPlatform = this.platformManager.getCurrentPlatform();
      const targetPlatform = this.platformManager.getTargetPlatform() || currentPlatform;
      
      // Determine fall direction based on landing state
      let horizontalVelocity = { x: 0, z: 0 };
      const fallSpeed = 0.15;
      
      if (state === -1) {
        // Fell short - move backward (away from target)
        horizontalVelocity[moveAxis] = direction === 'z' 
          ? -fallSpeed * (this.jumper.position.z > targetPlatform.position.z ? 1 : -1)
          : -fallSpeed * (this.jumper.position.x > targetPlatform.position.x ? 1 : -1);
      } else if (state === -3) {
        // Overshot - move forward (past target)
        horizontalVelocity[moveAxis] = direction === 'z'
          ? fallSpeed * (this.jumper.position.z > targetPlatform.position.z ? 1 : -1)
          : fallSpeed * (this.jumper.position.x > targetPlatform.position.x ? 1 : -1);
      }

      const self = this;
      const animateRotation = () => {
        const currentRotation = self.jumper.rotation[rotateAxis];
        const reachedTarget = rotateDirection === -1
          ? currentRotation <= targetRotation
          : currentRotation >= targetRotation;

        if (!reachedTarget) {
          self.jumper.rotation[rotateAxis] += rotateDirection * 0.1;
          // Move horizontally while rotating
          self.jumper.position[moveAxis] += horizontalVelocity[moveAxis] * 0.5;
          self.sceneManager.render();
          requestAnimationFrame(animateRotation);
        } else {
          self._animateFallDown(horizontalVelocity, () => self._triggerGameOver());
        }
      };

      animateRotation();
    }

    _triggerGameOver() {
      this.state.isGameOver = true;

      if (this.failCallback) {
        // Small delay to let the final frame render
        setTimeout(() => {
          this.failCallback(this.scoreManager.getScore());
        }, 300);
      }
    }

    _onResize() {
      this.sceneManager.onResize();
      this.sceneManager.render();
    }

    set onFail(callback) {
      this.failCallback = callback;
    }

    get score() {
      return this.scoreManager.getScore();
    }

    dispose() {
      this.eventManager.dispose();
      this.platformManager.dispose();
      this.scoreManager.dispose();
      this.powerBarManager.dispose();
      this.shadowSystem.dispose();
      this.textureSystem.dispose();
      this.backgroundSystem.dispose();

      if (this.jumper) {
        this.jumper.dispose();
      }

      this.sceneManager.dispose();
    }
  }

  // Expose Game class globally
  window.Game = Game;

})();
