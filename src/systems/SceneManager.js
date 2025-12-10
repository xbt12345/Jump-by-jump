// SceneManager v11 - Simplified and fixed
import * as THREE from 'three';
import { GameConfig } from '../core/GameConfig.js';

export class SceneManager {
    constructor(canvas) {
        this.canvas = canvas;
        this.setupRenderer();
        this.setupScene();
        this.setupCamera();
        this.setupLights();
        this.setupResizeHandler();
        
        console.log('SceneManager initialized');
    }

    setupRenderer() {
        this.renderer = new THREE.WebGLRenderer({
            canvas: this.canvas,
            antialias: true
        });
        
        this.renderer.setSize(window.innerWidth, window.innerHeight);
        this.renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
        this.renderer.shadowMap.enabled = true;
        this.renderer.shadowMap.type = THREE.PCFSoftShadowMap;
        
        console.log('Renderer setup complete');
    }

    setupScene() {
        this.scene = new THREE.Scene();
        this.scene.background = new THREE.Color(GameConfig.visuals.backgroundColor);
        
        console.log('Scene setup complete, background:', GameConfig.visuals.backgroundColor.toString(16));
    }

    setupCamera() {
        const cfg = GameConfig.camera;
        const aspect = window.innerWidth / window.innerHeight;
        
        this.camera = new THREE.PerspectiveCamera(cfg.fov, aspect, cfg.near, cfg.far);
        this.camera.position.set(cfg.position.x, cfg.position.y, cfg.position.z);
        
        this.cameraTarget = new THREE.Vector3(0, 0, 0);
        this.camera.lookAt(this.cameraTarget);
        
        console.log('Camera at:', cfg.position.x, cfg.position.y, cfg.position.z);
    }

    setupLights() {
        const cfg = GameConfig.visuals;
        
        // Ambient light
        const ambient = new THREE.AmbientLight(cfg.ambientLightColor, cfg.ambientLightIntensity);
        this.scene.add(ambient);
        
        // Directional light with shadows
        this.directionalLight = new THREE.DirectionalLight(cfg.directionalLightColor, cfg.directionalLightIntensity);
        this.directionalLight.position.set(
            cfg.directionalLightPosition.x,
            cfg.directionalLightPosition.y,
            cfg.directionalLightPosition.z
        );
        
        this.directionalLight.castShadow = true;
        this.directionalLight.shadow.mapSize.width = cfg.shadowMapSize;
        this.directionalLight.shadow.mapSize.height = cfg.shadowMapSize;
        this.directionalLight.shadow.camera.near = 0.5;
        this.directionalLight.shadow.camera.far = 50;
        this.directionalLight.shadow.camera.left = -20;
        this.directionalLight.shadow.camera.right = 20;
        this.directionalLight.shadow.camera.top = 20;
        this.directionalLight.shadow.camera.bottom = -20;
        
        this.scene.add(this.directionalLight);
        this.scene.add(this.directionalLight.target);
        
        // Hemisphere light
        const hemi = new THREE.HemisphereLight(0x87ceeb, 0x444444, 0.4);
        this.scene.add(hemi);
        
        console.log('Lights setup complete');
    }

    setupResizeHandler() {
        window.addEventListener('resize', () => {
            const w = window.innerWidth;
            const h = window.innerHeight;
            this.camera.aspect = w / h;
            this.camera.updateProjectionMatrix();
            this.renderer.setSize(w, h);
        });
    }

    updateCamera(targetPosition, deltaTime) {
        const cfg = GameConfig.camera;
        const speed = cfg.followSpeed || 0.1;
        
        // Target camera position
        const targetCamPos = new THREE.Vector3(
            targetPosition.x + cfg.position.x,
            cfg.position.y,
            targetPosition.z + cfg.position.z
        );
        
        this.camera.position.lerp(targetCamPos, speed);
        this.cameraTarget.lerp(targetPosition, speed);
        this.camera.lookAt(this.cameraTarget);
        
        // Update light position
        this.directionalLight.position.set(
            targetPosition.x + GameConfig.visuals.directionalLightPosition.x,
            GameConfig.visuals.directionalLightPosition.y,
            targetPosition.z + GameConfig.visuals.directionalLightPosition.z
        );
        this.directionalLight.target.position.copy(targetPosition);
    }

    render() {
        this.renderer.render(this.scene, this.camera);
    }

    getScene() {
        return this.scene;
    }

    getCamera() {
        return this.camera;
    }

    dispose() {
        this.renderer.dispose();
    }
}

console.log('SceneManager module v11');
