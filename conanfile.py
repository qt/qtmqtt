#############################################################################
##
## Copyright (C) 2021 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the release tools of the Qt Toolkit.
##
## $QT_BEGIN_LICENSE:GPL-EXCEPT$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 3 as published by the Free Software
## Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

from conans import ConanFile, tools, CMake
import os


class QtConanError(Exception):
    pass


class QtMQTT(ConanFile):
    name = "qtmqtt"
    version = "6.1.0"
    license = "GPL-3.0+, Commercial Qt License Agreement"
    author = "The Qt Company <https://www.qt.io/contact-us>"
    url = "https://code.qt.io/cgit/qt/qtmqtt.git"
    description = "Qt MQTT is a machine-to-machine (M2M) protocol utilizing the publish-and-subscribe paradigm."
    topics = ("qt", "qt6", "MQTT", "M2M")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False],
               "qt6": "ANY"}  # this is needed to model unique package_id for the Add-on build per used Qt6 version
    default_options = {"shared": False,
                       "qt6": None}
    exports_sources = "*", "!conan*.*"

    def source(self):
        # sources are installed next to recipe, no need to clone etc. sources here
        pass

    def _get_cmake_prefix_path(self):
        # 'QT_PATH' provided as env variable in profile file which is part of the Qt essential binary
        # package(s). Installed under .conan/profiles
        cmake_prefix_path = os.environ.get("QT_PATH")
        if not cmake_prefix_path:
            raise QtConanError("'QT_PATH' not defined! The 'QT_PATH' needs to point to Qt installation directory.")
        print(f"CMAKE_PREFIX_PATH for '{self.name}/{self.version}' build is: {cmake_prefix_path}")
        return cmake_prefix_path

    def _read_env(self, key):
        value = os.environ.get(key)
        if not value:
            raise QtConanError(f"{self.settings.os} build specified but '{key}' was not defined?")
        return value

    def _get_qtcmake(self):
        qt_install_path = self._get_cmake_prefix_path()
        ext = ".bat" if tools.os_info.is_windows else ""
        qtcmake = os.path.abspath(os.path.join(qt_install_path, "bin", "qt-cmake" + ext))
        if not os.path.exists(qtcmake):
            raise QtConanError(f"Unable to locate {qtcmake} from 'QT_PATH': {qt_install_path}")
        return qtcmake

    def _get_cmake_tool(self):
        cmake = CMake(self, cmake_program=self._get_qtcmake())
        cmake.verbose = True

        # Qt modules need to be 'installed'.
        # We need to direct the 'make install' to some directory under Conan cache,
        # place it under the current build directory which is also under the Conan cache.
        # Note, the actual 'make install' is called in "package()".
        install_dir = os.path.join(os.getcwd(), "_install_tmp")
        cmake.definitions["CMAKE_INSTALL_PREFIX"] = install_dir

        cmake_toolchain_file = os.environ.get("CMAKE_TOOLCHAIN_FILE")
        if cmake_toolchain_file:
            cmake.definitions["CMAKE_TOOLCHAIN_FILE"] = cmake_toolchain_file

        if self.settings.os == "Android":
            cmake.definitions["ANDROID_SDK_ROOT"] = self._read_env('ANDROID_SDK_ROOT')
            cmake.definitions["ANDROID_NDK_ROOT"] = self._read_env('ANDROID_NDK_ROOT')

        if self.settings.os == "iOS":
            # Instead of Conan's auto-added 'Darwin', explicitly pass 'iOS'.
            cmake.definitions["CMAKE_SYSTEM_NAME"] = "iOS"

            # Remove the explicit sysroot, let CMake detect the sysroots, to ensure
            # that multi-arch builds work.
            del cmake.definitions["CMAKE_OSX_SYSROOT"]

            # Remove the conan provided architecture, instead rely on the architectures set
            # by the Qt toolchain file, which with official Qt packages most likely means
            # multi-arch iOS.
            del cmake.definitions["CMAKE_OSX_ARCHITECTURES"]

        return cmake

    def build(self):
        cmake = self._get_cmake_tool()
        self.run('%s "%s" %s' % (self._get_qtcmake(), self.source_folder, cmake.command_line))
        self.run('cmake --build . %s' % cmake.build_config)

    def package(self):
        install_dir = os.path.join(os.getcwd(), "_install_tmp")  # see 'CMAKE_INSTALL_PREFIX' above
        self.run('cmake --build . --target install')
        self.copy("*", src=install_dir, dst=".")

    def package_info(self):
        self.cpp_info.libs = ["Qt6Mqtt"]  # used for the actual library filename, Ordered list with the library names

    def deploy(self):
        self.copy("*")  # copy from current package
        self.copy_deps("*")  # copy from dependencies

