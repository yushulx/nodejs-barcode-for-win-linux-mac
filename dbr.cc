#include <node.h>
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

void DecodeFile(const FunctionCallbackInfo<Value>& args) {

	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	// convert v8 string to char *
	String::Utf8Value license(args[0]->ToString());
	String::Utf8Value fileName(args[1]->ToString());
	char *pFileName = *fileName;
	char *pszLicense = *license;
	// Dynamsoft Barcode Reader: init
	__int64 llFormat = args[2]->IntegerValue();
	int iMaxCount = 0x7FFFFFFF;
	ReaderOptions ro = {0};
	pBarcodeResultArray pResults = NULL;

	// Initialize license
	if (pszLicense && (strcmp(pszLicense, "null") !=0) )
	{
		printf("license: %s\n", pszLicense);
		DBR_InitLicense(pszLicense);
	}
	else {
		#ifdef WINDOWS_DBR
				printf("Windows dbr license\n");
				DBR_InitLicense("B3E1E81166C5AB81F245D1F0EF130807");
		#elif LINUX_DBR
				printf("Linux dbr license\n");
				DBR_InitLicense("AC4561856D63EF392F46D7454052372D");
		#elif MAC_DBR
				printf("Mac dbr license\n");
				DBR_InitLicense("B3E1E81166C5AB819AB61611257829F6");
		#endif
	}

	ro.llBarcodeFormat = llFormat;
	ro.iMaxBarcodesNumPerPage = iMaxCount;

	// Decode barcode image
	int ret = DBR_DecodeFile(pFileName, &ro, &pResults);
	printf("DBR_DecodeFile ret: %d\n", ret);

	if (ret == DBR_OK){
		int count = pResults->iBarcodeCount;
		pBarcodeResult* ppBarcodes = pResults->ppBarcodes;
		pBarcodeResult tmp = NULL;

		// javascript callback function
		Local<Function> cb = Local<Function>::Cast(args[3]);
		const unsigned argc = 1;

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
}

void Init(Handle<Object> exports) {
	NODE_SET_METHOD(exports, "decodeFile", DecodeFile);
}

NODE_MODULE(dbr, Init)
