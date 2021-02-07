#!/bin/bash
set -xe

sed -i 's/enabled=0/enabled=1/' /etc/yum.repos.d/CentOS-Linux-PowerTools.repo

yum install -y  gcc \
				make \
				protobuf \
				protobuf-c \
				protobuf-c-compiler \
				protobuf-c-devel \
				python2-devel \
				python2-setuptools \
				zlib-devel \
				python2-pip \
				gdb 

pip2 install protobuf

ulimit -c unlimited
cd /tmp/otus/src
protoc-c --c_out=. deviceapps.proto
protoc --python_out=tests/ deviceapps.proto
python2 setup.py test
