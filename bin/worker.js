"use strict"

let encoder = new TextEncoder();
let decoder = new TextDecoder('utf-8');

function encode(str, value) {
	const bytes = encoder.encode(value);
	const mem = new Uint8Array(memory.buffer, str, bytes.length + 1);
	
	mem.set(bytes);
	mem[bytes.length] = 0;
}

function encode(str, value, len) {
	const bytes = encoder.encode(value);
	const mem = new Uint8Array(memory.buffer, str, bytes.length + 1);
	
	mem.set(bytes);
	mem[len] = 0;
}

function decode(str) {
	let end = str;

	while (memory[end]) {
		end++;
	}

	return decoder.decode(memory.subarray(str, end));
}

const env = {
	memory: new WebAssembly.Memory({
		initial: 10,
		maximum: 100,
		shared: true
	}),

	//
	// libc
	//
	
	atof(x)     { return Number(decode(x)); },
	sinf(x)     { return Math.sin(x); },
	fmodf(x, y) { return x % y; },
	puts(text)  { console.log(decode(text)); },

	//
	// javascript exports
	//

	js_post_message(e)                { postMessage(e); },

	js_console_log(text)              { console.log(decode(text)); },
	js_console_error(text)            { console.error(decode(text)); },

	js_string(text)                   { return decode(text); },
	js_obj()                          { return {}; },

	js_set_ptr(obj, proprety, ptr)    { obj[decode(property)] = ptr; },
	js_set_string(obj, property, v)   { obj[decode(property)] = decode(v); },

	js_get_int(obj, property)         { return obj[decode(property)]; },
	js_get_string(obj, property, out) { encode(out, obj[decode(property)]); },
	js_get(obj, property)             { return obj[decode(property)]; },
};

let wasm;
let memory;

let result = false;

function execute() {
	const view = new Uint8Array(wasm.instance.exports.memory.buffer, wasm.instance.exports.executing_ptr(), 1);		
	
	if (!Atomics.load(view, 0)) {
		return;
	}


}

onmessage = function(e) {
	const msg = e.data;

	if (msg.type == 'init') {
		console.log('Got module, initializing');

		WebAssembly.instantiate(msg.module, { env: env }).then(function(instance) {
			wasm = {
				module: msg.module,
				instance: instance
			}

			memory = new Uint8Array(wasm.instance.exports.memory.buffer);
		
			wasm.instance.exports.init();
		});
	}

	if (msg.type == 'run') {
		const view = new Uint8Array(wasm.instance.exports.memory.buffer, wasm.instance.exports.executing_ptr(), 1);		

		Atomics.store(view, 0, true);

		if (Atomics.load(view, 0)) {
			Atomics.store(view, 0, false);
		}

		wasm.instance.exports.compile({ source: msg.source, length: msg.source.length });
	

	}
}