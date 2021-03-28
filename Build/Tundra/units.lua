require 'tundra.syntax.glob'
require 'tundra.syntax.files'
require 'tundra.syntax.ispc'

local npath = require 'tundra.native.path'

local SourceDir = "Source";

local LibIncludes = {
    SourceDir.."/Noice/Public/"
}

local Libs = {
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
