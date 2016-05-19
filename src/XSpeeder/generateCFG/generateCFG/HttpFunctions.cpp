#include "HttpFunctions.h"

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	std::string *__fpw = (std::string *)userp;
	if (!__fpw) {
		return 0;
	}
	__fpw->append((char *)contents,size*nmemb);
	return size * nmemb;
}

bool GetResourceFromHttp(std::string urls,std::string *szbuf) {
	CURL* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, urls.data());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, szbuf);

	CURLcode res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	if(res==CURLE_OK){
		return true;
	}
	else{
		return false;
	}
}

bool ParseConfigList(std::string &contents,std::vector<std::string> &lstItems) {
	Json::Value jsValue;
	Json::Reader reader;
	if (!reader.parse(contents,jsValue,false) ) {
		return false;
	}
	if (!jsValue.isArray()) {
		return false;
	}
	for (int i=0; i<jsValue.size(); i++) {
		lstItems.push_back(jsValue[i].asString());
	}
	return true;
}