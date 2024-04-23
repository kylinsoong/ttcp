#!/bin/bash

# Extract metadata from release.spec
NAME=ttcp
VERSION=1.14
RELEASE=1

# Display release information
echo "Release: $NAME-$VERSION-$RELEASE"

set -e
set -x

# Clean up any existing Debian build directory
rm -fr ~/debianbuild
mkdir -p ~/debianbuild/$NAME-$VERSION

# Prepare package directory
cp -r echoS ~/debianbuild/$NAME-$VERSION 
cp -r chat  ~/debianbuild/$NAME-$VERSION 
cp -r ttcp  ~/debianbuild/$NAME-$VERSION 
cp -r bancs  ~/debianbuild/$NAME-$VERSION 

cp ttcp.conf ~/debianbuild/$NAME-$VERSION
cp bancs/bancs.data ~/debianbuild/$NAME-$VERSION

cp Makefile ~/debianbuild/$NAME-$VERSION

cp -r debian ~/debianbuild/$NAME-$VERSION

# Build the Debian package
cd ~/debianbuild/$NAME-$VERSION
debuild -us -uc

# Display the contents of the Debian package
dpkg-deb -c ../$NAME_$VERSION-$RELEASE.deb
