'use strict';

const CANVAS_WIDTH = 240;
const CANVAS_HEIGHT = 320;

const TOUCH_BUTTON_RADIUS = 25;

let touchButtons = [];
let touchMap = new Map();

let worker = null;
let module = null;

let memory = null;
let shouldStopPtr = null;

let state = null;

const ctx = mainCanvas.getContext('2d');

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
				timeSinceStartPtr: msg.timeSinceStartPtr
			};
		}
	}

	worker.postMessage({ type: 'init', module: module });

	return worker;
}

function addListeners() {
	mainCanvas.onkeydown = function(e) {
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

	mainCanvas.ontouchstart = function(e) {
		if (!memory || !isScreenSmall()) {
			return;
		}

		e.preventDefault();

		const rect = mainCanvas.getBoundingClientRect();

		for (const button of touchButtons) {
			for (const touch of e.changedTouches) {
				const distance = Math.hypot(touch.clientX - rect.left - button.x, touch.clientY - rect.top - button.y);

				if (distance <= TOUCH_BUTTON_RADIUS) {
					touchMap.set(touch.identifier, button.key);
					memory[touchMap.get(touch.identifier)] = 1;
				}
			}
		};
	}

	mainCanvas.ontouchend = function(e) {
		if (!memory || !isScreenSmall()) {
			return;
		}

		e.preventDefault();

		for (const button of touchButtons) {
			for (const touch of e.changedTouches) { 
				if (touchMap.get(touch.identifier) == button.key) {
					memory[button.key] = 0;
					touchMap.delete(touch.identifier);
				}
			}
		};
	}
}

function isScreenSmall() {
	return matchMedia('(max-width: 600px)').matches; 
}

function clearCanvas() {
	ctx.imageSmoothingEnabled = false;

	if (isScreenSmall()) {
		mainCanvas.width = window.innerWidth;
		mainCanvas.height = window.innerHeight - toolStrip.clientHeight;
	
		ctx.scale(1, 1);

		window.onresize = function() {
			mainCanvas.width = window.innerWidth;
			mainCanvas.height = window.innerHeight - toolStrip.clientHeight;
		};

		const canvasX = (mainCanvas.width / 2) - (CANVAS_WIDTH / 2);

		if (state) {
			touchButtons = [
				{
					key: state.upKeyPtr,
					x: canvasX + TOUCH_BUTTON_RADIUS * 2, 
					y: 50 + CANVAS_HEIGHT + 30 + TOUCH_BUTTON_RADIUS,
				},
				{
					key: state.downKeyPtr,
					x: canvasX + TOUCH_BUTTON_RADIUS * 2, 
					y: 50 + CANVAS_HEIGHT + 30 + TOUCH_BUTTON_RADIUS * 5
				},
				{
					key: state.leftKeyPtr,
					x: canvasX, 
					y: 50 + CANVAS_HEIGHT + 30 + TOUCH_BUTTON_RADIUS * 3,
				},
				{
					key: state.rightKeyPtr,
					x: canvasX + TOUCH_BUTTON_RADIUS * 4, 
					y: 50 + CANVAS_HEIGHT + 30 + TOUCH_BUTTON_RADIUS * 3,
				},
				{
					key: state.aKeyPtr,
					x: canvasX + CANVAS_WIDTH - TOUCH_BUTTON_RADIUS * 2, 
					y: 50 + CANVAS_HEIGHT + 30 + TOUCH_BUTTON_RADIUS * 2,
				},
				{
					key: state.bKeyPtr,
					x: canvasX + CANVAS_WIDTH, 
					y: 50 + CANVAS_HEIGHT + 30 + TOUCH_BUTTON_RADIUS * 4,
				},
			];
		}
	} else {
		mainCanvas.width = CANVAS_WIDTH * 2;
		mainCanvas.height = CANVAS_HEIGHT * 2;

		ctx.scale(2, 2);

		window.onresize = null;
	}
}

function initCanvas() {
	canvasSection.style.display = 'none';

	clearCanvas();
}

let rgba = new Uint8ClampedArray(CANVAS_WIDTH * CANVAS_HEIGHT * 4);

async function renderCanvas() {
	clearCanvas();
	
	ctx.fillStyle = 'darkgray';
	ctx.clearRect(0, 0, mainCanvas.width, mainCanvas.height);
	ctx.fillRect(0, 0, mainCanvas.width, mainCanvas.height);

	if (state) {
		for (let i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT * 4; i += 4) {
			const pixel = memory[state.frameBuffer + (i / 4)];
			rgba[i + 0] = (((pixel >> 5) & 0b111) * 255) / 7;
			rgba[i + 1] = (((pixel >> 3) & 0b111) * 255) / 7;
			rgba[i + 2] = ((pixel & 0b11) * 255) / 3;
			rgba[i + 3] = 0xff;
		}

		const img = await createImageBitmap(new ImageData(rgba, CANVAS_WIDTH, CANVAS_HEIGHT));
		
		if (isScreenSmall()) {
			ctx.drawImage(img, mainCanvas.width / 2 - CANVAS_WIDTH / 2, 50);

			ctx.fillStyle = 'red';

			for (const button of touchButtons) {
				ctx.beginPath();
				ctx.arc(button.x, button.y, TOUCH_BUTTON_RADIUS, 0, 2 * Math.PI);
				ctx.fill();
			}
		} else {
			ctx.drawImage(img, 0, 0);
		}
	}

	requestAnimationFrame(renderCanvas);
}

let scanInterval = null;

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

			clearInterval(scanInterval);

			worker.postMessage({ type: 'runFromBlob', buffer: result[0].data });

			inputSection.style.display = 'none';
			canvasSection.style.display = 'block';
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

	scanInterval = setInterval(scan, 100);
}

async function run() {
	initCanvas();

	module = await WebAssembly.compileStreaming(fetch('piqro.wasm'));

	worker = makeWorker();

	launchScanner();

	addListeners();

	renderCanvas();
}

run();