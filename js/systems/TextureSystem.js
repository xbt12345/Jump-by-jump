/**
 * TextureSystem - Realistic matte plastic textures
 * Creates textures that look like real plastic game pieces
 */
class TextureSystem {
  constructor() {
    this.textureCache = new Map();
    this.platformColors = [
      { base: '#4CAF50', accent: '#388E3C', highlight: '#81C784' }, // Green plastic
      { base: '#2196F3', accent: '#1976D2', highlight: '#64B5F6' }, // Blue plastic
      { base: '#9C27B0', accent: '#7B1FA2', highlight: '#BA68C8' }, // Purple plastic
      { base: '#F44336', accent: '#D32F2F', highlight: '#E57373' }, // Red plastic
      { base: '#FF9800', accent: '#F57C00', highlight: '#FFB74D' }, // Orange plastic
      { base: '#00BCD4', accent: '#0097A7', highlight: '#4DD0E1' }, // Cyan plastic
      { base: '#795548', accent: '#5D4037', highlight: '#A1887F' }, // Brown plastic
      { base: '#607D8B', accent: '#455A64', highlight: '#90A4AE' }, // Gray plastic
    ];
    this.colorIndex = 0;
  }

  /**
   * Create a procedural texture using canvas
   */
  _createCanvasTexture(width, height, drawFunc) {
    const canvas = document.createElement('canvas');
    canvas.width = width;
    canvas.height = height;
    const ctx = canvas.getContext('2d');
    drawFunc(ctx, width, height);
    
    const texture = new THREE.CanvasTexture(canvas);
    texture.wrapS = THREE.RepeatWrapping;
    texture.wrapT = THREE.RepeatWrapping;
    return texture;
  }

  /**
   * Add subtle plastic grain texture
   */
  _addPlasticGrain(ctx, w, h, intensity = 8) {
    const imageData = ctx.getImageData(0, 0, w, h);
    const data = imageData.data;
    
    for (let i = 0; i < data.length; i += 4) {
      // Very subtle noise for plastic grain
      const noise = (Math.random() - 0.5) * intensity;
      data[i] = Math.max(0, Math.min(255, data[i] + noise));
      data[i + 1] = Math.max(0, Math.min(255, data[i + 1] + noise));
      data[i + 2] = Math.max(0, Math.min(255, data[i + 2] + noise));
    }
    ctx.putImageData(imageData, 0, 0);
  }

  /**
   * Add subtle surface imperfections for realism
   */
  _addSurfaceImperfections(ctx, w, h) {
    // Very subtle scratches
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.02)';
    ctx.lineWidth = 0.5;
    for (let i = 0; i < 15; i++) {
      const x1 = Math.random() * w;
      const y1 = Math.random() * h;
      const length = Math.random() * 30 + 10;
      const angle = Math.random() * Math.PI;
      ctx.beginPath();
      ctx.moveTo(x1, y1);
      ctx.lineTo(x1 + Math.cos(angle) * length, y1 + Math.sin(angle) * length);
      ctx.stroke();
    }

