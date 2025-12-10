/**
 * BackgroundSystem - Enhanced animated background with themed elements
 * Creates dynamic, immersive atmosphere with trees, clouds, and particles
 */
class BackgroundSystem {
  constructor(sceneManager) {
    this.sceneManager = sceneManager;
    this.particles = null;
    this.particleCount = 150;
    this.particleVelocities = [];
    this.skyMesh = null;
    this.groundMesh = null;
    this.decorations = [];
    this.clouds = [];
    this.time = 0;
    this.isAnimating = false;
    
    this._init();
  }

  _init() {
    this._createGradientSky();
    this._createGroundPlane();
    this._createParticles();
    this._createDecorations();
    this._createClouds();
    this._startAnimation();
  }

  _createGradientSky() {
    const canvas = document.createElement('canvas');
    canvas.width = 512;
    canvas.height = 512;
    const ctx = canvas.getContext('2d');

    // Beautiful sunset/sunrise gradient
    const gradient = ctx.createLinearGradient(0, 0, 0, canvas.height);
    gradient.addColorStop(0, '#0f0c29');
    gradient.addColorStop(0.2, '#302b63');
    gradient.addColorStop(0.4, '#24243e');
    gradient.addColorStop(0.6, '#667eea');
    gradient.addColorStop(0.8, '#f093fb');
    gradient.addColorStop(1, '#ffecd2');
    ctx.fillStyle = gradient;
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    // Add stars in upper portion
    ctx.fillStyle = 'rgba(255, 255, 255, 0.8)';
    for (let i = 0; i < 100; i++) {
      const x = Math.random() * canvas.width;
      const y = Math.random() * canvas.height * 0.4;
      const size = Math.random() * 2 + 0.5;
      ctx.beginPath();
      ctx.arc(x, y, size, 0, Math.PI * 2);
      ctx.fill();
    }

    // Add subtle clouds
    ctx.fillStyle = 'rgba(255, 255, 255, 0.08)';
    for (let i = 0; i < 15; i++) {
      const x = Math.random() * canvas.width;
      const y = canvas.height * 0.4 + Math.random() * canvas.height * 0.3;
      const w = Math.random() * 120 + 60;
      const h = Math.random() * 40 + 15;
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

    // Grass-like gradient
    const gradient = ctx.createRadialGradient(256, 256, 0, 256, 256, 360);
    gradient.addColorStop(0, '#90EE90');
    gradient.addColorStop(0.3, '#7CCD7C');
    gradient.addColorStop(0.6, '#6B8E23');
    gradient.addColorStop(1, '#556B2F');
    ctx.fillStyle = gradient;
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    // Add grass texture pattern
    ctx.strokeStyle = 'rgba(0, 100, 0, 0.1)';
    ctx.lineWidth = 1;
    for (let i = 0; i < 200; i++) {
      const x = Math.random() * canvas.width;
      const y = Math.random() * canvas.height;
      const len = Math.random() * 8 + 4;
      ctx.beginPath();
      ctx.moveTo(x, y);
      ctx.lineTo(x + (Math.random() - 0.5) * 3, y - len);
      ctx.stroke();
    }

    // Subtle grid for depth
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.15)';
    ctx.lineWidth = 1;
    const gridSize = 64;
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
    texture.repeat.set(15, 15);

    const geometry = new THREE.PlaneGeometry(1000, 1000);
    const material = new THREE.MeshLambertMaterial({ map: texture });
    
    this.groundMesh = new THREE.Mesh(geometry, material);
    this.groundMesh.rotation.x = -Math.PI / 2;
    this.groundMesh.position.y = -1.5;
    this.groundMesh.receiveShadow = true;
    this.sceneManager.add(this.groundMesh);
  }

  _createDecorations() {
    // Create 3D grass patches around the play area
    this._createGrassPatches();
    
    // Create low-poly trees around the play area
    const treePositions = [
      { x: -40, z: -40 }, { x: 40, z: -50 }, { x: -50, z: 30 },
      { x: 50, z: 40 }, { x: -30, z: 60 }, { x: 60, z: -20 },
      { x: -60, z: -10 }, { x: 30, z: -60 }, { x: -20, z: 50 },
      { x: 70, z: 10 }, { x: -70, z: 20 }, { x: 10, z: 70 }
    ];

    treePositions.forEach(pos => {
      const tree = this._createTree();
      tree.position.set(pos.x, -1.5, pos.z);
      tree.scale.setScalar(0.8 + Math.random() * 0.4);
      this.sceneManager.add(tree);
      this.decorations.push(tree);
    });

    // Create small bushes
    for (let i = 0; i < 20; i++) {
      const bush = this._createBush();
      const angle = Math.random() * Math.PI * 2;
      const dist = 30 + Math.random() * 50;
      bush.position.set(Math.cos(angle) * dist, -1.2, Math.sin(angle) * dist);
      bush.scale.setScalar(0.5 + Math.random() * 0.5);
      this.sceneManager.add(bush);
      this.decorations.push(bush);
    }

    // Create decorative rocks
    for (let i = 0; i < 15; i++) {
      const rock = this._createRock();
      const angle = Math.random() * Math.PI * 2;
      const dist = 25 + Math.random() * 60;
      rock.position.set(Math.cos(angle) * dist, -1.3, Math.sin(angle) * dist);
      rock.scale.setScalar(0.3 + Math.random() * 0.4);
      rock.rotation.y = Math.random() * Math.PI * 2;
      this.sceneManager.add(rock);
      this.decorations.push(rock);
    }
  }

