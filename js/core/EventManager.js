import Helpers from '../utils/Helpers.js';

/**
 * EventManager - centralized input and event handling
 * Supports both mouse and touch events
 */
class EventManager {
  constructor(canvas) {
    this.canvas = canvas;
    this.inputEvents = Helpers.getInputEvents();
    this.listeners = new Map();
    this.boundHandlers = new Map();
  }

  /**
   * Register event listener
   * @param {string} eventType - 'down', 'up', 'resize'
   * @param {Function} callback - Event handler
   */
  on(eventType, callback) {
    if (!this.listeners.has(eventType)) {
      this.listeners.set(eventType, []);
    }
    this.listeners.get(eventType).push(callback);
  }

  /**
   * Remove event listener
   * @param {string} eventType - Event type
   * @param {Function} callback - Handler to remove
   */
  off(eventType, callback) {
    if (this.listeners.has(eventType)) {
      const callbacks = this.listeners.get(eventType);
      const index = callbacks.indexOf(callback);
      if (index > -1) {
        callbacks.splice(index, 1);
      }
    }
  }

  /**
   * Emit event to all listeners
   * @param {string} eventType - Event type
   * @param {Event} event - Original event object
   */
  emit(eventType, event) {
    if (this.listeners.has(eventType)) {
      this.listeners.get(eventType).forEach(callback => callback(event));
    }
  }

  /**
   * Start listening to events
   */
  startListening() {
    // Mouse/Touch down
    const downHandler = (e) => this.emit('down', e);
    this.boundHandlers.set('down', downHandler);
    this.canvas.addEventListener(this.inputEvents.down, downHandler);

    // Mouse/Touch up
    const upHandler = (e) => this.emit('up', e);
    this.boundHandlers.set('up', upHandler);
    this.canvas.addEventListener(this.inputEvents.up, upHandler);

    // Window resize
    const resizeHandler = () => this.emit('resize');
    this.boundHandlers.set('resize', resizeHandler);
    window.addEventListener('resize', resizeHandler, false);
  }

  /**
   * Stop listening to events
   */
  stopListening() {
    if (this.boundHandlers.has('down')) {
      this.canvas.removeEventListener(
        this.inputEvents.down,
        this.boundHandlers.get('down')
      );
    }
    if (this.boundHandlers.has('up')) {
      this.canvas.removeEventListener(
        this.inputEvents.up,
        this.boundHandlers.get('up')
      );
    }
    if (this.boundHandlers.has('resize')) {
      window.removeEventListener('resize', this.boundHandlers.get('resize'));
    }
    this.boundHandlers.clear();
  }

  /**
   * Dispose of event manager
   */
  dispose() {
    this.stopListening();
    this.listeners.clear();
  }
}

export default EventManager;
