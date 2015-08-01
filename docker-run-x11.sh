#!/bin/sh
# This script runs docker container with host X11 access.
set -e
CUR=$(cd $(dirname $BASH_SOURCE) && pwd)
IMAGE_NAME=protobuf-qml/x11

docker build -t $IMAGE_NAME -f $CUR/Dockerfile $CUR

XSOCK=/tmp/.X11-unix
XAUTH=/tmp/.docker.xauth
xauth nlist :0 | sed -e 's/^..../ffff/' | xauth -f $XAUTH nmerge -
docker run -it -v $XSOCK:$XSOCK -v $XAUTH:$XAUTH -e XAUTHORITY=$XAUTH -e DISPLAY=$DISPLAY $* $IMAGE_NAME
