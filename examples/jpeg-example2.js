var fs  = require('fs');
var sys = require('sys');
var Jpeg = require('../jpeg').Jpeg;
var Buffer = require('buffer').Buffer;

var WIDTH = 400, HEIGHT = 300;

var rgb = new Buffer(WIDTH*HEIGHT*3);

for (var i=0; i<HEIGHT; i++) {
    for (var j=0; j<WIDTH; j++) {
        rgb[i*WIDTH*3 + j*3 + 0] = 255*j/WIDTH;
        rgb[i*WIDTH*3 + j*3 + 1] = 255*i/HEIGHT;
        rgb[i*WIDTH*3 + j*3 + 2] = 0xff/2;
    }
}

var jpeg = new Jpeg(rgb, WIDTH, HEIGHT, 'rgb');
var jpeg_img = jpeg.encodeSync().toString('binary');

fs.writeFileSync('./jpeg-gradient.jpeg', jpeg_img, 'binary');

