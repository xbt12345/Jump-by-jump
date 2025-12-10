import * as THREE from 'three';

/**
 * Environment - Background elements and atmosphere
 */
export class Environment {
    constructor(scene) {
        this.scene = scene;
        this.createGround();
        this.createBackgroundElements();
    }

    createGround() {
        // Infinite-looking ground plane
        const groundGeometry = new THREE.PlaneGeometry(200, 200);
        const groundMaterial = new THREE.MeshStandardMaterial({
            color: 0x16162a,
            roughness: 0.9,
            metalness: 0.1
        });
        
        this.ground = new THREE.Mesh(groundGeometry, groundMaterial);
        this.ground.rotation.x = -Math.PI / 2;
        this.ground.position.y = -0.5;
        this.ground.receiveShadow = true;
        
        this.scene.add(this.ground);
    }

    createBackgroundElements() {
        // Add some floating decorative elements
        const decorGeometry = new THREE.IcosahedronGeometry(0.3, 0);
        
        this.decorations = [];
        
        for (let i = 0; i < 20; i++) {
            const material = new THREE.MeshStandardMaterial({
                color: new THREE.Color().setHSL(0.6 + Math.random() * 0.2, 0.5, 0.3),
                roughness: 0.8,
                metalness: 0.2,
                transparent: true,
                opacity: 0.6
            });
            
            const decor = new THREE.Mesh(decorGeometry, material);
            
            // Random position in a ring around the play area
            const angle = Math.random() * Math.PI * 2;
            const radius = 15 + Math.random() * 20;
            
            decor.position.set(
                Math.cos(angle) * radius,
                2 + Math.random() * 8,
                Math.sin(angle) * radius
            );
            
            decor.rotation.set(
                Math.random() * Math.PI,
                Math.random() * Math.PI,
                Math.random() * Math.PI
            );
            
            decor.userData = {
                rotationSpeed: {
                    x: (Math.random() - 0.5) * 0.5,
                    y: (Math.random() - 0.5) * 0.5,
                    z: (Math.random() - 0.5) * 0.5
                },
                floatOffset: Math.random() * Math.PI * 2,
                floatSpeed: 0.5 + Math.random() * 0.5,
                baseY: decor.position.y
            };
            
            this.scene.add(decor);
            this.decorations.push(decor);
        }
    }

    update(deltaTime, time) {
        // Animate decorations
        this.decorations.forEach(decor => {
            const data = decor.userData;
            
            // Rotate
            decor.rotation.x += data.rotationSpeed.x * deltaTime;
            decor.rotation.y += data.rotationSpeed.y * deltaTime;
            decor.rotation.z += data.rotationSpeed.z * deltaTime;
            
            // Float up and down
            decor.position.y = data.baseY + Math.sin(time * data.floatSpeed + data.floatOffset) * 0.5;
        });
    }

    updatePosition(targetPosition) {
        // Move ground to follow player
        this.ground.position.x = targetPosition.x;
        this.ground.position.z = targetPosition.z;
    }

    dispose() {
        this.scene.remove(this.ground);
        this.ground.geometry.dispose();
        this.ground.material.dispose();
        
        this.decorations.forEach(decor => {
            this.scene.remove(decor);
            decor.geometry.dispose();
            decor.material.dispose();
        });
    }
}
