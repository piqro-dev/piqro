"use strict";

const CANVAS_WIDTH = 240;
const CANVAS_HEIGHT = 320;

let worker = null;
let module = null;

let memory = null;
let shouldStopPtr = null;
let canvasBuffer = null;

console.log('Guten morgen');

const ctx = document.getElementById('main-canvas').getContext('2d');

ctx.fillStyle = 'black';
ctx.imageSmoothingEnabled = false;

ctx.fillRect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT);
ctx.scale(2, 2);

WebAssembly.compileStreaming(fetch('piqro.wasm')).then(function(m) {
	module = m;

	worker = new Worker('worker.js');
	
	worker.onmessage = async function(e) {
		const msg = e.data;

		if (msg.type == 'error') {
			alert('Error: ' + msg.details);
		}
	
		if (msg.type == 'postInit') {
			memory = msg.memory;
			shouldStopPtr = msg.shouldStopPtr;
		}
	
		if (msg.type == 'canvas') {
			canvasBuffer = msg.buffer;
		}
	}
	
	document.getElementById('run-button').onclick = function() {
		if (memory) {
			Atomics.store(memory, shouldStopPtr, true);
		}
	
		worker.postMessage({ type: 'run', source: document.getElementById('editor').value });
	}
	
	document.getElementById('export-button').onclick = function() {
		alert('Not implemented yet');
	}

	worker.postMessage({ type: 'init', module: module });

	function renderCanvas() {
		if (canvasBuffer) {
			let rgba = new Uint8ClampedArray(CANVAS_WIDTH * CANVAS_HEIGHT * 4);
	
			for (let i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT * 4; i += 4) {
				rgba[i + 0] = memory[canvasBuffer + (i / 4)];
				rgba[i + 1] = memory[canvasBuffer + (i / 4)];
				rgba[i + 2] = memory[canvasBuffer + (i / 4)];
				rgba[i + 3] = 0xff;
			}
	
			createImageBitmap(new ImageData(rgba, CANVAS_WIDTH, CANVAS_HEIGHT)).then(function(img) {
				ctx.drawImage(img, 0, 0);
			});	
		}

		setTimeout(function() {
			requestAnimationFrame(renderCanvas);
		}, 1000.0 / 30.0);
	}

	renderCanvas();
});