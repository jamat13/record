#! /bin/sh
echo "const char *gitrev = \"`git log |grep commit |sed s/commit\ // |head -c7`\";" > version.h
