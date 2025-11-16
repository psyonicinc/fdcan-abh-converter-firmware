#!/bin/bash

X=$(git rev-parse --short HEAD)
Y=$(git tag --points-at HEAD)

echo "-------------"
echo "git rev (short):"
echo "$X"
echo "-------------"

FPATH="../Core/Inc/version.h"
echo "$FPATH"

cat > "$FPATH" << EOF
#ifndef VERSION_H
#define VERSION_H

static const char firmware_version[] = "$X";
static const char firmware_tag[] = "$Y";

#endif

EOF
