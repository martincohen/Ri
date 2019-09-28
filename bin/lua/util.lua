local inspect = require 'inspect'
local JSON = require 'json'
local fs = require 'lfs'
local make_template = require 'template'

function string:split(sep)
    local sep, fields = sep or ":", {}
    local pattern = string.format("([^%s]+)", sep)
    self:gsub(pattern, function(c) fields[#fields+1] = c end)
    return fields
 end

local function dump(x)
    print(inspect(x))
end

local function dir(path, f)
    for name in fs.dir(path) do
        if name ~= '.' and name ~= '..' then
            local s = path .. '/' .. name
            local attr = fs.attributes(s)
            f(path, name, attr)
            if attr.mode == 'directory' then
                dir(s, f)
            end
        end
    end
end

local function dir_list_files_(path, pattern, recursive, list)
    if recursive then
        dir(path, function(path, name, attr)
            if not pattern or name:match(pattern) then
                table.insert(list, string.format('%s/%s', path, name))
            end
        end)
    else
        for name in fs.dir(path) do
            if not pattern or name:match(pattern) then
                table.insert(list, string.format('%s/%s', path, name))
            end
        end
    end
end

local function dir_list_files(path_or_list, pattern, recursive)
    if type(path_or_list) ~= 'table' then
        assert(type(path_or_list) == 'string')
        path_or_list = { path_or_list }
    end
    local list = {}
    for _, path in ipairs(path_or_list) do
        dir_list_files_(path, pattern, recursive, list)
    end
    return list
end

local function load(path)
    local f = io.open(path, 'rb')
    if f then
        local s = f:read('*all')
        f:close()
        return s
    else
        error(string.format("unable to load '%s'", path))
    end
end

local function save(path, contents)
    local f = io.open(path, 'wb+')
    if f then
        f:write(contents)
        f:close()
        return true
    end
    error(string.format("unable to save '%s'", path))
end

local function json_load(path)
    return JSON.decode(load(path))
end

local function reverse(arr)
    local i, j = 1, #arr
    while i < j do
        arr[i], arr[j] = arr[j], arr[i]
        i = i + 1
        j = j - 1
    end
end

local function path_dir(path)
    local dir = string.match(path, '(.*[\\/])')
    return dir or ''
end

local function path_name(path)
    local name = string.match(path, '[^\\^/]+$')
    return name or ''
end

local function path_name_only(path)
    local name_ext = path_name(path)
    local name = string.match(name_ext, '^.*[%.]')
    if name then
        return name:sub(1, #name - 1)
    end
    return ''
end

local function path_extension(path)
    local extension = string.match(path, '[^%.]+$')
    return extension or ''
end

local PATH_SEPARATOR = '\\'
local PATH_SEPARATOR_OTHER = '/'

local function path_normalize(path)
    local path = path:gsub(PATH_SEPARATOR_OTHER, PATH_SEPARATOR):gsub(PATH_SEPARATOR .. '+', PATH_SEPARATOR)
    local names = path:split(PATH_SEPARATOR)
    local normalized = {}
    for _, name in ipairs(names) do
        if name == '..' then
            table.remove(normalized)
        elseif name ~= '.' then
            table.insert(normalized, name)
        end
    end
    return table.concat(normalized, PATH_SEPARATOR)
end

local function path_append(path, ...)
    return table.concat({ path_normalize(path), ... }, PATH_SEPARATOR)
end

-- converts v to valid value in C
local function to_c(v)
    if v == nil then
        return "NULL"
    elseif type(v) == 'string' then
        return string.format('"%s"', v:gsub('"', '\\"'):gsub("\\", '\\\\'))
    end
    return v
end

local function to_csym(sym)
    assert(type(sym) == 'string')
    assert(sym ~= '')

    sym = sym:gsub('-', '_')

    return sym
end

local function flatten(v, t)
    t = t or {}
    for _, val in ipairs(v) do
        if type(val) == 'table' then
            flatten(val, t)
        else
            table.insert(t, val)
        end
    end
    return t
end

local function execute(...)
    local cl = table.concat(flatten({...}), ' ')
    if os.execute(cl) ~= 0 then
        error(string.format("execute failed for:\n%s", cl))
    end
end

local function mkpath(path)
    local parts = path:split('\\/')
    table.remove(parts)
    local s = nil
    for _, part in ipairs(parts) do
        if not s then
            s = part
        else
            s = string.format('%s/%s', s, part)
        end
        lfs.mkdir(s)
    end
end

local function copy(from, to)
    local i = io.open(from, 'rb')
    if not i then
        error(string.format("cannot open input file '%s'", from))
    end
    local s = i:read('*all')
    i:close()

    local o = io.open(to, 'wb')
    if not o then
        error(string.format("cannot open output file '%s'", to))
    end
    o:write(s)
    o:close()
end

local function merge(a, ...)
    local L = {...}
    for _, b in ipairs(L) do
        for k, v in pairs(b) do
            a[k] = v
        end
    end
    return a
end

local function template(path, args)
    local argn = {}
    local argv = {}
    for k, v in pairs(args) do
        table.insert(argn, k)
        table.insert(argv, v)
    end

    local template_f = make_template(path, table.concat(argn, ", "))
    return template_f(unpack(argv))
end

return {
    dump = dump,
    dir = dir,
    load = load,
    save = save,
    json_load = json_load,
    reverse = reverse,
    path_dir = path_dir,
    path_name = path_name,
    path_name_only = path_name_only,
    path_extension = path_extension,
    path_normalize = path_normalize,
    path_append = path_append,
    to_c = to_c,
    to_csym = to_csym,
    dir_list_files = dir_list_files,
    flatten = flatten,
    execute = execute,
    mkpath = mkpath,
    copy = copy,
    merge = merge,
    template = template
}