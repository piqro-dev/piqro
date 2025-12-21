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
			strlen(str) {
				let len = 0;
				
				while (memory[str + len] !== 0) {
					len++;
				}
			
				return len;
			},
			strcmp(l, r) {
				return decode(l).localeCompare(decode(r));
			},
			memcmp(l, r, s) {
				const lBuf = new Uint8Array(memory.buffer, l, s);
				const rBuf = new Uint8Array(memory.buffer, r, s);

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
			atoi(x) { 
				return Number(decode(x)); 
			},
			sinf: Math.sin,

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
			to_lower(dst, src) {
				encode(dst, decode(src).toLowerCase());
			},

			add_event_listener(obj, event, fn) {
				obj.addEventListener(decode(event), wasm.instance.exports.__indirect_function_table.get(fn));
			},
			remove_event_listener(obj, event, fn) {
				obj.removeEventListener(decode(event), wasm.instance.exports.__indirect_function_table.get(fn));
			},
			get_element_by_id(name) {
				return document.getElementById(decode(name));
			},
			create_element(name) {
				return document.createElement(decode(name));
			},
			create_element_ns(uri, name) {
				return document.createElementNS(decode(uri), decode(name));
			},
			append_child(node, element) {
				return node.appendChild(element);
			},
			request_animation_frame(fn) {
				requestAnimationFrame(wasm.instance.exports.__indirect_function_table.get(fn));
			},
			console_log(text) {
				console.log(decode(text));
			},
			console_error(text) {
				console.error(decode(text));
			},
			console_log_obj(obj) {
				console.log(obj);
			},
			set_timeout(fn, t) {
				return setTimeout(wasm.instance.exports.__indirect_function_table.get(fn), t);
			},
			clear_timeout(id) {
				clearTimeout(id);
			},
			prevent_default(obj) {
				obj.preventDefault();
			},
			set_pointer_capture(obj, pointer_id) {
				obj.setPointerCapture(pointer_id);
			},
			release_pointer_capture(obj, pointer_id) {
				obj.releasePointerCapture(pointer_id);
			},
			remove(node) {
				node.remove();
			},
			set_attribute(obj, name, value) {
				obj.setAttribute(decode(name), decode(value));
			},
			get_computed_style(element) {
				return window.getComputedStyle(element);
			},
			parse_float(str) {
				return parseFloat(decode(str));
			},

			get_context(canvas, ctx, options) {
				return canvas.getContext(decode(ctx), options);
			},
			fill_rect(ctx, x, y, w, h) {
				ctx.fillRect(x, y, w, h);
			},
			fill_text(ctx, text, x, y) {
				ctx.fillText(decode(text), x, y);
			},
			clear_rect(ctx, x, y, w, h) {
				ctx.clearRect(x, y, w, h);
			},
		}
	});
	
	let memory = new Uint8Array(wasm.instance.exports.memory.buffer);

	wasm.instance.exports.main();
}
		
run();
