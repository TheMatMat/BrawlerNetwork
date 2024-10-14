option("examples", { description = "Enable examples", default = true })

add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

add_requires("chipmunk2d", "fmt", "libsdl", "libsdl_image", "libsdl_ttf", "entt", "lz4", "dr_wav")
add_requires("openal-soft", { configs = { shared = true }})
add_requires("nlohmann_json") -- rapidjson
add_requires("imgui", { configs = { sdl2 = true, sdl2_renderer = true }})
add_requireconfs("libsdl", "**.libsdl", { configs = { sdlmain = false }})

set_project("SelEngine")

set_languages("cxx20")
set_exceptions("cxx")
set_encodings("utf-8")
set_rundir(".")
set_warnings("allextra")
add_includedirs("include")
add_includedirs("thirdparty/include")

add_cxflags("/wd4251")

target("SelEngine", function ()
    set_kind("shared")
    add_headerfiles("include/(Sel/**.hpp)", "include/(Sel/**.inl)")
    add_headerfiles("src/Sel/**.hpp", "src/Sel/**.inl")
    add_files("src/Sel/**.cpp")
    add_packages("entt", "fmt", "imgui", "libsdl", "libsdl_image", "libsdl_ttf", "nlohmann_json", { public = true })
    add_packages("chipmunk2d", "lz4")
    add_defines("SEL_ENGINE_BUILD")
    add_includedirs("src")
end)

if has_config("examples") then
    add_requires("wgpu-native")
    add_requires("sol2")

    target("SelGame", function ()
        set_kind("binary")
        add_files("src/main.cpp")
        add_deps("SelEngine")
        add_packages("sol2")
    end)

    target("SelTest", function ()
        set_kind("binary")
        add_files("src/test.cpp")
        add_deps("SelEngine")
        add_packages("nlohmann_json", "chipmunk2d", "openal-soft", "dr_wav")
    end)

    target("Sel3D", function ()
        set_kind("binary")
        add_files("src/3d.cpp")
        add_deps("SelEngine")
        add_packages("wgpu-native")
    end)
end

--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro definition
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--

