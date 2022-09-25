#!/bin/bash

NAME=$(cat release.spec | grep Name | awk '{print $2}')
VERSION=$(cat release.spec | grep Version | awk '{print $2}')
RELEASE=$(cat release.spec | grep Release | awk '{print $2}')

echo "release $NAME-$VERSION-$RELEASE"

set -e
set -x

rm -fr ~/rpmbuild
rpmdev-setuptree
tree ~/rpmbuild

mkdir $NAME-$VERSION

cp -r echoS $NAME-$VERSION 
cp -r chat  $NAME-$VERSION 
cp -r ttcp  $NAME-$VERSION 
cp -r bancs  $NAME-$VERSION 

cp ttcp.conf $NAME-$VERSION
cp bancs/bancs.data $NAME-$VERSION

cp Makefile $NAME-$VERSION

tar -cf $NAME-$VERSION.tar.gz $NAME-$VERSION

rm -fr $NAME-$VERSION
mv $NAME-$VERSION.tar.gz ~/rpmbuild/SOURCES/

rpmbuild -bs release.spec
rpmbuild -bb release.spec

tree ~/rpmbuild
