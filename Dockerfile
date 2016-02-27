FROM ubuntu:14.04
MAINTAINER Nobuaki Sukegawa <nsukeg@gmail.com>

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      software-properties-common \
      python-software-properties
RUN add-apt-repository -y ppa:beineri/opt-qt551-trusty

ENV PROTOBUF_QML_DIR=/usr/src/protobuf-qml
ADD . $PROTOBUF_QML_DIR

RUN buildDeps=" \
               clang \
               cmake \
               g++ \
               gcc \
               golang \
               ninja-build \
               python \
               python-mako \
               python-setuptools \
               python-yaml \
               zlib1g-dev \
               " \
    && apt-get update && apt-get install -y --no-install-recommends \
        $buildDeps \
        libgl1-mesa-dev \
        qt55base \
        qt55declarative \
    && ${PROTOBUF_QML_DIR}/tools/build_dependencies.py -j4 --shared --prefix=/usr && \
    ${PROTOBUF_QML_DIR}/tools/bootstrap.py --qt5dir /opt/qt55/lib/cmake -D /usr && \
    ninja -C ${PROTOBUF_QML_DIR}/out/Release && \
    mv ${PROTOBUF_QML_DIR}/out/Release/bin/protoc-gen-qml /usr/bin/ && \
    apt-get purge -y --auto-remove $buildDeps && \
    rm -rf ${PROTOBUF_QML_DIR} /var/lib/apt/lists/* /var/tmp/*

ENTRYPOINT ["protoc"]
