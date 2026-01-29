'use strict';

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
	const mem = memory.slice();

	let end = str;

	while (mem[end]) {
		end++;
	}

	return decoder.decode(mem.subarray(str, end));
}

const env = {
	//
	// libc
	//
	
	putchar(c)   { console.log(String.fromCharCode(c)); },
	puts(text)   { console.log(decode(text)); },
	atof(x)      { return Number(decode(x)); },
	
	fmodf(x, y)  { return Math.fround(x % y); },
	sinf(x)      { return Math.sin(x); },
	cosf(x)      { return Math.cos(x); },
	tanf(x)      { return Math.tan(x); },
	asinf(x)     { return Math.asin(x); },
	acosf(x)     { return Math.acos(x); },
	atanf(x)     { return Math.atan(x); },
	powf(x, y)   { return Math.pow(x, y); },
	logf(x)      { return Math.log(x); },
	log10f(x)    { return Math.log10(x); },

	//
	// javascript exports
	//

	js_alert(text)                     { alert(decode(text)); },
	js_post_message(e)                 { postMessage(e); },
 
	js_console_log(text)               { console.log(decode(text)); },
	js_console_error(text)             { console.error(decode(text)); },
 
	js_string(text)                    { return decode(text); },
	js_obj()                           { return {}; },
 
	js_set_int(obj, property, v)       { obj[decode(property)] = v; },
	js_set_string(obj, property, v)    { obj[decode(property)] = decode(v); },
 
	js_get_int(obj, property)          { return obj[decode(property)]; },
	js_get_string(obj, property, out)  { encode(out, obj[decode(property)]); },
	js_get(obj, property)              { return obj[decode(property)]; },
	
	js_memcpy(dst, src, n)             { for (let i = 0; i < n; i++) memory[dst + i] = src[i]; }   
};
	
let wasm = null;
let memory = null;

onmessage = async function(e) {
	const msg = e.data;

	if (msg.type == 'init') {
		const instance = await WebAssembly.instantiate(msg.module, { env: env });

		wasm = {
			module: msg.module,
			instance: instance
		};
	
		memory = new Uint8Array(wasm.instance.exports.memory.buffer);

		wasm.instance.exports.init();

		postMessage({ type: 'postInit', memory, shouldStopPtr: wasm.instance.exports.should_stop_ptr() });
	}

	if (msg.type == 'compileAndRun') {
		const shouldStopPtr = wasm.instance.exports.should_stop_ptr();

		if (Atomics.load(memory, shouldStopPtr)) {	
			Atomics.store(memory, shouldStopPtr, false);
			wasm.instance.exports.compile_and_run({ source: msg.source, length: msg.source.length });
			Atomics.store(memory, shouldStopPtr, true);
		}
	}

	if (msg.type == 'runFromBlob') {
		const shouldStopPtr = wasm.instance.exports.should_stop_ptr();

		if (Atomics.load(memory, shouldStopPtr)) {	
			Atomics.store(memory, shouldStopPtr, false);
			wasm.instance.exports.run_from_blob({ buffer: msg.buffer, length: msg.buffer.length });
			Atomics.store(memory, shouldStopPtr, true);
		}
	}

	if (msg.type == 'compileAndExport') {
		wasm.instance.exports.compile_and_export({ source: msg.source, length: msg.source.length });
	}
}
