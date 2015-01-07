#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <unordered_map>
#include <string>
#include <vector>

///Class that reads a configuration file.
/**Configuration files are a series of key - value pairs separated by
 *  white space. Multiple entries with the same key are not permitted.
 * Text following a comment character '#' is ignored.
 *
 */
class ConfigFile {
	private:
		std::unordered_map <std::string, std::vector< std::string > > fOptions;

	public:
		///Default constructor.
		ConfigFile();
		///Default destructor.
		virtual ~ConfigFile();
		///Retrieve the number of entries matching the specified key.
		unsigned int GetCount(std::string key);
		///Retrieve the value of the option specified by the key and count.
		std::string GetOption(std::string key, unsigned int count=0);
		///Retrieve the number of options set with the specified key.
		size_t GetNumEntries(std::string key);
		///Read the specified configuration file and store the options.
		bool ReadFile(const char* filename);

};

#endif

