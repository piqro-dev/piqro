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
	}

	worker.postMessage({ type: 'init', module: module });

	return worker;
}

function addListeners() {
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

function initCanvas() {
	ctx.scale(2, 2);

	ctx.fillStyle = 'black';
	ctx.imageSmoothingEnabled = false;

	ctx.fillRect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT);
}

let scanForProgramInterval = null;

async function scan() {
	const offscreenCanvas = new OffscreenCanvas(inputStream.videoWidth, inputStream.videoHeight);
	const offscreenCtx = offscreenCanvas.getContext('2d');

	offscreenCtx.drawImage(inputStream, 0, 0);

	const image = offscreenCtx.getImageData(0, 0, offscreenCanvas.width, offscreenCanvas.height);
	const result = await zbarWasm.scanImageData(image, undefined);

	if (result.length > 0) {
		let magic = '';

		for (let i = 0; i < 4; i++) {
			magic += String.fromCharCode(result[0].data[i]);
		}

		if (magic == 'PIQR') {
			inputStream.srcObject.getVideoTracks()[0].stop();

			inputDiv.innerHTML = '';
			inputDiv.remove();

			clearInterval(scanForProgramInterval);

			worker.postMessage({ type: 'runFromBlob', buffer: result[0].data });
		} else {
			alert('Invalid program provided');
		}
	}
}

async function launchScanner() {
	inputStream.srcObject = await navigator.mediaDevices.getUserMedia({
		video: { 
			facingMode: 'environment'
		}
	});

	inputStream.onloadedmetadata = function() {
		inputStream.play();
	};

	scanForProgramInterval = setInterval(scan, 100);
}

async function run() {
	module = await WebAssembly.compileStreaming(fetch('piqro.wasm'));
	
	launchScanner();

	initCanvas();

	worker = makeWorker();

	addListeners();

	renderCanvas();
}

run();