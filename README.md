# Node.js Barcode Reader for Windows

The sample demonstrates how to use [Dynamsoft Barcode Reader SDK][1] and [Node.js][2] to create Web barcode reader on Windows.

## DBR Version
**v6.0**

## Node.js Version
**v7.9.0**

## Install node-gyp

```
npm install -g node-gyp
```

## Windows
Install [DBR for Windows][3], Visual Studio and Python v2.7.

## Getting Started
1. Edit **binding.gyp**. Replace include dir and library dir with yours. 
2. Configure building environment:

    ```
    node-gyp configure
    ```
3. Build project:

    ```
    node-gyp build
    ```
4. Run server:
    
    ```
    npm install express formidable
    node server.js
    ```
5. Visit **http://localhost:2018/index.htm**

    ![online barcode reader with nodejs](http://www.codepool.biz/wp-content/uploads/2016/01/node-online-barcode-reader.png)

## Reference
* [node-gyp][5]


[1]:http://www.dynamsoft.com/Products/Dynamic-Barcode-Reader.aspx
[2]:https://nodejs.org
[3]:http://www.dynamsoft.com/Downloads/Dynamic-Barcode-Reader-Download.aspx
[5]:https://github.com/nodejs/node-gyp

