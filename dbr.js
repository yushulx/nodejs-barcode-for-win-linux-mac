var dbr = require('./build/Release/dbr');
var readline = require('readline');
var fs = require('fs');

var rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
});

rl.question("Please input a barcode image path: ", function(answer) {

    dbr.decodeFileAsync(
        answer, 0x3FF | 0x2000000 | 0x8000000 | 0x4000000,
        function(msg) {
            var result = null;
            for (index in msg) {
                result = msg[index]
                console.log("Format: " + result['format']);
                console.log("Value : " + result['value']);
                console.log("##################");
            }
        }
    );

    rl.close();
});