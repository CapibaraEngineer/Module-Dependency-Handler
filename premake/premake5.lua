include("common.lua")

workspace("ProjectName")
	location("../build")
	architecture("x86_64")
	configurations({ "Debug", "Release", "Sanitize" })
	startproject("ProjectName")

local rootDir = "../"

-- Main application.
project("ProjectName")
	location(rootDir .. "build/ProjectName")
	kind("ConsoleApp")
	common_settings(rootDir)

	files({
		rootDir .. "ProjectName/**.cpp",
		rootDir .. "ProjectName/**.hpp",
		rootDir .. "ProjectName/**.cppm",
	})

	includedirs({
		rootDir .. "ProjectName/include",
		rootDir .. "ProjectName/modules",
		rootDir .. "ProjectName/source",
	})

-- Separate tests binary. Compiles the shared (non-main) sources plus
-- everything under tests/. Keeps the template dependency-free: no
-- external test framework required.
project("ProjectNameTests")
	location(rootDir .. "build/ProjectNameTests")
	kind("ConsoleApp")
	common_settings(rootDir)

	files({
		rootDir .. "tests/**.cpp",
		rootDir .. "tests/**.hpp",
		rootDir .. "ProjectName/source/example.cpp",
		rootDir .. "ProjectName/include/example.hpp",
		rootDir .. "ProjectName/modules/example.cppm",
	})

	includedirs({
		rootDir .. "ProjectName/include",
		rootDir .. "ProjectName/modules",
		rootDir .. "ProjectName/source",
		rootDir .. "tests",
	})
