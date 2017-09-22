#!/bin/sh

do_abort()
{
    echo "autogen.sh : Fatal Error : $@." 1>&2
    exit 1
}

#Check for libtool
command -v libtoolize >/dev/null 2>&1 ||
    command -v libtool >/dev/null 2>&1 ||
        do_abort "Could not find libtool."

#Check for autoreconf
command -v autoreconf >/dev/null 2>&1 ||
    do_abort "Could not find autoreconf."

#Generate configure script
autoreconf --install --force --verbose  ||
    do_abort "autoreconf exited with status $?"