  _createGrassPatches() {
    // Create multiple grass patches around the play area
    const grassColors = [0x228B22, 0x32CD32, 0x3CB371, 0x2E8B57, 0x6B8E23];
    
    for (let patch = 0; patch < 40; patch++) {
      const patchGroup = new THREE.Group();
      const angle = Math.random() * Math.PI * 2;
      const dist = 15 + Math.random() * 70;
      const patchX = Math.cos(angle) * dist;
      const patchZ = Math.sin(angle) * dist;
      
      // Each patch has multiple grass blades
      const bladesPerPatch = 8 + Math.floor(Math.random() * 8);
      
      for (let i = 0; i < bladesPerPatch; i++) {
        const blade = this._createGrassBlade(
          grassColors[Math.floor(Math.random() * grassColors.length)]
        );
        blade.position.set(
          (Math.random() - 0.5) * 2,
          0,
          (Math.random() - 0.5) * 2
        );
        blade.rotation.y = Math.random() * Math.PI * 2;
        patchGroup.add(blade);
      }
      
      patchGroup.position.set(patchX, -1.5, patchZ);
      this.sceneManager.add(patchGroup);
      this.decorations.push(patchGroup);
    }
  }

  _createGrassBlade(color) {
    const group = new THREE.Group();
    
    // Create a grass blade using a thin cone
    const height = 0.4 + Math.random() * 0.6;
    const width = 0.05 + Math.random() * 0.05;
    
    // Main blade
    const bladeGeo = new THREE.ConeGeometry(width, height, 4);
    const bladeMat = new THREE.MeshLambertMaterial({ 
      color: color,
      side: THREE.DoubleSide
    });
    const blade = new THREE.Mesh(bladeGeo, bladeMat);
    blade.position.y = height / 2;
    
    // Slight random tilt for natural look
    blade.rotation.x = (Math.random() - 0.5) * 0.3;
    blade.rotation.z = (Math.random() - 0.5) * 0.3;
    
    blade.castShadow = true;
    blade.receiveShadow = true;
    group.add(blade);
    
    // Sometimes add a second smaller blade
    if (Math.random() > 0.5) {
      const blade2Geo = new THREE.ConeGeometry(width * 0.7, height * 0.7, 4);
      const blade2 = new THREE.Mesh(blade2Geo, bladeMat);
      blade2.position.set(width * 2, height * 0.35, 0);
      blade2.rotation.x = (Math.random() - 0.5) * 0.4;
      blade2.rotation.z = 0.2 + Math.random() * 0.2;
      blade2.castShadow = true;
      group.add(blade2);
    }
    
    return group;
  }

  _createTree() {
    const group = new THREE.Group();

    // Trunk
    const trunkGeo = new THREE.CylinderGeometry(0.3, 0.5, 3, 8);
    const trunkMat = new THREE.MeshLambertMaterial({ color: 0x8B4513 });
    const trunk = new THREE.Mesh(trunkGeo, trunkMat);
    trunk.position.y = 1.5;
    trunk.castShadow = true;
    group.add(trunk);

    // Foliage layers (cone shapes)
    const foliageColors = [0x228B22, 0x2E8B57, 0x32CD32];
    const sizes = [{ r: 2, h: 3 }, { r: 1.5, h: 2.5 }, { r: 1, h: 2 }];
    let yOffset = 3;

    sizes.forEach((size, i) => {
      const foliageGeo = new THREE.ConeGeometry(size.r, size.h, 8);
      const foliageMat = new THREE.MeshLambertMaterial({ 
        color: foliageColors[i % foliageColors.length] 
      });
      const foliage = new THREE.Mesh(foliageGeo, foliageMat);
      foliage.position.y = yOffset;
      foliage.castShadow = true;
      group.add(foliage);
      yOffset += size.h * 0.6;
    });

    return group;
  }

  _createBush() {
    const group = new THREE.Group();
    const bushColors = [0x228B22, 0x32CD32, 0x3CB371];

    for (let i = 0; i < 5; i++) {
      const geo = new THREE.SphereGeometry(0.5 + Math.random() * 0.3, 8, 6);
      const mat = new THREE.MeshLambertMaterial({ 
        color: bushColors[Math.floor(Math.random() * bushColors.length)] 
      });
      const sphere = new THREE.Mesh(geo, mat);
      sphere.position.set(
        (Math.random() - 0.5) * 0.8,
        Math.random() * 0.3,
        (Math.random() - 0.5) * 0.8
      );
      sphere.castShadow = true;
      group.add(sphere);
    }

    return group;
  }

