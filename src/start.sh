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
				gdb 

ulimit -c unlimited
cd /tmp/otus/src
protoc-c --c_out=. deviceapps.proto
python2 setup.py test
