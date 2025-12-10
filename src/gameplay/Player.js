// Player v10 - Stays vertical during jump
import * as THREE from 'three';
import { GameConfig } from '../core/GameConfig.js';
import { GameEvents } from '../core/EventBus.js';

export class Player {
    constructor(scene, eventBus, animationManager, particleSystem) {
        this.scene = scene;
        this.eventBus = eventBus;
        this.animationManager = animationManager;
        this.particleSystem = particleSystem;
        
        this.velocity = new THREE.Vector3();
        this.isJumping = false;
        this.isCharging = false;
        this.isDead = false;
        this.jumpDirection = 'z'; // 'z' or 'x'
        
        this.createMesh();
        this.setupEventListeners();
    }

    createMesh() {
        const cfg = GameConfig.player;
        this.group = new THREE.Group();
        
        // Body - cylinder
        const bodyGeo = new THREE.CylinderGeometry(
            cfg.size.width / 2 * 0.7,
            cfg.size.width / 2,
            cfg.size.height,
            32
        );
        const bodyMat = new THREE.MeshStandardMaterial({
            color: cfg.color,
            roughness: 0.5
        });
        this.body = new THREE.Mesh(bodyGeo, bodyMat);
        this.body.castShadow = true;
        this.body.position.y = cfg.size.height / 2;
        
        // Head - white sphere
        const headGeo = new THREE.SphereGeometry(cfg.size.width / 2 * 0.5, 32, 16);
        const headMat = new THREE.MeshStandardMaterial({ color: 0xffffff });
        this.head = new THREE.Mesh(headGeo, headMat);
        this.head.castShadow = true;
        this.head.position.y = cfg.size.height * 0.85;
        
        this.group.add(this.body);
        this.group.add(this.head);
        
        // Start position
        this.group.position.set(
            cfg.startPosition.x,
            cfg.startPosition.y,
            cfg.startPosition.z
        );
        
        this.scene.add(this.group);
    }

    setupEventListeners() {
        this.eventBus.on(GameEvents.CHARGE_START, () => {
            if (!this.isJumping && !this.isDead) {
                this.isCharging = true;
            }
        });
        
        this.eventBus.on(GameEvents.CHARGE_UPDATE, ({ normalized }) => {
            if (!this.isCharging || this.isJumping || this.isDead) return;
            
            // Squash down while charging
            const cfg = GameConfig.player.squash;
            const squashY = 1 - (1 - cfg.maxSquash) * normalized;
            const stretchXZ = 1 + (cfg.stretchFactor - 1) * normalized * 0.3;
            this.group.scale.set(stretchXZ, squashY, stretchXZ);
        });
        
        this.eventBus.on(GameEvents.CHARGE_RELEASE, ({ power }) => {
            if (!this.isCharging || this.isJumping || this.isDead) return;
            this.isCharging = false;
            this.jump(power);
        });
    }

    jump(power) {
        this.isJumping = true;
        
        const cfg = GameConfig.player.jump;
        
        // Vertical velocity for arc
        this.velocity.y = power * 2.2;
        
        // Horizontal velocity in jump direction
        const hSpeed = power * cfg.horizontalSpeed;
        if (this.jumpDirection === 'x') {
            this.velocity.x = hSpeed;
            this.velocity.z = 0;
        } else {
            this.velocity.x = 0;
            this.velocity.z = hSpeed;
        }
        
        // Stretch up animation
        this.animationManager.animate({
            target: this.group.scale,
            from: { x: this.group.scale.x, y: this.group.scale.y, z: this.group.scale.z },
            to: { x: 0.8, y: GameConfig.player.squash.stretchFactor, z: 0.8 },
            duration: 0.1,
            easing: 'easeOutQuad'
        });
        
        this.eventBus.emit(GameEvents.JUMP_START, { power });
    }

    update(deltaTime, platforms) {
        if (!this.isJumping || this.isDead) return;
        
        const g = GameConfig.physics.gravity;
        
        // Apply gravity
        this.velocity.y += g * deltaTime;
        
        // Update position - piece stays VERTICAL (no rotation)
        this.group.position.x += this.velocity.x * deltaTime;
        this.group.position.y += this.velocity.y * deltaTime;
        this.group.position.z += this.velocity.z * deltaTime;
        
        // Check landing only when falling
        if (this.velocity.y < 0) {
            this.checkLanding(platforms);
        }
    }

    checkLanding(platforms) {
        const cfg = GameConfig.player;
        const playerBottom = this.group.position.y - cfg.size.height / 2;
        const px = this.group.position.x;
        const pz = this.group.position.z;
        
        // Check all platforms
        for (let i = platforms.length - 1; i >= 0; i--) {
            const platform = platforms[i];
            if (platform.isSpawning) continue;
            
            const platformTop = platform.getTopY();
            
            // Check if at platform height
            if (playerBottom <= platformTop + 0.1 && playerBottom >= platformTop - 0.3) {
                // Check if within platform bounds
                if (platform.containsPoint(px, pz)) {
                    this.land(platform, i);
                    return;
                }
            }
        }
        
        // Check if fallen
        if (playerBottom < -1) {
            this.die();
        }
    }

    land(platform, platformIndex) {
        this.isJumping = false;
        this.velocity.set(0, 0, 0);
        
        // Snap to platform top
        const landY = platform.getTopY() + GameConfig.player.size.height / 2;
        this.group.position.y = landY;
        
        // Squash animation on landing
        const cfg = GameConfig.player.squash;
        this.animationManager.animate({
            target: this.group.scale,
            from: { x: 0.8, y: cfg.stretchFactor, z: 0.8 },
            to: { x: 1.2, y: cfg.maxSquash, z: 1.2 },
            duration: 0.08,
            easing: 'easeOutQuad'
        }).then(() => {
            this.animationManager.animate({
                target: this.group.scale,
                from: { x: 1.2, y: cfg.maxSquash, z: 1.2 },
                to: { x: 1, y: 1, z: 1 },
                duration: 0.2,
                easing: 'easeOutElastic'
            });
        });
        
        // Calculate distance from center
        const dist = platform.getDistanceFromCenter(
            this.group.position.x,
            this.group.position.z
        );
        
        // Emit landed event
        this.eventBus.emit(GameEvents.PLAYER_LANDED, {
            platform,
            platformIndex,
            distanceFromCenter: dist
        });
    }

    die() {
        if (this.isDead) return;
        this.isDead = true;
        this.isJumping = false;
        
        if (this.particleSystem) {
            this.particleSystem.createPlayerDisperseEffect(this.group.position.clone());
        }
        
        this.group.visible = false;
        this.eventBus.emit(GameEvents.PLAYER_FELL, {});
    }

    setJumpDirection(dir) {
        this.jumpDirection = dir;
    }

    getPosition() {
        return this.group.position.clone();
    }

    reset() {
        const cfg = GameConfig.player;
        this.group.position.set(cfg.startPosition.x, cfg.startPosition.y, cfg.startPosition.z);
        this.group.scale.set(1, 1, 1);
        this.group.rotation.set(0, 0, 0);
        this.group.visible = true;
        this.velocity.set(0, 0, 0);
        this.isJumping = false;
        this.isCharging = false;
        this.isDead = false;
        this.jumpDirection = 'z';
    }

    dispose() {
        this.body.geometry.dispose();
        this.body.material.dispose();
        this.head.geometry.dispose();
        this.head.material.dispose();
        this.scene.remove(this.group);
    }
}

console.log('Player v11');
