#!/bin/bash
# TODO: Test config for OSX
# TODO: Determine appropriate architecture (i.g. "args+=(arch=nocona)")

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
if [ -z "$2" ]; then
  ARCH=x86-64
else
  ARCH=$2
fi
echo Architecture is set to $ARCH

if [ "$1" = "clean" ] || [ "$1" = "debug" ] || [ "$1" = "release" ]; then
  declare -a args=()
  SYSTEM_NAME=`uname -s`
  args+=(sse=0)
  args+=(arch=$ARCH)
  if [ "$SYSTEM_NAME" = "Darwin" ]; then
    # OSX specific config:
    args+=(use_openmesh=0 openmesh_libpath=#/../OpenMesh-2.0/build/Build/lib/OpenMesh openmesh_publiclibs='OpenMeshCore,OpenMeshTools' openmesh_include=#/../OpenMesh-2.0/src)
    echo "" # Can't have an empty if block
  elif echo "$SYSTEM_NAME" | grep -q "MINGW64_NT"; then
    # Windows specific config:
    args+=(libs_extra=psapi)
    args+=(use_openmesh=0 Werror=0 openmesh_libpath=#/../OpenMesh-2.0/build/Build/lib openmesh_publiclibs='OpenMeshCore,OpenMeshTools' openmesh_include=#/../OpenMesh-2.0/src)
  elif echo "$SYSTEM_NAME" | grep -q "MSYS"; then
    echo "ERROR: the MSYS shell is not supported. Please use the MinGW-w64 Win64 Shell instead."
    exit 1
  elif echo "$SYSTEM_NAME" | grep -q "Linux"; then
    args+=(use_openmesh=0 openmesh_libpath=#/../OpenMesh-2.0/build/Build/lib/OpenMesh openmesh_publiclibs='OpenMeshCore,OpenMeshTools' openmesh_include=#/../OpenMesh-2.0/src)
  else
    echo "ERROR: Unrecognized or unsupported platform: $SYSTEM_NAME!"
    exit 1
  fi

  args+=(use_python=0 use_libjpeg=0 use_libpng=0 use_openexr=0 use_boost=0)
  args+=(shared=1 install=0)
  args+=(use_gmp=1 gmp_libpath=#/../mpir/.libs/ gmp_include=#/../mpir/)

  if [ "$1" = "debug" ]; then
    echo "Building geode for debug only, using $ARCH architecture"
    types=debug
  elif [ "$1" = "release" ]; then
    echo "Building geode for release only, using $ARCH architecture"
    types=release
  else
    echo "Build type was not specified, building geode for both debug and release, using $ARCH architecture"
    types="debug release"
  fi

  if [ "$ARCH" = "arm" ]; then
    cxx="cxx=/usr/bin/arm-linux-gnueabihf-g++"
  fi

  for t in $types; do
    scons_args="--config=force -j7 prefix=#build/\$arch/\$type type=$t $cxx ${args[@]}"
    if [ "$1" = "clean" ]; then
      echo "Cleaning $t with: $scons_args"
      (cd $DIR && scons -c $scons_args) || exit 1
    else
      echo "Building $t with: $scons_args"
      (cd $DIR && scons $scons_args) || exit 1
    fi
    echo ""
  done
else
  echo 'Missing or unrecognized metaBuild argument: '\'$1\'
  exit 1
fi
