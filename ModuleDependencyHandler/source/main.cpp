#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <expected>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <memory>
#include <regex>
#include <string>
#include <vector>
#include <algorithm>
#include <ranges>
#include <utility>

namespace fs = std::filesystem;

using moduleRepresentationPosiiton = size_t; 

struct moduleRepresentation {
	bool isPartition = false;
	std::string moduleName;
	std::string partitionName;
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

[[nodiscard]] std::expected<std::pair<std::string, std::string>, std::string> regexPartitionExport(const std::string& searchString) {
	static const std::regex pattern(R"(export module (\w+):(\w+);)");
	std::smatch match;
	if(std::regex_match(searchString, match, pattern)) {
		return std::pair{match[1].str(), match[2].str()};
	}
	return std::unexpected("");
}

[[nodiscard]] std::expected<std::string, std::string> regexModuleImport(const std::string& searchString) {
	static const std::regex pattern(R"(import (\w+);)");
	return getRegexMatch(searchString, pattern);
}

[[nodiscard]] std::expected<std::string, std::string> regexPartionImport(const std::string& searchString) {
	static const std::regex pattern(R"(import :(\w+);)");
	return getRegexMatch(searchString, pattern);
}

[[nodiscard]] std::expected<std::string, std::string> checkFileForModuleExport(const fs::path& file) {
	std::string exportedModule;
	bool alreadyFoundModuleExport = false;

	std::string currentLine;
	std::ifstream ifStreamFile(file) ;
	while (std::getline(ifStreamFile, currentLine)) {
		const auto regexResult = regexModuleExport(currentLine);
		if(regexResult) {
			if(regexResult.value().empty()) {
				return std::unexpected(std::string(file.string() + " Exports nothing"));
			}
			if (not alreadyFoundModuleExport) {
				exportedModule = regexResult.value();
				alreadyFoundModuleExport = true;
			} else {
				return std::unexpected(std::string(file.string() + " Has two module exports"));
			}
		}
	}	
	return exportedModule;
}

[[nodiscard]] std::expected<std::pair<std::string, std::string>, std::string> checkFileForPartitonExport(const fs::path& file) {
	std::pair<std::string, std::string> exportedPartition;
	bool alreadyFoundPartitionExport = false;

	std::string currentLine;
	std::ifstream ifStreamFile(file) ;
	while (std::getline(ifStreamFile, currentLine)) {
		const auto regexResult =  regexPartitionExport(currentLine);
		if(regexResult) {
			if(regexResult.value().first.empty() || regexResult.value().second.empty()) {
				return std::unexpected(std::string(file.string() + " Exports nothing"));
			}
			if(not alreadyFoundPartitionExport) {
				exportedPartition = regexResult.value();
				alreadyFoundPartitionExport = true;
			} else {
				return std::unexpected(std::string(file.string() + " Has two partition exports"));
			}
		}
	}	

	return exportedPartition;
}

[[nodiscard]] std::expected<std::vector<std::string> , std::string> checkFileForModulesImports(const fs::path& file) {
	std::vector<std::string> importedModules;
	static const std::regex invalidPartitionImport(R"(import \w+:\w+;)");

	std::string currentLine;
	std::ifstream ifStreamFile(file) ;
	while (std::getline(ifStreamFile, currentLine)) {
		if(std::regex_match(currentLine, invalidPartitionImport)) {
			return std::unexpected(std::string(file.string() + " uses invalid qualified partition import syntax"));
		}
		const auto regexResult =  regexModuleImport(currentLine);
		if(regexResult) {
			importedModules.push_back(regexResult.value());
		}
	}	

	for(const std::string& importedModule : importedModules) {
		if(importedModule.empty()) {
			return std::unexpected(std::string(file.string() + " Has an empty import, skiping it"));
		}
	}
	return importedModules;
}

[[nodiscard]] std::expected<std::vector<std::string> , std::string> checkFileForPartitionsImports(const fs::path& file) {
	std::vector<std::string> importedPartitions;

	std::string currentLine;
	std::ifstream ifStreamFile(file) ;
	while (std::getline(ifStreamFile, currentLine)) {
		const auto regexResult =  regexPartionImport(currentLine);
		if(regexResult) {
			importedPartitions.push_back(regexResult.value());
		}
	}	

	for(const std::string& importedPartition : importedPartitions) {
		if(importedPartition.empty()) {
			return std::unexpected(std::string(file.string() + " Has an empty partition import, skiping it"));
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
		builtModule.moduleName = partitionExportResult.value().first;
		builtModule.partitionName = partitionExportResult.value().second;
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
 
	for(std::size_t i = 0; i < foundModules.size(); ++i) {
		for(std::size_t j = i + 1; j < foundModules.size(); ++j) {
			const auto& moduleI = foundModules[i];
			const auto& moduleJ = foundModules[j];
			if(moduleI->moduleName == moduleJ->moduleName && moduleI->partitionName == moduleJ->partitionName) {
				std::cerr << "duplicate module" << moduleI->definitionFile << " and " << moduleJ->definitionFile << " define thse same name!\n";
				std::cerr << "Aborting, damn you\n";
				std::abort();
			}
		}
	}

	return foundModules;
}

[[nodiscard]] std::expected<std::shared_ptr<moduleRepresentation>, std::string> CompleteModuleRepresentation(const std::shared_ptr<moduleRepresentation>& currentModule, const std::vector<std::shared_ptr<moduleRepresentation>>& halfModulesReps) {
	std::shared_ptr<moduleRepresentation> fullModule = currentModule;
	// There is probaly some better way to build dependenciesNames, but I'm too lazy to do it
	const auto resultOfModuleImport = checkFileForModulesImports(currentModule->definitionFile);
	if(not resultOfModuleImport) {
		std::cerr << resultOfModuleImport.error() << "\n";
		std::abort();
	}

	const auto resultOfPartitionImport = checkFileForPartitionsImports(currentModule->definitionFile);
	if(not resultOfPartitionImport) {
		std::cerr << resultOfPartitionImport.error() << "\n";
		std::abort();
	}

	const std::vector<std::string>& moduleDependencies = resultOfModuleImport.value();
	const std::vector<std::string>& partitionDependencies = resultOfPartitionImport.value();

	for (const auto& dependencyName : moduleDependencies) {

		auto foundDependecy = std::ranges::find_if(
			halfModulesReps,
			[&dependencyName](const std::shared_ptr<moduleRepresentation>& orangutango) -> bool {
				return orangutango->moduleName == dependencyName && orangutango->partitionName.empty();
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

	for (const auto& partitionName : partitionDependencies) {
		auto foundDependecy = std::ranges::find_if(
			halfModulesReps,
			[&currentModule, &partitionName](const std::shared_ptr<moduleRepresentation>& orangutango) -> bool {
				return orangutango->moduleName == currentModule->moduleName && orangutango->partitionName == partitionName;
			}
		);

		if(foundDependecy == halfModulesReps.end()) {
			std::cerr << currentModule->moduleName << ":" << partitionName << " in " << currentModule->definitionFile << " does NOT exist!\n";
			std::cerr << "Aborting, damn you\n";
			std::abort();
		}

		if(currentModule == *foundDependecy) {
			std::cerr << currentModule->moduleName << ":" << partitionName << " in " << currentModule->definitionFile << " depends on itself, dumbass\n";
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
		auto resultOfComplete = CompleteModuleRepresentation(halfRep, halfModulesReps);
		if(not resultOfComplete) {
			continue;
		}
		fullModules.push_back(resultOfComplete.value()); 
	}
	return fullModules;
}

std::vector<std::shared_ptr<moduleRepresentation>> buildAllModuleRepresentations(const std::vector<fs::path>& moduleFiles) {
	const std::vector<std::shared_ptr<moduleRepresentation>> halfOnes = constructHalfModulesReps(moduleFiles);
	const std::vector<std::shared_ptr<moduleRepresentation>> completeModules = completeAllModuleRepresentations(halfOnes);
	return completeModules;
}

[[nodiscard]] std::size_t findModuleIndex(
	const std::vector<std::shared_ptr<moduleRepresentation>>& allModules,
	const std::shared_ptr<moduleRepresentation>& module) {

	const auto foundModule = std::ranges::find(allModules, module);
	if(foundModule == allModules.end()) {
		std::cerr << "A module dependency is not present in the module list\n";
		std::abort();
	}
	return static_cast<std::size_t>(std::distance(allModules.begin(), foundModule));
}

//the sub vecotr is a level of the graph
std::vector<std::vector<std::shared_ptr<moduleRepresentation>>> buildModuleDepdencyGraph(const std::vector<std::shared_ptr<moduleRepresentation>>& allModules) {
	if(allModules.empty()) {
		return {};
	}

	std::vector<std::size_t> moduleLevels(allModules.size(), 0);
	std::vector<std::uint8_t> visitState(allModules.size(), 0);

	for(std::size_t rootIndex = 0; rootIndex < allModules.size(); ++rootIndex) {
		if(visitState.at(rootIndex) == 2) {
			continue;
		}

		std::vector<std::pair<std::size_t, std::size_t>> workStack;
		workStack.emplace_back(rootIndex, 0);
		visitState.at(rootIndex) = 1;

		while(not workStack.empty()) {
			auto& [moduleIndex, dependencyIndex] = workStack.back();
			if(dependencyIndex == allModules.at(moduleIndex)->dependencies.size()) {
				std::size_t moduleLevel = 0;
				for(const auto& dependency : allModules[moduleIndex]->dependencies) {
					const std::size_t dependencyModuleIndex = findModuleIndex(allModules, dependency);
					moduleLevel = std::max(moduleLevel, moduleLevels[dependencyModuleIndex] + 1);
				}
				moduleLevels[moduleIndex] = moduleLevel;
				visitState[moduleIndex] = 2;
				workStack.pop_back();
				continue;
			}

			const std::size_t dependencyModuleIndex = findModuleIndex(allModules,
				allModules.at(moduleIndex)->dependencies[dependencyIndex++]);
			if(visitState.at(dependencyModuleIndex) == 1) {
				std::cerr << "Circular module dependency involving "
				          << allModules[dependencyModuleIndex]->moduleName << "\n";
				std::abort();
			}
			if(visitState.at(dependencyModuleIndex) == 0) {
				visitState.at(dependencyModuleIndex) = 1;
				workStack.emplace_back(dependencyModuleIndex, 0);
			}
		}
	}

	const std::size_t highestLevel = allModules.empty()
		? 0
		: *std::ranges::max_element(moduleLevels);
	std::vector<std::vector<std::shared_ptr<moduleRepresentation>>> graph(highestLevel + 1);
	for(std::size_t moduleIndex = 0; moduleIndex < allModules.size(); ++moduleIndex) {
		graph[moduleLevels[moduleIndex]].push_back(allModules[moduleIndex]);
	}
	return graph;
}

void dunnoModuleDependencyGraph(const fs::path& searchPath) {
	const std::vector<fs::path> foundModuleFiles = findFilesByExtension(searchPath, ".cppm");

	const std::vector<std::shared_ptr<moduleRepresentation>> allModules = buildAllModuleRepresentations(foundModuleFiles);


	
}

int main() {
	std::cout << "ModuleDependencyHandler-cli\n";
	return 0;
}
