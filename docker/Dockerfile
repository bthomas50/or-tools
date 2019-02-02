FROM ubuntu:16.04

RUN apt-get update && apt-get install -y \
  curl \
  git \
  wget \
  pkg-config \
  build-essential \
  cmake \
  autoconf \
  libtool \
  zlib1g-dev \
  lsb-release

COPY . /opt/or-tools
WORKDIR /opt/or-tools

ARG ARTIFACTORY_API_KEY

RUN make third_party
RUN make cc

RUN docker/scripts/archive.sh
