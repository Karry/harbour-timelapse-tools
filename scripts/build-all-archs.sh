#!/bin/bash

set -xe
PATH=$PATH:~/SailfishOS/bin/

export OS_VERSION=5.1.0.11

sfdk config "no-fix-version"
for ARCHITECTURE in aarch64 armv7hl i486; do
    sfdk config "target=SailfishOS-${OS_VERSION}-${ARCHITECTURE}"
    # sudo modprobe -r kvm_intel
    sfdk build --enable-debug
done
