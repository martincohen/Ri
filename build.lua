local path = arg[1] or './'
package.path = table.concat({
    package.path,
    path .. "src/build/?.lua",
    path .. "src/build/lib/?.lua",
}, ';')

local config = {
    release = false,
    path = path,
}

require('build-app')(config)