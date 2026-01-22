"use strict";

let worker = null;

let module = null;
let memory = null;
let shouldStopPtr = null;

(async function() {
	module = await WebAssembly.compileStreaming(fetch('piqro.wasm'));
	
	worker = new Worker('worker.js');

	worker.onmessage = function(e) {
		const msg = e.data;
	
		if (msg.type == 'error') {
			alert('Error: ' + msg.details);
		}

		if (msg.type == 'memory') {
			memory = msg.memory;
			shouldStopPtr = msg.shouldStopPtr;
		}
	}

	worker.postMessage({ type: 'init', module: module });

	document.getElementById('run-button').onclick = function() {
		if (memory) {
			Atomics.store(memory, shouldStopPtr, true);
		}
	
		worker.postMessage({ type: 'run', source: document.getElementById('editor').value });
	}
	
	document.getElementById('export-button').onclick = function() {
		alert('Not implemented yet');
	}
	
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
})();