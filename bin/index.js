"use strict";

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
		
		atof(x)     { return Number(decode(x)); },
		sinf(x)     { return Math.sin(x); },
		fmodf(x, y) { return x % y; },
		
		//
		// javascript exports
		//

		js_console_log(text)   { console.log(decode(text)); },
		js_console_error(text) { console.error(decode(text)); },
		js_alert(text)         { alert(decode(text)); },
	
		js_get_element_by_id(id)              { return document.getElementById(decode(id)) },
		js_add_event_listener(obj, event, fn) { obj.addEventListener(decode(event), wasm.instance.exports.__indirect_function_table.get(fn)); },

		js_set_string(obj, property, v) { obj[decode(property)] = decode(v); },

		js_get_int(obj, property)         { return obj[decode(property)]; },
		js_get_string(obj, property, out) { encode(out, obj[decode(property)]); },
		js_get(obj, property)             { return obj[decode(property)]; },

		js_get_context(canvas, type)   { console.log(canvas); return canvas.getContext(decode(type)); },
		js_clear_rect(ctx, x, y, w, h) { ctx.clearRect(x, y, w, h); },
		js_fill_rect(ctx, x, y, w, h)  { ctx.fillRect(x, y, w, h); },
	}

	const wasm = await WebAssembly.instantiateStreaming(fetch('piqro.wasm'), {
		env: env
	});
	
	let memory = new Uint8Array(wasm.instance.exports.memory.buffer);

	wasm.instance.exports.main();
}
		
run();
