lib_setup_args="use_python=0 use_libjpeg=0 use_libpng=0 use_openexr=0 use_openmesh=1"
echo "Using: $lib_setup_args"
scons -j7 prefix='#build/$arch/$type' arch='nocona' install=0 $lib_setup_args openmesh_libpath='#/../OpenMesh-2.0/build/Build/lib/OpenMesh/' openmesh_include='#/../OpenMesh-2.0/src/' type=debug 
scons -j7 prefix='#build/$arch/$type' arch='nocona' install=0 $lib_setup_args openmesh_libpath='#/../OpenMesh-2.0/build/Build/lib/OpenMesh/' openmesh_include='#/../OpenMesh-2.0/src/'
