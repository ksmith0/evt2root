#include "configFile.h"
#include <fstream>

ConfigFile::ConfigFile() {

};

ConfigFile::~ConfigFile() {

};

/**Options are expected in the format: key, white space, value, new line.
 *  Comments are permitted as whole lines or at the end of a line starting
 *  with the '#' character. White space characters include spaces and tabs.
 *  If a key is found with no matching value a warning is displayed and the
 *  next line is parsed. Blank lines are ignored.
 *
 * \param[in] filename Filename of the configuration file.
 * \return Boolean value indicating reading success.
 */
bool ConfigFile::ReadFile(const char* filename) {
	std::ifstream fileStream(filename); 
	if (!fileStream.good() || !fileStream.is_open()) {
		fprintf(stderr,"ERROR: Unable to option option file: %s!\n",filename);
		return false;
	}
		
	std::string line;
	while(std::getline(fileStream,line).good()) {
		size_t pos = line.find('#');
		if (pos != std::string::npos) 
			line.erase(pos);

		//String from first not white space to first white space.
		size_t wordStart = line.find_first_not_of(" \t");
		//In case of a line with only white space we continue.
		if (wordStart == std::string::npos) continue;

		size_t wordStop = line.find_first_of(" \t",wordStart);
		//We didn't find a white space so there must be no value.
		if (wordStop == std::string::npos) {
			std::string key = line.substr(wordStart);
			fflush(stdout);
			fprintf(stderr,"WARNING: Unable to find value for key: '%s'\n",key.c_str());
			continue;
		}
		std::string key = line.substr(wordStart,wordStop - wordStart);

		//Second string from first not white space to first white space.
		wordStart = line.find_first_not_of(" \t",wordStop);
		if (wordStart == std::string::npos) {
			fflush(stdout);
			fprintf(stderr,"WARNING: Unable to find value for key: '%s'\n",key.c_str());
			continue;
		}
			
		//Following line looks for tab, backspace, carriage return, vertical
		// tab, form feed or a newline.
		wordStop = line.find_last_not_of(" \t\b\r\v\f\n");
		std::string value = line.substr(wordStart,wordStop - wordStart + 1);

		fOptions[key].push_back(value);
	}

	fileStream.close();

	return true;
};

/**Returns a value matching the key and count provided. If the option
 * requested does not exist an empty string is returned. The count is 
 * set by default to the first item. If the count is greater than the
 * number of pairs for this key then an empty string is returned.
 *
 * \param[in] key Key of requested pair.
 * \param[in] count Which ordered pair to retrieve.
 * \return Value matching the provided key. If no key[count] - value pair
 *  is found an empty string is returned.
 */
std::string ConfigFile::GetOption(std::string key, unsigned int count) {
	std::unordered_map<std::string, std::vector< std::string > >::iterator loc = fOptions.find(key);
	if (loc == fOptions.end()) {
		return "";
	}
	if (loc->second.size() < count) {
		return "";
	}
	return loc->second.at(count);
}

size_t ConfigFile::GetNumEntries(std::string key) {
	std::unordered_map<std::string, std::vector< std::string > >::iterator loc = fOptions.find(key);
	if (loc == fOptions.end()) {
		return 0;
	}
	return loc->second.size();
}

