import GameConfig from '../utils/Constants.js';

/**
 * ScoreManager - handles scoring logic and UI
 * Implements center landing bonus with combo multiplier
 */
class ScoreManager {
  constructor() {
    this.score = 0;
    this.centerCombo = 0;  // Consecutive center landings
    this.scoreElement = null;
    this.bonusElement = null;
    this.config = GameConfig.scoring;
    
    this._initUI();
  }

  _initUI() {
    // Create score display element
    this.scoreElement = document.createElement('div');
    this.scoreElement.id = 'score';
    this.scoreElement.innerHTML = '0';
    document.body.appendChild(this.scoreElement);
    
    // Create bonus animation element
    this.bonusElement = document.createElement('div');
    this.bonusElement.id = 'bonus-popup';
    this.bonusElement.style.cssText = `
      position: fixed;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      font-size: 48px;
      font-weight: bold;
      color: #FFD700;
      text-shadow: 2px 2px 4px rgba(0,0,0,0.5);
      opacity: 0;
      pointer-events: none;
      z-index: 1000;
      transition: all 0.3s ease-out;
    `;
    document.body.appendChild(this.bonusElement);
  }

  /**
   * Add points to score
   * @param {string} landingType - Type of landing ('center' or 'normal')
   * @param {Object} landingData - Additional data for scoring
   */
  addScore(landingType = 'normal', landingData = {}) {
    let points = 0;
    
    if (landingType === 'center') {
      // Center landing: 2 points base, increases by 2 for each consecutive center
      this.centerCombo++;
      points = this.config.centerScore * this.centerCombo;
      this._showCenterAnimation(points);
    } else {
      // Normal landing: 1 point, resets combo
      this.centerCombo = 0;
      points = this.config.baseScore;
    }
    
    this.score += points;
    this._updateUI();
    
    return points;
  }

  /**
   * Show center landing animation
   */
  _showCenterAnimation(points) {
    if (!this.bonusElement) return;
    
    // Set text with combo indicator
    let text = `+${points}`;
    if (this.centerCombo > 1) {
      text += ` x${this.centerCombo} COMBO!`;
    } else {
      text += ' CENTER!';
    }
    
    this.bonusElement.innerHTML = text;
    this.bonusElement.style.opacity = '1';
    this.bonusElement.style.transform = 'translate(-50%, -50%) scale(1.2)';
    
    // Animate out
    setTimeout(() => {
      this.bonusElement.style.opacity = '0';
      this.bonusElement.style.transform = 'translate(-50%, -80%) scale(0.8)';
    }, 800);
  }

  /**
   * Get current score
   */
  getScore() {
    return this.score;
  }

  /**
   * Get current center combo
   */
  getCombo() {
    return this.centerCombo;
  }

  /**
   * Reset score to zero
   */
  reset() {
    this.score = 0;
    this.centerCombo = 0;
    this._updateUI();
  }

  /**
   * Update the score display
   */
  _updateUI() {
    if (this.scoreElement) {
      this.scoreElement.innerHTML = this.score;
    }
  }

  /**
   * Dispose of score manager
   */
  dispose() {
    if (this.scoreElement && this.scoreElement.parentNode) {
      this.scoreElement.parentNode.removeChild(this.scoreElement);
    }
    if (this.bonusElement && this.bonusElement.parentNode) {
      this.bonusElement.parentNode.removeChild(this.bonusElement);
    }
  }
}

export default ScoreManager;
