#pragma once

#include <optional>
#include <string>
#include <filesystem>
#include <vector>
#include <memory>
#include <expected>

namespace fs = std::filesystem;

struct moduleRepresentation {
	bool isPartition = false;
	std::string moduleName;
	std::optional<std::string> partitionName;
	fs::path definitionFile{};
	std::vector<std::shared_ptr<moduleRepresentation>> dependencies;
};

[[nodiscard]] std::expected<moduleRepresentation, std::string> buildHalfModuleRepresentationFromFile(const fs::path& file);
[[nodiscard]] std::vector<std::shared_ptr<moduleRepresentation>> constructHalfModulesReps(const std::vector<fs::path>& moduleFiles);
[[nodiscard]] std::shared_ptr<moduleRepresentation> CompleteModuleRepresentation(const std::shared_ptr<moduleRepresentation>& currentModule, const std::vector<std::shared_ptr<moduleRepresentation>>& halfModulesReps);
[[nodiscard]] std::vector<std::shared_ptr<moduleRepresentation>> completeAllModuleRepresentations(const std::vector<std::shared_ptr<moduleRepresentation>>& halfModulesReps);
[[nodiscard]] std::vector<std::shared_ptr<moduleRepresentation>> buildAllModuleRepresentations(const std::vector<fs::path>& moduleFiles);
