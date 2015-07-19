var fs  = require('fs');
var sys = require('sys');
var Jpeg = require('../').Jpeg;
var Buffer = require('buffer').Buffer;

var rgba = fs.readFileSync(__dirname + '/rgba-terminal.dat');

var jpeg = new Jpeg(rgba, 720, 400, 'rgba');
jpeg.encode(function (image) {
    fs.writeFileSync(__dirname + '/jpeg-async.jpeg', image.toString('binary'), 'binary');
})

