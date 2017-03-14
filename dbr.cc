#include <node.h>
#include <node_buffer.h>
#include <string.h>
#include <uv.h>
#include "DynamsoftBarcodeReader.h"

using namespace v8;

// barcode reader handler
void* hBarcode = NULL; 

// Barcode format
const char * GetFormatStr(int format)
{
	if (format == BF_CODE_39)
		return "CODE_39";
	if (format == BF_CODE_128)
		return "CODE_128";
	if (format == BF_CODE_93)
		return "CODE_93";
	if (format == BF_CODABAR)
		return "CODABAR";
	if (format == BF_ITF)
		return "ITF";
	if (format == BF_UPC_A)
		return "UPC_A";
	if (format == BF_UPC_E)
		return "UPC_E";
	if (format == BF_EAN_13)
		return "EAN_13";
	if (format == BF_EAN_8)
		return "EAN_8";
	if (format == BF_INDUSTRIAL_25)
		return "INDUSTRIAL_25";
	if (format == BF_QR_CODE)
		return "QR_CODE";
	if (format == BF_PDF417)
		return "PDF417";
	if (format == BF_DATAMATRIX)
		return "DATAMATRIX";

	return "UNKNOWN";
}

typedef enum {
	NO_BUFFER,
	FILE_STREAM,
	YUYV_BUFFER,	
} BufferType;

struct BarcodeWorker
{
    uv_work_t request;              // libuv
	Persistent<Function> callback;	// javascript callback		
	int iFormat;				// barcode types
	char filename[128];				// file name
	SBarcodeResultArray *pResults;	// result pointer
	unsigned char *buffer;		    
	int size;						// file size
	int errorCode;					// detection error code
	int width;						// image width
	int height; 					// image height
	BufferType bufferType;			// buffer type
};

bool ConvertCameraGrayDataToDIBBuffer(unsigned char* psrc, int size, int width, int height, unsigned char** ppdist, int *psize)
{
	BITMAPINFOHEADER bitmapInfo;
	memset(&bitmapInfo, 0, sizeof(BITMAPINFOHEADER));

	bitmapInfo.biBitCount = 8;
	bitmapInfo.biWidth = width;
	bitmapInfo.biHeight = height;
	bitmapInfo.biSize = sizeof(BITMAPINFOHEADER);

	int iStride = ((width * 8 + 31) >> 5) << 2;
	int iImageSize = iStride * height;
	if (size < iImageSize)
		return false;

	bitmapInfo.biSizeImage = iImageSize;

	*psize = iImageSize + bitmapInfo.biSize + 1024;
	*ppdist = new unsigned char[*psize];

	//1. copy BITMAPINFOHEADER
	memcpy(*ppdist, &bitmapInfo, sizeof(BITMAPINFOHEADER));

	//2. copy gray color map
	char rgba[1024] = { 0 };
	for (int i = 0; i < 256; ++i)
	{
		rgba[i * 4] = rgba[i * 4 + 1] = rgba[i * 4 + 2] = rgba[i * 4 + 3] = i;
	}
	memcpy(*ppdist + sizeof(BITMAPINFOHEADER), rgba, 1024);

	//3. copy gray data (should be fliped)
	unsigned char* srcptr = psrc + (height - 1)*width;
	unsigned char* dstptr = *ppdist + sizeof(BITMAPINFOHEADER) + 1024;

	for (int j = 0; j < height; ++j, srcptr -= width, dstptr += iStride)
	{
		memcpy(dstptr, srcptr, width);
	}

	return true;
}

/*
 *	uv_work_cb
 */
static void DetectionWorking(uv_work_t *req)
{
	if (!hBarcode)
	{
		printf("Barcode reader handler not initialized.\n");
		return;
	}
	// get the reference to BarcodeWorker
    BarcodeWorker *worker = static_cast<BarcodeWorker *>(req->data);

	// initialize Dynamsoft Barcode Reader
	int iMaxCount = 0x7FFFFFFF;
	SBarcodeResultArray *pResults = NULL;
	DBR_SetBarcodeFormats(hBarcode, worker->iFormat);
	DBR_SetMaxBarcodesNumPerPage(hBarcode, iMaxCount);

	// decode barcode image
	int ret = 0;
	switch(worker->bufferType)
	{
		case FILE_STREAM:
			{
				if (worker->buffer)
					ret = DBR_DecodeStreamEx(hBarcode, worker->buffer, worker->size, &pResults);
			}
			break;
		case YUYV_BUFFER:
			{
				if (worker->buffer)
				{
					int dibsize = 0;
					int width = worker->width, height = worker->height;
					int size = width * height;
					int index = 0;
					unsigned char* data = new unsigned char[size];
					// get Y from YUYV
					for (int i = 0; i < size; i++)
					{
						data[i] = worker->buffer[index];
						index += 2;
					}
					// gray conversion
					// ConvertCameraGrayDataToDIBBuffer(data, size, width, height, &pdibdata, &dibsize);
					// read barcode
					ret = DBR_DecodeBufferEx(hBarcode, data, width, height, width, IPF_GrayScaled, &pResults);
					// release memory
					delete []data, data=NULL;
				}
			}
			break;
		default:
			{
				ret = DBR_DecodeFileEx(hBarcode, worker->filename, &pResults);
			}
	}
	
	if (ret)
		printf("Detection error code: %d\n", ret);

	// save results to BarcodeWorker
	worker->errorCode = ret;
	worker->pResults = pResults;
}

