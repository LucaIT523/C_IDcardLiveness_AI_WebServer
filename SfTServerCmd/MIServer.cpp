
#include <fstream>
#include "MIServer.h"
#include <docsdk/c_api.h>
#include "licenseproc.h"

#ifdef WIN64
#include <windows.h>
#else
#include <ctime>
#endif

uint64_t getMilliseconds() {
#ifdef WIN64
	return GetTickCount64();
#else
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
}

typedef DocSdkImage* (*DocSdkCreateImageFromFile_t)(const char*, DocSdkErrorCode*);
typedef void (*DocSdkDestroyImage_t)(DocSdkImage*);
typedef DocSdkLivenessPipelineResult(*DocSdkLivenessPipelineCheckLiveness_t)(DocSdkLivenessPipeline*, DocSdkImage*, DocSdkLivenessCheckOptions*, DocSdkErrorCode*);
typedef void (*DocSdkDestroyLivenessPipeline_t)(DocSdkLivenessPipeline*);
typedef DocSdkLivenessPipeline* (*DocSdkCreateLivenessPipeline_t)(const char*, DocSdkErrorCode*);
typedef DocSdkLivenessCheckOptions (*DocSdkLivenessCheckOptionsDefault_t)();

extern HMODULE                 g_hDocDll;
extern char                    g_path_to_pipeline_config[1024];
//extern DocSdkLivenessPipeline* g_liveness_pipeline;
extern std::string             g_data_path;

DocSdkLivenessPipeline*			g_pipeline_List[16];


ST_RESPONSE* lv_pstRes = NULL;
#define LD_MAX_TRIAL_COUNT 100
int lv_nTrialCount = 50 * 2;

