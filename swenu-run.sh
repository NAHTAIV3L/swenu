#!/bin/sh

IFS=:
exec $(
	for dir in $PATH; do
		[ -e ${dir} ]  && find ${dir} -executable -printf '%f\n'
	done | sort -u | ./swenu -cal)
