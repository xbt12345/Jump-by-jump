// Platform v10 - Realistic spawn animation from far end
import * as THREE from 'three';
import { GameConfig } from '../core/GameConfig.js';

export class Platform {
    constructor(position, size, color, animationManager) {
        this.animationManager = animationManager;
        this.size = { ...size };
        this.position = { ...position };
        this.isSpawning = true;
        
        this.createMesh(color);
    }

    createMesh(color) {
        const geo = new THREE.BoxGeometry(
            this.size.width,
            this.size.height,
            this.size.depth
        );
        const mat = new THREE.MeshStandardMaterial({
            color,
            roughness: 0.6,
            metalness: 0.1
        });
        
        this.mesh = new THREE.Mesh(geo, mat);
        this.mesh.castShadow = true;
        this.mesh.receiveShadow = true;
        this.mesh.position.set(this.position.x, this.position.y, this.position.z);
        // Scale will be set by createPlatform based on animate flag
        this.mesh.scale.set(1, 1, 1);
    }

    async playSpawnAnimation(direction) {
        const cfg = GameConfig.platform.spawnAnimation;
        
        // Start from far end in the direction
        const offset = 5;
        const startX = this.position.x + (direction === 'x' ? offset : 0);
        const startZ = this.position.z + (direction === 'z' ? offset : 0);
        const startY = this.position.y + 3;
        
        this.mesh.position.set(startX, startY, startZ);
        this.mesh.scale.set(0.2, 0.2, 0.2);
        
        // Animate position and scale together
        await Promise.all([
            this.animationManager.animate({
                target: this.mesh.position,
                from: { x: startX, y: startY, z: startZ },
                to: { x: this.position.x, y: this.position.y, z: this.position.z },
                duration: cfg.duration,
                easing: 'easeOutCubic'
            }),
            this.animationManager.animate({
                target: this.mesh.scale,
                from: { x: 0.2, y: 0.2, z: 0.2 },
                to: { x: 1, y: 1, z: 1 },
                duration: cfg.duration,
                easing: 'easeOutBack'
            })
        ]);
        
        // Bounce effect
        await this.animationManager.animate({
            target: this.mesh.position,
            from: { y: this.position.y },
            to: { y: this.position.y + cfg.bounceHeight },
            duration: 0.1,
            easing: 'easeOutQuad'
        });
        
        await this.animationManager.animate({
            target: this.mesh.position,
            from: { y: this.position.y + cfg.bounceHeight },
            to: { y: this.position.y },
            duration: 0.1,
            easing: 'easeOutBounce'
        });
        
        this.isSpawning = false;
    }

    async playRemoveAnimation() {
        await this.animationManager.animate({
            target: this.mesh.position,
            from: { y: this.mesh.position.y },
            to: { y: this.mesh.position.y - 5 },
            duration: 0.4,
            easing: 'easeInCubic'
        });
    }

    getTopY() {
        return this.position.y + this.size.height / 2;
    }

    containsPoint(x, z) {
        const hw = this.size.width / 2;
        const hd = this.size.depth / 2;
        return x >= this.position.x - hw && x <= this.position.x + hw &&
               z >= this.position.z - hd && z <= this.position.z + hd;
    }

    getDistanceFromCenter(x, z) {
        const dx = x - this.position.x;
        const dz = z - this.position.z;
        return Math.sqrt(dx * dx + dz * dz);
    }

    getCenterPosition() {
        return new THREE.Vector3(this.position.x, this.position.y, this.position.z);
    }

    dispose() {
        this.mesh.geometry.dispose();
        this.mesh.material.dispose();
    }
}

console.log('Platform v11');
