#include <cstdio>
#include <array>
#include <string>

std::string get_logic_project_name();

static std::string rtrim_newlines(std::string s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
        s.pop_back();
    return s;
}

static std::string run_cmd(const std::string& cmd) {
    std::array<char, 4096> buf{};
    std::string out;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";

    while (fgets(buf.data(), buf.size(), pipe))
        out += buf.data();

    pclose(pipe);
    return rtrim_newlines(out);
}

static std::string strip_suffix(std::string s, const std::string& suffix) {
    if (s.size() >= suffix.size() &&
        s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0) {
        s.resize(s.size() - suffix.size());
    }
    return s;
}

std::string get_logic_project_name() {
    const std::string cmd =
        "osascript -e 'if application \"Logic Pro\" is running then "
        "tell application \"Logic Pro\" "
        "try "
        "set d to front document "
        "return name of d "
        "on error "
        "return \"\" "
        "end try "
        "end tell "
        "else "
        "return \"\" "
        "end if'";

    std::string name = run_cmd(cmd);
    if (name.empty()) return "";

    name = strip_suffix(name, ".logicx");
    return name;
}
