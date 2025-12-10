// GameState - Manages game state
import { GameEvents } from './EventBus.js';

export class GameState {
    constructor(eventBus) {
        this.eventBus = eventBus;
        this.reset();
    }

    reset() {
        this.score = 0;
        this.combo = 0;
        this.maxCombo = 0;
        this.isPlaying = true;
        this.isCharging = false;
        this.chargePower = 0;
        this.jumpCount = 0;
        this.perfectCount = 0;
        this.goodCount = 0;
    }

    addScore(points) {
        this.score += points;
        this.eventBus.emit(GameEvents.SCORE_UPDATE, { 
            score: this.score, 
            added: points,
            combo: this.combo 
        });
    }

    incrementCombo() {
        this.combo++;
        this.maxCombo = Math.max(this.maxCombo, this.combo);
    }

    resetCombo() {
        this.combo = 0;
    }

    getComboMultiplier(maxMultiplier = 3.0, step = 0.1) {
        return Math.min(maxMultiplier, 1 + this.combo * step);
    }

    setGameOver() {
        this.isPlaying = false;
        this.eventBus.emit(GameEvents.GAME_OVER, { 
            score: this.score,
            maxCombo: this.maxCombo,
            perfectCount: this.perfectCount,
            goodCount: this.goodCount
        });
    }

    setCharging(charging) {
        this.isCharging = charging;
    }

    setChargePower(power) {
        this.chargePower = power;
    }
}
