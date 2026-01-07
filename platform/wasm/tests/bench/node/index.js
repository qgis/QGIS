#!/usr/bin/env node

const { JSDOM } = require('jsdom');
const path = require('path');
const fs = require('fs');

console.log('Setting up browser environment with jsdom...');

// Create a jsdom instance with comprehensive browser API support
const dom = new JSDOM('<!DOCTYPE html><html><head></head><body></body></html>', {
  url: 'http://localhost/',
  pretendToBeVisual: true,
  resources: 'usable',
  runScripts: 'dangerously',
  beforeParse(window) {
    // Add matchMedia API (required by QWasmTheme for color scheme detection)
    window.matchMedia = window.matchMedia || function (query) {
      return {
        matches: false,  // Default to light mode
        media: query,
        onchange: null,
        addListener: function () { },  // Deprecated
        removeListener: function () { },  // Deprecated
        addEventListener: function () { },
        removeEventListener: function () { },
        dispatchEvent: function () { return true; }
      };
    };

    // Add visualViewport API (not included in jsdom by default)
    window.visualViewport = {
      width: 1920,
      height: 1080,
      scale: 1,
      offsetLeft: 0,
      offsetTop: 0,
      pageLeft: 0,
      pageTop: 0,
      addEventListener: (event, callback) => { },
      removeEventListener: (event, callback) => { },
      dispatchEvent: (event) => true
    };

    // Enhanced screen object for Qt
    Object.defineProperty(window.screen, 'orientation', {
      value: {
        type: 'landscape-primary',
        angle: 0,
        addEventListener: () => { },
        removeEventListener: () => { }
      },
      configurable: true
    });

    // Add devicePixelRatio if not present
    if (!window.devicePixelRatio) {
      window.devicePixelRatio = 1;
    }

    // Add requestAnimationFrame/cancelAnimationFrame
    let frameId = 0;
    window.requestAnimationFrame = window.requestAnimationFrame || function (callback) {
      const id = ++frameId;
      setTimeout(() => callback(Date.now()), 16);
      return id;
    };
    window.cancelAnimationFrame = window.cancelAnimationFrame || function (id) {
      clearTimeout(id);
    };

    // Redirect console output
    ['log', 'info', 'warn', 'error', 'debug'].forEach(method => {
      const original = console[method];
      window.console[method] = function (...args) {
        original.apply(console, ['[QGIS]', ...args]);
      };
    });
  }
});

// Expose window globals to Node.js global scope
const { window } = dom;

// Create a navigator proxy that adds maxTouchPoints and other Qt WASM requirements
// jsdom's navigator might not support adding properties directly
const navigatorProxy = new Proxy(window.navigator, {
  get(target, prop) {
    // Qt WASM required properties
    if (prop === 'maxTouchPoints') return 0;
    if (prop === 'hardwareConcurrency') return 4;
    if (prop === 'userActivation') return { isActive: false };
    if (prop === 'language') return target.language || 'en-US';
    if (prop === 'languages') return target.languages || ['en-US', 'en'];

    const value = target[prop];
    if (typeof value === 'function') {
      return value.bind(target);
    }
    return value;
  },
  has(target, prop) {
    if (['maxTouchPoints', 'hardwareConcurrency', 'userActivation', 'language', 'languages'].includes(prop)) {
      return true;
    }
    return prop in target;
  }
});

global.window = window;
global.document = window.document;
global.navigator = navigatorProxy;
global.screen = window.screen;
global.visualViewport = window.visualViewport;
global.location = window.location;
global.HTMLElement = window.HTMLElement;
global.HTMLCanvasElement = window.HTMLCanvasElement;
global.ImageData = window.ImageData;
global.requestAnimationFrame = window.requestAnimationFrame;
global.cancelAnimationFrame = window.cancelAnimationFrame;
global.devicePixelRatio = window.devicePixelRatio;

// Copy all other window properties to global and globalThis
Object.getOwnPropertyNames(window).forEach(property => {
  if (typeof global[property] === 'undefined') {
    global[property] = window[property];
  }
  if (typeof globalThis[property] === 'undefined') {
    globalThis[property] = window[property];
  }
});

// Explicitly ensure navigator is on globalThis (emscripten uses globalThis)
globalThis.navigator = navigatorProxy;
globalThis.window = window;
globalThis.document = window.document;
globalThis.screen = window.screen;