std::string replaceAll(std::string original, const std::string& search, const std::string& replace) {
	size_t pos = 0;
	while ((pos = original.find(search, pos)) != std::string::npos) {
		original.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return original;
}
DocSdkLivenessPipeline* GetPipeLine(std::string& p_pipeName)
{
	if (p_pipeName == "toucan-sr") {
		return g_pipeline_List[0];
	}
	else if (p_pipeName == "ibis-sr") {
		return g_pipeline_List[1];
	}
	else if (p_pipeName == "ibis-sr-soft") {
		return g_pipeline_List[2];
	}
	else if (p_pipeName == "robin-pc") {
		return g_pipeline_List[3];
	}
	else if (p_pipeName == "penguin-pc") {
		return g_pipeline_List[4];
	}
	else if (p_pipeName == "umbrellabird-ps") {
		return g_pipeline_List[5];
	}
	else if (p_pipeName == "stork-ps") {
		return g_pipeline_List[6];
	}
	else if (p_pipeName == "hawk-sd") {
		return g_pipeline_List[7];
	}
	else {
		return g_pipeline_List[10];
	}
}
int GetPipeLinePos(std::string& p_pipeName)
{
	if (p_pipeName == "toucan-sr") {
		return 0;
	}
	else if (p_pipeName == "ibis-sr") {
		return 1;
	}
	else if (p_pipeName == "ibis-sr-soft") {
		return 2;
	}
	else if (p_pipeName == "robin-pc") {
		return 3;
	}
	else if (p_pipeName == "penguin-pc") {
		return 4;
	}
	else if (p_pipeName == "umbrellabird-ps") {
		return 5;
	}
	else if (p_pipeName == "stork-ps") {
		return 6;
	}
	else if (p_pipeName == "hawk-sd") {
		return 7;
	}
	else {
		return 10;
	}
}
std::string GetErrorMsg(DocSdkValidationStatusCode	p_StatusCode)
{
	std::string			w_Rtn = "";

	switch (p_StatusCode) {
	case kDocSdkValidationStatusCodeDocumentNotFound:
		w_Rtn = "Document is not detected in the image";
		break;
	case kDocSdkValidationStatusCodeDocumentTooCloseToCamera:
		w_Rtn = "Document is too close to the camera";
		break;
	case kDocSdkValidationStatusCodeDocumentTooCloseToBorder:
		w_Rtn = "Document is too close to the image border";
		break;
	case kDocSdkValidationStatusCodeDocumentCropped:
		w_Rtn = "Document is cropped (part of the document is not present in the image)";
		break;
	case kDocSdkValidationStatusCodeDocumentTooSmall:
		w_Rtn = "Document size in the image is too small";
		break;
	case kDocSdkValidationStatusCodeTooManyDocuments:
		w_Rtn = "Too many documents were detected in the image";
		break;
	case kDocSdkValidationStatusCodeDocumentIsColorless:
		w_Rtn = "Document is colorless (black and white)";
		break;
	case kDocSdkValidationStatusCodeDocumentPhotoNotFound:
		w_Rtn = "Document photo is not detected in the image";
		break;
	default:
		w_Rtn = "Liveness check is not possible";
		break;
	}

	return w_Rtn;
}
Object::Ptr	 GetSubProc(std::string p_pipename, std::string p_filepath, int	p_pipeInit)
{
	DocSdkErrorCode error_code = kDocSdkErrorCodeOk;
	DocSdkImage* image = NULL;
	DocSdkLivenessPipelineResult result;

	DocSdkCreateImageFromFile_t DocSdkCreateImageFromFile = NULL;
	DocSdkDestroyImage_t DocSdkDestroyImage = NULL;
	DocSdkLivenessPipelineCheckLiveness_t DocSdkLivenessPipelineCheckLiveness = NULL;
	DocSdkDestroyLivenessPipeline_t DocSdkDestroyLivenessPipeline = NULL;
	DocSdkCreateLivenessPipeline_t DocSdkCreateLivenessPipeline = NULL;
	DocSdkLivenessCheckOptionsDefault_t DocSdkLivenessCheckOptionsDefault = NULL;
	DocSdkLivenessPipeline* w_liveness_pipeline = NULL;


	DocSdkCreateImageFromFile = (DocSdkCreateImageFromFile_t)GetProcAddress(g_hDocDll, "DocSdkCreateImageFromFile");
	DocSdkDestroyImage = (DocSdkDestroyImage_t)GetProcAddress(g_hDocDll, "DocSdkDestroyImage");
	DocSdkLivenessPipelineCheckLiveness = (DocSdkLivenessPipelineCheckLiveness_t)GetProcAddress(g_hDocDll, "DocSdkLivenessPipelineCheckLiveness");
	DocSdkDestroyLivenessPipeline = (DocSdkDestroyLivenessPipeline_t)GetProcAddress(g_hDocDll, "DocSdkDestroyLivenessPipeline");
	DocSdkCreateLivenessPipeline = (DocSdkCreateLivenessPipeline_t)GetProcAddress(g_hDocDll, "DocSdkCreateLivenessPipeline");
	DocSdkLivenessCheckOptionsDefault = (DocSdkLivenessCheckOptionsDefault_t)GetProcAddress(g_hDocDll, "DocSdkLivenessCheckOptionsDefault");

	Object::Ptr subItem = new Object;

	w_liveness_pipeline = GetPipeLine(p_pipename);
	if (w_liveness_pipeline == NULL) {
		snprintf(g_path_to_pipeline_config, sizeof(g_path_to_pipeline_config), g_data_path.c_str(), p_pipename.c_str());
		w_liveness_pipeline = DocSdkCreateLivenessPipeline(g_path_to_pipeline_config, &error_code);
		g_pipeline_List[GetPipeLinePos(p_pipename)] = w_liveness_pipeline;
	}
	if (p_pipeInit == 1) {
		return subItem;
	}

	image = DocSdkCreateImageFromFile(p_filepath.c_str(), &error_code);
	if (error_code != kDocSdkErrorCodeOk) {
		subItem->set("error ", "Image has a bad quality");
	}
	else {
		DocSdkLivenessCheckOptions	w_Option = DocSdkLivenessCheckOptionsDefault();
		result = DocSdkLivenessPipelineCheckLiveness(w_liveness_pipeline, image, &w_Option, &error_code);

		std::string		w_attack_name = "";
		if (p_pipename == "toucan-sr") {
			w_attack_name = "Screen Replay";
		}
		else if (p_pipename == "robin-pc") {
			w_attack_name = "Printed Copy";
		}
		else if (p_pipename == "umbrellabird-ps") {
			w_attack_name = "Portrait Substitution";
		}
		else {
			w_attack_name = "";
		}

		//. OK
		subItem->set("attack_method ", w_attack_name.c_str());
		subItem->set("liveness_probability ", result.liveness_probability);
		subItem->set("liveness_score ", result.liveness_score);
		subItem->set("quality_score ", result.quality_score);
		if (error_code == kDocSdkErrorCodeOk) {
			subItem->set("state ", "OK");
		}
		else {
			std::string	w_strErr = GetErrorMsg(result.status_code);
			subItem->set("state ", w_strErr.c_str());
		}
	}
	if (image) {
		DocSdkDestroyImage(image);
	}
	return subItem;
}
// Critical section object
CRITICAL_SECTION g_cs;

unsigned int TF_READ_LIC(void*) {
	INT64 nSts = 0;
	ST_RESPONSE* p = (ST_RESPONSE*)malloc(sizeof(ST_RESPONSE));

	InitializeCriticalSection(&g_cs);

	while (TRUE)
	{
		Sleep(10 * 1000);

		memset(p, 0, sizeof(ST_RESPONSE));
		p->m_nProduct = GD_PRODUCT_LIVENESS_DOC;
		if ((nSts = mil_read_license(p)) <= 0) {
			if (lv_pstRes != NULL) free(lv_pstRes);
			lv_pstRes = NULL;
		}
		else {
			if (lv_pstRes == NULL) {
				lv_pstRes = (ST_RESPONSE*)malloc(sizeof(ST_RESPONSE));
			}
			memcpy(lv_pstRes, p, sizeof(ST_RESPONSE));
		}
	}
	free(p); // Clean up allocated memory
	DeleteCriticalSection(&g_cs);

}

void ClaHTTPServerWrapper::launch() {
	
	memset(g_pipeline_List, 0x00, sizeof(DocSdkLivenessPipeline*) * 16);

	GetSubProc("toucan-sr", "", 1);
	GetSubProc("robin-pc", "", 1);
	GetSubProc("umbrellabird-ps", "", 1);

	if (lv_pstRes == NULL) {
		lv_pstRes = (ST_RESPONSE*)malloc(sizeof(ST_RESPONSE));
		lv_pstRes->m_nProduct = GD_PRODUCT_LIVENESS_DOC;
		if (mil_read_license(lv_pstRes) <= 0) {
			free(lv_pstRes);
			lv_pstRes = NULL;
		}
	}

	DWORD dwTID = 0;
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TF_READ_LIC, NULL, 0, &dwTID);
	run();
}

