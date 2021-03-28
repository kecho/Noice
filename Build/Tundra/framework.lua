require 'tundra.syntax.glob'
local path     = require "tundra.path"

DefRule {
  Name = "ISPC2",
  Command = "", 

  Blueprint = {
    Headers = { Required = false, Type = "table" },
    Source = { Required = true, Type = "string" },
  },

  Setup = function (env, data)
    local inputFiles = {}
    if (data.Headers ~= nil) then
        for i, v in ipairs(data.Headers) do
            table.insert(inputFiles, v)
        end
    end
    table.insert(inputFiles, data.Source)

    local src = data.Source
    local base_name = path.drop_suffix(src) 
    local objFile = "$(OBJECTDIR)$(SEP)" .. base_name .. "__" .. path.get_extension(src):sub(2) .. "$(OBJECTSUFFIX)"
    local hFile = "$(OBJECTDIR)$(SEP)" .. base_name .. ".ispc.h"
    return {
        InputFiles = inputFiles,
        OutputFiles = { objFile, hFile },
        Command = "$(ISPC) $(ISPCOPTS) -o $(@:[1]) -h $(@:[2]) "..src
    }
  end,
}

function _G.ISPCGlob(folder, headers, pass)
    local files = Glob {
        Dir = folder,
        Extensions = { ".ispc" },
        Recursive = true
    }
    
    local nodes = {}
    for i, f in ipairs(files) do
        local n = ISPC2 {
            Pass = pass,
            Headers = headers,
            Source = f            
        }
        table.insert(nodes, n)
    end

    return nodes
end

