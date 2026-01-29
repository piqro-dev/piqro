'use strict';

const CANVAS_WIDTH = 240;
const CANVAS_HEIGHT = 320;

let worker = null;
let module = null;

let memory = null;
let shouldStopPtr = null;

let state = null;

const ctx = mainCanvas.getContext('2d');

let editor = null;

function showExportResult(canvas) {
	const background = document.createElement('div');

	background.style.backgroundColor = 'rgba(0, 0, 0, 0.25)';

	background.style.position = 'fixed';
	background.style.top = 0;
	background.style.left = 0;
	background.style.width = '100%';
	background.style.height = '100%';

	background.style.display = 'flex';

	background.style.justifyContent = 'center';
	background.style.alignItems = 'center';
	background.style.alignContent = 'center';

	const box = background.appendChild(document.createElement('div'));

	box.innerHTML += 'Export result:';

	box.style.backgroundColor = 'lightgray';
	box.style.padding = '30px';

	box.appendChild(document.createElement('br'));
	box.appendChild(document.createElement('br'));

	box.appendChild(canvas);

	box.appendChild(document.createElement('br'));
	box.appendChild(document.createElement('br'));

	const closeButton = box.appendChild(document.createElement('button'));

	closeButton.innerText = 'close';

	closeButton.onclick = function() {
		background.innerHTML = '';
		background.remove();
	}

	document.body.appendChild(background);
}

function makeQRCode(msg) {
	const output = new Uint8Array(msg.size);

	for (let i = 0; i < msg.size; i++) {
		output[i] = memory[msg.buffer + i];
	}

	const qr = qrcodegen.QrCode.encodeBinary(output, qrcodegen.QrCode.Ecc.LOW);

	const qrCanvas = document.createElement('canvas');

	const scale = 4;

	const width = qr.size + 20;
	const height = qr.size + 20;

	qrCanvas.width = width * scale;
	qrCanvas.height = height * scale;

	const qrCtx = qrCanvas.getContext('2d');

	qrCtx.scale(scale, scale);
	
	qrCtx.fillStyle = 'white';
	qrCtx.fillRect(0, 0, width, height);

	for (let x = 0; x < qr.size; x++) {
		for (let y = 0; y < qr.size; y++) {
			qrCtx.fillStyle = qr.getModule(x, y) ? 'black' : 'white';
			qrCtx.fillRect(10 + x, 10 + y, 1, 1);
		}
	}

	showExportResult(qrCanvas);
}

async function renderCanvas() {
	if (state) {
		let rgba = new Uint8ClampedArray(CANVAS_WIDTH * CANVAS_HEIGHT * 4);

		for (let i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT * 4; i += 4) {
			const pixel = memory[state.frameBuffer + (i / 4)];
			rgba[i + 0] = (((pixel >> 5) & 0b111) * 255) / 7;
			rgba[i + 1] = (((pixel >> 3) & 0b111) * 255) / 7;
			rgba[i + 2] = ((pixel & 0b11) * 255) / 3;
			rgba[i + 3] = 0xff;
		}

		const img = await createImageBitmap(new ImageData(rgba, CANVAS_WIDTH, CANVAS_HEIGHT));
		ctx.drawImage(img, 0, 0);
	}

	//setTimeout(function() {
		requestAnimationFrame(renderCanvas);
	//}, 1000.0 / 30.0);
}

function makeWorker() {
	const worker = new Worker('worker.js');
	
	worker.onmessage = async function(e) {
		const msg = e.data;

		if (msg.type == 'error') {
			alert('Error: ' + msg.details);
		}
	
		if (msg.type == 'postInit') {
			memory = msg.memory;
			shouldStopPtr = msg.shouldStopPtr;
		}
	
		if (msg.type == 'state') {
			state = {
				frameBuffer: msg.frameBuffer,
				leftKeyPtr: msg.leftKeyPtr,
				rightKeyPtr: msg.rightKeyPtr,
				upKeyPtr: msg.upKeyPtr,
				downKeyPtr: msg.downKeyPtr,
				aKeyPtr: msg.aKeyPtr,
				bKeyPtr: msg.bKeyPtr,
			};
		}
	
		if (msg.type == 'output') {
			makeQRCode(msg);
		}
	}

	worker.postMessage({ type: 'init', module: module });

	return worker;
}

function addListeners() {
	runButton.onclick = async function() {
		if (memory) {
			Atomics.store(memory, shouldStopPtr, true);
		}
	
		worker.postMessage({ type: 'compileAndRun', source: editor.getValue()});
	}

	stopButton.onclick = async function() {	
		if (memory) {
			Atomics.store(memory, shouldStopPtr, true);
		}
	}
	
	exportButton.onclick = async function() {	
		if (memory) {
			Atomics.store(memory, shouldStopPtr, true);
		}
	
		worker.postMessage({ type: 'compileAndExport', source: editor.getValue() });
	}

	mainCanvas.onkeydown = function(e) {
		console.log('hoi');

		if (!memory) {
			return;
		}

		if (e.key == 'ArrowUp') {
			memory[state.upKeyPtr] = 1;	
		}
		if (e.key == 'ArrowDown') {
			memory[state.downKeyPtr] = 1;	
		}
		if (e.key == 'ArrowLeft') {
			memory[state.leftKeyPtr] = 1;	
		}
		if (e.key == 'ArrowRight') {
			memory[state.rightKeyPtr] = 1;	
		}

		if (e.key == 'a') {
			memory[state.aKeyPtr] = 1;	
		}
		if (e.key == 'b') {
			memory[state.bKeyPtr] = 1;	
		}
	};

	mainCanvas.onkeyup = function(e) {
		if (!memory) {
			return;
		}

		if (e.key == 'ArrowUp') {
			memory[state.upKeyPtr] = 0;	
		}
		if (e.key == 'ArrowDown') {
			console.log('n');
			memory[state.downKeyPtr] = 0;	
		}
		if (e.key == 'ArrowLeft') {
			memory[state.leftKeyPtr] = 0;	
		}
		if (e.key == 'ArrowRight') {
			memory[state.rightKeyPtr] = 0;	
		}

		if (e.key == 'a') {
			memory[state.aKeyPtr] = 0;	
		}
		if (e.key == 'b') {
			memory[state.bKeyPtr] = 0;	
		}
	};
}

function initEditors() {
	editor = CodeMirror.fromTextArea(editorArea, {
		lineNumbers: true,
	});

	editor.setSize(null, 640);

	editorArea.remove();

	const output = CodeMirror.fromTextArea(outputArea, {
		readOnly: 'nocursor',
	});

	output.setSize(null, 200);
	output.setValue('hello world');
	
	outputArea.remove();
}

function initCanvas() {
	ctx.scale(2, 2);

	ctx.fillStyle = 'black';
	ctx.imageSmoothingEnabled = false;

	ctx.fillRect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT);
}

async function run() {
	module = await WebAssembly.compileStreaming(fetch('piqro.wasm'));
	
	initEditors();
	initCanvas();

	worker = makeWorker();

	addListeners();

	renderCanvas();
}

run();