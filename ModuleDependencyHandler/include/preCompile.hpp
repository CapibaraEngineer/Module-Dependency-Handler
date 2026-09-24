#include <vector>
#include <memory>

#include "moduleRepresentation.hpp"

void runClang(const std::vector<std::vector<std::shared_ptr<moduleRepresentation>>>& moduleDependencyGraph, const fs::path& outPath);