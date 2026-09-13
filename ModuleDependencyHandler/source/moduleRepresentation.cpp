#include <expected>
#include <string>
#include <iostream>

#include "moduleRepresentation.hpp"
#include "parsing.hpp"

[[nodiscard]] std::expected<moduleRepresentation, std::string> buildHalfModuleRepresentationFromFile(const fs::path& file) {
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

[[nodiscard]] std::vector<std::shared_ptr<moduleRepresentation>> completeAllModuleRepresentations(const std::vector<std::shared_ptr<moduleRepresentation>>& halfModulesReps) {
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

[[nodiscard]] std::vector<std::shared_ptr<moduleRepresentation>> buildAllModuleRepresentations(const std::vector<fs::path>& moduleFiles) {
	const std::vector<std::shared_ptr<moduleRepresentation>> halfOnes = constructHalfModulesReps(moduleFiles);
	const std::vector<std::shared_ptr<moduleRepresentation>> completeModules = completeAllModuleRepresentations(halfOnes);
	return completeModules;
}
