
Build {
    Units = {
        "Build/Tundra/framework.lua",
        "Build/Tundra/units.lua"
    },

    Passes = {
        CodeGeneration = { Name = "CodeGeneration" , BuildOrder = 1 },
        BuildCode = { Name = "BuildCode", BuildOrder = 2 },
        Deploy = { Name = "Deploy", BuildOrder = 3 }
    },

    Configs = {
        {
            Name = "win64-msvc",
            DefaultOnHost = "windows",
            Tools = { "msvc-vs2019", "ispc" },
            Env = {
                ISPCOPTS = {
                    "--target=sse4 --cpu=corei7",
                    { "--arch=x86-64" },
                },
                CPPPATH = {
                    "$(OBJECTDIR)$(SEP)Source"
                },
                CXXOPTS = { "/EHsc", "/MD", "/std:c++17" },
            },
            ReplaceEnv = {
                ISPC = "Tools$(SEP)ispc-v1.15.0-windows$(SEP)bin$(SEP)ispc.exe", 
            },
        },
        {
            Name = "linux-gcc",
            DefaultOnHost = "linux",
            Tools = { "gcc", "ispc" },
            Env = {
                ISPCOPTS = {
                    "--target=sse4 --cpu=corei7 --pic",
                    { "--arch=x86-64" },
                },
                CPPPATH = {
                    "$(OBJECTDIR)$(SEP)Source"
                },
                CXXOPTS = { "-std=c++17" },
            },
            ReplaceEnv = {
                LD = "$(CXX)",
                ISPC = "Tools$(SEP)ispc-v1.15.0-linux$(SEP)bin$(SEP)ispc", 
            },
        },
    },
    
    Env = {
        GENERATE_PDB = "1"
    },

    IdeGenerationHints = {
        Msvc = {
            -- Remap config names to MSVC platform names (affects things like header scanning & debugging)
            PlatformMappings = {
                ['win64-msvc'] = 'Win',
            },
            -- Remap variant names to MSVC friendly names
            VariantMappings = {
                ['release-default']    = 'release',
                ['debug-default']  = 'debug',
                ['production-default']    = 'production',
            },
        },
        
        -- Override solutions to generate and what units to put where.
        MsvcSolutions = {
            ['Noice.sln'] = {},          -- receives all the units due to empty set
        },
        
        BuildAllByDefault = true,
    }
}