void MyRequestHandler::OnVersion(HTTPServerRequest& request, HTTPServerResponse& response)
{
	response.setStatus(HTTPResponse::HTTP_OK);
	response.setContentType("text/plain");

	response.set("Access-Control-Allow-Origin", "*");
	response.set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
	response.set("Access-Control-Allow-Headers", "Content-Type, Authorization");

	ostream& ostr = response.send();
	char szOut[MAX_PATH]; memset(szOut, 0, sizeof(szOut));
	sprintf_s(szOut, "Version : %s\nUpdate : %s", GD_ID_VERSION, GD_ID_UPDATE);
	ostr << szOut;
}

void MyRequestHandler::OnProcessProc(HTTPServerRequest& request, HTTPServerResponse& response, Poco::Dynamic::Var procName, int base64)
{
	DocSdkErrorCode error_code = kDocSdkErrorCodeOk;
	DocSdkImage* image = NULL;
	DocSdkLivenessPipelineResult result;

	DocSdkCreateImageFromFile_t DocSdkCreateImageFromFile = NULL;
	DocSdkDestroyImage_t DocSdkDestroyImage = NULL;
	DocSdkLivenessPipelineCheckLiveness_t DocSdkLivenessPipelineCheckLiveness = NULL;
	DocSdkDestroyLivenessPipeline_t DocSdkDestroyLivenessPipeline = NULL;
	DocSdkCreateLivenessPipeline_t DocSdkCreateLivenessPipeline = NULL;
	DocSdkLivenessCheckOptionsDefault_t DocSdkLivenessCheckOptionsDefault = NULL;
	DocSdkLivenessPipeline* w_liveness_pipeline = NULL;


	DocSdkCreateImageFromFile = (DocSdkCreateImageFromFile_t)GetProcAddress(g_hDocDll, "DocSdkCreateImageFromFile");
	DocSdkDestroyImage = (DocSdkDestroyImage_t)GetProcAddress(g_hDocDll, "DocSdkDestroyImage");
	DocSdkLivenessPipelineCheckLiveness = (DocSdkLivenessPipelineCheckLiveness_t)GetProcAddress(g_hDocDll, "DocSdkLivenessPipelineCheckLiveness");
	DocSdkDestroyLivenessPipeline = (DocSdkDestroyLivenessPipeline_t)GetProcAddress(g_hDocDll, "DocSdkDestroyLivenessPipeline");
	DocSdkCreateLivenessPipeline = (DocSdkCreateLivenessPipeline_t)GetProcAddress(g_hDocDll, "DocSdkCreateLivenessPipeline");
	DocSdkLivenessCheckOptionsDefault = (DocSdkLivenessCheckOptionsDefault_t)GetProcAddress(g_hDocDll, "DocSdkLivenessCheckOptionsDefault");

	std::string pipeName = "";

	// Get the current time point
	auto now = std::chrono::system_clock::now();

	// Convert the time point to a time_t object
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);

