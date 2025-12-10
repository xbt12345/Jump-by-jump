import * as THREE from 'three';
import { GameEvents } from '../core/EventBus.js';
import { GameConfig } from '../core/GameConfig.js';

/**
 * ParticleSystem - Visual effects
 * - Landing particles
 * - Player death/disperse effect when falling
 */
export class ParticleSystem {
    constructor(scene, eventBus) {
        this.scene = scene;
        this.eventBus = eventBus;
        this.particles = [];
        
        this.setupEventListeners();
    }

    setupEventListeners() {
        this.eventBus.on(GameEvents.PLAYER_LANDED, this.onPlayerLanded.bind(this));
        this.eventBus.on(GameEvents.PLAYER_FELL, this.onPlayerFell.bind(this));
    }

    onPlayerLanded({ platform }) {
        const position = platform.getCenterPosition();
        position.y = platform.getTopY();
        this.createLandingParticles(position);
    }

    onPlayerFell() {
        // Get player position from game (we'll emit it with the event)
        // For now, create particles at a default position
    }

    /**
     * Create disperse effect when player falls - like the piece breaking apart
     */
    createPlayerDisperseEffect(position) {
        const particleCount = 30;
        const playerColor = GameConfig.player.color;
        
        for (let i = 0; i < particleCount; i++) {
            const geometry = new THREE.BoxGeometry(0.15, 0.15, 0.15);
            const material = new THREE.MeshBasicMaterial({
                color: i < particleCount / 2 ? playerColor : 0xffffff,
                transparent: true,
                opacity: 1
            });
            
            const particle = new THREE.Mesh(geometry, material);
            particle.position.copy(position);
            
            // Random velocity in all directions
            const speed = 3 + Math.random() * 4;
            const theta = Math.random() * Math.PI * 2;
            const phi = Math.random() * Math.PI;
            
            particle.userData = {
                velocity: new THREE.Vector3(
                    Math.sin(phi) * Math.cos(theta) * speed,
                    Math.random() * 5 + 2, // Upward burst
                    Math.sin(phi) * Math.sin(theta) * speed
                ),
                rotationSpeed: new THREE.Vector3(
                    (Math.random() - 0.5) * 10,
                    (Math.random() - 0.5) * 10,
                    (Math.random() - 0.5) * 10
                ),
                life: 1,
                decay: 0.8 + Math.random() * 0.4
            };
            
            this.scene.add(particle);
            this.particles.push(particle);
        }
    }

    createLandingParticles(position) {
        const particleCount = 10;
        const geometry = new THREE.SphereGeometry(0.06, 6, 6);
        
        for (let i = 0; i < particleCount; i++) {
            const material = new THREE.MeshBasicMaterial({
                color: new THREE.Color().setHSL(Math.random() * 0.1 + 0.55, 0.8, 0.6),
                transparent: true,
                opacity: 1
            });
            
            const particle = new THREE.Mesh(geometry, material);
            particle.position.copy(position);
            
            const angle = (i / particleCount) * Math.PI * 2;
            const speed = 1.5 + Math.random() * 1.5;
            
            particle.userData = {
                velocity: new THREE.Vector3(
                    Math.cos(angle) * speed,
                    2 + Math.random() * 1.5,
                    Math.sin(angle) * speed
                ),
                life: 1,
                decay: 1.5 + Math.random() * 0.5
            };
            
            this.scene.add(particle);
            this.particles.push(particle);
        }
    }

    update(deltaTime) {
        for (let i = this.particles.length - 1; i >= 0; i--) {
            const particle = this.particles[i];
            const data = particle.userData;
            
            // Gravity
            data.velocity.y -= 15 * deltaTime;
            
            // Update position
            particle.position.add(data.velocity.clone().multiplyScalar(deltaTime));
            
            // Rotation (if has rotation speed)
            if (data.rotationSpeed) {
                particle.rotation.x += data.rotationSpeed.x * deltaTime;
                particle.rotation.y += data.rotationSpeed.y * deltaTime;
                particle.rotation.z += data.rotationSpeed.z * deltaTime;
            }
            
            // Decay
            data.life -= data.decay * deltaTime;
            particle.material.opacity = Math.max(0, data.life);
            particle.scale.setScalar(Math.max(0.1, data.life));
            
            // Remove dead particles
            if (data.life <= 0) {
                this.scene.remove(particle);
                particle.geometry.dispose();
                particle.material.dispose();
                this.particles.splice(i, 1);
            }
        }
    }

    dispose() {
        this.particles.forEach(particle => {
            this.scene.remove(particle);
            particle.geometry.dispose();
            particle.material.dispose();
        });
        this.particles = [];
    }
}
