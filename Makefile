# moses Find symbol in shared libraries.
# Copyright (C) 2022  Mathias Schmitt
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

COMPILER := gcc
ERROR_FLAGS := -Wall \
		-Werror \
		-Wextra \
		-pedantic \
		-Wshadow \
		-Wdouble-promotion \
		-fno-common \
		-Wconversion

FORMAT_FLAGS := -Wformat=2 -Wundef
OPTIMIZATION_FLAGS := -O3
FLAGS := ${ERROR_FLAGS} ${FORMAT_FLAGS} ${OPTIMIZATION_FLAGS}

OUTPUT_DIR := out

PROG_NAME := moses
SRC := main.c \
       pipe.c
SOURCES := $(addprefix src/, ${SRC})

INCLUDES := includes

INSTALL_DIR := /usr/local/bin

.PHONY: all
all: moses

.PHONY: moses
moses: ${SOURCES}
	@mkdir -p ${OUTPUT_DIR}
	@${COMPILER} ${SOURCES} -I${INCLUDES} ${FLAGS} -o ${OUTPUT_DIR}/${PROG_NAME}

.PHONY: install
install: ${OUTPUT_DIR}/${PROG_NAME}
	@mkdir -p ${DESTDIR}${INSTALL_DIR}
	@cp $< ${DESTDIR}${INSTALL_DIR}/${PROG_NAME}

.PHONY: uninstall
uninstall: uninstall_man
	@rm -f ${DESTDIR}${INSTALL_DIR}/${PROG_NAME}

.PHONY: clean
clean:
	@rm -rf ${OUTPUT_DIR}

.PHONY: help
help:
	@echo "Use one of the following targets:"
	@echo "  help     Print this help message"
	@echo "  all      Build moses"
	@echo "  clean    Clean output from previous build"
	@echo "  install  Install moses on your system"
