/*--------------------------------------------------------------------------
**
**  Copyright (c) 2023, Phil Claridge
**
**  console-text.js
**    This module provides classes that emulate text mode the Cyber console.
**
**--------------------------------------------------------------------------
*/

/*
 *  CyberConsoleText
 *
 *  This class emulates the Cyber console in 2d space via a text mode console.
 *
 *  The class is based on the terminal-kit library, and uses that library's character
 *  based ScreenBuffer.js to provide an off-screen character buffer so that only screen
 *  changes are sent to the terminal emulation.
 */


let CyberConsoleBase = require("../../www/js/console-base");
const terminalKit = require("terminal-kit");

class CyberConsoleText extends CyberConsoleBase {

  constructor() {
    super();
    this.xRatio = 1;
    this.yRatio = 1;
    this.fontWidths = [2, 8, 16, 32];
    this.charHeight = 10;
    this.charWidth = 8;
    //
    // Terminal instance
    //
    this.terminal = terminalKit.terminal;
    //
    // Console offsets
    //
    this.SCREEN_GAP = 40;
    this.SCREEN_MARGIN = 20;
    this.SCREEN_WIDTH_COLUMNS = 64;
    this.SCREEN_GAP_COLUMNS = 4;
    //
    // Screen control
    //
    this.screenBuffer = null;
    this.screenOffsetColumns = 0
    //
    // View selection: which screen(s) to show. Mirrors the presentation
    // control switch on a CC545 (F2/F3/F4 on a CC598B).
    //
    this.VIEWS = ['left', 'right', 'dual'];
    this.view = 'dual';
    this.currentScreen = 0;
    this.isVisible = true;
    this.forceVisible = false;
    //
    // Shutdown
    //
    this.shutdownListener = null;
  }

  setShutdownListener(callback) {
    this.shutdownListener = callback;
  }

  put(x, y, char, fg = 'brightgreen', bg = 'black') {
    if (this.screenBuffer && (this.isVisible || this.forceVisible)) {
      this.screenBuffer.put({x: x + this.screenOffsetColumns, y: y, attr: {color: fg, bgColor: bg}}, char);
    }
  }

  createScreenBuffer() {
    // this.reset();
    this.terminal.fullscreen(true);
    this.terminal.hideCursor(true);

    this.screenBuffer = new terminalKit.ScreenBuffer({
      dst: this.terminal,
      width: this.terminal.width,
      height: this.terminal.height,
      wrap: false,
      noFill: false
    });
    this.clearScreen();
    this.drawAllScreen();
  }


  drawAllScreen() {
    if (this.screenBuffer) {
      this.screenBuffer.draw({delta: false});
    }
  }

  drawPoint() {
    // Not supported
  }


  /*
      this.MEDIUM_FONT = 2;
    this.LARGE_FONT = 3;
   */

  drawChar(b) {
    let xCharGrid = Math.floor(this.x / this.charWidth);
    let yCharGrid = Math.floor(this.y / this.charHeight);
    this.put(xCharGrid, yCharGrid, String.fromCharCode(b))
    if (this.currentFont === this.SMALL_FONT) {
    } else if (this.currentFont === this.MEDIUM_FONT) {
      this.put(xCharGrid + 1, yCharGrid, ' ')
    } else if (this.currentFont === this.LARGE_FONT) {
      this.put(xCharGrid + 1, yCharGrid, ' ')
      this.put(xCharGrid + 2, yCharGrid, ' ')
      this.put(xCharGrid + 3, yCharGrid, ' ')
    }
  }

  setScreen(screenNumber) {
    this.currentScreen = screenNumber === 0 ? 0 : 1;
    if (this.view === 'dual') {
      this.screenOffsetColumns = this.currentScreen === 0 ? 0 : this.SCREEN_WIDTH_COLUMNS + this.SCREEN_GAP_COLUMNS;
      this.isVisible = true;
    } else {
      this.screenOffsetColumns = 0;
      this.isVisible = this.currentScreen === (this.view === 'left' ? 0 : 1);
    }
  }

  /*
   * Select which screen(s) are shown: 'left', 'right', or 'dual'.
   * NOS always sends both screens; this only affects what is rendered.
   */
  setView(view) {
    if (!this.VIEWS.includes(view)) {
      throw new Error(`View must be one of: ${this.VIEWS.join(', ')}`);
    }
    this.view = view;
    this.setScreen(this.currentScreen);
    this.clearScreen();
    this.drawAllScreen();
  }

  getView() {
    return this.view;
  }

  drawViewLabel() {
    if (this.screenBuffer && this.view !== 'dual') {
      let label = this.view === 'left' ? 'LEFT' : 'RIGHT';
      let x = this.SCREEN_WIDTH_COLUMNS + 2;
      for (let i = 0; i < label.length; i++) {
        this.screenBuffer.put({x: x + i, y: 0, attr: {color: 'brightgreen', bgColor: 'black'}}, label[i]);
      }
    }
  }

  displayNotification(font, x, y, s) {
    // Notifications are always shown, whichever screen is being viewed.
    this.forceVisible = true;
    try {
      super.displayNotification(font, x, y, s);
    } finally {
      this.forceVisible = false;
    }
  }

  clearScreen() {
    if (this.screenBuffer) {
      this.screenBuffer.fill({
        char: ' ',
        attr: {
          color: 'brightgreen',
          bgColor: 'black'
        }
      });
    }
  }

  updateScreen() {
    if (this.screenBuffer) {
      this.drawViewLabel();
      this.screenBuffer.draw({delta: true});
      this.clearScreen();
    }
  }

  shutdown() {
    this.screenBuffer = null;
    this.terminal.fullscreen(false);
    this.terminal.hideCursor(false);
  }

  processExit() {
    // Ensures console cleaned up
    this.terminal.processExit(0);
  }

  createScreen() {
    this.createScreenBuffer(); // Create initial screen buffer

    // Event listener for terminal resize
    this.terminal.on('resize', (width, height) => {
      if (this.screenBuffer) {
        this.screenBuffer.resize({width: width, height: height, x: 0, y: 0})
        this.clearScreen();
        this.drawAllScreen();
      }
    });

    this.terminal.grabInput({});

    this.terminal.on('key', (key /*, matches, data*/ ) => {
      switch (key) {
        case 'CTRL_R' :
          this.drawAllScreen();
          break;
        case 'F2' :
          this.setView('left');
          break;
        case 'F3' :
          this.setView('dual');
          break;
        case 'F4' :
          this.setView('right');
          break;
        case 'CTRL_C' :
        case 'CTRL_D' :
          if (this.shutdownListener) {
            this.shutdownListener();
          } else {
            this.processExit();
          }
          break;
        default:
          this.processKeyboardEvent(key, false, false, false);
          break;
      }
    });

    this.terminal.on('mouse', function (name, data) {
    });
  }

  reset() {
    super.reset()
  }
}

module.exports = CyberConsoleText
