#include "pipepp/options.hpp"
#include <format>

std::string pipepp::detail::path_tostr(const char* path, int line)
{
    auto out = std::string(path);
    if (auto sz = out.find_last_of("\\/"); sz != std::string::npos) {
        out = out.substr(sz + 1);
    }
    std::format_to(std::back_inserter(out), " ({:>5})", line);
    return std::move(out);
}