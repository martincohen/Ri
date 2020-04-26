local util = require 'util'
local inspect = require 'inspect'
local template = require 'template'
local execute = util.execute
local push = table.insert

local name = 'lever'
local minimal = true
local dll = false
local map = true
local luajit = false
local freetype = false

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
        '-I../src/id',
        '-I../src/lib',
        '-I../../rt/src',
    }
    local linker = {
        '-link',
        '-incremental:no',
        '-stack:0x100000,0x100000',
        '-subsystem:console',
    }

    if luajit then
        print('adding luajit')
        push(compiler, {
            '-I../../rt/src/LuaJit-2.0.5/src',
        })
        push(linker, {
            '-libpath:../../rt/src/LuaJit-2.0.5/src',
            'lua51.lib'
        })
    end

    if freetype then
        print('adding freetype')
        push(compiler, {'-I../../rt/src/freetype', '-DBUILD_LIBRARY_FREETYPE', '-DCOFONT_FREETYPE'})
        push(linker, {'-LIBPATH:../../rt/src/freetype', 'freetype.lib'})
    end

    if dll then
        print('as DLL')
        push(linker, {
            '-DLL',
            string.format('-OUT:%s.dll', name),
        })
    else
        push(linker, {
            string.format('-OUT:%s.exe', name),
        })
    end

    if map then
        push(linker, {
            string.format('-map:%s.map', name),
            '-mapinfo:exports'
        })
    end

    local libs = {}

    if config.release then
        push(compiler, {
            '-O2',
            '-GL',
            '-Gy'
        })
        push(linker, {
            '-ltcg', -- needed if GL is specified
            '-opt:ref',
            '-opt:icf',
        })

        if minimal then
            push(compiler, {
                '-GS-',       -- Turns off buffer security check
                '-Gs9999999', -- The number of bytes that local variables can occupy before a stack probe is initiated
                '-DCO_MINIMAL=1'
            })
            push(linker, {
                '-nodefaultlib',
                '-entry:rt_main',
                '-LIBPATH:../../rt/src/win32/msvcrt'
            })
            push(libs, {
                'msvcrt.lib',
                'chkstk64.obj',
            })
        else
            push(compiler, {
                '-MT'
            })
        end
    else
        push(compiler, {
            '-DCO_DEBUG=1',
            '-MTd',
            '-Od',
            '-Z7',
        })
        push(linker, {
            '-debug:fastlink'
        })
    end

    if config.driver == 'opengl33' then
        push(compiler, '-DCO_GLCORE33')
    end

    if config.driver == 'd3d11' then
        push(compiler, '-DCO_D3D11')
    end

    push(libs, {
        'kernel32.lib',
        'user32.lib',
        'gdi32.lib',

        'UxTheme.lib',
        'Dwmapi.lib',
        'Shell32.lib',

        'dxguid.lib',
        'dxgi.lib',
        'd3d11.lib',
    })

    -- execute('cl', '../src/rt.h', compiler, { "/P" })
    execute('cl', '../src/main.c', compiler, linker, libs)
end