#include "pid_storage.h"
#include "common.h"

#include <stdlib.h>

int main() {
    bool ok = true;
    ok &= cleanup_containers();
    ok &= system((AUCONT_PREFIX"bin/scripts/cpu_cg_cleanup.sh " + std::string(AUCONT_PREFIX)).c_str()) == 0;
    return ok ? 0 : 1;
}
