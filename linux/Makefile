#   Basic info

#   Only "debug" and "release" are available.
ifeq "$(BUILD_TYPE)" ""
    BUILD_TYPE=debug
endif

#   Version.
VERSION_MAJOR=2
VERSION_MINOR=0
VERSION_RELEASE=1
VERSION_BUILD=4

#   According to the target file type, the suffix of TARGET_NAME should be
#   Executable file : (no suffix)
#   Static library  : .a
#   Dynamic library : .so
TARGET_NAME=libreceiver.a
CATEGORY_NAME=network

#   Only "c" and "cpp" are available.
TARGET_LANGUAGE=c

DEP_ALSRC=
DEP_OPENSRC=

TARGET_CFLAGS=-pthread
TARGET_LFLAGS=-lpthread -ldl

#
#   Path
#
ifeq "$(REPOS_ROOT)" ""
    $(error Please set the repository root path. export REPOS_ROOT=/xxx/repos)
endif

include $(REPOS_ROOT)/makefile/source.mak
