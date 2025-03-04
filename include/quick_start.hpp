#pragma once

#include <vector>
#include <string>
#include <format>
#include <array>
#include <string_view>
#include <fast_io.h>
#include <fast_io_device.h>

namespace evqovv {
class quick_start {
public:
  void run(int argc, char **argv) {
    parse({argv + 1, argv + argc});

    init();
    create_cmake_lists();
    create_cmake_presets();
    create_gitignore();
    create_header_files();
    create_vs_code();
  }

private:
  void parse(::std::vector<::std::string_view> const &parts) {
    if (parts.empty()) {
      throw ::std::runtime_error("Parsing failed.");
    }

    option.project_name = parts[0];

    for (decltype(parts.size()) i = 1; i != parts.size(); ++i) {
      if (parts[i][0] == '-') {
        if (parts[i][1] == '-') {
        } else {
          decltype(i) param_cnt{};
          for (auto &&ch : parts[i].substr(1)) {
            switch (ch) {
            case 's':
              ++param_cnt;
              if (i + param_cnt >= parts.size()) {
                throw ::std::runtime_error("Missing argument to -s option.");
              }
              option.cxx_standard =
                  ::std::stoul(::std::string(parts[i + param_cnt]));
              break;
            case 'f':
              option.fast_io_required = true;
              break;
            case 'i':
              for (auto it = parts.begin() + i;
                   it != parts.end() && (*it)[0] != '-'; ++it) {
                option.header_files.emplace_back(*it);
              }
              break;
            default:
              throw ::std::runtime_error("Parsing failed.");
              break;
            }
          }
          i += param_cnt;
        }
      } else {
        throw ::std::runtime_error("Parsing failed.");
      }
    }
  }

  void init() {
    ::std::system("mkdir include");
    ::std::system("mkdir src");
    ::std::system("git init");
  }

  void create_header_files() {
    for (auto const &name : option.header_files) {
      ::fast_io::print(::fast_io::obuf_file("include/" + name + ".hpp"),
                       "#pragma once\n");
    }
  }

  void create_cmake_lists() {
    auto const content = ::std::format(
        R"(cmake_minimum_required(VERSION 3.20.0)

project(quick_start)

set(CMAKE_CXX_STANDARD {})
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${{CMAKE_CXX_FLAGS}} -stdlib=libc++")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB SRC_FILES CONFIGURE_DEPENDS ${{CMAKE_SOURCE_DIR}}/src/*.cpp)

add_executable(quick_start ${{SRC_FILES}})

target_include_directories(quick_start PRIVATE
    {}
    ${{CMAKE_SOURCE_DIR}}/include
))",
        option.cxx_standard,
        option.fast_io_required ? "${CMAKE_SOURCE_DIR}/fast_io/include" : "");

    ::fast_io::print(::fast_io::obuf_file("CMakeLists.txt"), content);
  }

  void create_cmake_presets() {
    auto const content = ::std::format(
        R"({{
    "version": 8,
    "configurePresets": [
        {{
            "name": "{}",
            "displayName": "Configure preset using toolchain file",
            "description": "Sets Ninja generator, build and install directory",
            "generator": "Ninja",
            "binaryDir": "${{sourceDir}}/build",
            "cacheVariables": {{
                "CMAKE_BUILD_TYPE": "Debug",
                "CMAKE_TOOLCHAIN_FILE": "",
                "CMAKE_INSTALL_PREFIX": "${{sourceDir}}/install",
                "CMAKE_C_COMPILER": "clang",
                "CMAKE_CXX_COMPILER": "clang++"
            }}
        }}
    ]
}})",
        option.project_name);

    ::fast_io::print(::fast_io::obuf_file("CMakePresets.json"), content);
  }

  void prepare_fast_io() {
    ::std::system(
        "git submodule add -b next https://github.com/cppfastio/fast_io.git");
  }

  void create_clang_format() {
    ::fast_io::print(::fast_io::obuf_file(".clang-format"),
                     R"(BasedOnStyle: LLVM
SortIncludes: false)");
  }

  void create_gitignore() {
    ::fast_io::print(::fast_io::obuf_file(".gitignore"), R"(.vscode/
.cache/
build/)");
  }

  void create_vs_code() {
    ::std::system("mkdir .vscode");

    ::fast_io::print(::fast_io::obuf_file(".vscode/launch.json"), R"({
    "version": "0.2.0",
    "configurations": [
        {
            "type": "lldb",
            "request": "launch",
            "name": "Debug",
            "program": "${command:cmake.launchTargetPath}",
            "args": [],
            "cwd": "${workspaceFolder}"
        }
    ]
})");

    ::fast_io::print(::fast_io::obuf_file(".vscode/settings.json"), R"({
    "cmake.debugConfig": {
        "args": [
            // "${env:HOME}"
        ],
    },
})");
  }

  struct {
    ::std::string project_name;
    unsigned int cxx_standard = 23;
    bool fast_io_required = false;
    ::std::vector<::std::string> header_files;
  } option;

  ::std::array<unsigned int, 4> possible_standards{11, 17, 20, 23};
};
} // namespace evqovv