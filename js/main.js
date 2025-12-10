/**
 * Main entry point for Jump Jump game
 * Works with both the legacy game.js and the new game.bundle.js
 */
window.onload = function() {
  var game = new Game();

  var startPage = document.querySelector('.startPage');
  var restartPage = document.querySelector('.restartPage');
  var startBtn = document.querySelector('.startBtn');
  var restartBtn = document.querySelector('.restartBtn');
  var scoreEl = document.querySelector('.scoreNum');

  // Initial UI state
  startPage.style.display = 'flex';
  restartPage.style.display = 'none';

  // Start game handler
  startBtn.addEventListener('click', function() {
    startPage.style.display = 'none';
    game.start();
  });

  // Restart game handler
  restartBtn.addEventListener('click', function() {
    restartPage.style.display = 'none';
    game.restart();
  });

  // Game over callback - supports both old and new API
  if (typeof game.onFail !== 'undefined') {
    // New API
    game.onFail = function(score) {
      restartPage.style.display = 'flex';
      scoreEl.innerHTML = score;
    };
  } else {
    // Legacy API
    game.failCallback = function(score) {
      restartPage.style.display = 'flex';
      scoreEl.innerHTML = score;
    };
  }
};
