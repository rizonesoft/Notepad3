#!/bin/bash
#
# Copyright 2016 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
################################################################################

# Run this script to generate the configure script and other files that will
# be included in the distribution.  These files are not checked in because they
# are automatically generated.

set -e

if [ ! -z "$@" ]; then
  for argument in "$@"; do
    case $argument in
	  # make curl silent
      "-s")
        curlopts="-s"
        ;;
    esac
  done
fi


# Check that we're being run from the right directory.
if test ! -f compact_enc_det/compact_enc_det.h; then
  cat >&2 << __EOF__
Could not find source code.  Make sure you are running this script from the
root of the distribution tree.
__EOF__
  exit 1
fi

# Check that gtest is present. It is used to build unit test suite.
if test ! -e gtest; then
  if test -z $(which curl); then
    echo "curl cannot be found. Please install it to build the package."
    exit 1
  fi

  echo "Google Test not present.  Fetching from the web..."
  curl $curlopts -L -O https://github.com/google/googletest/archive/master.zip
  unzip -q master.zip
  rm master.zip
  mv googletest-master gtest
fi

if test -z $(which cmake); then
  echo "CMake cannot be found. Please install it to build the package."
  exit 1
fi

# Build gtest
(cd gtest && cmake . && make)

# Build the main package
cmake . && make

set -ex

exit 0
