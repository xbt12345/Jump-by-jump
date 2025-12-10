/**
 * Jump Jump Game - Bundled Version
 * This file combines all modules for non-ES6 module environments
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
      xAcceleration: 0.0008,
      yAcceleration: 0.0016,
      gravity: 0.01,
      scaleDecrement: 0.002,
      scaleIncrement: 0.02,
      minScale: 0.02,
      fallSpeed: 0.06,
      maxXSpeed: 0.25,
      maxYSpeed: 0.5
    },
    camera: {
      frustumSize: 80,
      near: 0.1,
      far: 5000,
      position: { x: 100, y: 100, z: 100 },
      moveSpeed: 0.05,
      smoothFactor: 0.08
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
  // Entities: Platform
  // ============================================
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
      }
    }
  }

  // ============================================
  // Entities: Jumper
  // ============================================
  class Jumper {
    constructor() {
      this.mesh = this._createMesh();
      this.config = GameConfig.jumper;
      this.physicsConfig = GameConfig.physics;
    }

    _createMesh() {
      const { topRadius, bottomRadius, height, color, segments } = GameConfig.jumper;
      const geometry = new THREE.CylinderGeometry(topRadius, bottomRadius, height, segments);
      const material = new THREE.MeshLambertMaterial({ color });
      const mesh = new THREE.Mesh(geometry, material);
      geometry.translate(0, height / 2, 0);
      mesh.position.set(0, height / 2, 0);
      return mesh;
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
  // Managers: PlatformManager
  // ============================================
  class PlatformManager {
    constructor(sceneManager) {
      this.sceneManager = sceneManager;
      this.platforms = [];
      this.config = GameConfig.platform;
    }

    createPlatform(options = {}) {
      const type = options.type || (Helpers.getRandomBoolean() ? 'cube' : 'cylinder');
      const direction = options.direction || (Helpers.getRandomBoolean() ? 'xDir' : 'zDir');
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
      return maxDistance * 0.85;
    }

    _calculateNextPosition(newPlatform, direction, options = {}) {
      const lastPlatform = this.platforms[this.platforms.length - 1];
      const lastPos = lastPlatform.position;
      const lastSize = lastPlatform.getHalfSize();
      const newSize = newPlatform.getHalfSize();

      const maxJumpable = this._getMaxJumpableDistance();
      const maxGap = Math.min(this.config.maxDistance, maxJumpable);
      const distance = options.distance || Helpers.getRandomValue(this.config.minDistance, maxGap);

      let x = lastPos.x, y = lastPos.y, z = lastPos.z;

      if (direction === 'zDir') {
        z = lastPos.z - distance - lastSize.z - newSize.z;
      } else {
        x = lastPos.x + distance + lastSize.x + newSize.x;
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
      if (from.position.z === to.position.z) return 'x';
      if (from.position.x === to.position.x) return 'z';
      return null;
    }

    get count() { return this.platforms.length; }

    clear() {
      this.platforms.forEach(platform => {
        this.sceneManager.remove(platform.mesh);
        platform.dispose();
      });
      this.platforms = [];
    }

    dispose() { this.clear(); }
  }

  // ============================================
  // Managers: CameraController (Smooth Lerp)
  // ============================================
  class CameraController {
    constructor(sceneManager) {
      this.sceneManager = sceneManager;
      this.config = GameConfig.camera;
      this.currentPosition = new THREE.Vector3(0, 0, 0);
      this.targetPosition = new THREE.Vector3(0, 0, 0);
      this.isAnimating = false;
    }

    _lerp(start, end, factor) {
      return start + (end - start) * factor;
    }

    updateTarget(fromPlatform, toPlatform) {
      const fromPos = fromPlatform.position;
      const toPos = toPlatform.position;
      this.targetPosition.set((fromPos.x + toPos.x) / 2, 0, (fromPos.z + toPos.z) / 2);
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
      const smoothFactor = this.config.smoothFactor;

      const dx = Math.abs(target.x - current.x);
      const dz = Math.abs(target.z - current.z);
      const threshold = 0.01;

      if (dx < threshold && dz < threshold) {
        current.x = target.x;
        current.z = target.z;
        this.sceneManager.lookAt(new THREE.Vector3(current.x, 0, current.z));
        this.sceneManager.render();
        this.isAnimating = false;
        return;
      }

      current.x = this._lerp(current.x, target.x, smoothFactor);
      current.z = this._lerp(current.z, target.z, smoothFactor);

      this.sceneManager.lookAt(new THREE.Vector3(current.x, 0, current.z));
      this.sceneManager.render();

      requestAnimationFrame(() => this._animate());
    }

    lookAtCurrent() { this.sceneManager.lookAt(this.currentPosition); }

    reset() {
      this.currentPosition.set(0, 0, 0);
      this.targetPosition.set(0, 0, 0);
      this.isAnimating = false;
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

      const jMinX = jumperPos.x - jumperRadius;
      const jMaxX = jumperPos.x + jumperRadius;
      const jMinZ = jumperPos.z - jumperRadius;
      const jMaxZ = jumperPos.z + jumperRadius;

      const pMinX = platPos.x - halfSize.x;
      const pMaxX = platPos.x + halfSize.x;
      const pMinZ = platPos.z - halfSize.z;
      const pMaxZ = platPos.z + halfSize.z;

      const centerInX = jumperPos.x >= pMinX && jumperPos.x <= pMaxX;
      const centerInZ = jumperPos.z >= pMinZ && jumperPos.z <= pMaxZ;
      const centerInPlatform = centerInX && centerInZ;

      const tolerance = jumperRadius * 0.3;
      const fullyInX = jMinX >= pMinX - tolerance && jMaxX <= pMaxX + tolerance;
      const fullyInZ = jMinZ >= pMinZ - tolerance && jMaxZ <= pMaxZ + tolerance;
      const fullyOn = centerInPlatform && fullyInX && fullyInZ;

      const overlapX = jMaxX > pMinX && jMinX < pMaxX;
      const overlapZ = jMaxZ > pMinZ && jMinZ < pMaxZ;
      const partiallyOn = overlapX && overlapZ && !fullyOn;

      if (platform.type === 'cylinder') {
        const dx = jumperPos.x - platPos.x;
        const dz = jumperPos.z - platPos.z;
        const distFromCenter = Math.sqrt(dx * dx + dz * dz);
        const platformRadius = halfSize.x;

        const fullyOnCylinder = distFromCenter + jumperRadius * 0.7 <= platformRadius;
        const partiallyOnCylinder = distFromCenter - jumperRadius < platformRadius && 
                                     distFromCenter + jumperRadius > platformRadius;

        return { fully: fullyOnCylinder, partial: partiallyOnCylinder && !fullyOnCylinder };
      }

      return { fully: fullyOn || centerInPlatform, partial: partiallyOn };
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
  // Core: Game (Main Orchestrator)
  // ============================================
  class Game {
    constructor() {
      this.sceneManager = new SceneManager();
      this.eventManager = new EventManager(this.sceneManager.canvas);
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
          this.platformManager.createPlatform();
          this._updateCamera();
          break;
        case 2:
          this.scoreManager.addScore('normal');
          this._resetJumperState();
          this.platformManager.createPlatform();
          this._updateCamera();
          break;
        case -2:
          this._animateFallDown();
          this._triggerGameOver();
          break;
        case -1:
        case -3:
          this._animateFallOff(landingState);
          this._triggerGameOver();
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
      }
    }

    _animateFallDown() {
      const groundLevel = -GameConfig.jumper.height / 2;
      const animate = () => {
        if (this.jumper.position.y >= groundLevel) {
          this.jumper.position.y -= GameConfig.physics.fallSpeed;
          this.sceneManager.render();
          requestAnimationFrame(animate);
        }
      };
      animate();
    }

    _animateFallOff(state) {
      const direction = this.platformManager.getDirection();
      const rotateAxis = direction === 'z' ? 'x' : 'z';
      const rotateDirection = state === -1 ? -1 : 1;
      const targetRotation = rotateDirection * Math.PI / 2;

      const animateRotation = () => {
        const currentRotation = this.jumper.rotation[rotateAxis];
        const reachedTarget = rotateDirection === -1 
          ? currentRotation <= targetRotation
          : currentRotation >= targetRotation;

        if (!reachedTarget) {
          this.jumper.rotation[rotateAxis] += rotateDirection * 0.1;
          this.sceneManager.render();
          requestAnimationFrame(animateRotation);
        } else {
          this._animateFallDown();
        }
      };
      animateRotation();
    }

    _triggerGameOver() {
      this.state.isGameOver = true;
      if (this.failCallback) {
        setTimeout(() => {
          this.failCallback(this.scoreManager.getScore());
        }, 1000);
      }
    }

    _onResize() {
      this.sceneManager.onResize();
      this.sceneManager.render();
    }

    set onFail(callback) { this.failCallback = callback; }
    get score() { return this.scoreManager.getScore(); }

    dispose() {
      this.eventManager.dispose();
      this.platformManager.dispose();
      this.scoreManager.dispose();
      this.powerBarManager.dispose();
      if (this.jumper) this.jumper.dispose();
      this.sceneManager.dispose();
    }
  }

  // Export to global scope
  window.Game = Game;

})();
