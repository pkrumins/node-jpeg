This is a node.js module, writen in C++, that uses libjpeg to produce a JPEG
image (in memory) from a buffer of RGBA or RGB values. Since JPEG has no notion
of A (alpha), the module always uses just RGB values.

It was written by Peteris Krumins (peter@catonmat.net).
His blog is at http://www.catonmat.net  --  good coders code, great reuse.

------------------------------------------------------------------------------

The module exports `Jpeg` object that takes 5 arguments in its constructor:

    var png = new Jpeg(buffer, width, height, quality, buffer_type);

The first argument, `buffer`, is a nodee.js `Buffer` filled with RGBA or RGB
values.
The second argument is integer width of the image.
The third argument is integer height of the image.
The fourth argument is the quality of output image.
The fifth argument is buffer type, either 'rgb' or 'rgba'.

See `examples/` directory for examples.

To get it compiled, you need to have libjpeg and node installed. Then just run

    node-waf configure build

to build the Jpeg module. It will produce a `jpeg.node` file as the module.

See also http://github.com/pkrumins/node-png module that produces PNG images.

------------------------------------------------------------------------------

Have fun!


Sincerely,
Peteris Krumins
http://www.catonmat.net

