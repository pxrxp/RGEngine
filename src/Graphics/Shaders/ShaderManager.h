#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include <unordered_map>
#include <string>

class ShaderManager {
private:
	std::unordered_map<std::string, unsigned int> database;
	ShaderManager();

public:
	static ShaderManager& get_instance();
	unsigned int get(const std::string& program_name);

	ShaderManager(const ShaderManager&) = delete;
	ShaderManager& operator=(const ShaderManager&) = delete;
};

#endif // SHADERMANAGER_H
