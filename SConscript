from os.path import abspath
build_root = abspath('../../../..')
print build_root

Import('env library external windows clang toplevel')

toplevel('othercore','.')

external('openmesh',libpath=[build_root+'/OpenMesh-2.0/build/Build/lib/OpenMesh'],cpppath=[build_root+'/OpenMesh-2.0/src', build_root+'/gmm-4.2/include'],flags=['USE_OPENMESH'],libs=['OpenMeshCore','OpenMeshTools'],requires=['boost_link'])
env = env.Clone(use_libpng=1,use_libjpeg=1,use_openexr=0,use_openmesh=1,use_gmp=1)
# Minimal dependencies:
# env = env.Clone(use_libpng=0,use_libjpeg=0,use_openexr=0,use_openmesh=0,use_python=0)
library(env,'other_core')