//
// Title:	        Speculation UI
// Description:		Scripts to support the emulator UI
// Author:	        Dean Belfield
// Created:	        08/08/2026
// Last Updated:	08/08/2026
//
// Modinfo:

let canvasElement = document.getElementById('canvas');
let outputElement = document.getElementById('output');

function buttonOpen() {
	let el = document.getElementById("files");
	if (el) {
       	Module.open(el.value);
	}
}	

if (outputElement) outputElement.value = '';

canvasElement.addEventListener('webglcontextlost', (e) => {
	alert('WebGL context lost. You will need to reload the page.');
    e.preventDefault();
}, false);

var Module = {
	print(...args) {
		console.log(...args);
		if (outputElement) {
			let text = args.join(' ');
			outputElement.value += text + '\n';
			outputElement.scrollTop = outputElement.scrollHeight;
		}
	},
	canvas: canvasElement,
	arguments: [
		"scale=2"
	],
};

window.onerror = window.onunhandledrejection = () => {
	Module.print("Exception thrown: See JavaScript Console");
};