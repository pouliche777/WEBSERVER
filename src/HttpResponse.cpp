
#include "../inc/HttpResponse.hpp"
// constructor
HttpResponse::HttpResponse(const HttpRequest& clientRequest){
    analyseRequest(clientRequest);
}

int HttpResponse::analyseRequest(const HttpRequest& clientRequest){
    if (!fileExist(clientRequest.path)){
        this->statusCode = "404";
        this->body=extractFileContent("/Users/slord/Desktop/13-WEBSERVER/html/404.html");
        this->headers["contentType"] = "text/html";
        return (1);
    }
    else if (clientRequest.method == "GET"){
        this->statusCode = "200";
        this->body = extractFileContent(clientRequest.path);
        this->headers["contentType"] = "text/html";
        this->headers["contentLength"] = std::to_string(this->body.length());
        return (0);

    }
    std::string CgiPath = clientRequest.config.getCgiRoot();
    if (CgiPath + "test.php" ==  clientRequest.path) {
        if (clientRequest.headers.find("Content-Type") != clientRequest.headers.end() && clientRequest.headers.at("Content-Type") == "application/x-www-form-urlencoded"){
            try{
                std::string output  = executeCgi(clientRequest.path, clientRequest.body);
                analyseCgiOutput(output);
                return(0);
            }
            catch (const std::exception& e) {
                return(1);
            }
        }
    }
    // else if (clientRequest.method == "POST"){
        
    // }
    // else if (clientRequest.method == "DELETE"){
        
    // }
    else{
        this->statusCode = "501";
        this->headers["contentType"] = " text/html";
        this->body = extractFileContent("/Users/slord/Desktop/13-WEBSERVER/html/501.html");       
        return (1);
    }
    return (0);
}

 //this fucntion will write on the socket the response to send to the client
int HttpResponse::writeOnSocket(const int& clientSocket){
    std::string response = "HTTP/1.1 " + statusCode + "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        response += it->first + ": " + it->second + "\r\n";
    }
    response += "\r\n" + body;
    send(clientSocket, response.c_str(), response.length(), 0);
    return (0);
}

bool HttpResponse::fileExist(const std::string& filename) {
    std::ifstream file(filename.c_str());
    return file.good();
}

std::string HttpResponse::executeCgi(const std::string& scriptPath, const std::string& requestBody) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        throw std::runtime_error("Erreur lors de la création du tube (pipe).");
    }

    pid_t pid = fork();
    if (pid == -1) {
        throw std::runtime_error("Erreur lors de la création du processus fils.");
    } else if (pid == 0) {
        // Processus fils

        // Fermer l'extrémité de lecture inutilisée du tube
        close(pipefd[0]);

        // Rediriger la sortie standard vers l'extrémité d'écriture du tube
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        // Exécuter le script CGI
        execl("/usr/bin/php-cgi", "php-cgi", "-q", scriptPath.c_str(), nullptr);

        // En cas d'erreur lors de l'exécution du script CGI
        std::cerr << "Erreur lors de l'exécution du script CGI." << std::endl;
        exit(EXIT_FAILURE);
    } else {
        // Processus parent

        // Fermer l'extrémité d'écriture inutilisée du tube
        close(pipefd[1]);

        // Lire la sortie générée par le processus fils à partir du tube de lecture
        std::string output;
        char buffer[128];
        ssize_t bytesRead;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            output += std::string(buffer, bytesRead);
        }

        close(pipefd[0]);

        // Attendre la fin du processus fils
        int status;
        waitpid(pid, &status, 0);

        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            throw std::runtime_error("Le script CGI a retourné une erreur.");
        }

        return output;
    }
    //exemple d<utilisation de exeve.
//     std::string HttpResponse::executeCgi(const std::string& scriptPath, const std::string& queryString) {
//     // ...

//     if (pid == 0) {
//         // Processus fils

//         // ...

//         // Séparer la chaîne de requête en paramètres individuels
//         std::istringstream iss(queryString);
//         std::string param;
//         std::vector<const char*> params;
//         while (std::getline(iss, param, '&')) {
//             params.push_back(param.c_str());
//         }
//         params.push_back(nullptr); // Terminer la liste d'arguments

//         // Préparer le tableau d'environnement
//         extern char** environ; // Déclarer la variable d'environnement externe
//         std::vector<const char*> env;
//         for (char** envp = environ; *envp != nullptr; ++envp) {
//             env.push_back(*envp);
//         }
//         env.push_back(nullptr); // Terminer le tableau d'environnement

//         // Exécuter le script CGI avec les paramètres
//         execve("/usr/bin/php-cgi", const_cast<char**>(params.data()), const_cast<char**>(env.data()));

//         // ...
//     } else {
//         // Processus parent

//         // ...
//     }
// }
}
void HttpResponse::analyseCgiOutput(const std::string& output){
    std::size_t headerEnd = output.find("\r\n\r\n");
    if (headerEnd != std::string::npos) {
        std::string responseBody = output.substr(headerEnd + 4);
        this->body = (responseBody);
        return;
    }

    this->body = ("Error: Invalid CGI output format");

 }
    //We will fork, and execute de CGI script in a child process, redirect the output to a fd  and finally store it the variable CgiOut.
    


