#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <regex>
#include <filesystem>

namespace fs = std::filesystem;

enum class regexError : std::uint8_t {
    no_export_module, 
    no_export_partition, // these are only really errors if both are true for the same file
    two_module_exports,
    two_partition_exports,
    invalidPartitionImportSyntax,
    emptyModuleImport,
    emptyPartitonImport,
};

[[nodiscard]] std::optional<std::string> getRegexMatch(const std::string& searchString, const std::regex& pattern);

[[nodiscard]] std::optional<std::string> regexModuleExport(const std::string& searchString);

[[nodiscard]] std::optional<std::pair<std::string, std::string>> regexPartitionExport(const std::string& searchString);

[[nodiscard]] std::optional<std::string> regexModuleImport(const std::string& searchString);

[[nodiscard]] std::optional<std::string> regexInavlidPartitionImport(const std::string& searchString);

[[nodiscard]] std::optional<std::string> regexPartionImport(const std::string& searchString);

[[nodiscard]] std::expected<std::string, regexError> checkFileForModuleExport(const fs::path& file);

[[nodiscard]] std::expected<std::pair<std::string, std::string>, regexError> checkFileForPartitonExport(const fs::path& file);

[[nodiscard]] std::expected<std::vector<std::string> , regexError> checkFileForModulesImports(const fs::path& file);

[[nodiscard]] std::expected<std::vector<std::string> , regexError> checkFileForPartitionsImports(const fs::path& file);