#include "ri-common.h"

CoString ri_path_get_dir_(CoString path) {
    char *it = path.items + path.count - 1;
    for (; it >= path.items; --it) {
        if (*it == '/' || *it == '\\') {
            return (CoString){ .items = path.items, .count = it - path.items };
        }
    }
    return (CoString){ .items = path.items, .count = 0 };
}