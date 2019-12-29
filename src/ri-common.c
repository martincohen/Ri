#include "ri-common.h"

String ri_path_get_dir_(String path) {
    char *it = path.items + path.count - 1;
    for (; it >= path.items; --it) {
        if (*it == '/' || *it == '\\') {
            return (String){ .items = path.items, .count = it - path.items };
        }
    }
    return (String){ .items = path.items, .count = 0 };
}