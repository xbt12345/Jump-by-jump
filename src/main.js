// Jump Jump v11
import { Game } from './Game.js';
import { GameConfig } from './core/GameConfig.js';

console.log('========================================');
console.log(`  Jump Jump v${GameConfig.version}`);
console.log('  Hold SPACE to charge, release to jump');
console.log('========================================');

document.addEventListener('DOMContentLoaded', () => {
    console.log('DOM loaded, initializing game...');
    
    const canvas = document.getElementById('game-canvas');
    
    if (!canvas) {
        console.error('Canvas not found!');
        return;
    }
    
    try {
        const game = new Game(canvas);
        game.start();
        window.game = game;
        console.log('Game started successfully');
    } catch (error) {
        console.error('Failed to start game:', error);
    }
});
