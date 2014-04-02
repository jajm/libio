#!/bin/sh

set -e

run () {
	echo "Running ${1}..." && $@
}

run aclocal
run libtoolize -i
run autoconf

echo ""
echo "Project bootstrapped successfully."
echo "Now you can run the following commands:"
echo ""
echo "\t./configure"
echo "\tmake"
echo ""
