#!/bin/sh
./swenu-path.sh | ./swenu "$@" | ${SHELL:-"/bin/sh"} &
