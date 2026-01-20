"use strict";

//
// worker
//

let worker = new Worker('worker.js');

worker.onmessage = function(e) {
	const msg = e.data;

	if (msg.type == 'error') {
		alert('Error: ' + msg.details);
	} else {
		console.error('Got invalid message type!');
	}
}

// callbacks
document.getElementById('run-button').onclick = function() {
	const textArea = document.getElementById('editor');

	worker.postMessage({ type: 'run', source: textArea.value });
}

// compile module and send it over to the worker thread
WebAssembly.compileStreaming(fetch('piqro.wasm')).then(function(module) {
	worker.postMessage({ type: 'wasm', module: module });
});

//
// rendering
//

// render once at start
let screenBuffer = {};

const canvas = document.getElementById('main-canvas');
const ctx = canvas.getContext('2d');

function renderCanvas() {
	if (screenBuffer) {
		ctx.fillStyle = 'black';

		ctx.clearRect(0, 0, canvas.width, canvas.height);
		ctx.fillRect(0, 0, canvas.width, canvas.height);

		screenBuffer = null;	
	} 

	requestAnimationFrame(renderCanvas);
}

renderCanvas();