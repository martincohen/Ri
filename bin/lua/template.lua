local inspect = require'inspect'

local function map(t, fn)
  local res = {}
  for k, v in pairs(t) do res[k] = fn(v) end
  return res
end

local function read(str, params, chunkname)
  params = params and ("," .. params) or ""
  local f = function(x)
    local s, t = string.match(x, "([\r\n]*)(.*)")
    -- print(inspect(s), inspect(m))
    return string.format("%s echo(%q)", s, t)
  end
  str = ("?>"..str.."<?"):gsub("%?>([\r\n]*.-)[\t]*<%?", f)
  str = "local echo " .. params .. " = ..." .. str
  -- print(str)
  local fn = assert(loadstring(str, chunkname))
  return function(...)
    local output = {}
    local echo = function(...)
      for _, v in ipairs({...}) do
        table.insert(output, v)
      end
    end
    fn(echo, ...)
    return table.concat(map(output, tostring))
  end
end

local function make_template(path, params)
  local f = io.open(path)
  if not f then
      error(string.format('cannot open %s', path))
  end
  local template_string = f:read('*all')
  f:close()

  return read(template_string, params, path)
end

return make_template