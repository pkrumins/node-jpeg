var fs  = require('fs');
var sys = require('sys');
var Jpeg = require('../jpeg').Jpeg;
var Buffer = require('buffer').Buffer;

// the rgba-terminal.dat file is 1152000 bytes long.
var rgba = new Buffer(1152000);
rgba.write(fs.readFileSync('./rgba-terminal.dat', 'binary'), 'binary');

var jpeg = new Jpeg(rgba, 720, 400, 'rgba');
jpeg.encode(function (image) {
    fs.writeFileSync('./jpeg-async.jpeg', image, 'binary');
})

