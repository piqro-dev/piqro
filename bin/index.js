"use strict";

//<script src="index.js"></script>

async function run() {
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
		//
		// libc
		//
		
		atof(x)           { return Number(decode(x)); },
		sinf(x)           { return Math.sin(x); },
		fmodf(x, y)       { return x % y; },
		
		//
		// javascript exports
		//

		js_console_log(text)   { console.log(decode(text)); },
		js_console_error(text) { console.error(decode(text)); },
	}

	const wasm = await WebAssembly.instantiateStreaming(fetch('index.wasm'), {
		env: env
	});
	
	let memory = new Uint8Array(wasm.instance.exports.memory.buffer);

	wasm.instance.exports.main();
}
		
run();
