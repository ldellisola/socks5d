FROM ubuntu:20.04

RUN DEBIAN_FRONTEND="noninteractive" apt-get update && apt-get -y install tzdata

RUN apt-get update \
  && apt-get install -y build-essential \
      gcc \
      g++ \
      gdb \
      clang \
      make \
      ninja-build \
      cmake \
      autoconf \
      automake \
      locales-all \
      dos2unix \
      rsync \
      tar \
      python \
      python-dev \
      check \
      netcat \
      curl \
      dovecot-pop3d \
  && apt-get clean

  ARG UID=1000
  RUN useradd -m -u ${UID} -s /bin/bash builder
  USER builder
