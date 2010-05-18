var fs  = require('fs');
var sys = require('sys');
var Jpeg = require('../jpeg').Jpeg;
var Buffer = require('buffer').Buffer;

// the rgba-terminal.dat file is 1152000 bytes long.
var rgba = new Buffer(1152000);
rgba.write(fs.readFileSync('./rgba-terminal.dat', 'binary'), 'binary');

var jpeg = new Jpeg(rgba, 720, 400, 70, 'rgba');

var fd = fs.openSync('./jpeg.jpeg', 'w+', 0660);
var total = 0, written = 0;
jpeg.addListener('data', function(chunk, length) {
    sys.log('Got a chunk. Size: ' + length);
    written += fs.writeSync(fd, chunk, written, 'binary');
    total += length;
});
jpeg.addListener('end', function() {
    fs.closeSync(fd);
    sys.log('Total: ' + total + ' bytes. Written: ' + written + ' bytes.');
});
jpeg.encode();

