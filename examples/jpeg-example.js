var fs  = require('fs');
var sys = require('sys');
var Jpeg = require('../').Jpeg;
var Buffer = require('buffer').Buffer;

var rgba = fs.readFileSync(__dirname + '/rgba-terminal.dat');

var jpeg = new Jpeg(rgba, 720, 400, 'rgba');
var jpeg_img = jpeg.encodeSync().toString('binary');

fs.writeFileSync(__dirname + '/jpeg.jpeg', jpeg_img, 'binary');
