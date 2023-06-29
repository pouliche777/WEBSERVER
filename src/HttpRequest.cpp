
#include "HttpRequest.hpp"

void HttpRequest::parseRequest(std::string rawRequest, const ServerConfiguration& config) {
    std::string method, path, line;
    std::map<std::string, std::string> headers;
    std::istringstream request(rawRequest);
    // Parse the request line to get the method and path
    request >> method >> path;
    if (method == "GET") {
        size_t quePos = path.find("?");
        if (quePos != std::string::npos) {
            this->queryString = path.substr(quePos + 1);
            path = path.substr(0, quePos);
        }
    }
    // Parse the headers
    while (std::getline(request, line) && !line.empty()) {
        std::string headerName, headerValue;
        size_t colonPos = line.find(":");
        if (colonPos != std::string::npos) {
            headerName = line.substr(0, colonPos);
            headerValue = line.substr(colonPos + 1);
            headerValue = std::regex_replace(headerValue, std::regex("^\\s+|\\s+$"), "");
            headers[headerName] = headerValue;
        }
    }
    //find the end of the headers and throw the rest into the ody varariable
    size_t separatorPos = rawRequest.find("\r\n\r\n");
    if (separatorPos != std::string::npos) {
        this->body = rawRequest.substr(separatorPos + 4);
    } else {
        this->body = "";
    }
    if (path.empty() || path == "/") {
        path = "/index.html";
    }
    this->headers = headers;
    this->method = method;
    this->path = config.getDocumentRoot() + path;
}

void HttpRequest::validityCheck(){
}
void HttpRequest::checkCgi(const ServerConfiguration& config){
    if(this->path == config.getCgiRoot() + "/test.php"){
        this->isCgi = true;
    }
     else
        this->isCgi = false;
 }
 void HttpRequest::checkDownload(const ServerConfiguration& config){
    if(endsWith(this->path, ".pdf")){
        this->toBeDownloaded = true;
    }
     else
        this->toBeDownloaded = false;
    std::string test = config.getCgiRoot();

 }
// Default destructor
HttpRequest::~HttpRequest() { return; }
