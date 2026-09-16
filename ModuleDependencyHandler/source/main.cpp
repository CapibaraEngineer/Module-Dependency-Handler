
#include <iostream>
#include <filesystem>
#include <memory>
#include <string>
#include <vector> 

#include "moduleRepresentation.hpp"
#include "parsing.hpp"
#include "moduleDependencyGraph.hpp"
#include "cliArgHandler.hpp"

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

[[nodiscard("You shall not discard the return of a goated function like this")]]
std::vector<std::vector<std::shared_ptr<moduleRepresentation>>> generateModuleDependencyGraph(const fs::path& searchPath) {
	const std::vector<fs::path> foundModuleFiles = findFilesByExtension(searchPath, ".cppm");

	const std::vector<std::shared_ptr<moduleRepresentation>> allModules = buildAllModuleRepresentations(foundModuleFiles);
	
	const std::vector<std::vector<std::shared_ptr<moduleRepresentation>>> moduleDependencyGraph = buildModuleDependencyGraph(allModules);

	// I make no changes 
	// I only create objects
	// And mark them const
	// - A poem

	return moduleDependencyGraph;
}

int main(int argc, char **argv) {
	handleArg(argc, argv);
	std::cout << "ModuleDependencyHandler-cli\n";
	return 0;
}
