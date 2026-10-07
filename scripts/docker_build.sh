#!/usr/bin/env bash
# Docker (Ubuntu 20.04 arm64) 上でワークスペースをビルドしてコンパイル確認
set -eu
ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
docker build --platform linux/arm64 -t g1-lab-build "${ROOT_DIR}/docker"
docker run --rm --platform linux/arm64 -v "${ROOT_DIR}:/ws" g1-lab-build \
  bash -c "mkdir -p build-docker && cd build-docker && cmake .. && make -j\$(nproc) && ls bin"
