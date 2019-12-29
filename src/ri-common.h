#pragma once

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

// TODO: Optionally replace with Ri's own smaller version.
#include <co-lib.h>

#define RI_CHECK CHECK
#define RI_ASSERT ASSERT
#define RI_ABORT FAIL
#define RI_UNREACHABLE RI_ABORT("unreachable")
#define RI_TODO RI_ABORT("todo")
#define RI_LOG LOG
#define RI_LOG_DEBUG LOG_DEBUG

#define RI_POS_OUTSIDE (RiPos){ -1, -1 }
#define RI_ID_NULL (String){ NULL, 0 }
#define RI_INVALID_SLOT (-1)

typedef struct RiPos RiPos;

struct RiPos {
    iptr row, col;
};

String ri_path_get_dir_(String path);