#pragma once

#include <expected>
#include <string>
#include <regex>
#include <filesystem>

namespace fs = std::filesystem;

[[nodiscard]] std::optional<std::string> getRegexMatch(const std::string& searchString, const std::regex& pattern);

[[nodiscard]] std::optional<std::string> regexModuleExport(const std::string& searchString);

[[nodiscard]] std::optional<std::pair<std::string, std::string>> regexPartitionExport(const std::string& searchString);

[[nodiscard]] std::optional<std::string> regexModuleImport(const std::string& searchString);

[[nodiscard]] std::optional<std::string> regexInavlidPartitionImport(const std::string& searchString);

[[nodiscard]] std::optional<std::string> regexPartionImport(const std::string& searchString);

[[nodiscard]] std::expected<std::string, std::string> checkFileForModuleExport(const fs::path& file);

[[nodiscard]] std::expected<std::pair<std::string, std::string>, std::string> checkFileForPartitonExport(const fs::path& file);

[[nodiscard]] std::expected<std::vector<std::string> , std::string> checkFileForModulesImports(const fs::path& file);

[[nodiscard]] std::expected<std::vector<std::string> , std::string> checkFileForPartitionsImports(const fs::path& file);