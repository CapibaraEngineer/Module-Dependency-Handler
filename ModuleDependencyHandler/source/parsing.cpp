#include <expected>
#include <optional>
#include <string>
#include <regex>
#include <filesystem>
#include <fstream>

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

[[nodiscard]] std::expected<std::pair<std::string, std::string>, std::string> regexPartitionExport(const std::string& searchString) {
	static const std::regex pattern(R"(export module (\w+):(\w+);)");
	//std::smatch match;
	//if(std::regex_match(searchString, match, pattern)) {
	//	return std::pair{match[1].str(), match[2].str()};
	//}
	//return std::unexpected("");

}

[[nodiscard]] std::optional<std::string> regexModuleImport(const std::string& searchString) {
	static const std::regex pattern(R"(import (\w+);)");
	return getRegexMatch(searchString, pattern);
}

[[nodiscard]] std::optional<std::string> regexPartionImport(const std::string& searchString) {
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