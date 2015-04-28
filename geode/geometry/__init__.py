"""geometry module"""

from __future__ import (division,absolute_import)

from geode import *
from numpy import asarray

BoxTrees = {2:BoxTree2d,3:BoxTree3d}
def BoxTree(X,leaf_size):
  X = asarray(X)
  return BoxTrees[X.shape[1]](X,leaf_size)

ParticleTrees = {2:ParticleTree2d,3:ParticleTree3d}
def ParticleTree(X,leaf_size=1):
  X = asarray(X)
  return ParticleTrees[X.shape[1]](X,leaf_size)

SimplexTrees = {(2,1):SegmentTree2d,(3,1):SegmentTree3d,(2,2):TriangleTree2d,(3,2):TriangleTree3d}
def SimplexTree(mesh,X,leaf_size=1):
  X = asarray(X)
  return SimplexTrees[X.shape[1],mesh.d](mesh,X,leaf_size)

Boxes = {1:Box1d,2:Box2d,3:Box3d}
def Box(min,max):
  try:
    d = len(min)
  except TypeError:
    d = len(max)
  return Boxes[d](min,max)

Spheres = {2:Sphere2d,3:Sphere3d}
def Sphere(center,radius):
  center = asarray(center)
  return Spheres[len(center)](center,radius)

Capsules = {2:Capsule2d,3:Capsule3d}
def Capsule(x0,x1,radius):
  try:
    d = len(x0)
  except TypeError:
    d = len(x1)
  return Capsules[d](x0,x1,radius)

empty_boxes = {1:empty_box_1d,2:empty_box_2d,3:empty_box_3d}
def empty_box(d):
  return empty_boxes[d]()

FrameImplicits = {2:FrameImplicit2d,3:FrameImplicit3d}
def FrameImplicit(frame,object):
  return FrameImplicits[object.d](frame,object)

surface_levelsets = {1:surface_levelset_c3d,2:surface_levelset_s3d}
def surface_levelset(particles,surface,max_distance=inf,compute_signs=True):
  return surface_levelsets[surface.d](particles,surface,max_distance,compute_signs)
