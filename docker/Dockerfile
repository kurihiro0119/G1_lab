# G1 内部PC相当 (Ubuntu 20.04 / aarch64) のビルド確認用環境
# 実機に繋がずに Mac 上でコンパイルが通るか確認する目的 (DDS 通信の実行は不可)
FROM --platform=linux/arm64 ubuntu:20.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      cmake g++ build-essential libyaml-cpp-dev libeigen3-dev libboost-all-dev libfmt-dev \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /ws
