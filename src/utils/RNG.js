// Seeded Random Number Generator
// Allows deterministic platform generation for testing and fairness
export class RNG {
    constructor(seed = 12345) {
        this.seed = seed;
        this.current = seed;
    }

    // Mulberry32 algorithm - fast and good distribution
    next() {
        let t = this.current += 0x6D2B79F5;
        t = Math.imul(t ^ t >>> 15, t | 1);
        t ^= t + Math.imul(t ^ t >>> 7, t | 61);
        return ((t ^ t >>> 14) >>> 0) / 4294967296;
    }

    // Returns float in [0, 1)
    nextFloat() {
        return this.next();
    }

    // Returns float in [min, max)
    nextFloatRange(min, max) {
        return min + this.next() * (max - min);
    }

    // Returns integer in [min, max]
    nextInt(min, max) {
        return Math.floor(min + this.next() * (max - min + 1));
    }

    // Returns true with given probability
    nextBool(probability = 0.5) {
        return this.next() < probability;
    }

    // Pick random element from array
    pick(array) {
        return array[Math.floor(this.next() * array.length)];
    }

    // Reset to initial seed
    reset() {
        this.current = this.seed;
    }

    // Set new seed
    setSeed(seed) {
        this.seed = seed;
        this.current = seed;
    }
}
