/**
 * Main entry point for Jump Jump game
 * Handles UI interactions and game lifecycle
 */
window.onload = function() {
  var game = new Game();

  // UI Elements
  var startPage = document.querySelector('.startPage');
  var restartPage = document.querySelector('.restartPage');
  var startBtn = document.querySelector('.startBtn');
  var restartBtn = document.querySelector('.restartBtn');
  var shareBtn = document.querySelector('.shareBtn');
  var scoreEl = document.querySelector('.scoreNum');
  var stars = document.querySelectorAll('.star');
  var messageEl = document.querySelector('.game-over-message');

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

  // Share button handler
  if (shareBtn) {
    shareBtn.addEventListener('click', function() {
      var score = scoreEl.innerHTML;
      var text = '我在跳一跳游戏中获得了 ' + score + ' 分！来挑战我吧！🎮';
      
      // Try to use Web Share API
      if (navigator.share) {
        navigator.share({
          title: 'Jump Jump 跳一跳',
          text: text,
          url: window.location.href
        }).catch(function() {
          // Fallback: copy to clipboard
          copyToClipboard(text);
        });
      } else {
        copyToClipboard(text);
      }
    });
  }

  function copyToClipboard(text) {
    var textarea = document.createElement('textarea');
    textarea.value = text;
    document.body.appendChild(textarea);
    textarea.select();
    document.execCommand('copy');
    document.body.removeChild(textarea);
    alert('成绩已复制到剪贴板！');
  }

  // Update stars based on score
  function updateStars(score) {
    var activeStars = 0;
    if (score >= 5) activeStars = 1;
    if (score >= 15) activeStars = 2;
    if (score >= 30) activeStars = 3;

    stars.forEach(function(star, index) {
      star.classList.remove('active');
      if (index < activeStars) {
        setTimeout(function() {
          star.classList.add('active');
        }, (index + 1) * 200);
      }
    });
  }

  // Get encouraging message based on score
  function getMessage(score) {
    if (score === 0) return '别灰心，再试一次！💪';
    if (score < 5) return '不错的开始！继续加油！';
    if (score < 15) return '很棒！你正在进步！🌟';
    if (score < 30) return '太厉害了！高手风范！🔥';
    if (score < 50) return '难以置信！你是跳跃大师！🏆';
    return '传奇级别！无人能敌！👑';
  }

  // Game over callback - set directly on failCallback for compatibility
  var gameOverHandler = function(score) {
    console.log('Game Over! Score:', score);
    restartPage.style.display = 'flex';
    scoreEl.innerHTML = score;
    updateStars(score);
    if (messageEl) {
      messageEl.innerHTML = getMessage(score);
    }
  };

  // Set callback using both methods for compatibility
  game.failCallback = gameOverHandler;
  if (typeof game.onFail !== 'undefined') {
    game.onFail = gameOverHandler;
  }
};
