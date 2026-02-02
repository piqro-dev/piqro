'use strict';

const CANVAS_WIDTH = 240;
const CANVAS_HEIGHT = 320;

const TOUCH_BUTTON_RADIUS = 25;

let touchButtons = [];
let touchMap = new Map();

let editor = null;
let output = null;

let worker = null;
let module = null;

let memory = null;
let shouldStopPtr = null;

let state = null;

const ctx = mainCanvas.getContext('2d');

function isScreenSmall() {
	return matchMedia('(max-width: 600px)').matches;
}

function showExportResult(canvas) {
	const background = document.body.appendChild(document.createElement('div'));

	background.style.backgroundColor = 'rgba(0, 0, 0, 0.5)';

	background.style.position = 'fixed';
	background.style.top = 0;
	background.style.left = 0;
	background.style.width = '100%';
	background.style.height = '100%';

	background.style.display = 'flex';

	background.style.justifyContent = 'center';
	background.style.alignItems = 'center';
	background.style.alignContent = 'center';

	background.style.zIndex = 999;

	const box = background.appendChild(document.createElement('div'));

	box.innerHTML += 'result:';

	box.style.backgroundColor = 'lightgray';
	box.style.padding = '30px';

	box.appendChild(document.createElement('br'));
	box.appendChild(document.createElement('br'));

	box.appendChild(canvas);

	box.appendChild(document.createElement('br'));
	box.appendChild(document.createElement('br'));

	const close = box.appendChild(document.createElement('button'));

	close.innerText = 'close';

	close.style.width = '50px';
	close.style.height = '30px';

	close.onclick = function() {
		background.remove();
	}
}

function makeQRCode(msg) {
	const output = new Uint8Array(msg.size);

	for (let i = 0; i < msg.size; i++) {
		output[i] = memory[msg.buffer + i];
	}

	const qr = qrcodegen.QrCode.encodeBinary(output, qrcodegen.QrCode.Ecc.LOW);

	const qrCanvas = document.createElement('canvas');

	const scale = matchMedia('(max-width: 600px)').matches ? 2 : 4;

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

function handleScreen() {
	ctx.imageSmoothingEnabled = false;

	if (isScreenSmall()) {
		infoSection.style.display = 'none';
		outputSection.style.display = 'none';

		if (memory && shouldStopPtr) {
			if (Atomics.load(memory, shouldStopPtr, true)) {
				canvasSection.style.display = 'none';
				toolSmallStop.style.display = 'none';
				smallInfoSection.style.display = 'block';
				editorSection.style.display = 'block';
			} else {
				smallInfoSection.style.display = 'none';
				editorSection.style.display = 'none';
				toolSmallStop.style.display = 'block';
			}
		}

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
		smallInfoSection.style.display = 'none';
		toolSmallStop.style.display = 'none';

		editorSection.style.display = 'block';
		outputSection.style.display = 'block';
		infoSection.style.display = 'block';
		canvasSection.style.display = 'block';

		mainCanvas.width = CANVAS_WIDTH * 2;
		mainCanvas.height = CANVAS_HEIGHT * 2;
	
		editor.setSize(null, 640);

		ctx.scale(2, 2);

		window.onresize = null;
	}

	canvasSection.style.width = mainCanvas.width + 'px';
	canvasSection.style.height = mainCanvas.height + 'px';
}

function initCanvas() {
	if (isScreenSmall()) {
		canvasSection.style.display = 'none';
	}

	handleScreen();
}

let rgba = new Uint8ClampedArray(CANVAS_WIDTH * CANVAS_HEIGHT * 4);

async function renderCanvas() {
	handleScreen();

	ctx.fillStyle = isScreenSmall() ? 'darkgray' : 'black';
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

			for (const button of touchButtons) {
				ctx.fillStyle = 'red';
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
	
		if (msg.type == 'output') {
			makeQRCode(msg);
		}

		if (msg.type == 'print') {
			output.setValue(output.getValue() + msg.text);
			output.setCursor(output.lastLine());
		}
	}

	worker.postMessage({ type: 'init', module: module });

	return worker;
}

function addListeners() {
	// small screen
	toolSmallStop.onclick = async function() {
		if (memory) {
			Atomics.store(memory, shouldStopPtr, true);
		}

		toolSmallStop.style.display = 'none';
	}

	smallRun.onclick = async function() {
		output.setValue('');

		canvasSection.style.display = 'block';

		if (memory) {
			Atomics.store(memory, shouldStopPtr, true);
		}
	
		worker.postMessage({ type: 'compileAndRun', source: editor.getValue() });
	}

	smallExport.onclick = async function() {	
		if (memory) {
			Atomics.store(memory, shouldStopPtr, true);
		}
	
		worker.postMessage({ type: 'compileAndExport', source: editor.getValue() });
	}

	smallUndo.onclick = async function() {
		editor.undo();
	}

	smallRedo.onclick = async function() {
		editor.redo();
	}

	smallClear.onclick = async function() {	
		editor.setValue('');
	}

	smallCopy.onclick = async function() {	
		navigator.clipboard.writeText(editor.getValue());
	}

	smallPaste.onclick = async function() {
		try {
			const toPaste = await navigator.clipboard.readText();
			const editorText = editor.getValue();
			const cursorIdx = editor.indexFromPos(editor.getCursor());

			if (editor.somethingSelected()) {
				editor.replaceSelection(toPaste);
			} else {
				editor.setValue(editorText.substring(0, cursorIdx) + toPaste + editorText.substring(cursorIdx));
			}
		} catch (e) {
			alert('Clipboard permission has been denied. Enable it and try again');
		}
	}

	// regular screen
	regRun.onclick = async function() {
		output.setValue('');

		if (memory) {
			Atomics.store(memory, shouldStopPtr, true);
		}

		worker.postMessage({ type: 'compileAndRun', source: editor.getValue() });
	}

	regStop.onclick = async function() {	
		if (memory) {
			Atomics.store(memory, shouldStopPtr, true);
		}
	}
	
	regExport.onclick = async function() {	
		if (memory) {
			Atomics.store(memory, shouldStopPtr, true);
		}
	
		worker.postMessage({ type: 'compileAndExport', source: editor.getValue() });
	}

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
	}

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
	}

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

function initEditors() {
	editor = CodeMirror.fromTextArea(editorArea, {
		lineNumbers: true,
	});

	if (localStorage.sourceCode && localStorage.sourceCode !== '') {
		editor.setValue(localStorage.sourceCode);
	}

	setInterval(function() {
		localStorage.sourceCode = editor.getValue();
	}, 2.5);

	editor.setSize(null, 640);

	output = CodeMirror.fromTextArea(outputArea, {
		readOnly: 'nocursor',
	});

	output.setSize(null, 200);
}

async function run() {
	initEditors();
	initCanvas();

	module = await WebAssembly.compileStreaming(fetch('piqro.wasm'));
	
	worker = makeWorker();

	addListeners();

	renderCanvas();
}

run();