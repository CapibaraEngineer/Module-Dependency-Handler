-- Shared build settings for every project in this workspace.
-- Keeps premake5.lua small and guarantees new projects created by
-- configure.sh behave identically.
--
-- Usage in premake5.lua:
--   include "common.lua"
--   project("MyApp")
--     location(rootDir .. "build/MyApp")
--     kind("ConsoleApp")
--     common_settings(rootDir)

function common_settings(rootDir)
	language("C++")
	cppdialect("C++23")
	warnings("Extra")
	staticruntime("on")
	exceptionhandling("Default")
	rtti("On")

	targetdir(rootDir .. "bin/%{prj.name}/%{cfg.system}_%{cfg.architecture}/%{cfg.buildcfg}")
	objdir(rootDir .. "build/obj/%{prj.name}/%{cfg.system}_%{cfg.architecture}/%{cfg.buildcfg}")

	-- Named modules (.cppm) need explicit modules support on GCC/Clang.
	-- Without this, any .cppm file fails with
	-- "'module' does not name a type ... only available with '-fmodules'".
	filter("toolset:gcc or toolset:clang")
		buildoptions({ "-fmodules" })

	-- Clang links via lld: bfd ld would need the LLVMgold plugin for LTO,
	-- which standalone Clang installs typically do not ship.
	filter("toolset:clang")
		linkoptions({ "-fuse-ld=lld" })

	filter("configurations:Debug")
		runtime("Debug")
		defines({ "DEBUG" })
		linktimeoptimization("off")
		optimize("off")
		symbols("full")

	filter("configurations:Release")
		runtime("Release")
		defines({ "NDEBUG" })
		linktimeoptimization("on")
		optimize("on")
		symbols("off")

	-- Sanitized debug build: AddressSanitizer + UndefinedBehaviorSanitizer.
	filter("configurations:Sanitize")
		runtime("Debug")
		defines({ "DEBUG" })
		sanitize({ "Address", "UndefinedBehavior" })
		linktimeoptimization("off")
		optimize("off")
		symbols("full")

	filter({})
end
