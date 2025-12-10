import { GameEvents } from '../core/EventBus.js';

/**
 * UISystem - Manages all UI updates
 */
export class UISystem {
    constructor(eventBus) {
        this.eventBus = eventBus;
        
        // Cache DOM elements
        this.scoreElement = document.getElementById('score');
        this.powerBar = document.getElementById('power-bar');
        this.gameOverPanel = document.getElementById('game-over');
        this.finalScoreElement = document.getElementById('final-score');
        this.restartButton = document.getElementById('restart-btn');
        this.instructions = document.getElementById('instructions');
        
        this.setupEventListeners();
    }

    setupEventListeners() {
        // Listen to game events
        this.eventBus.on(GameEvents.CHARGE_UPDATE, this.onChargeUpdate.bind(this));
        this.eventBus.on(GameEvents.CHARGE_RELEASE, this.onChargeRelease.bind(this));
        this.eventBus.on(GameEvents.SCORE_UPDATE, this.onScoreUpdate.bind(this));
        this.eventBus.on(GameEvents.GAME_OVER, this.onGameOver.bind(this));
        
        // Restart button
        this.restartButton.addEventListener('click', () => {
            this.eventBus.emit(GameEvents.GAME_RESTART, {});
        });
    }

    onChargeUpdate({ normalized }) {
        this.powerBar.style.width = `${normalized * 100}%`;
        
        // Color gradient based on power
        if (normalized < 0.5) {
            this.powerBar.style.background = 'linear-gradient(90deg, #4facfe, #00f2fe)';
        } else if (normalized < 0.8) {
            this.powerBar.style.background = 'linear-gradient(90deg, #f093fb, #f5576c)';
        } else {
            this.powerBar.style.background = 'linear-gradient(90deg, #ff6b6b, #feca57)';
        }
    }

    onChargeRelease() {
        this.powerBar.style.width = '0%';
    }

    onScoreUpdate({ score, added }) {
        this.scoreElement.textContent = score;
        
        // Score pop animation
        this.scoreElement.style.transform = 'scale(1.3)';
        setTimeout(() => {
            this.scoreElement.style.transform = 'scale(1)';
        }, 150);
    }

    onGameOver({ score }) {
        this.finalScoreElement.textContent = score;
        this.gameOverPanel.classList.remove('hidden');
    }

    hideGameOver() {
        this.gameOverPanel.classList.add('hidden');
    }

    reset() {
        this.scoreElement.textContent = '0';
        this.powerBar.style.width = '0%';
        this.hideGameOver();
    }
}
