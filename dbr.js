var dbr = require('./build/Release/dbr');
var readline = require('readline');
var fs = require('fs');

fs.readFile('./license.txt', 'utf8', function (err, data) {
  if (err) throw err;
  var rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
  });

  var license = data.trim();
  rl.question("Please input a barcode image path: ", function(answer) {

    dbr.decodeFile(
      license, answer, 0x3FF | 0x2000000 | 0x8000000 | 0x4000000,
      function(msg){
        var result = null;
        for (index in msg) {
          result = msg[index]
          console.log(result['format']);
          console.log(result['value']);
          console.log("##################");
        }
      }
    );

    rl.close();
  });
});