    // Tiny dust specks
    ctx.fillStyle = 'rgba(0, 0, 0, 0.015)';
    for (let i = 0; i < 30; i++) {
      const x = Math.random() * w;
      const y = Math.random() * h;
      const r = Math.random() * 1.5 + 0.5;
      ctx.beginPath();
      ctx.arc(x, y, r, 0, Math.PI * 2);
      ctx.fill();
    }
  }

  /**
   * Generate realistic matte plastic platform texture
   */
  createPlatformTexture(type = 'cube') {
    const colors = this.platformColors[this.colorIndex % this.platformColors.length];
    this.colorIndex++;

    return this._createCanvasTexture(512, 512, (ctx, w, h) => {
      // Base color - solid matte plastic look
      ctx.fillStyle = colors.base;
      ctx.fillRect(0, 0, w, h);

      // Subtle gradient for depth (very subtle for matte look)
      const baseGradient = ctx.createLinearGradient(0, 0, w * 0.7, h * 0.7);
      baseGradient.addColorStop(0, colors.highlight + '20'); // Very transparent
      baseGradient.addColorStop(0.5, 'transparent');
      baseGradient.addColorStop(1, colors.accent + '30');
      ctx.fillStyle = baseGradient;
      ctx.fillRect(0, 0, w, h);

      // Top surface highlight (soft, diffuse - not shiny)
      const topHighlight = ctx.createLinearGradient(0, 0, 0, h * 0.25);
      topHighlight.addColorStop(0, 'rgba(255, 255, 255, 0.15)');
      topHighlight.addColorStop(1, 'rgba(255, 255, 255, 0)');
      ctx.fillStyle = topHighlight;
      ctx.fillRect(0, 0, w, h * 0.25);

      // Left edge subtle highlight
      const leftHighlight = ctx.createLinearGradient(0, 0, w * 0.15, 0);
      leftHighlight.addColorStop(0, 'rgba(255, 255, 255, 0.08)');
      leftHighlight.addColorStop(1, 'rgba(255, 255, 255, 0)');
      ctx.fillStyle = leftHighlight;
      ctx.fillRect(0, 0, w * 0.15, h);

      // Bottom edge shadow (soft ambient occlusion)
      const bottomShadow = ctx.createLinearGradient(0, h * 0.8, 0, h);
      bottomShadow.addColorStop(0, 'rgba(0, 0, 0, 0)');
      bottomShadow.addColorStop(1, 'rgba(0, 0, 0, 0.15)');
      ctx.fillStyle = bottomShadow;
      ctx.fillRect(0, h * 0.8, w, h * 0.2);

      // Right edge shadow
      const rightShadow = ctx.createLinearGradient(w * 0.85, 0, w, 0);
      rightShadow.addColorStop(0, 'rgba(0, 0, 0, 0)');
      rightShadow.addColorStop(1, 'rgba(0, 0, 0, 0.1)');
      ctx.fillStyle = rightShadow;
      ctx.fillRect(w * 0.85, 0, w * 0.15, h);

      // Subtle embossed grid pattern (like injection molded plastic)
      ctx.strokeStyle = 'rgba(0, 0, 0, 0.03)';
      ctx.lineWidth = 1;
      const gridSize = 64;
      for (let x = gridSize; x < w; x += gridSize) {
        ctx.beginPath();
        ctx.moveTo(x, 0);
        ctx.lineTo(x, h);
        ctx.stroke();
      }
      for (let y = gridSize; y < h; y += gridSize) {
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(w, y);
        ctx.stroke();
      }

      // Grid highlight (raised edge effect)
      ctx.strokeStyle = 'rgba(255, 255, 255, 0.02)';
      for (let x = gridSize - 1; x < w; x += gridSize) {
        ctx.beginPath();
        ctx.moveTo(x, 0);
        ctx.lineTo(x, h);
        ctx.stroke();
      }
      for (let y = gridSize - 1; y < h; y += gridSize) {
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(w, y);
        ctx.stroke();
      }

      // Add plastic grain and imperfections
      this._addPlasticGrain(ctx, w, h, 6);
      this._addSurfaceImperfections(ctx, w, h);
    });
  }

  /**
   * Generate realistic matte plastic jumper texture (chess piece style)
   */
  createJumperTexture() {
    return this._createCanvasTexture(512, 1024, (ctx, w, h) => {
      // Base golden/yellow plastic color
      const baseColor = '#FFC107';
      const darkColor = '#FF8F00';
      const lightColor = '#FFE082';

      // Solid base
      ctx.fillStyle = baseColor;
      ctx.fillRect(0, 0, w, h);

      // Vertical gradient for cylindrical shape illusion
      const cylinderGradient = ctx.createLinearGradient(0, 0, w, 0);
      cylinderGradient.addColorStop(0, darkColor + '60');
      cylinderGradient.addColorStop(0.2, 'transparent');
      cylinderGradient.addColorStop(0.35, lightColor + '30');
      cylinderGradient.addColorStop(0.5, lightColor + '20');
      cylinderGradient.addColorStop(0.65, 'transparent');
      cylinderGradient.addColorStop(0.8, 'transparent');
      cylinderGradient.addColorStop(1, darkColor + '50');
      ctx.fillStyle = cylinderGradient;
      ctx.fillRect(0, 0, w, h);

      // Horizontal bands (like a chess piece)
      for (let y = 0; y < h; y += 200) {
        // Dark band
        ctx.fillStyle = 'rgba(0, 0, 0, 0.08)';
        ctx.fillRect(0, y, w, 6);
        // Light edge
        ctx.fillStyle = 'rgba(255, 255, 255, 0.1)';
        ctx.fillRect(0, y + 6, w, 2);
      }

      // Top dome highlight (soft, matte)
      const topHighlight = ctx.createLinearGradient(0, 0, 0, h * 0.2);
      topHighlight.addColorStop(0, 'rgba(255, 255, 255, 0.25)');
      topHighlight.addColorStop(0.5, 'rgba(255, 255, 255, 0.1)');
      topHighlight.addColorStop(1, 'rgba(255, 255, 255, 0)');
      ctx.fillStyle = topHighlight;
      ctx.fillRect(0, 0, w, h * 0.2);

      // Bottom shadow (base of piece)
      const bottomShadow = ctx.createLinearGradient(0, h * 0.85, 0, h);
      bottomShadow.addColorStop(0, 'rgba(0, 0, 0, 0)');
      bottomShadow.addColorStop(1, 'rgba(0, 0, 0, 0.2)');
      ctx.fillStyle = bottomShadow;
      ctx.fillRect(0, h * 0.85, w, h * 0.15);

      // Add plastic grain
      this._addPlasticGrain(ctx, w, h, 5);
      this._addSurfaceImperfections(ctx, w, h);
    });
  }

  /**
   * Create material for platform with matte plastic texture
   */
  createPlatformMaterial(type = 'cube') {
    const texture = this.createPlatformTexture(type);
    return new THREE.MeshLambertMaterial({
      map: texture
    });
  }

  /**
   * Create material for jumper with matte plastic texture
   */
  createJumperMaterial() {
    const texture = this.createJumperTexture();
    return new THREE.MeshLambertMaterial({
      map: texture
    });
  }

  /**
   * Reset color index for consistent platform colors on restart
   */
  reset() {
    this.colorIndex = 0;
  }

  dispose() {
    this.textureCache.forEach(texture => texture.dispose());
    this.textureCache.clear();
  }
}

export default TextureSystem;
