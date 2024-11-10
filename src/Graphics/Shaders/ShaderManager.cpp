#include <stdexcept>
#include <Graphics/Shaders/ShaderManager.h>

ShaderManager& ShaderManager::get_instance() {
	static ShaderManager instance;
	return instance;
}

unsigned int ShaderManager::get(const std::string& program_name) {
        auto it = database.find(program_name);
        if (it != database.end()) {
            return it->second;
        }
        throw std::out_of_range("Program not found");
}
