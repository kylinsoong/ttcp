#!/bin/bash

GROUP="cloudadc"
NAME=$(cat release.spec | grep Name | awk '{print $2}')
VERSION=$(cat release.spec | grep Version | awk '{print $2}')
RELEASE=$(cat release.spec | grep Release | awk '{print $2}')
REPOS=$GROUP/$NAME:$VERSION-$RELEASE
TMPDIR="target"
PLATFORM="x86_64"

echo "build $REPOS"

if [[ -d $TMPDIR ]]
then
  rm -fr $TMPDIR
fi

set -e
set -x

./release.sh

mkdir $TMPDIR
cp ~/rpmbuild/RPMS/$PLATFORM/* $TMPDIR

docker build -f ttcp.dockerfile -t cloudadc/ttcp:$VERSION-$RELEASE --build-arg TTCP_NAME=$NAME --build-arg TTCP_VERSION=$VERSION-$RELEASE --build-arg TTCP_PLATFORM=$PLATFORM .
docker build -f ttcp.dockerfile.cli -t cloudadc/ttcp:$VERSION-$RELEASE-cli --build-arg TTCP_NAME=$NAME --build-arg TTCP_VERSION=$VERSION-$RELEASE --build-arg TTCP_PLATFORM=$PLATFORM .
docker build -f ttcp.dockerfile.udp -t cloudadc/ttcp:$VERSION-$RELEASE-udp --build-arg TTCP_NAME=$NAME --build-arg TTCP_VERSION=$VERSION-$RELEASE --build-arg TTCP_PLATFORM=$PLATFORM .

docker images | grep $GROUP

rm -fr $TMPDIR

