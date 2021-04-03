require 'tundra.syntax.glob'

local nodegen  = require "tundra.nodegen"
local files    = require "tundra.syntax.files"
local path     = require "tundra.path"
local util     = require "tundra.util"
local depgraph = require "tundra.depgraph"
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


local install2_mt = nodegen.create_eval_subclass ({
  Suffix = "$(OBJECTSUFFIX)",
  Label = "Object $(<)",
})

local install2_blueprint = {
  Name = { Type = "string", Required = true },
  Sources = { Type = "table", Required = true },
  TargetDir = { Type = "string", Required = true },
}

local function copy_file2(env, src, dst, pass, deps)
  return depgraph.make_node {
    Env = env,
    Label = dst,
    Annotation = "CopyFile $(<)",
    Action = "$(_COPY_FILE)",
    InputFiles = { src },
    OutputFiles = { dst },
    Dependencies = deps,
    Pass = pass,
  }
end

function install2_mt:create_dag(env, data, deps)
    local my_pass = data.Pass
    local build_id = env:get("BUILD_ID")
    local sources = data.Sources
    local target_dir = data.TargetDir

    local copies = {}; 

    -- all the copy operations will depend on all the incoming deps
    for _, src in util.nil_ipairs(sources) do
        local base_fn = select(2, path.split(src))
        local target = target_dir .. '/' .. base_fn
        local cpdag = copy_file2(env, src, target, my_pass, deps)
        cpdag.Label = target;
        copies[#copies + 1] = cpdag;
    end


    local dag = depgraph.make_node {
        Env          = env,
        InputFiles   = data.Sources,
        Name = data.Name,
        Label        = "Install group for " .. data.Name .. build_id,
        Pass         = my_pass,
        Dependencies = copies
    }
    return dag;
end

nodegen.add_evaluator("Install2", install2_mt, install2_blueprint)

