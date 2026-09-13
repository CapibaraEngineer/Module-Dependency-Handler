#pragma once

#include <memory>
#include <vector>

#include "moduleRepresentation.hpp"

[[nodiscard]] std::size_t findModuleIndex(
	const std::vector<std::shared_ptr<moduleRepresentation>>& allModules,
	const std::shared_ptr<moduleRepresentation>& module);

[[nodiscard]] std::vector<std::vector<std::shared_ptr<moduleRepresentation>>> buildModuleDependencyGraph(const std::vector<std::shared_ptr<moduleRepresentation>>& allModules);