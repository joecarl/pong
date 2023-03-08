
#include <iostream>
#include <stdexcept>

#include <curl/curl.h>
#include "../../utils.h"

using namespace std;

static size_t readToString(void *contents, size_t size, size_t nmemb, void *userp) {

	((string*)userp)->append((char*)contents, size * nmemb);

	return size * nmemb;

}

size_t readToFile(void *ptr, size_t size, size_t nmemb, FILE *stream) {

	size_t written = fwrite(ptr, size, nmemb, stream);
	
	return written;

}

enum {
	READ_TO_STRING = 0,
	READ_TO_FILE
};

CURLcode perform_request(const char* url, void *readBuffer, int readType = READ_TO_STRING) {

	CURL *curl;
	CURLcode res;
	
	curl = curl_easy_init();
	
	if (curl) {

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

		if (readType == READ_TO_STRING) {
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, readToString);
		} else if (readType == READ_TO_FILE) {
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, readToFile);
		} else {
			throw runtime_error("readType no valido");
		}

		curl_easy_setopt(curl, CURLOPT_WRITEDATA, readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

	}

	return res;

}

string get_remote_version_num() {

	string readBuffer;

	perform_request("https://copinstar.com/pong/version/", &readBuffer);
	
	return readBuffer;

}

void download_remote_version() {
	
	cout << "Downloading ..." << endl;

	FILE *fp = fopen("newpong.exe", "wb");
	perform_request("https://copinstar.com/pong/asdf.zip", fp, READ_TO_FILE);
	fclose(fp);

	cout << "Finished" << endl;
}


int main(int argc, char **argv) {

	try {

		string v = trim(exec("PONG --version"));
		string rv = get_remote_version_num();

		if (v != rv) {
			cout << "New version available " << rv << endl;
		} else {
			cout << "Already up to date" << endl;
		}

		return 0;

	} catch (exception &e) {

		cerr << "Runtime exception: " << e.what() << endl;
		return 1;
	
	}

}



