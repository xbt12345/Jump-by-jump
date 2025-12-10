import SceneManager from './SceneManager.js';
import EventManager from './EventManager.js';
import PlatformManager from '../managers/PlatformManager.js';
import CameraController from '../managers/CameraController.js';
import PhysicsManager from '../managers/PhysicsManager.js';
import ScoreManager from '../managers/ScoreManager.js';
import PowerBarManager from '../managers/PowerBarManager.js';
import Jumper from '../entities/Jumper.js';
import Platform from '../entities/Platform.js';
import GameConfig from '../utils/Constants.js';
import ShadowSystem from '../systems/ShadowSystem.js';
import AudioSystem from '../systems/AudioSystem.js';
import TextureSystem from '../systems/TextureSystem.js';
import BackgroundSystem from '../systems/BackgroundSystem.js';

/**
 * Game - main game orchestrator
 * Coordinates all managers and systems
 */
class Game {
  constructor() {
    // Core systems
    this.sceneManager = new SceneManager();
    this.eventManager = new EventManager(this.sceneManager.canvas);
    
    // Rendering systems - initialize first
    this.textureSystem = new TextureSystem();
    this.shadowSystem = new ShadowSystem(this.sceneManager);
    this.backgroundSystem = new BackgroundSystem(this.sceneManager);
    this.audioSystem = new AudioSystem();
    
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
    
    // Entities
    this.jumper = null;
    
    // Game state
    this.state = {
      mouseDown: false,
      isJumping: false,
      isGameOver: false
    };
    
    // Callbacks
    this.failCallback = null;
    
    // Bind event handlers
    this._bindEvents();
  }

  _bindEvents() {
    this.eventManager.on('down', () => this._onInputDown());
    this.eventManager.on('up', () => this._onInputUp());
    this.eventManager.on('resize', () => this._onResize());
  }

  /**
   * Start the game
   */
  start() {
    // Create initial platforms
    this.platformManager.createPlatform();
    this.platformManager.createPlatform();
    
    // Create jumper
    this._createJumper();
    
    // Initialize camera to look at the scene
    this._initializeCamera();
    
    // Start listening for input
    this.eventManager.startListening();
    
    // Initial render
    this.sceneManager.render();
  }

  /**
   * Initialize camera position and target
   */
  _initializeCamera() {
    const currentPlatform = this.platformManager.getCurrentPlatform();
    const targetPlatform = this.platformManager.getTargetPlatform();
    
    if (currentPlatform && targetPlatform) {
      // Calculate center point between platforms
      const centerX = (currentPlatform.position.x + targetPlatform.position.x) / 2;
      const centerZ = (currentPlatform.position.z + targetPlatform.position.z) / 2;
      
      this.sceneManager.lookAt(new THREE.Vector3(centerX, 0, centerZ));
    } else {
      this.sceneManager.lookAt(new THREE.Vector3(0, 0, 0));
    }
  }

  /**
   * Restart the game
   */
  restart() {
    // Clear existing entities
    this.platformManager.clear();
    
    if (this.jumper) {
      this.sceneManager.remove(this.jumper.mesh);
      this.jumper.dispose();
    }
    
    // Reset state
    this.state.mouseDown = false;
    this.state.isJumping = false;
    this.state.isGameOver = false;
    
    // Reset managers and systems
    this.physicsManager.reset();
    this.scoreManager.reset();
    this.cameraController.reset();
    this.textureSystem.reset();
    
    // Reset background position
    this.backgroundSystem.updatePosition(0, 0);
    
    // Recreate game
    this.platformManager.createPlatform();
    this.platformManager.createPlatform();
    this._createJumper();
    
    // Re-initialize camera
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
    
    // Compress jumper and accumulate power
    if (this.jumper.compress()) {
      this.physicsManager.charge();
      
      // Update power bar
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
      // Still in air
      this.sceneManager.render();
      requestAnimationFrame(() => this._executeJump());
    } else {
      // Landed - check result
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
        // Stayed on current platform
        this._resetJumperState();
        break;
        
      case 3:
        // Center landing - bonus points with combo
        this.scoreManager.addScore('center');
        this._resetJumperState();
        // Update jumper landing position before creating next platform
        this.platformManager.setJumperLandingPosition(this.jumper.position);
        this.platformManager.createPlatform();
        this._updateCamera();
        break;
        
      case 2:
        // Normal landing on target
        this.scoreManager.addScore('normal');
        this._resetJumperState();
        // Update jumper landing position before creating next platform
        this.platformManager.setJumperLandingPosition(this.jumper.position);
        this.platformManager.createPlatform();
        this._updateCamera();
        break;
        
      case -2:
        // Fell between platforms or beyond
        const fallVel = { x: 0, z: 0 };
        const currentPlat = this.platformManager.getCurrentPlatform();
        if (currentPlat) {
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
        // Fell off edge
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
      
      // Update shadow camera and background to follow action
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

  /**
   * Set callback for game over
   */
  set onFail(callback) {
    this.failCallback = callback;
  }

  /**
   * Get current score
   */
  get score() {
    return this.scoreManager.getScore();
  }

  /**
   * Dispose of all resources
   */
  dispose() {
    this.eventManager.dispose();
    this.platformManager.dispose();
    this.scoreManager.dispose();
    this.powerBarManager.dispose();
    this.shadowSystem.dispose();
    this.audioSystem.dispose();
    this.textureSystem.dispose();
    this.backgroundSystem.dispose();
    
    if (this.jumper) {
      this.jumper.dispose();
    }
    
    this.sceneManager.dispose();
  }
}

export default Game;
