#include <iostream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

struct moduleRepresentation {
	std::string moduleName;
	fs::path definitionFile{};
	std::vector<std::string> dependencies;
};

struct level {
	std::vector<moduleRepresentation> modules;
	std::size_t positionInList = 0; 
};

// TODO: make fileExtension a std::span so it can take multiple extensions
[[nodiscard]] std::vector<fs::path> findFilesByExtension(const fs::path& searchPath, const std::string& fileExtension) {
	std::vector<fs::path> foundFiles{};
	for(const fs::directory_entry& entry : fs::recursive_directory_iterator(searchPath)) {
		if(entry.path().extension() == fileExtension) {
			foundFiles.push_back(entry.path());
		}
	}
	return foundFiles;
}

void buildModuleDependencyGraph(const fs::path& searchPath) {
	const std::vector<fs::path> foundModuleFiles = findFilesByExtension(searchPath, ".cppm");
	
}

int main() {
	std::cout << "ModuleDependencyHandler-cli\n";
	return 0;
}
