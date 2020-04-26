#pragma once

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

// TODO: Optionally replace with Ri's own smaller version.
#include <co.h>

#define RI_CHECK COCHECK
#define RI_ASSERT COASSERT
#define RI_ABORT COFAIL
#define RI_UNREACHABLE RI_ABORT("unreachable")
#define RI_TODO RI_ABORT("todo")
#define RI_LOG COLOG
#define RI_LOG_DEBUG COLOG_DEBUG

#define RI_POS_OUTSIDE (RiPos){ -1, -1 }
#define RI_ID_NULL (CoString){ 0 }
#define RI_INVALID_SLOT (-1)

typedef struct RiPos RiPos;

struct RiPos {
    intptr_t row, col;
};

CoString ri_path_get_dir_(CoString path);