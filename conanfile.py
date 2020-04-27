from conans import ConanFile, CMake

class WssConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    requires = "fmt/6.0.0@bincrafters/stable"
    generators = "cmake", "gcc", "txt"