#ifdef NDEBUG
	if (lv_pstRes == NULL || lv_pstRes->m_lExpire < now_c) {
		OnNoLicense(request, response); 
		return;
	}
#endif
	//EnterCriticalSection(&g_cs);


	std::string FileImage = "";
	try
	{
		if (base64 == 0) {
			MyPartHandler hPart;
			Poco::Net::HTMLForm form(request, request.stream(), hPart);
			Object::Ptr jsonResponse = new Poco::JSON::Object();
			FileImage = hPart.fileData();
		}
		else {
			std::string encodedImage = "";

			std::istream& input = request.stream();
			std::ostringstream ss;
			StreamCopier::copyStream(input, ss);
			std::string data = ss.str();

			Parser parser;
			auto result = parser.parse(data);
			Object::Ptr object = result.extract<Object::Ptr>();

			encodedImage = object->getValue<std::string>("image");
			// Decode Base64
			std::istringstream istr(encodedImage);
			Poco::Base64Decoder decoder(istr);
			std::vector<char> decodedBytes((std::istreambuf_iterator<char>(decoder)), std::istreambuf_iterator<char>());

			// Print decoded bytes as a string (for demonstration purposes)
			std::string decodedImage(decodedBytes.begin(), decodedBytes.end());
			FileImage = decodedImage;
		}
	}
	catch (const Exception& ex)
	{
		FileImage = "";
	}
	//if (pipeName.size() <= 0) {
	//	pipeName = "toucan-sr";
	//}
	//. Default 
	pipeName = "all";

	uint64_t milliseconds = getMilliseconds();
	std::string millisecondsStr = std::to_string(milliseconds);
	std::string filePath = millisecondsStr + "output_file.dat";
	std::ofstream outFile(filePath, std::ios::binary);
	outFile.write(FileImage.c_str(), FileImage.size());
	outFile.close();

	try
	{
		// Send POST request
		std::string strRsp;
		HTTPServerResponse::HTTPStatus status = HTTPResponse::HTTP_OK;
		std::string out;
		std::ostringstream oss;
		Object::Ptr root = new Object;
		Poco::JSON::Array	w_arr;

		if (pipeName == "all") {
			pipeName = "toucan-sr";
			Object::Ptr w_subJSON = GetSubProc(pipeName, filePath, 0);
			w_arr.add(w_subJSON);

			pipeName = "robin-pc";
			w_subJSON = GetSubProc(pipeName, filePath, 0);
			w_arr.add(w_subJSON);

			pipeName = "umbrellabird-ps";
			w_subJSON = GetSubProc(pipeName, filePath, 0);
			w_arr.add(w_subJSON);
		}
		else {
			w_liveness_pipeline = GetPipeLine(pipeName);
			if (w_liveness_pipeline == NULL) {
				snprintf(g_path_to_pipeline_config, sizeof(g_path_to_pipeline_config), g_data_path.c_str(), pipeName.c_str());
				w_liveness_pipeline = DocSdkCreateLivenessPipeline(g_path_to_pipeline_config, &error_code);
				g_pipeline_List[GetPipeLinePos(pipeName)] = w_liveness_pipeline;
			}

			if (w_liveness_pipeline == NULL) {
				root->set("error ", "The pipeline is not correct. (ex : toucan-sr, ibis-sr, ibis-sr-soft, robin-pc, penguin-pc, umbrellabird-ps, stork-ps, hawk-sd)");
			}
			else {
				image = DocSdkCreateImageFromFile(filePath.c_str(), &error_code);
				if (error_code != kDocSdkErrorCodeOk) {
					root->set("error ", "CreateImageFromFile");
				}
				else {
					DocSdkLivenessCheckOptions	w_Option = DocSdkLivenessCheckOptionsDefault();
					result = DocSdkLivenessPipelineCheckLiveness(w_liveness_pipeline, image, &w_Option, &error_code);
					//. OK
					root->set("pipeline ", pipeName.c_str());
					root->set("liveness_probability ", result.liveness_probability);
					root->set("liveness_score ", result.liveness_score);
					root->set("quality_score ", result.quality_score);
					if (error_code == kDocSdkErrorCodeOk) {
						root->set("state ", "OK");
					}
					else {
						std::string	w_strErr = GetErrorMsg(result.status_code);
						root->set("state ", w_strErr.c_str());
					}
				}
				if (image) {
					DocSdkDestroyImage(image);
				}

			}
		}

		//if (g_liveness_pipeline) {
		//	DocSdkDestroyLivenessPipeline(g_liveness_pipeline);
		//	g_liveness_pipeline = NULL;
		//}

		Stringifier::stringify(w_arr, oss);
		out = oss.str();
		//.
		std::remove(filePath.c_str());

		response.setStatus(status);
		response.setContentType("application/json");
		response.setContentLength(out.length());

		response.set("Access-Control-Allow-Origin", "*");
		response.set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
		response.set("Access-Control-Allow-Headers", "Content-Type, Authorization");

		response.send() << out.c_str();

	}
	catch (const Exception& ex)
	{
		response.setStatus(HTTPResponse::HTTP_CONFLICT);
		response.setContentType("application/json");

		response.set("Access-Control-Allow-Origin", "*");
		response.set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
		response.set("Access-Control-Allow-Headers", "Content-Type, Authorization");

		response.setContentLength(ex.displayText().length());
		response.send() << ex.displayText();
	}

	//LeaveCriticalSection(&g_cs);

}

