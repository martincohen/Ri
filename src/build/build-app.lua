local util = require 'util'
local inspect = require 'inspect'
local template = require 'template'
local execute = util.execute
local push = table.insert

-- minimal base: 2560 bytes

return function (config)
    print('building executable...')

    local compiler = {
        '-nologo',
        '-utf-8',
        '-GR-',
        '-Gm-',
        '-EHsc-',
        '-FC',
        -- Fast floating point. # https://msdn.microsoft.com/en-us/library/e7s85ffb.aspx
        '-fp:fast',
        '-Oi',
        '-DWINVER=0x0A00',
        '-D_WIN32_WINNT=0x0A00',
        '-DNOMINMAX',
        -- disable warning C4116: unnamed type definition in parentheses
        '-wd4116',
        '-WX',
        '-I../src',
        '-I../src/lib',
        '-I../src/id',
    }
    local linker = {
        '-link',
        '-incremental:no',
        '-stack:0x100000,0x100000',
        '-subsystem:console',
    }
    local libs = {}

    local minimal = false

    if config.release then
        minimal = config.minimal
        push(compiler, {
            '-O2',
            '-GL',
            '-DBUILD_RELEASE'
        })
        if not minimal then
            push(compiler, '-MT')
        end
        push(linker, {
            '-ltcg', -- needed if GL is specified
            '-opt:ref',
            '-opt:icf',
        })
    else
        push(compiler, {
            '-MTd',
            '-Od',
            '-Z7',
        })
        push(linker, {
            '-debug:fastlink'
        })
    end

    if config.driver == 'opengl33' then
        push(compiler, '-DSOKOL_GLCORE33')
        push(libs, {
            'kernel32.lib',
            'user32.lib',
            'gdi32.lib'
        })
    end

    if config.driver == 'd3d11' then
        push(compiler, '-DSOKOL_D3D11')
    end

    if minimal then
        push(compiler, {
            '-DBUILD_MINIMAL',
            '-DSOKOL_NO_ENTRY',
            -- '-D_NO_CRT_STDIO_INLINE',
            '-GS-',       -- Turns off buffer security check
            '-Gs9999999', -- The number of bytes that local variables can occupy before a stack probe is initiated
        })
        push(linker, {
            '-nodefaultlib',
            '-entry:core_main',
            '-LIBPATH:../src/lib/win32/msvcrt'
        })
        push(libs, {
            'kernel32.lib',
            'msvcrt.lib',
            'chkstk64.obj'
        })
    end

    push(compiler, '-DMATH_SSE')

    execute('cl', '../src/main.c', compiler, linker, libs)
end