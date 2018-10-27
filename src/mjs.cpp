#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <chrono>

#include <mjs/value.h>
#include <mjs/parser.h>
#include <mjs/interpreter.h>
#include <mjs/printer.h>

#include <fstream>
#include <streambuf>
#include <cstring>

std::shared_ptr<mjs::source_file> read_ascii_file(const char* filename) {
    std::ifstream in(filename);
    if (!in) throw std::runtime_error("Could not open " + std::string(filename));
    return std::make_shared<mjs::source_file>(std::wstring(filename, filename+std::strlen(filename)), std::wstring((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
}

std::shared_ptr<mjs::source_file> make_source(const std::wstring_view& s) {
    return std::make_shared<mjs::source_file>(std::wstring(L"inline code"), std::wstring(s));
}

int interpret_file(const std::shared_ptr<mjs::source_file>& source) {
    auto bs = mjs::parse(source);
    mjs::interpreter i{*bs};
    mjs::value res{};
    for (const auto& s: bs->l()) {
        res = i.eval(*s).result;
    }
    return to_int32(res);
}

int main(int argc, char* argv[]) {
    try {
        if (argc > 1) {
            return interpret_file(read_ascii_file(argv[1]));
        }

        mjs::interpreter i{*mjs::parse(make_source(L""))};
        // Keep previous statements alive in case any of them contained functions
        std::vector<std::unique_ptr<mjs::block_statement>> previous_blocks;
        for (;;) {
            std::wcout << "> " << std::flush;
            std::wstring line;
            if (!getline(std::wcin, line)) {
                break;
            }
            previous_blocks.push_back(mjs::parse(make_source(line)));
            mjs::value res{};
            for (const auto& s: previous_blocks.back()->l()) {
                res = i.eval(*s).result;
            }
            mjs::debug_print(std::wcout, res, 2);
            std::wcout << "\n";
        }

    } catch (const std::exception& e) {
        std::wcout << e.what() << "\n";
        return 1;
    }
}