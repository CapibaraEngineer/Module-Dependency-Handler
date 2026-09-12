#include <expected>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <regex>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct moduleRepresentation {
	bool isPartition = false;
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


[[nodiscard]] std::expected<std::string, std::string> getRegexMatch(const std::string& searchString, const std::regex& pattern) {
	std::smatch match;
	if(std::regex_match(searchString, match, pattern)) {
		return match[1];
	}
	return std::unexpected("");
}

[[nodiscard]] std::expected<std::string, std::string> regexModuleExport(const std::string& searchString) {
	static const std::regex pattern(R"(export module (\w+);)");
	return getRegexMatch(searchString, pattern);
}

[[nodiscard]] std::expected<std::string, std::string> regexPartitionExport(const std::string& searchString) {
	static const std::regex pattern(R"(export module \w(:\w+);)");
	return getRegexMatch(searchString, pattern);
}

[[nodiscard]] std::expected<std::string, std::string> regexModuleImport(const std::string& searchString) {
	static const std::regex pattern(R"(import (\w+);)");
	return getRegexMatch(searchString, pattern);
}

[[nodiscard]] std::expected<std::string, std::string> regexPartionImport(const std::string& searchString) {
	static const std::regex pattern(R"(import \w+(:\w+);)");
	return getRegexMatch(searchString, pattern);
}

[[nodiscard]] std::expected<std::string, std::string> checkFileForModuleExport(const fs::path& file) {
	std::string exportedModule;
	bool alreadyFoundModuleExport = false;

	std::string currentLine;
	while (std::getline(std::ifstream(file), currentLine)) {
		const auto regexResult = regexModuleExport(currentLine);
		if(regexResult) {
			if (not alreadyFoundModuleExport) {
				exportedModule = regexResult.value();
			} else {
				return std::unexpected(std::string(file.string() + " Has two module exports"));
			}
		}
	}	
	return exportedModule;
}

[[nodiscard]] std::expected<std::string, std::string> checkFileForPartitonExport(const fs::path& file) {
	std::string exportedPartition;
	bool alreadyFoundPartitionExport = false;

	std::string currentLine;
	while (std::getline(std::ifstream(file), currentLine)) {
		const auto regexResult =  regexPartitionExport(currentLine);
		if(regexResult) {
			if(not alreadyFoundPartitionExport) {
				exportedPartition = regexResult.value();
			} else {
				return std::unexpected(std::string(file.string() + " Has two partition exports"));
			}
		}
	}	

	return exportedPartition;
}

[[nodiscard]] std::vector<std::string> checkFileForModulesImports(const fs::path& file) {
	std::vector<std::string> importedModules;

	std::string currentLine;
	while (std::getline(std::ifstream(file), currentLine)) {
		const auto regexResult =  regexModuleImport(currentLine);
		if(regexResult) {
			importedModules.push_back(regexResult.value());
		}
	}	
	return importedModules;
}

[[nodiscard]] std::vector<std::string> checkFileForPartitionsImports(const fs::path& file) {
	std::vector<std::string> importedPartitions;

	std::string currentLine;
	while (std::getline(std::ifstream(file), currentLine)) {
		const auto regexResult =  regexModuleImport(currentLine);
		if(regexResult) {
			importedPartitions.push_back(regexResult.value());
		}
	}	
	return importedPartitions;
}

std::expected<moduleRepresentation, std::string> buildModuleRepresentationFromFile(const fs::path& file) {
	moduleRepresentation builtModule;
	builtModule.definitionFile = file;

	const auto moduleExportResult = checkFileForModuleExport(file);
	const auto partitionExportResult = checkFileForPartitonExport(file);

	if(moduleExportResult and partitionExportResult) {
		return std::unexpected(std::string(file.string() + " Is a module export and a partition export"));
	}

	if(not moduleExportResult and not partitionExportResult) {
		return std::unexpected(std::string(file.string() + " Is nothing, it exports nothing, it is USELESS, that is, not a module"));
	}

	if(moduleExportResult) {
		builtModule.moduleName = moduleExportResult.value();
	}

	if(partitionExportResult) {
		builtModule.moduleName = partitionExportResult.value();
		builtModule.isPartition = true;
	}

	builtModule.dependencies.append_range(checkFileForModulesImports(file));
	builtModule.dependencies.append_range(checkFileForPartitionsImports(file));

	return builtModule;
}


[[nodiscard]] std::vector<moduleRepresentation> findModules(const std::vector<fs::path>& moduleFiles) {
	std::vector<moduleRepresentation> foundModules; 

	foundModules.reserve(moduleFiles.size()); // This prealocates the exact amount. the only time this over alocates is if there are invalid files.
	for(const fs::path& file : moduleFiles) {
		const auto builtModuleResult = buildModuleRepresentationFromFile(file);
		if(builtModuleResult) {
			foundModules.push_back(builtModuleResult.value());
		} else {
			std::cout << builtModuleResult.error() << "\n";
		}
	}

	return foundModules;
}



void dunnoModuleDependencyGraph(const fs::path& searchPath) {
	const std::vector<fs::path> foundModuleFiles = findFilesByExtension(searchPath, ".cppm");

	const std::vector<moduleRepresentation> modulesRepresentations = findModules(foundModuleFiles);
	

	
}

int main() {
	std::cout << "ModuleDependencyHandler-cli\n";
	return 0;
}
