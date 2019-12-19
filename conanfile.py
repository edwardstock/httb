import os
from conans import ConanFile, CMake, tools


def get_version():
    with open(os.path.join(os.path.dirname(__file__), 'version'), 'r') as f:
        content = f.read()
        try:
            content = content.decode()
        except AttributeError:
            pass
        return content.strip()


class HttbConan(ConanFile):
    name = "httb"
    version = get_version()
    license = "MIT"
    author = "Eduard Maximovich edward.vstock@gmail.com"
    url = "https://github.com/edwardstock/httb"
    description = "Lightweight C++ HTTP Client based on Boost.Beast"
    topics = ("boost", "http", "cpp-http", "cpp-http-client", "http-client", "boost-http", "boost-beast")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {
        "shared": False,
        "OpenSSL:shared": False,
        "boost:shared": False,
    }
    exports = (
        "version",
    )
    exports_sources = (
        "cfg/*",
        "modules/*",
        "include/*",
        "tests/*",
        "src/*",
        "CMakeLists.txt",
        "conanfile.py",
        "conanfile.txt",
        "LICENSE",
        "README.md"
    )
    generators = "cmake"
    default_user = "scatter"
    default_channel = "latest"

    requires = (
        "OpenSSL/1.1.1b@conan/stable",
        "toolbox/3.1.1@edwardstock/latest",
        "boost/1.70.0@conan/stable",
    )
    build_requires = (
        "gtest/1.8.1@bincrafters/stable"
    )

    def source(self):
        if "CONAN_LOCAL" not in os.environ:
            self.run("rm -rf *")
            self.run("git clone https://github.com/edwardstock/httb.git .")

    def configure(self):
        if self.settings.compiler == "Visual Studio":
            del self.settings.compiler.runtime

    def build(self):
        cmake = CMake(self)
        opts = {
            'CMAKE_BUILD_TYPE': self.settings.build_type,
            'ENABLE_TEST': "Off",
            'ENABLE_SHARED': 'Off'
        }
        if self.options.shared:
            opts['ENABLE_SHARED'] = 'On'

        cmake.configure(defs=opts)
        cmake.build()

    def package(self):
        self.copy("*", dst="include", src="include", keep_path=True)
        dir_types = ['bin', 'lib', 'Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel']
        file_types = ['lib', 'dll', 'dll.a', 'a', 'so', 'exp', 'pdb', 'ilk', 'dylib']

        for dirname in dir_types:
            for ftype in file_types:
                self.copy("*." + ftype, src=dirname, dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = self.collect_libs()

    def test(self):
        cmake = CMake(self)
        cmake.configure(defs={'ENABLE_TEST': 'On'})
        cmake.build(target="httb-test")
        if self.settings.compiler == "Visual Studio":
            self.run("bin/httb-test.exe")
        else:
            self.run("bin/httb-test")
