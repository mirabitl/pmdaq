#!/usr/bin/env bash
set -euo pipefail

LIBHV_VERSION="v1.3.4"
LIBHV_REPOSITORY="https://github.com/ithewei/libhv.git"
INSTALL_PREFIX="/usr/local"

if ! command -v apt-get >/dev/null 2>&1; then
    printf 'Erreur : ce script nécessite Debian ou Ubuntu (apt-get introuvable).\n' >&2
    exit 1
fi

if [[ "${EUID}" -eq 0 ]]; then
    SUDO=()
elif command -v sudo >/dev/null 2>&1; then
    SUDO=(sudo)
else
    printf 'Erreur : exécutez ce script en root ou installez sudo.\n' >&2
    exit 1
fi

"${SUDO[@]}" apt-get update
"${SUDO[@]}" apt-get install -y --no-install-recommends \
    build-essential ca-certificates cmake git

WORK_DIR="$(mktemp -d)"
trap 'rm -rf "$WORK_DIR"' EXIT

git clone --depth 1 --branch "$LIBHV_VERSION" "$LIBHV_REPOSITORY" "$WORK_DIR/libhv"

cmake -S "$WORK_DIR/libhv" -B "$WORK_DIR/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DBUILD_SHARED=ON \
    -DBUILD_STATIC=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_UNITTEST=OFF
cmake --build "$WORK_DIR/build" --parallel "$(nproc)"
"${SUDO[@]}" cmake --install "$WORK_DIR/build"

if command -v ldconfig >/dev/null 2>&1; then
    "${SUDO[@]}" ldconfig
fi

printf 'libhv %s a été installé dans %s.\n' "$LIBHV_VERSION" "$INSTALL_PREFIX"
