#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>

#include "moduleRepresentation.hpp"

#include "moduleDependencyGraph.hpp"


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
[[nodiscard]] std::vector<std::vector<std::shared_ptr<moduleRepresentation>>> buildModuleDependencyGraph(const std::vector<std::shared_ptr<moduleRepresentation>>& allModules) {
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
