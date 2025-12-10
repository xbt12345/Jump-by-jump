// ScoreManager v10 - Simplified scoring
// Center = 2^combo (2, 4, 8, 16...)
// Edge = 1 point, resets combo
import { GameConfig } from '../core/GameConfig.js';
import { GameEvents } from '../core/EventBus.js';

export class ScoreManager {
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

    onPlayerLanded({ platformIndex, distanceFromCenter }) {
        // Only score when landing on a NEW platform (moving forward)
        if (platformIndex <= this.lastPlatformIndex) {
            return;
        }
        
        this.lastPlatformIndex = platformIndex;
        
        const cfg = GameConfig.scoring;
        let score = 0;
        let rating = 'edge';
        
        // Ensure distance is valid
        const dist = (typeof distanceFromCenter === 'number' && !isNaN(distanceFromCenter))
            ? distanceFromCenter : 1;
        
        if (dist <= cfg.centerZone) {
            // Exact center - exponential scoring: 2^combo
            this.centerCombo++;
            score = Math.pow(2, this.centerCombo); // 2, 4, 8, 16...
            rating = 'center';
        } else {
            // Edge - 1 point, reset combo
            score = cfg.edgeScore;
            rating = 'edge';
            this.centerCombo = 0;
        }
        
        // Safety
        score = Math.floor(score);
        if (isNaN(score) || score < 1) score = 1;
        
        this.gameState.addScore(score);
        
        this.eventBus.emit('score:detail', {
            score,
            rating,
            combo: this.centerCombo,
            distanceFromCenter: dist
        });
    }

    onPlayerFell() {
        this.gameState.setGameOver();
    }

    reset() {
        this.lastPlatformIndex = 0;
        this.centerCombo = 0;
    }
}

console.log('ScoreManager v11');