(async () => {
  console.log('Browser environment ready. Booting QGIS WebAssembly...');

  const qgisBenchPath = path.resolve(__dirname, '../../../../../../build/debug/output/bin/qgis_bench.js');
  const qtLoaderPath = path.resolve(__dirname, '../../../../../../build/debug/output/bin/qtloader.js');
  console.log('QGIS Bench path:', qgisBenchPath);
  console.log('Qt Loader path:', qtLoaderPath);

  if (!fs.existsSync(qgisBenchPath) || !fs.existsSync(qtLoaderPath)) {
    if (!fs.existsSync(qgisBenchPath)) {
      console.log(qgisBenchPath);
      console.error('Error: qgis_bench.js not found');
    }
    if (!fs.existsSync(qtLoaderPath)) {
      console.log(qtLoaderPath);
      console.error('Error: qtloader.js not found');
    }
    console.error('Please build QGIS first by running: ./build.sh');
    process.exit(1);
  }

  console.log('Loading QGIS WebAssembly...');
  const vm = require('vm');

  // Make require available globally for Emscripten-generated code
  global.require = require;
  global.module = module;
  global.__filename = __filename;
  global.__dirname = __dirname;

  let qgis_bench_entry;

  try {
    // qtloader.js just defines global functions, use vm
    const qtLoaderCode = fs.readFileSync(qtLoaderPath, 'utf8');
    vm.runInThisContext(qtLoaderCode, { filename: 'qtloader.js' });

    // qgis_bench.js exports qgis_bench_entry via module.exports
    qgis_bench_entry = require(qgisBenchPath);

    console.log("Scripts loaded");

    // Re-apply navigator proxy after loading emscripten module (it may have overwritten it)
    Object.defineProperty(globalThis, 'navigator', {
      value: navigatorProxy,
      writable: false,
      configurable: false
    });
    Object.defineProperty(global, 'navigator', {
      value: navigatorProxy,
      writable: false,
      configurable: false
    });

  } catch (error) {
    console.error('Error loading QGIS module:', error);
    process.exit(1);
  }

  try {
    console.log("Initializing QGIS Bench...");

    const args = [
      //'--help'
      
      '--parallel',
      '--iterations', '0',
      '--project', '/project/project.qgs',
      '--snapshot', '/download/snapshot.png'
      
    ]


    if (args.length > 0) {
      console.log('QGIS Bench arguments:', args);
    }

    // Directory containing the wasm files
    const wasmDir = path.dirname(qgisBenchPath);

    const instance = await global.qtLoad({
      qt: {
        onLoaded: () => {
          console.log('QGIS Bench loaded');
        },
        onExit: exitData => {
          console.log('QGIS Bench exited', exitData);
        },
        entryFunction: qgis_bench_entry,
        containerElements: [],
      },
      arguments: args,
      locateFile: (filename) => path.join(wasmDir, filename),
      preRun: [function (module) {
        console.log('QGIS Bench is starting...');

        // Create directories for project files and output
        try {
          module.FS.mkdir('/project');
          module.FS.mkdir('/download');
          console.log('Created /project and /download directories');

          // Copy project file and related files to virtual filesystem
          const projectPath = '/workspaces/qgis-wasm/qgis/tests/testdata/qgis_server_accesscontrol/project_shp.qgs';
          const projectDir = path.dirname(projectPath);

          // Read and copy the main project file
          const projectContent = fs.readFileSync(projectPath);
          module.FS.writeFile('/project/project.qgs', projectContent);
          console.log('Copied project file to /project/project.qgs');

          // Copy all shapefiles and related data files from the same directory
          const files = fs.readdirSync(projectDir);
          for (const file of files) {
            if (file.endsWith('.shp') || file.endsWith('.shx') || file.endsWith('.dbf') ||
              file.endsWith('.prj') || file.endsWith('.cpg') || file.endsWith('.qml') ||
              file.endsWith('.gpkg') || file.endsWith('.tif') || file.endsWith('.tiff')) {
              const filePath = path.join(projectDir, file);
              const fileContent = fs.readFileSync(filePath);
              module.FS.writeFile(`/project/${file}`, fileContent);
              console.log(`Copied ${file} to /project/`);
            }
          }
        } catch (e) {
          console.error('Failed to setup filesystem:', e);
        }
      }],
    });
  } catch (e) {
    console.log('QGIS Bench initialization failed');
    console.error(e);
    console.error(e.stack);
    process.exit(1);
  }
})();
