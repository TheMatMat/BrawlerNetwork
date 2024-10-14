package("selengine", function ()
	set_sourcedir("SelEngine-2024")

	add_versions("2024.10.05", "")

	add_deps("entt", "fmt", "libsdl", "libsdl_image", "nlohmann_json")
	add_deps("imgui", { configs = { sdl2 = true, sdl2_renderer = true }})

	on_install(function (package)
		local configs = {}
		configs.examples = false

		import("package.tools.xmake").install(package, configs)
	end)
end)

set_xmakever("2.8.7")

set_project("Brawler")

add_rules("mode.asan", "mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

set_allowedmodes("debug", "releasedbg", "asan")
set_allowedplats("windows", "mingw", "linux", "macosx")
set_allowedarchs("windows|x64", "mingw|x86_64", "linux|x86_64", "macosx|x86_64")
set_defaultmode("debug")

add_requires("enet6", { configs = { debug = is_mode("debug") }})

-- On rajoute le moteur en dépendance
add_requires("selengine", { configs = { debug = is_mode("debug") }})
add_requireconfs("libsdl", "**.libsdl", { configs = { sdlmain = false }})

-- Petite configuration de base
add_sysincludedirs("thirdparty/include")

set_exceptions("cxx")
set_encodings("utf-8")
set_languages("c89", "cxx17")
set_rundir(".")
set_symbols("debug", "hidden")
set_targetdir("./bin/$(plat)_$(arch)_$(mode)")
set_warnings("allextra")

if is_plat("windows") then
	set_runtimes("MD")
--	set_runtimes(is_mode("debug") and "MDd" or "MD")

	add_defines("_CRT_SECURE_NO_WARNINGS", "NOMINMAX")
	add_cxxflags("/permissive-", "/Zc:__cplusplus", "/Zc:externConstexpr", "/Zc:inline", "/Zc:lambda", "/Zc:preprocessor", "/Zc:referenceBinding", "/Zc:strictStrings", "/Zc:throwingNew")
	add_cxflags("/w44062") -- Enable warning: switch case not handled
end

if is_mode("debug") then
	add_defines("DEBUG_GHOSTS")
end

add_packages("enet6", "selengine")

target("BrawlerClient")
	set_kind("binary")

	add_headerfiles("cl_**.hpp", "sh_**.hpp", "cl_**.h", "sh_**.h")
	add_files("cl_**.cpp", "sh_**.cpp")

target("BrawlerServer")
	set_kind("binary")

	add_headerfiles("sv_**.hpp", "sh_**.hpp", "sv_**.h", "sh_**.h")
	add_files("sv_**.cpp", "sh_**.cpp")
