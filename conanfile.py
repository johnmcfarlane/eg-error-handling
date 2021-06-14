from conans import ConanFile, CMake

class WssConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    requires = "fmt/7.1.3"
    generators = "cmake", "gcc", "txt"
