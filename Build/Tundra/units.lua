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
    "bsd",
    "External/OpenEXR/staticlib/IlmImf-2_5.lib",
    "External/OpenEXR/staticlib/IlmImfUtil-2_5.lib",
    "External/OpenEXR/staticlib/Half-2_5.lib",
    "External/OpenEXR/staticlib/Imath-2_5.lib",
    "External/OpenEXR/staticlib/Iex-2_5.lib",
    "External/OpenEXR/staticlib/IexMath-2_5.lib",
    "External/OpenEXR/staticlib/IlmThread-2_5.lib",
    "External/Zlib/lib/zlibstatic.lib",
    "Advapi32.lib",
    "User32.lib"
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
    Depends = { "noicelib" },
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

Program {
    Name = "noice_tests",
    Pass = "BuildCode",
    Includes = { LibIncludes, SourceDir },
    Depends = { "noicelib" },
    Libs = { Libs },
    Sources = {
        Glob {
            Dir = SourceDir.."/Test",
            Extensions = { ".cpp", ".h" },
            Recursive = true
        }
    }
}

Default("noice_tests")
