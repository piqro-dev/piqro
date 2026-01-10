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

	const wasm = await WebAssembly.instantiateStreaming(fetch('index.wasm'), {
		env: {
			// libc
			strlen(s) {
				let len = 0;
				
				while (memory[s + len] !== 0) {
					len++;
				}
			
				return len;
			},
			strcmp(l, r) {
				return decode(l).localeCompare(decode(r));
			},
			memcmp(l, r, len) {
				const lBuf = new Uint8Array(memory.buffer, l, len);
				const rBuf = new Uint8Array(memory.buffer, r, len);

				return indexedDB.cmp(lBuf, rBuf);
			},
			strcpy(dest, src) {
				encode(dest, src);
			}, 
			strncpy(dest, src, len) {
				encode(dest, src, len);
			}, 
			strchr(str, c) {
				let ptr = str;
		
				while (memory[ptr] !== 0) {
					if (memory[ptr] === c) {
						return ptr;
					}
					ptr++;
				}
				return 0;
			},
			strrchr(str, c) {
				let last = 0;
				let ptr = str;
				
				while (memory[ptr] !== 0) {
					if (memory[ptr] === c) {
						last = ptr;
					}
					ptr++;
				}

				return last;
			},
			atof(x) { 
				return Number(decode(x)); 
			},
			sinf(x) {
				return Math.sin(x);	
			},
			fmodf(x, y) {
				return x % y;
			},

			get_window() {
				return window;
			},
			get_document_body() {
				return document.body;
			},
			null() {
				return null;
			},
			obj() {
				return {};
			},

			set_value(obj, key, value) {
				return obj[decode(key)] = value;
			},
			set_number(obj, key, value) {
				return obj[decode(key)] = Number(value);
			},
			set_str(obj, key, value) {
				return obj[decode(key)] = decode(value);
			},
			get_value(obj, key) {
				return obj[decode(key)];
			},
			get_number(obj, key) {
				return Number(obj[decode(key)]);
			}, 
			get_str(obj, key, out) {
				encode(out, obj[decode(key)]);
			},
			
			js_console_log(text) {
				console.log(decode(text));
			},
			js_console_error(text) {
				console.error(decode(text));
			},
		}
	});
	
	let memory = new Uint8Array(wasm.instance.exports.memory.buffer);

	wasm.instance.exports.main();
}
		
run();
