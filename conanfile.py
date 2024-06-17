from os.path import join

from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy

class Groove(ConanFile):
    name = 'groove'
    version = '0.0.1'
    license = 'BSD-3-Clause, AGPL-3.0-or-later'
    url = 'https://github.com/msfrank/groove'
    description = ''

    settings = 'os', 'compiler', 'build_type', 'arch'
    options = {'shared': [True, False], 'compiler.cppstd': ['17', '20'], 'build_type': ['Debug', 'Release']}
    default_options = {'shared': True, 'compiler.cppstd': '20', 'build_type': 'Debug'}

    exports_sources = (
        'CMakeLists.txt',
        'bin/*',
        'cmake/*',
        'lib/*',
        )

    def requirements(self):
        self.requires('tempo/0.0.1')
        # requirements from timbre
        self.requires('absl/20230802.1@timbre')
        self.requires('antlr/4.9.3@timbre')
        self.requires('apachearrow/16.0.0@timbre')
        self.requires('boost/1.84.0@timbre')
        self.requires('fmt/9.1.0@timbre')
        self.requires('flatbuffers/23.5.26@timbre')
        self.requires('grpc/1.62.0@timbre')
        self.requires('gtest/1.14.0@timbre')
        self.requires('icu/74.1@timbre')
        self.requires('openssl/3.2.0@timbre')
        self.requires('protobuf/25.3@timbre')
        self.requires('rocksdb/8.5.3@timbre')
        self.requires('uv/1.44.1@timbre')

    def validate(self):
        check_min_cppstd(self, "20")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        antlr = self.dependencies['antlr'].buildenv_info.vars(self)
        flatbuffers = self.dependencies['flatbuffers'].buildenv_info.vars(self)
        grpc = self.dependencies['grpc'].buildenv_info.vars(self)
        protobuf = self.dependencies['protobuf'].buildenv_info.vars(self)

        tc = CMakeToolchain(self)
        tc.variables['GROOVE_PACKAGE_VERSION'] = self.version
        tc.variables['ANTLR_TOOL_JAR'] = antlr.get('ANTLR_TOOL_JAR')
        tc.variables['FLATBUFFERS_FLATC'] = flatbuffers.get('FLATBUFFERS_FLATC')
        tc.variables['GRPC_CPP_PLUGIN'] = grpc.get('GRPC_CPP_PLUGIN')
        tc.variables['GRPC_INCLUDE_GENS_DIR'] = join(self.dependencies['grpc'].cpp_info.includedirs[0], 'gens')
        tc.variables['PROTOBUF_PROTOC'] = protobuf.get('PROTOBUF_PROTOC')
        tc.generate()

        deps = CMakeDeps(self)
        deps.set_property("openssl::crypto", "cmake_target_name", "OpenSSL::Crypto")
        deps.set_property("openssl::ssl", "cmake_target_name", "OpenSSL::SSL")
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "none")
        self.cpp_info.builddirs.append(join("lib", "cmake", "groove"))
