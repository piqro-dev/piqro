"use strict";

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
	
	atof(x)     { return Number(decode(x)); },
	sinf(x)     { return Math.sin(x); },
	fmodf(x, y) { return Math.fround(x % y); },
	puts(text)  { console.log(decode(text)); },

	//
	// javascript exports
	//

	js_alert(text)                    { alert(decode(text)); },

	js_post_message(e)                { postMessage(e); },

	js_console_log(text)              { console.log(decode(text)); },
	js_console_error(text)            { console.error(decode(text)); },

	js_string(text)                   { return decode(text); },
	js_obj()                          { return {}; },

	js_set_int(obj, proprety, v)      { obj[decode(property)] = v; },
	js_set_string(obj, property, v)   { obj[decode(property)] = decode(v); },

	js_get_int(obj, property)         { return obj[decode(property)]; },
	js_get_string(obj, property, out) { encode(out, obj[decode(property)]); },
	js_get(obj, property)             { return obj[decode(property)]; },
};
	
let wasm = null;
let memory = null;

let ready = false;
let interval = null;

onmessage = async function(e) {
	const msg = e.data;

	if (msg.type == 'init') {
		WebAssembly.instantiate(msg.module, { env: env }).then(function(instance) {
			wasm = {
				module: msg.module,
				instance: instance
			};
	
	 		memory = new Uint8Array(wasm.instance.exports.memory.buffer);

			wasm.instance.exports.init();
	 		
	 		ready = true;
		});
	}

	if (msg.type == 'run') {
		if (!ready) {
			return;
		}

		const shouldStopPtr = wasm.instance.exports.should_stop_ptr();

		if (Atomics.load(memory, shouldStopPtr)) {	
			postMessage({ type: 'memory', memory, shouldStopPtr });
			
			Atomics.store(memory, shouldStopPtr, false);
			
			wasm.instance.exports.run({ source: msg.source, length: msg.source.length});
		}
	}
}
