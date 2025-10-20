#!/bin/sh

IFS=:
for dir in $PATH; do
	[ -e ${dir} ]  && find ${dir} -executable -printf '%f\n'
done | sort -u
