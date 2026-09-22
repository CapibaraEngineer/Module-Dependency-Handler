#include <cstddef>
#include <expected>
#include <optional>
#include <string>
#include <regex>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "parsing.hpp"

namespace fs = std::filesystem;

[[nodiscard]] std::optional<std::string> getRegexMatch(const std::string& searchString, const std::regex& pattern) {
	std::smatch match;
	if(std::regex_match(searchString, match, pattern)) {
		return match[1];
	}
	return {};
}

[[nodiscard]] std::optional<std::string> regexModuleExport(const std::string& searchString) {
	static const std::regex pattern(R"(export module (\w+);)");
	return getRegexMatch(searchString, pattern);
}

[[nodiscard]] std::optional<std::pair<std::string, std::string>> regexPartitionExport(const std::string& searchString) {
	static const std::regex pattern(R"(export module (\w+):(\w+);)");
	std::smatch match;
	if(std::regex_match(searchString, match, pattern)) {
		return std::pair{match[1].str(), match[2].str()};
	}
	return {};

}

[[nodiscard]] std::optional<std::string> regexModuleImport(const std::string& searchString) {
	static const std::regex pattern(R"(import (\w+);)");
	return getRegexMatch(searchString, pattern);
}

[[nodiscard]] std::optional<std::string> regexInavlidPartitionImport(const std::string& searchString) {
	static const std::regex pattern(R"(import (\w+:\w+);)");
	return getRegexMatch(searchString, pattern);
}

[[nodiscard]] std::optional<std::string> regexPartionImport(const std::string& searchString) {
	static const std::regex pattern(R"(import :(\w+);)");
	return getRegexMatch(searchString, pattern);
}

[[nodiscard]] std::expected<std::string, regexError> checkFileForModuleExport(const fs::path& file) {
	std::string exportedModule;
	size_t moduleExportCount = 0;

	std::string currentLine;
	std::ifstream ifStreamFile(file) ;
	
	while (std::getline(ifStreamFile, currentLine)) {
		const auto regexResult = regexModuleExport(currentLine);
		if(regexResult.has_value()) {
			exportedModule = regexResult.value();
			++moduleExportCount;
		}
	}	

	if(moduleExportCount > 1) {
		return std::unexpected(regexError::two_module_exports);
	}
	if(moduleExportCount < 1) {
		return std::unexpected(regexError::no_export_module);
	}
	if(exportedModule.empty()) {
		return std::unexpected(regexError::two_module_exports);
	}	

	return exportedModule;
}

[[nodiscard]] std::expected<std::pair<std::string, std::string>, regexError> checkFileForPartitonExport(const fs::path& file) {
	std::pair<std::string, std::string> exportedPartition;
	size_t modulePartitionExportCount = 0;

	std::string currentLine;
	std::ifstream ifStreamFile(file) ;
	while (std::getline(ifStreamFile, currentLine)) {
		const auto regexResult =  regexPartitionExport(currentLine);
		std::cout << "YESSSSSS" << regexResult.has_value() <<"|" << modulePartitionExportCount << "\n";
		if(regexResult.has_value()) {
			std::cout << "PRINT HEREEEEEEEEEEEEEE\n";
			std::cout << regexResult->first << "\n" << regexResult->second << "\n";
			exportedPartition = regexResult.value();
			++modulePartitionExportCount;
		}
	}	

	if(modulePartitionExportCount > 1) {
		return std::unexpected(regexError::two_partition_exports);
	}
	if(modulePartitionExportCount < 1) {
		return std::unexpected(regexError::no_export_partition);
	}
	if(exportedPartition.first.empty() or exportedPartition.second.empty()) {
		return std::unexpected(regexError::no_export_partition);
	}

	return exportedPartition;
}

[[nodiscard]] std::expected<std::vector<std::string>, regexError> checkFileForModulesImports(const fs::path& file) {
	std::vector<std::string> importedModules;

	std::string currentLine;
	std::ifstream ifStreamFile(file) ;
	while (std::getline(ifStreamFile, currentLine)) {
		{ // In a unnamed scope so it doesn't collide with the regex below
			const auto regexResult = regexInavlidPartitionImport(currentLine);
			if(regexResult.has_value()) {
				return std::unexpected(regexError::invalidPartitionImportSyntax);
			}
		}

		const auto regexResult =  regexModuleImport(currentLine);
		if(regexResult) {
			importedModules.push_back(regexResult.value());
		}
	}	

	for(const std::string& importedModule : importedModules) {
		if(importedModule.empty()) {
			return std::unexpected(regexError::emptyModuleImport);
		}
	}

	return importedModules;
}

[[nodiscard]] std::expected<std::vector<std::string> , regexError> checkFileForPartitionsImports(const fs::path& file) {
	std::vector<std::string> importedPartitions;

	std::string currentLine;
	std::ifstream ifStreamFile(file) ;
	while (std::getline(ifStreamFile, currentLine)) {
		{ // In a unnamed scope so it doesn't collide with the regex below
			const auto regexResult = regexInavlidPartitionImport(currentLine);
			if(regexResult.has_value()) {
				return std::unexpected(regexError::invalidPartitionImportSyntax);
			}
		}
		const auto regexResult =  regexPartionImport(currentLine);
		if(regexResult) {
			importedPartitions.push_back(regexResult.value());
		}
	}	

	for(const std::string& importedPartition : importedPartitions) {
		if(importedPartition.empty()) {
			return std::unexpected(regexError::emptyPartitonImport);
		}
	}
	return importedPartitions;
}