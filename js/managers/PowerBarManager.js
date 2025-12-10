import GameConfig from '../utils/Constants.js';

/**
 * PowerBarManager - displays a visual power bar during jump charging
 */
class PowerBarManager {
  constructor() {
    this.config = GameConfig.physics;
    this.container = null;
    this.bar = null;
    this.isVisible = false;
    
    this._initUI();
  }

  _initUI() {
    // Create container
    this.container = document.createElement('div');
    this.container.id = 'power-bar-container';
    this.container.style.cssText = `
      position: fixed;
      bottom: 80px;
      left: 50%;
      transform: translateX(-50%);
      width: 200px;
      height: 20px;
      background: rgba(0, 0, 0, 0.5);
      border-radius: 10px;
      border: 2px solid #fff;
      overflow: hidden;
      opacity: 0;
      transition: opacity 0.2s ease;
      z-index: 100;
    `;
    
    // Create power bar fill
    this.bar = document.createElement('div');
    this.bar.id = 'power-bar';
    this.bar.style.cssText = `
      width: 0%;
      height: 100%;
      background: linear-gradient(90deg, #4CAF50, #FFEB3B, #FF5722);
      border-radius: 8px;
      transition: width 0.05s linear;
    `;
    
    this.container.appendChild(this.bar);
    document.body.appendChild(this.container);
  }

  /**
   * Show the power bar
   */
  show() {
    if (!this.isVisible) {
      this.isVisible = true;
      this.container.style.opacity = '1';
      this.bar.style.width = '0%';
    }
  }

  /**
   * Hide the power bar
   */
  hide() {
    if (this.isVisible) {
      this.isVisible = false;
      this.container.style.opacity = '0';
      // Reset after fade out
      setTimeout(() => {
        this.bar.style.width = '0%';
      }, 200);
    }
  }

  /**
   * Update power bar based on current charge
   * @param {number} currentSpeed - Current x speed
   */
  update(currentSpeed) {
    const maxSpeed = this.config.maxXSpeed;
    const percentage = Math.min((currentSpeed / maxSpeed) * 100, 100);
    this.bar.style.width = `${percentage}%`;
    
    // Change color based on power level
    if (percentage < 33) {
      this.bar.style.background = 'linear-gradient(90deg, #4CAF50, #4CAF50)';
    } else if (percentage < 66) {
      this.bar.style.background = 'linear-gradient(90deg, #4CAF50, #FFEB3B)';
    } else {
      this.bar.style.background = 'linear-gradient(90deg, #4CAF50, #FFEB3B, #FF5722)';
    }
  }

  /**
   * Get current power percentage
   */
  getPowerPercentage(currentSpeed) {
    return Math.min((currentSpeed / this.config.maxXSpeed) * 100, 100);
  }

  /**
   * Dispose of power bar
   */
  dispose() {
    if (this.container && this.container.parentNode) {
      this.container.parentNode.removeChild(this.container);
    }
  }
}

export default PowerBarManager;
