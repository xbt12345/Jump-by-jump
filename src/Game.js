// Game v11
import { EventBus, GameEvents } from './core/EventBus.js';
import { GameState } from './core/GameState.js';
import { GameConfig } from './core/GameConfig.js';
import { Timer } from './core/Timer.js';
import { AnimationManager } from './utils/AnimationManager.js';
import { SceneManager } from './systems/SceneManager.js';
import { InputSystem } from './systems/InputSystem.js';
import { UISystem } from './systems/UISystem.js';
import { Player } from './gameplay/Player.js';
import { PlatformGenerator } from './gameplay/PlatformGenerator.js';
import { ScoreManager } from './gameplay/ScoreManager.js';
import { Environment } from './entities/Environment.js';
import { ParticleSystem } from './effects/ParticleSystem.js';

export class Game {
    constructor(canvas) {
        console.log(`Game v${GameConfig.version} constructor`);
        this.canvas = canvas;
        this.isRunning = false;
        this.lastTime = 0;
        
        this.initialize();
    }

    initialize() {
        console.log('Initializing game systems...');
        
        // Core
        this.eventBus = new EventBus();
        this.gameState = new GameState(this.eventBus);
        this.timer = new Timer();
        this.animationManager = new AnimationManager();
        
        // Rendering
        this.sceneManager = new SceneManager(this.canvas);
        console.log('SceneManager created');
        
        // Input & UI
        this.inputSystem = new InputSystem(this.eventBus, this.gameState);
        this.uiSystem = new UISystem(this.eventBus);
        
        // Effects
        this.environment = new Environment(this.sceneManager.getScene());
        console.log('Environment created');
        
        this.particleSystem = new ParticleSystem(this.sceneManager.getScene(), this.eventBus);
        
        // Gameplay
        this.platformGenerator = new PlatformGenerator(
            this.sceneManager.getScene(),
            this.eventBus,
            this.animationManager,
            Date.now()
        );
        console.log('PlatformGenerator created');
        
        this.player = new Player(
            this.sceneManager.getScene(),
            this.eventBus,
            this.animationManager,
            this.particleSystem
        );
        console.log('Player created');
        
        this.scoreManager = new ScoreManager(this.eventBus, this.gameState);
        
        // Events
        this.setupEventListeners();
        
        // Initialize world
        this.platformGenerator.initialize();
        this.player.setJumpDirection('z');
        
        this.timer.start();
        
        console.log(`Game v${GameConfig.version} initialized`);
    }

    setupEventListeners() {
        this.eventBus.on(GameEvents.GAME_RESTART, this.restart.bind(this));
        this.eventBus.on(GameEvents.GAME_OVER, ({ score }) => {
            console.log(`Game Over! Score: ${score}`);
        });
        this.eventBus.on(GameEvents.PLAYER_LANDED, () => {
            this.player.setJumpDirection(this.platformGenerator.getJumpDirection());
        });
    }

    restart() {
        console.log('Restarting game...');
        this.gameState.reset();
        this.scoreManager.reset();
        this.platformGenerator.reset(Date.now());
        this.player.reset();
        this.uiSystem.reset();
        this.animationManager.clear();
        this.timer.reset();
        this.player.setJumpDirection('z');
    }

    start() {
        if (this.isRunning) return;
        this.isRunning = true;
        this.lastTime = performance.now();
        console.log('Game loop starting...');
        this.gameLoop();
    }

    stop() {
        this.isRunning = false;
    }

    gameLoop() {
        if (!this.isRunning) return;
        
        const now = performance.now();
        const dt = Math.min((now - this.lastTime) / 1000, 0.1);
        this.lastTime = now;
        
        this.update(dt);
        this.render();
        
        requestAnimationFrame(() => this.gameLoop());
    }

    update(dt) {
        this.inputSystem.update(dt);
        this.animationManager.update(dt);
        
        if (this.gameState.isPlaying) {
            this.player.update(dt, this.platformGenerator.getPlatforms());
        }
        
        const pos = this.player.getPosition();
        this.environment.update(dt, this.timer.getTime());
        this.environment.updatePosition(pos);
        this.particleSystem.update(dt);
        this.sceneManager.updateCamera(pos, dt);
    }

    render() {
        this.sceneManager.render();
    }

    dispose() {
        this.stop();
        this.inputSystem.dispose();
        this.platformGenerator.dispose();
        this.player.dispose();
        this.environment.dispose();
        this.particleSystem.dispose();
        this.sceneManager.dispose();
        this.eventBus.clear();
    }
}

console.log('Game module v11 loaded');
