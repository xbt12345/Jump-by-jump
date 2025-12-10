/**
 * Utility helper functions
 */
const Helpers = {
  /**
   * Get random value between min and max (returns float)
   */
  getRandomValue(min, max) {
    return min + Math.random() * (max - min);
  },

  /**
   * Get random boolean with optional probability
   */
  getRandomBoolean(probability = 0.5) {
    return Math.random() > probability;
  },

  /**
   * Detect if running on PC or mobile
   */
  isPC() {
    const userAgentInfo = navigator.userAgent;
    const mobileAgents = ['Android', 'iPhone', 'SymbianOS', 'Windows Phone', 'iPad', 'iPod'];
    return !mobileAgents.some(agent => userAgentInfo.includes(agent));
  },

  /**
   * Get appropriate mouse/touch events based on device
   */
  getInputEvents() {
    const isPC = this.isPC();
    return {
      down: isPC ? 'mousedown' : 'touchstart',
      up: isPC ? 'mouseup' : 'touchend'
    };
  },

  /**
   * Validate position is not NaN
   */
  validatePosition(position) {
    if (isNaN(position.x) || isNaN(position.y) || isNaN(position.z)) {
      console.warn('Invalid position detected:', position);
      return false;
    }
    return true;
  },

  /**
   * Lerp between two values
   */
  lerp(start, end, factor) {
    return start + (end - start) * factor;
  },

  /**
   * Clamp value between min and max
   */
  clamp(value, min, max) {
    return Math.max(min, Math.min(max, value));
  }
};

export default Helpers;
