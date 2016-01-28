# Node.js Barcode Reader for Windows, Linux & Mac

The sample demonstrates how to use [Dynamsoft Barcode Reader SDK][1] and [Node.js][2] to create Web barcode reader on Windows, Linux and Mac.

## Install node-gyp

```
npm install -g node-gyp
```

## Install DBR for Windows
[DBR for Windows][3]

## Install DBR for Linux
[DBR for Linux][4]

## Install DBR for Mac
[DBR for Mac][3]

## Getting Started
1. Edit **binding.gyp**. Replace **< DBR Installation Directory of Includes and Libs >** with yours. 
2. Configure build:

    ```
    node-gyp configure
    ```
3. Build project:

    ```
    node-gyp build
    ```
4. Run server:
    
    ```
    node server.js
    ```
5. Visit **http://localhost:2015/index.htm**

    ![online barcode reader with nodejs](http://www.codepool.biz/wp-content/uploads/2016/01/node-online-barcode-reader.png)

[1]:http://www.dynamsoft.com/Products/Dynamic-Barcode-Reader.aspx
[2]:https://nodejs.org
[3]:http://www.dynamsoft.com/Downloads/Dynamic-Barcode-Reader-Download.aspx
[4]:https://www.dynamsoft.com/Downloads/DownloadLog.aspx?server=1&product=support/dbr-4.0.0-pre-alpha.tar.gz

