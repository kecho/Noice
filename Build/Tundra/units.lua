require 'tundra.syntax.glob'
require 'tundra.syntax.files'
require 'tundra.syntax.ispc'

local npath = require 'tundra.native.path'

local SourceDir = "Source";

local LibIncludes = {
    SourceDir.."/Noice/Public/",
    "External/OpenEXR/include/OpenEXR"
}

local Libs = {
    "External/OpenEXR/lib/IlmImf-2_5.lib",
    "External/OpenEXR/lib/IlmImfUtil-2_5.lib",
    "External/OpenEXR/lib/Half-2_5.lib",
    "External/OpenEXR/lib/Imath-2_5.lib",
    "External/OpenEXR/lib/Iex-2_5.lib",
    "External/OpenEXR/lib/IexMath-2_5.lib",
    "External/OpenEXR/lib/IlmThread-2_5.lib",
    "Advapi32.lib",
    "User32.lib"
}

local install2 = Install2 {
    Name = "DllInstall",
    Pass = "BuildCode",
    Sources = {
        "External/Zlib/bin/zlib.dll",
        "External/OpenEXR/bin/IlmImf-2_5.dll",
        "External/OpenEXR/bin/IlmImfUtil-2_5.dll",
        "External/OpenEXR/bin/Half-2_5.dll",
        "External/OpenEXR/bin/Imath-2_5.dll",
        "External/OpenEXR/bin/Iex-2_5.dll",
        "External/OpenEXR/bin/IexMath-2_5.dll",
        "External/OpenEXR/bin/IlmThread-2_5.dll"
    },
    TargetDir = "$(OBJECTDIR)"
}

StaticLibrary {
    Name = "noicelib",
    Pass = "BuildCode",
    Includes = LibIncludes,
    Sources = {
        Glob {
            Dir = SourceDir.."/Noice",
            Extensions = { ".cpp", ".h", ".hpp" },
            Recursive =  true
        },
        _G.ISPCGlob(
            SourceDir.."/Noice",
            Glob { Dir = SourceDir.."/Noice", Extensions = { ".ispch" }, Recursive = true },
            "CodeGeneration"
        )
    }
}

Default("noicelib")

Program {
    Name = "noice",
    Pass = "BuildCode",
    Includes = LibIncludes,
    Depends = { "noicelib", install2 },
    Libs = { Libs },
    Sources = {
        Glob {
            Dir = SourceDir.."/Cli",
            Extensions = { ".cpp", ".h" },
            Recursive = true
        }
    }
}

Default("noice")
