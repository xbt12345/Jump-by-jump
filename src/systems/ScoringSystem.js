// Version 8
import { GameConfig } from '../core/GameConfig.js';
import { GameEvents } from '../core/EventBus.js';

/**
 * Scoring:
 * - Land on center of platform: +2 points
 * - Land on edge of platform: +1 point
 * - Fall off: game over
 * - Landing on same platform: no points
 */
export class ScoringSystem {
    constructor(eventBus, gameState) {
        this.eventBus = eventBus;
        this.gameState = gameState;
        this.lastPlatformIndex = 0;
        this.centerCombo = 0;
        
        this.setupEventListeners();
    }

    setupEventListeners() {
        this.eventBus.on(GameEvents.PLAYER_LANDED, this.onPlayerLanded.bind(this));
        this.eventBus.on(GameEvents.PLAYER_FELL, this.onPlayerFell.bind(this));
    }

    onPlayerLanded({ distanceFromCenter, platformIndex }) {
        // Only score if moved to a NEW platform
        if (platformIndex <= this.lastPlatformIndex) {
            return;
        }
        
        this.lastPlatformIndex = platformIndex;
        
        const cfg = GameConfig.scoring;
        let score = 0;
        let rating = 'edge';
        
        // Ensure distance is valid
        const dist = (typeof distanceFromCenter === 'number' && !isNaN(distanceFromCenter))
            ? distanceFromCenter
            : 1;
        
        if (dist <= cfg.centerZone) {
            // Center landing
            this.centerCombo++;
            // First center = 2, consecutive centers = 2 * 2^(combo-1)
            score = cfg.centerScore * Math.pow(2, this.centerCombo - 1);
            rating = 'center';
        } else {
            // Edge landing
            score = cfg.edgeScore;
            rating = 'edge';
            this.centerCombo = 0;
        }
        
        // Safety check
        score = Math.floor(score);
        if (isNaN(score) || score < 1) score = 1;
        
        this.gameState.addScore(score);
        
        this.eventBus.emit('score:detail', { score, rating, combo: this.centerCombo });
    }

    onPlayerFell() {
        this.gameState.setGameOver();
    }

    reset() {
        this.lastPlatformIndex = 0;
        this.centerCombo = 0;
    }
}

console.log('ScoringSystem v8 loaded');
