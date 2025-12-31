#include <libproc.h>
#include <string.h>
#include <iostream>

bool is_logic_pro_running();

static bool is_process_running_exact(const char* exact_name) {
    pid_t pids[2048];

    int bytes = proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pids));
    if (bytes <= 0) return false;

    int num = bytes / (int)sizeof(pids[0]);

    for (int i = 0; i < num; ++i) {
        if (pids[i] == 0) continue;

        char buf[PROC_PIDTBSDINFO_SIZE];
        int status = proc_pidinfo(pids[i], PROC_PIDTBSDINFO, 0, buf, PROC_PIDTBSDINFO_SIZE);
        if (status != PROC_PIDTBSDINFO_SIZE) continue;

        auto* proc = (proc_bsdinfo*)buf;

        if (proc->pbi_name[0] == '\0') continue;

        if (strcmp(exact_name, proc->pbi_name) == 0) {
            return true;
        }
    }
    return false;
}

static bool is_process_running_contains(const char* needle) {
    pid_t pids[2048];

    int bytes = proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pids));
    if (bytes <= 0) return false;

    int num = bytes / (int)sizeof(pids[0]);

    for (int i = 0; i < num; ++i) {
        if (pids[i] == 0) continue;

        char buf[PROC_PIDTBSDINFO_SIZE];
        int status = proc_pidinfo(pids[i], PROC_PIDTBSDINFO, 0, buf, PROC_PIDTBSDINFO_SIZE);
        if (status != PROC_PIDTBSDINFO_SIZE) continue;

        auto* proc = (proc_bsdinfo*)buf;

        if (proc->pbi_name[0] == '\0') continue;

        if (strstr(proc->pbi_name, needle) != nullptr) {
            return true;
        }
    }
    return false;
}

bool is_logic_pro_running() {
    const char* exact_candidates[] = {
        "Logic Pro",
        "Logic Pro X",
        "LogicPro",
        "LogicProX"
    };

    for (const char* name : exact_candidates) {
        if (is_process_running_exact(name)) return true;
    }

    return is_process_running_contains("Logic");
}
