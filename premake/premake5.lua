include("common.lua")

workspace ("ModuleDependencyHandler")
	location("../build")
	architecture("x86_64")
	configurations({ "Debug", "Release", "Sanitize" })
    startproject ("ModuleDependencyHandler")

local rootDir = "../"

-- Main application.
project("ModuleDependencyHandler")
	location(rootDir .. "build/ModuleDependencyHandler")
	kind("ConsoleApp")
	common_settings(rootDir)

	files({
		rootDir .. "ModuleDependencyHandler/**.cpp",
		rootDir .. "ModuleDependencyHandler/**.hpp",
		rootDir .. "ModuleDependencyHandler/**.cppm",
	})

	includedirs({
	})

-- Separate tests binary. Everything under tests/.
project("ModuleDependencyHandlerTests")
	location(rootDir .. "build/ModuleDependencyHandlerTests")
	kind("ConsoleApp")
	common_settings(rootDir)

	files({
		rootDir .. "tests/**.cpp",
		rootDir .. "tests/**.hpp",
	})

	includedirs({
		rootDir .. "tests",
	})
