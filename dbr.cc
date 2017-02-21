#include <node.h>
#include <string.h>
#include <uv.h>
#include "If_DBR.h"
#include "BarcodeFormat.h"
#include "BarcodeStructs.h"
#include "ErrorCode.h"

using namespace v8;

// Barcode format
const char * GetFormatStr(__int64 format)
{
	if (format == CODE_39)
		return "CODE_39";
	if (format == CODE_128)
		return "CODE_128";
	if (format == CODE_93)
		return "CODE_93";
	if (format == CODABAR)
		return "CODABAR";
	if (format == ITF)
		return "ITF";
	if (format == UPC_A)
		return "UPC_A";
	if (format == UPC_E)
		return "UPC_E";
	if (format == EAN_13)
		return "EAN_13";
	if (format == EAN_8)
		return "EAN_8";
	if (format == INDUSTRIAL_25)
		return "INDUSTRIAL_25";
	if (format == QR_CODE)
		return "QR_CODE";
	if (format == PDF417)
		return "PDF417";
	if (format == DATAMATRIX)
		return "DATAMATRIX";

	return "UNKNOWN";
}

struct BarcodeWorker
{
    uv_work_t request;              // libuv
    pBarcodeResult* ppBarcodes;		// barcode result
	Persistent<Function> callback;	// javascript callback		
	__int64 llFormat;				// barcode format
	char filename[128];				// file name
	pBarcodeResultArray pResults;	// result pointer
};

static void DetectionWorking(uv_work_t *req)
{
	// get the reference to BarcodeWorker
    BarcodeWorker *worker = static_cast<BarcodeWorker *>(req->data);

	// Dynamsoft Barcode Reader initialization
	int iMaxCount = 0x7FFFFFFF;
	ReaderOptions ro = {0};
	pBarcodeResultArray pResults = NULL;

	ro.llBarcodeFormat = worker->llFormat;
	ro.iMaxBarcodesNumPerPage = iMaxCount;

	// Decode barcode image
	int ret = DBR_DecodeFile(worker->filename, &ro, &pResults);
	printf("DBR_DecodeFile ret: %d\n", ret);

	// save result pointer to BarcodeWorker
	worker->pResults = pResults;
}

static void DetectionDone(uv_work_t *req,int status)
{
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

    // get the reference to BarcodeWorker
    BarcodeWorker *worker = static_cast<BarcodeWorker *>(req->data);

	// construct the results
	pBarcodeResultArray pResults = worker->pResults;
	int count = pResults->iBarcodeCount;
	pBarcodeResult* ppBarcodes = pResults->ppBarcodes;
	pBarcodeResult tmp = NULL;

	// array for storing barcode results
	Local<Array> barcodeResults = Array::New(isolate);

	for (int i = 0; i < count; i++)
	{
		tmp = ppBarcodes[i];

		Local<Object> result = Object::New(isolate);
		result->Set(String::NewFromUtf8(isolate, "format"), String::NewFromUtf8(isolate, GetFormatStr(tmp->llFormat)));
		result->Set(String::NewFromUtf8(isolate, "value"), String::NewFromUtf8(isolate, tmp->pBarcodeData));

		barcodeResults->Set(Number::New(isolate, i), result);
	}

	// release memory
	DBR_FreeBarcodeResults(&pResults);

	const unsigned argc = 1;
	Local<Value> argv[argc] = { barcodeResults };

    // execute the callback
	Local<Function> cb = Local<Function>::New(isolate, worker->callback);
    cb->Call(isolate->GetCurrentContext()->Global(), argc, argv);

    delete worker;
}

void InitLicense(const FunctionCallbackInfo<Value>& args) {

	String::Utf8Value license(args[0]->ToString());
	char *pszLicense = *license;
	DBR_InitLicense(pszLicense);

	printf("License is %s\n", pszLicense);
}

void DecodeFile(const FunctionCallbackInfo<Value>& args) {

	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	// convert v8 string to char *
	String::Utf8Value fileName(args[0]->ToString());
	char *pFileName = *fileName;
	// get barcode format value
	__int64 llFormat = args[1]->IntegerValue();
	// javascript callback function
	Local<Function> cb = Local<Function>::Cast(args[2]);
	const unsigned argc = 1;

	// Dynamsoft Barcode Reader initialization
	int iMaxCount = 0x7FFFFFFF;
	ReaderOptions ro = {0};
	pBarcodeResultArray pResults = NULL;

	ro.llBarcodeFormat = llFormat;
	ro.iMaxBarcodesNumPerPage = iMaxCount;

	// Decode barcode image
	int ret = DBR_DecodeFile(pFileName, &ro, &pResults);
	printf("DBR_DecodeFile ret: %d\n", ret);

	int count = pResults->iBarcodeCount;
	pBarcodeResult* ppBarcodes = pResults->ppBarcodes;
	pBarcodeResult tmp = NULL;

	// array for storing barcode results
	Local<Array> barcodeResults = Array::New(isolate);

	for (int i = 0; i < count; i++)
	{
		tmp = ppBarcodes[i];

		Local<Object> result = Object::New(isolate);
		result->Set(String::NewFromUtf8(isolate, "format"), String::NewFromUtf8(isolate, GetFormatStr(tmp->llFormat)));
		result->Set(String::NewFromUtf8(isolate, "value"), String::NewFromUtf8(isolate, tmp->pBarcodeData));

		barcodeResults->Set(Number::New(isolate, i), result);
	}

	// release memory
	DBR_FreeBarcodeResults(&pResults);

	Local<Value> argv[argc] = { barcodeResults };
	cb->Call(isolate->GetCurrentContext()->Global(), argc, argv);
}

void DecodeFileAsync(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	// get arguments
	String::Utf8Value fileName(args[0]->ToString());
	char *pFileName = *fileName;
	__int64 llFormat = args[1]->IntegerValue();
	Local<Function> cb = Local<Function>::Cast(args[2]);

	BarcodeWorker *worker = new BarcodeWorker;
	worker->request.data = worker;
	worker->ppBarcodes = NULL;
	strcpy(worker->filename, pFileName);
	worker->callback.Reset(isolate, cb);
	worker->llFormat = llFormat;
	worker->pResults = NULL;
	
	uv_queue_work(uv_default_loop(), &worker->request, (uv_work_cb)DetectionWorking, (uv_after_work_cb)DetectionDone);
}

void Init(Handle<Object> exports) {
	NODE_SET_METHOD(exports, "initLicense", InitLicense);
	NODE_SET_METHOD(exports, "decodeFile", DecodeFile);
	NODE_SET_METHOD(exports, "decodeFileAsync", DecodeFileAsync);
}

NODE_MODULE(dbr, Init)