void MyRequestHandler::OnUnknown(HTTPServerRequest& request, HTTPServerResponse& response)
{
	response.setStatus(HTTPResponse::HTTP_OK);
	response.setContentType("text/plain");

	response.set("Access-Control-Allow-Origin", "*");
	response.set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
	response.set("Access-Control-Allow-Headers", "Content-Type, Authorization");

	ostream& ostr = response.send();
	ostr << "Not found";
}

void MyRequestHandler::OnNoLicense(HTTPServerRequest& request, HTTPServerResponse& response)
{
	response.setStatus(HTTPResponse::HTTP_OK);
	response.setContentType("text/plain");

	response.set("Access-Control-Allow-Origin", "*");
	response.set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
	response.set("Access-Control-Allow-Headers", "Content-Type, Authorization");

	ostream& ostr = response.send();
	ostr << "Please input license.";
}

void MyRequestHandler::OnStatus(HTTPServerRequest& request, HTTPServerResponse& response)
{
	response.setStatus(HTTPResponse::HTTP_OK);
	response.setContentType("text/plain");

	response.set("Access-Control-Allow-Origin", "*");
	response.set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
	response.set("Access-Control-Allow-Headers", "Content-Type, Authorization");

	ostream& ostr = response.send();
	

	if (lv_pstRes == NULL) {
		//. no license
		ostr << "License not found";
	}
	else {
		time_t t = lv_pstRes->m_lExpire;
		struct tm timeinfo;
		localtime_s(&timeinfo, &t);
		char szTime[260]; memset(szTime, 0, sizeof(szTime));
		if (lv_pstRes->m_lExpire < 32503622400) {
			strftime(szTime, 260, "License valid : %Y-%m-%d", &timeinfo);
		}
		else {
			sprintf_s(szTime, 260, "License valid : NO LIMIT");
		}
		ostr << szTime;
	}
}