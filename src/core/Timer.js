// Timer - High-precision game time, can sync with audio
export class Timer {
    constructor() {
        this.startTime = 0;
        this.pausedTime = 0;
        this.isPaused = false;
        this.audioContext = null;
    }

    setAudioContext(ctx) {
        this.audioContext = ctx;
    }

    start() {
        this.startTime = performance.now();
        this.isPaused = false;
    }

    pause() {
        if (!this.isPaused) {
            this.pausedTime = this.getTime();
            this.isPaused = true;
        }
    }

    resume() {
        if (this.isPaused) {
            this.startTime = performance.now() - this.pausedTime * 1000;
            this.isPaused = false;
        }
    }

    reset() {
        this.startTime = performance.now();
        this.pausedTime = 0;
        this.isPaused = false;
    }

    // Returns time in seconds
    getTime() {
        if (this.isPaused) return this.pausedTime;
        
        // Use audio context time if available for precise sync
        if (this.audioContext) {
            return this.audioContext.currentTime;
        }
        
        return (performance.now() - this.startTime) / 1000;
    }
}
