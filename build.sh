lib_setup_args="use_python=0 use_libjpeg=0 use_libpng=0 use_openexr=0 use_openmesh=1 use_gmp=1"
echo "Using: $lib_setup_args"
if [ "$1" = "clean" ] || [ "$1" = "distclean" ]; then
	scons -c --config=force $lib_setup_args
else
  for type in debug release; do
    scons -j7 prefix='#build/$arch/$type' arch='nocona' install=0 $lib_setup_args \
      openmesh_libpath='#/../OpenMesh-2.0/build/Build/lib/OpenMesh' openmesh_include='#/../OpenMesh-2.0/src' \
      gmp_linkflags='../gmp/.libs/libgmp.dylib' gmp_include='#/../gmp' gmp_libpath='' gmp_publiclibs='' type=$type
  done
fi
