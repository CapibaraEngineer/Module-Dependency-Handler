
#include <iostream>
#include <filesystem>
#include <memory>
#include <string>
#include <vector> 

#include "moduleRepresentation.hpp"
#include "parsing.hpp"
#include "moduleDependencyGraph.hpp"

namespace fs = std::filesystem;

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

void dunnoModuleDependencyGraph(const fs::path& searchPath) {
	const std::vector<fs::path> foundModuleFiles = findFilesByExtension(searchPath, ".cppm");

	const std::vector<std::shared_ptr<moduleRepresentation>> allModules = buildAllModuleRepresentations(foundModuleFiles);
	
	const std::vector<std::vector<std::shared_ptr<moduleRepresentation>>> moduleDependencyGraph = buildModuleDependencyGraph(allModules);
}

int main() {
	std::cout << "ModuleDependencyHandler-cli\n";
	return 0;
}
