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

ENV PROTOBUF_QML_DIR=/usr/src/protobuf-qml
ADD . $PROTOBUF_QML_DIR

RUN ${PROTOBUF_QML_DIR}/tools/build_dependencies.py -j4 --shared && \
    ${PROTOBUF_QML_DIR}/tools/bootstrap.py --qt5dir=/opt/qt55/lib/cmake \
    cp ${PROTOBUF_QML_DIR}/build/deps/Release/bin/* /usr/bin/ && \
    cp ${PROTOBUF_QML_DIR}/build/deps/Release/lib/* /usr/lib/ && \
    cp -r ${PROTOBUF_QML_DIR}/build/deps/Release/bin/* /usr/bin/ && \
    ninja -C ${PROTOBUF_QML_DIR}/out/Release && \
    cp ${PROTOBUF_QML_DIR}/out/Release/bin/protoc-gen-qml /usr/bin/

ENTRYPOINT ["protoc"]