/*
 *	uv_after_work_cb
 */
static void DetectionDone(uv_work_t *req,int status)
{
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

    // get the reference to BarcodeWorker
    BarcodeWorker *worker = static_cast<BarcodeWorker *>(req->data);

	// get barcode results
	SBarcodeResultArray *pResults = worker->pResults;
	int errorCode = worker->errorCode;
	int count = pResults->iBarcodeCount;
	SBarcodeResult** ppBarcodes = pResults->ppBarcodes;
	SBarcodeResult* tmp = NULL;

	// array for storing barcode results
	Local<Array> barcodeResults = Array::New(isolate);

	for (int i = 0; i < count; i++)
	{
		tmp = ppBarcodes[i];

		Local<Object> result = Object::New(isolate);
		result->Set(String::NewFromUtf8(isolate, "format"), String::NewFromUtf8(isolate, GetFormatStr(tmp->emBarcodeFormat)));
		result->Set(String::NewFromUtf8(isolate, "value"), String::NewFromUtf8(isolate, tmp->pBarcodeData));
		barcodeResults->Set(Number::New(isolate, i), result);
	}

	// release memory of barcode results
	DBR_FreeBarcodeResults(&pResults);

    // run the callback
	const unsigned argc = 1;
	Local<Value> argv[argc] = {barcodeResults};
	Local<Function> cb = Local<Function>::New(isolate, worker->callback);
    cb->Call(isolate->GetCurrentContext()->Global(), argc, argv);

	// release memory of BarcodeWorker
    delete worker;
}

/*
 *	initLicense(license)
 */
void InitLicense(const FunctionCallbackInfo<Value>& args) {

	String::Utf8Value license(args[0]->ToString());
	char *pszLicense = *license;
	hBarcode = DBR_CreateInstance();
	DBR_InitLicenseEx(hBarcode, pszLicense);
}

/*
 *	decodeFileAsync(fileName, barcodeTypes, callback)
 */
void DecodeFileAsync(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	// get arguments
	String::Utf8Value fileName(args[0]->ToString()); // file name
	char *pFileName = *fileName;
	int iFormat = args[1]->IntegerValue(); // barcode types
	Local<Function> cb = Local<Function>::Cast(args[2]); // javascript callback function

	// initialize BarcodeWorker
	BarcodeWorker *worker = new BarcodeWorker;
	worker->request.data = worker;
	strcpy(worker->filename, pFileName);
	worker->callback.Reset(isolate, cb);
	worker->iFormat = iFormat;
	worker->pResults = NULL;
	worker->buffer = NULL;
	worker->bufferType = NO_BUFFER;
	
	uv_queue_work(uv_default_loop(), &worker->request, (uv_work_cb)DetectionWorking, (uv_after_work_cb)DetectionDone);
}

/*
 *	decodeFileStreamAsync(fileStream, fileSize, barcodeTypes, callback)
 */
void DecodeFileStreamAsync(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	// get arguments
	unsigned char* buffer = (unsigned char*) node::Buffer::Data(args[0]->ToObject()); // file stream
	int fileSize = args[1]->IntegerValue();	// file size
	int iFormat = args[2]->IntegerValue(); // barcode types
	Local<Function> cb = Local<Function>::Cast(args[3]); // javascript callback function

	// initialize BarcodeWorker
	BarcodeWorker *worker = new BarcodeWorker;
	worker->request.data = worker;
	worker->callback.Reset(isolate, cb);
	worker->iFormat = iFormat;
	worker->pResults = NULL;
	worker->buffer = buffer;
	worker->size = fileSize;
	worker->bufferType = FILE_STREAM;
	
	uv_queue_work(uv_default_loop(), &worker->request, (uv_work_cb)DetectionWorking, (uv_after_work_cb)DetectionDone);
}

/*
 *	decodeYUYVAsync(buffer, width, height, barcodeTypes, callback)
 */
void DecodeYUYVAsync(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	// get arguments
	unsigned char* buffer = (unsigned char*) node::Buffer::Data(args[0]->ToObject()); // file stream
	int width = args[1]->IntegerValue();	// image width
	int height = args[2]->IntegerValue();	// image height
	int iFormat = args[3]->IntegerValue(); // barcode types
	Local<Function> cb = Local<Function>::Cast(args[4]); // javascript callback function

	// initialize BarcodeWorker
	BarcodeWorker *worker = new BarcodeWorker;
	worker->request.data = worker;
	worker->callback.Reset(isolate, cb);
	worker->iFormat = iFormat;
	worker->pResults = NULL;
	worker->buffer = buffer;
	worker->width = width;
	worker->height = height;
	worker->bufferType = YUYV_BUFFER;
	
	uv_queue_work(uv_default_loop(), &worker->request, (uv_work_cb)DetectionWorking, (uv_after_work_cb)DetectionDone);
}

void Init(Handle<Object> exports) {
	NODE_SET_METHOD(exports, "decodeYUYVAsync", DecodeYUYVAsync);
	NODE_SET_METHOD(exports, "decodeFileStreamAsync", DecodeFileStreamAsync);
	NODE_SET_METHOD(exports, "initLicense", InitLicense);
	NODE_SET_METHOD(exports, "decodeFileAsync", DecodeFileAsync);
}

NODE_MODULE(dbr, Init)
