# piqro
#### A tiny game console for making and sharing little games.

## Info
piqro is a game console with its own programming language and virtual machine. It runs in the browser through WebAssembly, so games can be written, compiled, and played without installing an editor or runtime.

**Note:** piqro is an unfinished work-in-progress project.

The project also includes an experimental Raspberry Pi Pico hardware target. 
- Note: the exact schematics are undocumented at this time.

See the console in action: [showcase video](https://www.youtube.com/watch?v=WX2VfJAwS3A)

## Usage
Open `bin/www/index.html` through a local web server, then choose:

- `editor` to write and run piqro programs, or export one as a QR code
- `player` to scan an exported QR code with a phone camera

A small piqro program looks like this:

```text
var x = 20
var y = 20

fore(color(255, 255, 255))
fill_rect(x, y, 20, 20)
present()
```

The language supports variables, arrays, procedures, conditionals, loops, arithmetic, and a small drawing/input runtime. Programs compile to a compact bytecode blob that is executed by the piqro VM.

## Building
#### Requirements
- [Clang](https://github.com/llvm/llvm-project/releases)
- Visual Studio / Build Tools
- A local web server for the browser build

#### Web build
```bat
build.cmd web
```

This produces `bin/www/piqro.wasm`. Serve `bin/www` with a local web server and open `index.html` in a browser.

#### CLI build
```bat
build.cmd cli
run.cmd
```

The CLI target builds the VM test bed and prints compiler, instruction, and test output.

#### Hardware build
```bat
build.cmd hw
```

The hardware target uses the [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk), CMake, and Ninja. It currently targets an ILI9341 display.

## Credits
#### Dependencies
- [CodeMirror](https://codemirror.net/)
- [qrcodegen](https://www.nayuki.io/page/qr-code-generator-library)
- [zbar.wasm](https://github.com/mchehab/zbar)
- [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)
