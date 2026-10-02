set_xmakever("3.1.1")
set_project("CombatMusicFixNG")
set_license("GPL-3.0")
set_policy("package.requires_lock", true)

local version = "2.0.0"

add_repositories("bmk https://github.com/gabriel-andreescu/BethesdaModKit")
add_repositories("xmake-luals https://github.com/gabriel-andreescu/xmake-luals.git")
add_addons("bmk 0.5.0", "xmake-luals 0.1.1")
includes("@addon/bmk/project", "@addon/xmake-luals/luals")
includes("@addon/bmk/native")

-- Dependencies
add_requires("commonlibsse-ng 8.0.1", { system = false })
add_requires("clib-util 1.5.0", { system = false })
add_requires("bmk")

-- Build targets

target("Native", function()
    set_default(false)
    set_basename("CombatMusicFixNG")
    set_version(version)
    add_rules("@commonlibsse-ng/plugin", {
        author = "GabonZ",
        description = "Stops lingering combat music.",
    })
    add_rules("@addon/bmk/skyrim.plugin")
    add_files("$(projectdir)/src/**.cpp")
    add_includedirs("$(projectdir)/src")
    set_pcxxheader("src/PCH.h")
    add_packages("commonlibsse-ng", "clib-util", "bmk")
end)

-- Packages
target("CombatMusicFixNG", function()
    set_version(version)
    add_rules("@addon/bmk/skyrim.package", {
        targets = {
            "Native",
        },
        nexus = {
            mod_id = "7318624382843",
            file_id = "3170236",
            category = "main",
            primary = true,
            display_name = "Combat Music Fix NG",
        },
    })
    add_installfiles("$(projectdir)/assets/(**)")
end)
