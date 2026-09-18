#include <cstdlib>
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
	
	if(not moduleExportResult.has_value()) {
		if(moduleExportResult.error() == regexError::two_module_exports) {
			return std::unexpected(std::string(file.string() + " Exports two modules!"));
		}
	}

	if(not partitionExportResult.has_value()) {
		if(partitionExportResult.error() == regexError::two_partition_exports) {
			return std::unexpected(std::string(file.string() + " Exports two Partitions!"));
		} 
	}

	if(moduleExportResult.has_value() and partitionExportResult.has_value()) {
		return std::unexpected(std::string(file.string() + " Is a module export and a partition export!"));
	}

	if(not moduleExportResult.has_value() and not partitionExportResult.has_value()) {
		if(moduleExportResult.error() == regexError::no_export_module and 
			partitionExportResult.error() == regexError::no_export_partition) {

			return std::unexpected(std::string(file.string() + " Is nothing, it exports nothing, it is USELESS, that is, not a module!"));
		}	
	}

	if(moduleExportResult.has_value()) {
		builtModule.moduleName = moduleExportResult.value();
	}

	if(partitionExportResult.has_value()) {
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
		if(builtModuleResult.has_value()) {
			foundModules.push_back(std::make_shared<moduleRepresentation>(builtModuleResult.value()));
		} else {
			std::cout << builtModuleResult.error() << "\n";
			std::cout << "Process will not continue with errors!";
			std::abort();
		}
	}
 
	// I don't like this for loop. TODO: Make it better
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

[[nodiscard]] std::shared_ptr<moduleRepresentation> CompleteModuleRepresentation(const std::shared_ptr<moduleRepresentation>& currentModule, const std::vector<std::shared_ptr<moduleRepresentation>>& halfModulesReps) {
	std::shared_ptr<moduleRepresentation> fullModule = currentModule;
	// There is probaly some better way to build dependenciesNames, but I'm too lazy to do it
	const auto resultOfModuleImport = checkFileForModulesImports(currentModule->definitionFile);
	if(not resultOfModuleImport.has_value()) {
		if(resultOfModuleImport.error() == regexError::invalidPartitionImportSyntax) {
			std::cout << currentModule->definitionFile.string() << " has invalid partition import syntax: import module:partition;\n The program will now stop.";
			std::abort();
		}
		if(resultOfModuleImport.error() == regexError::emptyModuleImport) {
			std::cout << currentModule->definitionFile.string() << " has a empty import. \n The program will now stop.";
			std::abort();
		}

		//fallback in case some dumbass adds a third error and doesnt add a if to handle it here
		std::cout << "Some dumbass programmer added a third error, and forgot to add handling for it, or it was cosmic rays\n";
		std::cout << "CompleteModuleRepresentation() module import checking\n";
		std::cout << "The program will now stop\n";
		std::abort();
	}

	const auto resultOfPartitionImport = checkFileForPartitionsImports(currentModule->definitionFile);

	if(not resultOfPartitionImport.has_value()) {
		if(resultOfPartitionImport.error() == regexError::invalidPartitionImportSyntax) {
			std::cout << currentModule->definitionFile.string() << " has invalid partition import syntax: import module:partition;\n The program will now stop.";
			std::abort();
		}
		if(resultOfPartitionImport.error() == regexError::emptyModuleImport) {
			std::cout << currentModule->definitionFile.string() << " has a empty import. \n The program will now stop.";
			std::abort();
		}

		//fallback in case some dumbass adds a third error and doesnt add a if to handle it here
		std::cout << "Some dumbass programmer added a third error, and forgot to add handling for it, or it was cosmic rays\n";
		std::cout << "CompleteModuleRepresentation() partition import checking\n";
		std::cout << "The program will now stop\n";
		std::abort();
	}

	const std::vector<std::string>& moduleDependencies = resultOfModuleImport.value();
	const std::vector<std::string>& partitionDependencies = resultOfPartitionImport.value();

	for (const auto& dependencyName : moduleDependencies) {

		auto foundDependecy = std::ranges::find_if(
			halfModulesReps,
			[&dependencyName](const std::shared_ptr<moduleRepresentation>& orangutango) -> bool {
				return orangutango->moduleName == dependencyName && not orangutango->partitionName.has_value();
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
		fullModules.push_back(CompleteModuleRepresentation(halfRep, halfModulesReps)); 
	}
	return fullModules;
}

[[nodiscard]] std::vector<std::shared_ptr<moduleRepresentation>> buildAllModuleRepresentations(const std::vector<fs::path>& moduleFiles) {
	const std::vector<std::shared_ptr<moduleRepresentation>> halfOnes = constructHalfModulesReps(moduleFiles);
	const std::vector<std::shared_ptr<moduleRepresentation>> completeModules = completeAllModuleRepresentations(halfOnes);
	return completeModules;
}
