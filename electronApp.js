const { app, BrowserWindow, ipcMain, globalShortcut, dialog } = require('electron');
const { autoUpdater } = require('electron-updater');
const path = require('path');
const fs = require('fs');

let mainWindow;
let termWindow;
let factoryWindow;
let promptWindow;
let promptOptions;
let promptAnswer;

autoUpdater.autoDownload = false;
autoUpdater.logger = null;

function createWindow() {
  const iconPath = path.join(__dirname, 'www', 'media', 'app.ico');
  const indexPath = path.join(__dirname, 'www', 'index.html');

  if (!fs.existsSync(indexPath)) {
    dialog.showErrorBox('Error', 'No se encontró www/index.html');
    app.quit();
    return;
  }

  mainWindow = new BrowserWindow({
    width: 1240,
    height: 700,
    icon: iconPath,
    frame: false,
    movable: true,
    webPreferences: {
      nodeIntegration: true,
      contextIsolation: false
    }
  });

  const urlParam = process.platform === 'win32' && process.argv.length >= 2
    ? `?url=${process.argv[1]}`
    : '';

  mainWindow.loadURL(`file://${indexPath}${urlParam}`);
  mainWindow.setMenu(null);

  mainWindow.on('closed', () => {
    mainWindow = null;
  });

  const SerialPort = require('serialport');

  SerialPort.list().then(ports => {
    console.log('Puertos detectados:');
    ports.forEach(port => console.log(port.path));
  }).catch(err => {
    console.error('Error al listar puertos:', err);
  });


}

function createTerm() {
  const termPath = path.join(__dirname, 'www', 'term.html');
  termWindow = new BrowserWindow({
    width: 640,
    height: 560,
    parent: mainWindow,
    resizable: false,
    movable: true,
    frame: false,
    modal: true
  });
  termWindow.loadURL(`file://${termPath}`);
  termWindow.setMenu(null);
  termWindow.on('closed', () => { termWindow = null; });
}

function createRepl() {
  const replPath = path.join(__dirname, 'www', 'repl.html');
  termWindow = new BrowserWindow({
    width: 640,
    height: 515,
    parent: mainWindow,
    resizable: false,
    movable: true,
    frame: false,
    modal: true
  });
  termWindow.loadURL(`file://${replPath}`);
  termWindow.setMenu(null);
  termWindow.on('closed', () => { termWindow = null; });
}

function createFactory() {
  const factoryPath = path.join(__dirname, 'www', 'factory.html');
  factoryWindow = new BrowserWindow({
    width: 1066,
    height: 640,
    parent: mainWindow,
    resizable: true,
    movable: true,
    frame: false,
    webPreferences: {
      nodeIntegration: true,
      enableRemoteModule: true,
      contextIsolation: false
    }
  });
  factoryWindow.loadURL(`file://${factoryPath}`);
  factoryWindow.setMenu(null);
  factoryWindow.on('closed', () => { factoryWindow = null; });
}

function promptModal(options, callback) {
  const modalPath = path.join(__dirname, 'www', 'modalVar.html');
  promptOptions = options;
  promptWindow = new BrowserWindow({
    width: 360,
    height: 135,
    parent: mainWindow,
    resizable: false,
    movable: true,
    frame: false,
    modal: true
  });
  promptWindow.loadURL(`file://${modalPath}`);
  promptWindow.on('closed', () => {
    promptWindow = null;
    callback(promptAnswer);
  });
}

function open_console(win = BrowserWindow.getFocusedWindow()) {
  if (win) win.webContents.toggleDevTools();
}

function refresh(win = BrowserWindow.getFocusedWindow()) {
  if (win) win.webContents.reloadIgnoringCache();
}

app.on('ready', () => {
  createWindow();
  globalShortcut.register('F8', open_console);
  globalShortcut.register('F5', refresh);
});

app.on('activate', () => {
  if (mainWindow === null) createWindow();
});

app.on('window-all-closed', () => {
  globalShortcut.unregisterAll();
  if (process.platform !== 'darwin') app.quit();
});

// IPC handlers
ipcMain.on('version', () => autoUpdater.checkForUpdates());
ipcMain.on('prompt', () => createTerm());
ipcMain.on('repl', () => createRepl());
ipcMain.on('factory', () => createFactory());

ipcMain.on('openDialog', (event) => {
  event.returnValue = JSON.stringify(promptOptions, null, '');
});

ipcMain.on('closeDialog', (event, data) => {
  promptAnswer = data;
});

ipcMain.on('modalVar', (event, arg) => {
  promptModal({ label: arg, value: '', ok: 'OK' }, (data) => {
    event.returnValue = data;
  });
});

// Save dialogs
const saveHandlers = {
  'save-bin': { title: 'Exporter les binaires', defaultPath: 'Otto_hex', ext: 'hex' },
  'save-ino': { title: 'Save format .INO', defaultPath: 'Otto_Arduino', ext: 'ino' },
  'save-py':  { title: 'Save format .PY', defaultPath: 'Otto_python', ext: 'py' },
  'save-bloc':{ title: 'Save format .BLOC', defaultPath: 'Otto_block', ext: 'bloc' },
  'save-csv': { title: 'Save format CSV', defaultPath: 'Otto_csv', ext: 'csv' }
};

for (const [channel, { title, defaultPath, ext }] of Object.entries(saveHandlers)) {
  ipcMain.on(channel, (event) => {
    dialog.showSaveDialog(mainWindow, {
      title,
      defaultPath,
      filters: [{ name: ext, extensions: [ext] }]
    }, (filename) => {
      event.sender.send(`saved-${ext}`, filename);
    });
  });
}

// AutoUpdater events
autoUpdater.on('error', (error) => {
  dialog.showErrorBox('Error', error ? (error.stack || error).toString() : 'unknown');
});

autoUpdater.on('update-available', () => {
  dialog.showMessageBox(mainWindow, {
    type: 'none',
    title: 'Update',
    message: 'A new version is available, do you want to download and install it now?',
    buttons: ['Yes', 'No'],
    cancelId: 1,
    noLink: true
  }, (buttonIndex) => {
    if (buttonIndex === 0) autoUpdater.downloadUpdate();
  });
});

autoUpdater.on('update-not-available', () => {
  dialog.showMessageBox(mainWindow, {
    title: 'Updated',
    message: 'Your version is up to date.'
  });
});

autoUpdater.on('update-downloaded', () => {
  dialog.showMessageBox(mainWindow, {
    title: 'Updated',
    message: 'Download finished, the application will install then restart.'
  }, () => {
    setImmediate(() => autoUpdater.quitAndInstall());
  });
});




module.exports.open_console = open_console;
module.exports.refresh = refresh;