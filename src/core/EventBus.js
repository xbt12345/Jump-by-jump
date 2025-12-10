// EventBus - Decoupled pub/sub event system
export class EventBus {
    constructor() {
        this.listeners = new Map();
    }

    on(event, callback) {
        if (!this.listeners.has(event)) {
            this.listeners.set(event, []);
        }
        this.listeners.get(event).push(callback);
        return () => this.off(event, callback);
    }

    off(event, callback) {
        if (!this.listeners.has(event)) return;
        const callbacks = this.listeners.get(event);
        const index = callbacks.indexOf(callback);
        if (index > -1) callbacks.splice(index, 1);
    }

    emit(event, data) {
        if (!this.listeners.has(event)) return;
        this.listeners.get(event).forEach(cb => cb(data));
    }

    clear() {
        this.listeners.clear();
    }
}

// Game events
export const GameEvents = {
    // Input
    CHARGE_START: 'input:charge:start',
    CHARGE_UPDATE: 'input:charge:update',
    CHARGE_RELEASE: 'input:charge:release',
    
    // Player
    JUMP_START: 'player:jump:start',
    PLAYER_LANDED: 'player:landed',
    PLAYER_FELL: 'player:fell',
    
    // Platform
    PLATFORM_SPAWN: 'platform:spawn',
    PLATFORM_REMOVE: 'platform:remove',
    
    // Game state
    GAME_START: 'game:start',
    GAME_OVER: 'game:over',
    GAME_RESTART: 'game:restart',
    SCORE_UPDATE: 'score:update',
    
    // Audio/Beat
    BEAT_DETECTED: 'audio:beat',
    MUSIC_START: 'audio:start',
    MUSIC_END: 'audio:end'
};
