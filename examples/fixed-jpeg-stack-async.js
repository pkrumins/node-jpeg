var JpegLib = require('../');
var fs = require('fs');
var sys = require('sys');
var Buffer = require('buffer').Buffer;

var jpegStack = new JpegLib.FixedJpegStack(720, 400, 'rgba');

function rectDim(fileName) {
    var m = fileName.match(/^\d+-rgba-(\d+)-(\d+)-(\d+)-(\d+).dat$/);
    var dim = [m[1], m[2], m[3], m[4]].map(function (n) {
        return parseInt(n, 10);
    });
    return { x: dim[0], y: dim[1], w: dim[2], h: dim[3] }
}

var files = fs.readdirSync(__dirname + '/push-data');

files.forEach(function(file) {
    var dim = rectDim(file);
    var rgba = fs.readFileSync(__dirname + '/push-data/' + file);
    jpegStack.push(rgba, dim.x, dim.y, dim.w, dim.h);
});

jpegStack.encode(function (image) {
    fs.writeFileSync(__dirname + '/fixed-async.jpg', image.toString('binary'), 'binary');
});

