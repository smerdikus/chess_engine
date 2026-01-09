#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
BIN_PATH="${BUILD_DIR}/sfml_chess"

if [[ ! -x "${BIN_PATH}" ]]; then
  cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}"
  cmake --build "${BUILD_DIR}"
fi

cd "${BUILD_DIR}"
exec "./sfml_chess"
