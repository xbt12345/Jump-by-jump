import { GameConfig } from '../core/GameConfig.js';
import { GameEvents } from '../core/EventBus.js';

/**
 * InputSystem - Handles all user input
 */
export class InputSystem {
    constructor(eventBus, gameState) {
        this.eventBus = eventBus;
        this.gameState = gameState;
        this.isCharging = false;
        this.chargeStartTime = 0;
        this.chargePower = 0;
        
        this.setupEventListeners();
    }

    setupEventListeners() {
        // Keyboard events
        window.addEventListener('keydown', this.onKeyDown.bind(this));
        window.addEventListener('keyup', this.onKeyUp.bind(this));
        
        // Touch/mouse events for mobile support
        window.addEventListener('mousedown', this.onPointerDown.bind(this));
        window.addEventListener('mouseup', this.onPointerUp.bind(this));
        window.addEventListener('touchstart', this.onPointerDown.bind(this));
        window.addEventListener('touchend', this.onPointerUp.bind(this));
    }

    onKeyDown(event) {
        if (event.code === 'Space' && !event.repeat) {
            event.preventDefault();
            this.startCharge();
        }
    }

    onKeyUp(event) {
        if (event.code === 'Space') {
            event.preventDefault();
            this.releaseCharge();
        }
    }

    onPointerDown(event) {
        // Ignore if clicking UI elements
        if (event.target.tagName === 'BUTTON') return;
        this.startCharge();
    }

    onPointerUp(event) {
        this.releaseCharge();
    }

    startCharge() {
        if (!this.gameState.isPlaying || this.isCharging) return;
        
        this.isCharging = true;
        this.chargeStartTime = performance.now();
        this.chargePower = 0;
        this.gameState.setCharging(true);
        
        this.eventBus.emit(GameEvents.CHARGE_START, {});
    }

    releaseCharge() {
        if (!this.isCharging) return;
        
        this.isCharging = false;
        this.gameState.setCharging(false);
        
        const { minPower, maxPower } = GameConfig.player.jump;
        const power = Math.max(minPower, Math.min(this.chargePower, maxPower));
        
        this.eventBus.emit(GameEvents.CHARGE_RELEASE, { power });
        this.chargePower = 0;
    }

    update(deltaTime) {
        if (!this.isCharging) return;
        
        const { chargeRate, maxPower } = GameConfig.player.jump;
        this.chargePower = Math.min(this.chargePower + chargeRate * deltaTime, maxPower);
        this.gameState.setChargePower(this.chargePower);
        
        // Emit update for UI
        const normalizedPower = this.chargePower / maxPower;
        this.eventBus.emit(GameEvents.CHARGE_UPDATE, { 
            power: this.chargePower, 
            normalized: normalizedPower 
        });
    }

    dispose() {
        window.removeEventListener('keydown', this.onKeyDown);
        window.removeEventListener('keyup', this.onKeyUp);
        window.removeEventListener('mousedown', this.onPointerDown);
        window.removeEventListener('mouseup', this.onPointerUp);
        window.removeEventListener('touchstart', this.onPointerDown);
        window.removeEventListener('touchend', this.onPointerUp);
    }
}
