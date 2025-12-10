// PlatformGenerator v10 - Generates platforms with alternating directions
import { GameConfig } from '../core/GameConfig.js';
import { GameEvents } from '../core/EventBus.js';
import { Platform } from './Platform.js';
import { RNG } from '../utils/RNG.js';

export class PlatformGenerator {
    constructor(scene, eventBus, animationManager, seed = Date.now()) {
        this.scene = scene;
        this.eventBus = eventBus;
        this.animationManager = animationManager;
        this.rng = new RNG(seed);
        
        this.platforms = [];
        this.currentPlatformIndex = 0;
        this.lastDirection = null;
        
        this.setupEventListeners();
    }

    setupEventListeners() {
        this.eventBus.on(GameEvents.PLAYER_LANDED, this.onPlayerLanded.bind(this));
    }

    initialize() {
        const cfg = GameConfig.platform;
        
        // Platform 0: starting platform at origin
        this.createPlatform(
            { x: 0, y: 0, z: 0 },
            { ...cfg.size },
            false,
            null
        );
        
        // Platform 1: first target in Z direction
        this.lastDirection = 'z';
        this.generateNextPlatform('z');
    }

    createPlatform(position, size, animate, direction) {
        const colors = GameConfig.platform.colors;
        const color = this.rng.pick(colors);
        
        const platform = new Platform(position, size, color, this.animationManager);
        this.scene.add(platform.mesh);
        this.platforms.push(platform);
        
        if (animate && direction) {
            // Start small for animation
            platform.mesh.scale.set(0.2, 0.2, 0.2);
            platform.playSpawnAnimation(direction);
        } else {
            // First platform - show immediately at full size
            platform.mesh.position.set(position.x, position.y, position.z);
            platform.mesh.scale.set(1, 1, 1);
            platform.isSpawning = false;
        }
        
        return platform;
    }

    generateNextPlatform(direction) {
        const cfg = GameConfig.platform;
        const last = this.platforms[this.platforms.length - 1];
        
        // Random distance within jumpable range
        const distance = this.rng.nextFloatRange(cfg.distance.min, cfg.distance.max);
        
        // Position: offset only in the specified direction
        const position = {
            x: last.position.x + (direction === 'x' ? distance : 0),
            y: 0,
            z: last.position.z + (direction === 'z' ? distance : 0)
        };
        
        this.lastDirection = direction;
        return this.createPlatform(position, { ...cfg.size }, true, direction);
    }

    onPlayerLanded({ platformIndex }) {
        // Only generate new platform when landing on a NEW platform
        if (platformIndex > this.currentPlatformIndex) {
            this.currentPlatformIndex = platformIndex;
            
            // Alternate direction: Z -> X -> Z -> X
            const nextDir = (this.lastDirection === 'z') ? 'x' : 'z';
            this.generateNextPlatform(nextDir);
            
            this.cleanupOldPlatforms();
        }
    }

    cleanupOldPlatforms() {
        const maxPlatforms = GameConfig.platform.maxPlatforms;
        
        while (this.platforms.length > maxPlatforms) {
            const old = this.platforms.shift();
            this.currentPlatformIndex--;
            
            old.playRemoveAnimation().then(() => {
                this.scene.remove(old.mesh);
                old.dispose();
            });
        }
    }

    getPlatforms() {
        return this.platforms;
    }

    // Returns direction to jump toward the NEXT platform
    getJumpDirection() {
        return this.lastDirection || 'z';
    }

    reset(newSeed) {
        this.platforms.forEach(p => {
            this.scene.remove(p.mesh);
            p.dispose();
        });
        this.platforms = [];
        this.currentPlatformIndex = 0;
        this.lastDirection = null;
        
        if (newSeed !== undefined) {
            this.rng.setSeed(newSeed);
        } else {
            this.rng.reset();
        }
        
        this.initialize();
    }

    dispose() {
        this.platforms.forEach(p => {
            this.scene.remove(p.mesh);
            p.dispose();
        });
        this.platforms = [];
    }
}

console.log('PlatformGenerator v11');
