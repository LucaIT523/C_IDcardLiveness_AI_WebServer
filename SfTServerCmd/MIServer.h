#pragma once

#include "MiConf.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Net/HTTPClientSession.h"
// #include "Poco/Net/HTTPClientRequest.h"
// #include "Poco/Net/HTTPClientResponse.h"

//#include "Poco/Net/StreamCopier.h"
#include "Poco/Net/MultipartReader.h"
#include "Poco/Net/MessageHeader.h"
#include "Poco/Net/MediaType.h"
#include <Poco/Net/NameValueCollection.h>
#include <Poco/Net/HTMLForm.h>
#include "Poco/Net/PartHandler.h"
#include <Poco/StringTokenizer.h>

#include <Poco/Net/MessageHeader.h>
#include <Poco/Base64Encoder.h>
#include <Poco/Base64Decoder.h>
#include <Poco/StreamCopier.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/URI.h>

#include "Poco/StreamCopier.h"
#include "Poco/Exception.h"
#include "Poco/Base64Encoder.h"
#include <iostream>
#include "../cmn/MiKeyMgr.h"
#include <unordered_map>
#include <string>

#include <sstream>

using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;
using namespace Poco::JSON;
using namespace std;



// Define a request handler to handle incoming HTTP requests
class MyRequestHandler : public HTTPRequestHandler {
public:
	void OnVersion(HTTPServerRequest& request, HTTPServerResponse& response);
	//. 	
	void OnProcessProc(HTTPServerRequest& request, HTTPServerResponse& response, Poco::Dynamic::Var procName, int base64 = 0);

	void OnUnknown(HTTPServerRequest& request, HTTPServerResponse& response);
	void OnNoLicense(HTTPServerRequest& request, HTTPServerResponse& response);
	void OnStatus(HTTPServerRequest& request, HTTPServerResponse& response);
public:
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
		try {
			OutputDebugStringA(request.getURI().c_str());
			if (request.getURI().compare(GD_API_VERSION) == 0) {
				OnVersion(request, response);
				return;
			}
			if (request.getURI().compare(GD_API_FULL_PROCESS) == 0) {
				OnProcessProc(request, response, "FullProcess");
				return;
			}
			if (request.getURI().compare(GD_API_FULL_PROCESS_BASE64) == 0) {
				OnProcessProc(request, response, "FullProcess", 1);
				return;
			}
			if (request.getURI().compare(GD_API_STATUS) == 0 ){
				OnStatus(request, response);
				return;
			}

			OnUnknown(request, response);
		}
		catch (Poco::Exception& ex) {
			response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
			response.send() << "Error: " << ex.displayText();
		}

	}
};

// Define a request handler factory to create instances of MyRequestHandler
class MyRequestHandlerFactory : public HTTPRequestHandlerFactory {
public:
	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest&) override {
		return new MyRequestHandler;
	}
};
// Define the main application class
class ClaHTTPServerWrapper : public ServerApplication {
public:
	void launch();
protected:
	int main(const vector<string>&) override {
		// Set up server parameters
		HTTPServerParams* params = new HTTPServerParams;
		params->setMaxQueued(100);
		params->setMaxThreads(16);

		// Create a new HTTPServer instance
		HTTPServer server(new MyRequestHandlerFactory, ServerSocket(GD_PORT_IN), params);

		// Start the server
		server.start();
		cout << "Server started on port " << GD_PORT_IN << "." << endl;

		// Wait for CTRL-C or termination signal
		waitForTerminationRequest();

		// Stop the server
		server.stop();
		cout << "Server stopped." << endl;

		return Application::EXIT_OK;
	}
};

class MyPartHandler : public Poco::Net::PartHandler
{
public:
	void handlePart(const Poco::Net::MessageHeader& header, std::istream& stream) override
	{
		if (header.has("Content-Disposition"))
		{
			std::string disposition = header.get("Content-Disposition");
			if (disposition.find("form-data") != std::string::npos)
			{
				std::string name = extractParameter(disposition, "name");
				std::string filename = extractParameter(disposition, "filename");

				if (!filename.empty())
				{
					// Handle file part
					std::ostringstream binaryStream;
					Poco::StreamCopier::copyStream(stream, binaryStream);
					_fileData = binaryStream.str();
					_filename = filename;

					// Encode the binary data to base64
					std::ostringstream base64Stream;
					Poco::Base64Encoder base64Encoder(base64Stream);
					base64Encoder << _fileData;
					base64Encoder.close();
					_base64Data = base64Stream.str();
				}
			}
		}
	}

	const std::string& fileData() const { return _fileData; }
	const std::string& base64Data() const { return _base64Data; }
	const std::string& filename() const { return _filename; }

private:
	std::string _fileData;
	std::string _base64Data;
	std::string _filename;

	std::string extractParameter(const std::string& headerValue, const std::string& paramName)
	{
		std::string param = paramName + "=\"";
		size_t startPos = headerValue.find(param);
		if (startPos != std::string::npos)
		{
			startPos += param.length();
			size_t endPos = headerValue.find("\"", startPos);
			if (endPos != std::string::npos)
			{
				return headerValue.substr(startPos, endPos - startPos);
			}
		}
		return "";
	}

};
