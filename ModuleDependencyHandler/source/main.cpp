#include <cstddef>
#include <cstdio>
#include <expected>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <memory>
#include <regex>
#include <string>
#include <vector>

namespace fs = std::filesystem;

using moduleRepresentationPosiiton = size_t; 

struct moduleRepresentation {
	bool isPartition = false;
	std::string moduleName;
	fs::path definitionFile{};
	std::vector<std::shared_ptr<moduleRepresentation>> dependencies;
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

std::expected<moduleRepresentation, std::string> buildHalfModuleRepresentationFromFile(const fs::path& file) {
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

	return builtModule;
}


[[nodiscard]] std::vector<std::shared_ptr<moduleRepresentation>> constructHalfModulesReps(const std::vector<fs::path>& moduleFiles) {
	std::vector<std::shared_ptr<moduleRepresentation>> foundModules; 

	foundModules.reserve(moduleFiles.size()); // This prealocates the exact amount. the only time this over alocates is if there are invalid files.
	for(const fs::path& file : moduleFiles) {
		const auto builtModuleResult = buildHalfModuleRepresentationFromFile(file);
		if(builtModuleResult) {
			foundModules.push_back(std::make_shared<moduleRepresentation>(builtModuleResult.value()));
		} else {
			std::cout << builtModuleResult.error() << "\n";
		}
	}
 
	for(const std::shared_ptr<moduleRepresentation>& moduleI : foundModules) {
		for(const std::shared_ptr<moduleRepresentation>& moduleJ: foundModules) {
			if(moduleI->moduleName == moduleJ->moduleName) {
				std::cerr << "duplicate module" << moduleI->definitionFile << " and " << moduleJ->definitionFile << " define thse same name!\n";
				std::cerr << "Aborting, damn you\n";
				std::abort();
			}
		}
	}

	return foundModules;
}



[[nodiscard]] std::shared_ptr<moduleRepresentation> CompleteModuleRepresentation(const std::shared_ptr<moduleRepresentation>& currentModule, const std::vector<std::shared_ptr<moduleRepresentation>>& halfModulesReps) {
	std::shared_ptr<moduleRepresentation> fullModule = currentModule;
	// There is probaly some better way to build dependenciesNames, but I'm to lazy to do it
	std::vector<std::string> dependenciesNames = checkFileForModulesImports(currentModule->definitionFile);
	dependenciesNames.append_range(checkFileForPartitionsImports(currentModule->definitionFile));

	for (const auto& dependencyName : dependenciesNames) {

		auto foundDependecy = std::ranges::find_if(
			halfModulesReps,
			[&dependencyName](const std::shared_ptr<moduleRepresentation>& orangutango) -> bool {
				return orangutango->moduleName == dependencyName;
			}
		);

		if(foundDependecy == halfModulesReps.end()) {
			std::cerr << dependencyName << " in " << currentModule->definitionFile << " does NOT exist!\n";
			std::cerr << "Aborting, damn you\n";
			std::abort();
		}

		if(currentModule == *foundDependecy) {
			std::cerr << currentModule->moduleName << " in " << currentModule->definitionFile << " depends on itself, dumbass\n";
			std::cerr << "Aborting, damn you\n";
			std::abort();
		}

		fullModule->dependencies.push_back(*foundDependecy);
	}

	return fullModule;	
}

std::vector<std::shared_ptr<moduleRepresentation>> completeAllModuleRepresentations(const std::vector<std::shared_ptr<moduleRepresentation>>& halfModulesReps) {
	std::vector<std::shared_ptr<moduleRepresentation>> fullModules;
	fullModules.reserve(halfModulesReps.size());
	for(const auto& halfRep : halfModulesReps) {
		fullModules.push_back(CompleteModuleRepresentation(halfRep, halfModulesReps)); 
	}
	return fullModules;
}

std::vector<std::shared_ptr<moduleRepresentation>> buildAllModuleRepresentations(const std::vector<fs::path>& moduleFiles) {
	const std::vector<std::shared_ptr<moduleRepresentation>> halfOnes = constructHalfModulesReps(moduleFiles);
	const std::vector<std::shared_ptr<moduleRepresentation>> completeModules = completeAllModuleRepresentations(halfOnes);
	return completeModules;
}

[[deprecated("I probaly should delete this function")]] moduleRepresentationPosiiton getModulePosFromName(const std::string& name, const std::vector<moduleRepresentation>& allModules) {
	for(moduleRepresentationPosiiton i = 0; i < allModules.size(); i++) {
		if(allModules.at(i).moduleName == name) {
			return i;
		}
	}
	static constexpr std::size_t invalid = 8446744073709551615;
	return invalid;
}

void buildModuleDepdencyGraph(const std::vector<moduleRepresentation>& allModules) {
	
}

/*bool checkModuleDependenciesCorrectnes(const moduleRepresentation& workModule, const std::vector<moduleRepresentation>& allModules) {
	size_t foundDependencies = 0;
	for(const std::string& dependecy : workModule.dependencies) {
		for(const moduleRepresentation& compared : allModules) {
			if(compared.moduleName == dependecy) {
				foundDependencies += 1;
			}
		}
	}
	if (foundDependencies == workModule.dependencies.size()) {
		return true;
	}
	if(foundDependencies < workModule.dependencies.size()) {
		return false;
	}
	if(foundDependencies > workModule.dependencies.size()) {
		std::cout << "What\n";
		return false;
	}
	std::cout << "Reality is a lie\n";
	return false;
}*/

void dunnoModuleDependencyGraph(const fs::path& searchPath) {
	const std::vector<fs::path> foundModuleFiles = findFilesByExtension(searchPath, ".cppm");

	const std::vector<std::shared_ptr<moduleRepresentation>> allModules = buildAllModuleRepresentations(foundModuleFiles);


	
}

int main() {
	std::cout << "ModuleDependencyHandler-cli\n";
	return 0;
}
