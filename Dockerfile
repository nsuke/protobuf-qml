FROM ubuntu:14.04
MAINTAINER Nobuaki Sukegawa <nsukeg@gmail.com>

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
      software-properties-common \
      python-software-properties
RUN add-apt-repository -y ppa:beineri/opt-qt551-trusty
RUN apt-get update && apt-get install -y \
      libgl1-mesa-dev \
      qt55base \
      qt55declarative \
      ninja-build \
      cmake \
      python \
      gcc \
      g++ \
      clang \
      git \
      python-setuptools \
      python-yaml \
      python-mako \
      golang \
      cmake \
      ninja-build \
      zlib1g-dev

ENV PROJECT_DIR=/opt/protobuf-qml
CMD mkdir -p $PROJECT_DIR
WORKDIR $PROJECT_DIR
