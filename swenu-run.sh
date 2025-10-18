#!/bin/sh

IFS=:
exec $(
	for dir in $PATH; do
		for file in "$dir"/*; do
			[ -f "$file" ] && [ -x "$file" ] && printf '%s\n' "${file##*/}"
		done
	done | sort -u | ./swenu -a)
