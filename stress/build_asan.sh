#!/usr/bin/env bash
# Mirror of stress/build_asan.ps1 for POSIX hosts: ASan+UBSan build of the dllai stress harness.
set -e
cd "$(dirname "$0")"
"${CXX:-clang++}" -std=c++23 -fsanitize=address,undefined -g -O1 -I../dllai \
  ../dllai/ai.cpp ../dllai/genmove.cpp ../dllai/tetris_gem.cpp ../dllai/dllai.cpp \
  ub_stress.cpp -o ub_stress
./ub_stress