  _createRock() {
    const geo = new THREE.DodecahedronGeometry(1, 0);
    const mat = new THREE.MeshLambertMaterial({ color: 0x808080 });
    const rock = new THREE.Mesh(geo, mat);
    rock.scale.set(1, 0.6, 1);
    rock.castShadow = true;
    return rock;
  }

  _createClouds() {
    // Create 3D floating clouds
    for (let i = 0; i < 8; i++) {
      const cloud = this._createCloud();
      const angle = (i / 8) * Math.PI * 2;
      const dist = 80 + Math.random() * 40;
      cloud.position.set(
        Math.cos(angle) * dist,
        30 + Math.random() * 20,
        Math.sin(angle) * dist
      );
      cloud.scale.setScalar(2 + Math.random() * 2);
      this.sceneManager.add(cloud);
      this.clouds.push({ mesh: cloud, speed: 0.01 + Math.random() * 0.02, angle: angle });
    }
  }

  _createCloud() {
    const group = new THREE.Group();
    const cloudMat = new THREE.MeshLambertMaterial({ 
      color: 0xffffff, 
      transparent: true, 
      opacity: 0.8 
    });

    // Create puffy cloud from multiple spheres
    const positions = [
      { x: 0, y: 0, z: 0, s: 1 },
      { x: 1.2, y: 0.2, z: 0, s: 0.8 },
      { x: -1, y: 0.1, z: 0.3, s: 0.9 },
      { x: 0.5, y: 0.4, z: -0.3, s: 0.7 },
      { x: -0.5, y: 0.3, z: -0.2, s: 0.75 }
    ];

    positions.forEach(p => {
      const geo = new THREE.SphereGeometry(p.s, 8, 6);
      const sphere = new THREE.Mesh(geo, cloudMat);
      sphere.position.set(p.x, p.y, p.z);
      group.add(sphere);
    });

    return group;
  }

  _createParticles() {
    const geometry = new THREE.BufferGeometry();
    const positions = new Float32Array(this.particleCount * 3);
    const colors = new Float32Array(this.particleCount * 3);

    for (let i = 0; i < this.particleCount; i++) {
      positions[i * 3] = (Math.random() - 0.5) * 200;
      positions[i * 3 + 1] = Math.random() * 40 + 5;
      positions[i * 3 + 2] = (Math.random() - 0.5) * 200;

      // Warm golden particles (like fireflies/dust)
      colors[i * 3] = 1.0;
      colors[i * 3 + 1] = 0.9 + Math.random() * 0.1;
      colors[i * 3 + 2] = 0.6 + Math.random() * 0.2;

      this.particleVelocities.push({
        x: (Math.random() - 0.5) * 0.015,
        y: (Math.random() - 0.5) * 0.008,
        z: (Math.random() - 0.5) * 0.015
      });
    }

    geometry.addAttribute('position', new THREE.BufferAttribute(positions, 3));
    geometry.addAttribute('color', new THREE.BufferAttribute(colors, 3));

    const material = new THREE.PointsMaterial({
      size: 0.4,
      vertexColors: THREE.VertexColors,
      transparent: true,
      opacity: 0.7,
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

    // Animate particles
    if (this.particles) {
      const positions = this.particles.geometry.attributes.position.array;
      for (let i = 0; i < this.particleCount; i++) {
        const vel = this.particleVelocities[i];
        positions[i * 3] += vel.x;
        positions[i * 3 + 1] += vel.y + Math.sin(this.time * 2 + i) * 0.008;
        positions[i * 3 + 2] += vel.z;

        if (positions[i * 3] > 100) positions[i * 3] = -100;
        if (positions[i * 3] < -100) positions[i * 3] = 100;
        if (positions[i * 3 + 2] > 100) positions[i * 3 + 2] = -100;
        if (positions[i * 3 + 2] < -100) positions[i * 3 + 2] = 100;
        if (positions[i * 3 + 1] > 45) positions[i * 3 + 1] = 5;
        if (positions[i * 3 + 1] < 5) positions[i * 3 + 1] = 45;
      }
      this.particles.geometry.attributes.position.needsUpdate = true;
    }

    // Animate clouds
    this.clouds.forEach(cloudData => {
      cloudData.angle += cloudData.speed * 0.01;
      const dist = 80 + Math.sin(this.time + cloudData.angle) * 10;
      cloudData.mesh.position.x = Math.cos(cloudData.angle) * dist;
      cloudData.mesh.position.z = Math.sin(cloudData.angle) * dist;
      cloudData.mesh.position.y += Math.sin(this.time * 0.5) * 0.01;
    });

    // Slowly rotate sky
    if (this.skyMesh) {
      this.skyMesh.rotation.y += 0.00005;
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
    // Move decorations with the scene
    this.decorations.forEach(dec => {
      dec.position.x += x * 0.001;
      dec.position.z += z * 0.001;
    });
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

    this.decorations.forEach(dec => {
      this.sceneManager.remove(dec);
    });

    this.clouds.forEach(cloudData => {
      this.sceneManager.remove(cloudData.mesh);
    });
  }
}

export default BackgroundSystem;
