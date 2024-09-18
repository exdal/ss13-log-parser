includes("xmake/**.lua")

add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", { outputdir = ".", lsp = "clangd" })

set_project("parselogs")
set_version("1.0.0")

set_encodings("utf-8")

-- Compiler Config --
-- WARNINGS --
set_warnings("allextra", "pedantic")
add_cxxflags(
    -- "-Wshadow-all", -- clang analyzer false positive
    "-Wno-gnu-line-marker",
    "-Wno-gnu-anonymous-struct",
    "-Wno-gnu-zero-variadic-macro-arguments",
    "-Wno-missing-braces",
    { tools = { "clang", "gcc" } })

add_ldflags("-fuse-ld=lld", { tools = { "clang" } })

target("parselogs")
    -- Basic Project Config --
    set_kind("binary")
    add_languages("cxx23")
    add_rpathdirs("@executable_path")
    set_runtimes("MT", "c++_static")

    -- Project Files --
    add_files("src/**.cc")
    add_includedirs("src", { public = true })
    add_forceincludes("src/pch.hh", { public = true })
    set_pcheader("src/pch.hh", { public = true })

    -- Packages --
    add_packages(
        "fmt",
        "simdjson",
        "re2",
        "gzip-hpp",
        { public = true }
    )

