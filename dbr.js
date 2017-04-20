var dbr = require('./build/Release/dbr');
var readline = require('readline');
var fs = require('fs');
var barcodeTypes = 0x3FF | 0x2000000 | 0x8000000 | 0x4000000; // 1D, QRCODE, PDF417, DataMatrix

function decodeFileAsync(fileName) {
    dbr
        .decodeFileAsync(fileName, barcodeTypes, function (msg) {
            var result = null;
            for (index in msg) {
                result = msg[index]
                console.log("Format: " + result['format']);
                console.log("Value : " + result['value']);
                console.log("##################");
            }
        });
}

function decodeFileStreamAsync(fileName) {
    var stats = fs.statSync(fileName);
    var fileSize = stats["size"];

    fs.open(fileName, 'r', function (status, fd) {
        if (status) {
            console.log(status.message);
            return;
        }
        var buffer = new Buffer(fileSize);
        fs.read(fd, buffer, 0, fileSize, 0, function (err, bytesRead, data) {
            dbr
                .decodeFileStreamAsync(buffer, fileSize, barcodeTypes, function (msg) {
                    var result = null;
                    for (index in msg) {
                        result = msg[index]
                        console.log("Format: " + result['format']);
                        console.log("Value : " + result['value']);
                        console.log("##################");
                    }
                });
        });
    });
}

function decodeYUYVAsync(fileName, width, height) {
    var stats = fs.statSync(fileName);
    var fileSize = stats["size"];

    fs.open(fileName, 'r', function (status, fd) {
        if (status) {
            console.log(status.message);
            return;
        }
        var buffer = new Buffer(fileSize);
        fs.read(fd, buffer, 0, fileSize, 0, function (err, bytesRead, data) {
            dbr
                .decodeYUYVAsync(buffer, width, height, barcodeTypes, function (msg) {
                    var result = null;
                    for (index in msg) {
                        result = msg[index]
                        console.log("Format: " + result['format']);
                        console.log("Value : " + result['value']);
                        console.log("##################");
                    }
                });
        });
    });
}

var rl = readline.createInterface({input: process.stdin, output: process.stdout});

rl.question("Please input a barcode image path: ", function (answer) {
    // Please contact support@dynamsoft.com to get a valid trial or full license.
    dbr.initLicense("t0260NQAAAAGDPD614k9U/RuRW4tF1CQ0GmFstgNuhZTHo8CCMqBskFO5P10LCIAW8EgN7rrKXsfqJGR" +
            "Hc9qtYJcwM3/tIPlczIkBPgam5x57zLzaPeTS5Qce70qAAUrUp8UTDBKwTbYq59braF1h61920DXuHuX" +
            "YHQCybmsD/f0YPOrgHg5MmZH1TaKT7NDeJM75sR+HJ+sQxOlV8g3Aca2Jg1iLEHZnA0PWVC0TqWxP2vE" +
            "CCEriUwZzRnKOn6wBKhcpkgbUCW6kjHpv0XPc/5+s1xWuwN0t/Pk15dz5lTelL6s5bvvWCLNpnOh4o4o" +
            "Efz1eYi5z51T+Y6fF91chBwTlAheCIzs=");
    decodeFileStreamAsync(answer);
    decodeFileAsync(answer);
    // decodeYUYVAsync(answer, 640, 480);
    rl.close();
});