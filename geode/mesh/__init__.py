from __future__ import absolute_import

from numpy import *
from geode import *
import struct
import platform

# this is getting seriously ugly, but we do need to be able to test isinstance on
# these somehow.
if platform.system()=='Windows':
  from geode.geode_all import TriangleTopology as CTriangleTopology
else:
  from geode.geode_wrap import TriangleTopology as CTriangleTopology

def TriangleTopology(soup=TriangleSoup(empty((0,3),int32))):
  if isinstance(soup,geode_wrap.TriangleTopology):
    return soup
  elif not isinstance(soup,TriangleSoup):
    soup = TriangleSoup(soup)
  return geode_wrap.TriangleTopology(soup)

def linear_subdivide(mesh,X,steps=1):
  for _ in xrange(steps):
    subdivide = TriangleSubdivision(mesh)
    mesh = subdivide.fine_mesh
    X = subdivide.linear_subdivide(X)
  return mesh,X

def loop_subdivide(mesh,X,steps=1,corners=zeros(0,dtype=int32)):
  for _ in xrange(steps):
    subdivide = TriangleSubdivision(mesh)
    subdivide.corners = corners
    mesh = subdivide.fine_mesh
    X = subdivide.loop_subdivide(X)
  return mesh,X

def read_obj(file):
  """Parse an obj file into a mesh and associated properties.
  Returns (mesh,props) where mesh is a PolygonSoup, and props is a dictionary containing some of X,normals,texcoord,material,face_normals,face_texcoords
  """

  # parse file
  material=None
  X,normals,texcoords=[],[],[]
  counts,vertices,face_normals,face_texcoords=[],[],[],[]
  file = open(file)
  while 1:
    line = file.readline()
    if not line:
      break
    line = line.rstrip('\r\n')
    if line.endswith('\\'):
      line = line[:-1] + file.readline()
    values=line.split()
    if not values: continue
    cmd=values[0]
    if cmd.startswith('#'):
      pass
    elif cmd=='v':
      X.append(values[1:4])
    elif cmd=='vn':
      normals.append(values[1:4])
    elif cmd == 'vt':
      texcoords.append(values[1:3])
    elif cmd in ('usemtl','usemat'):
      material=values[1]
    elif cmd=='mtllib':
      # Todo: handle reading the associated material file here
      pass
    elif cmd=='f':
      counts.append(len(values)-1)
      for v in values[1:]:
        w=v.split('/')
        vertices.append(w[0])
        if len(w)>=2 and w[1]:
          face_texcoords.append(w[1])
        if len(w)>=3 and w[2]:
          face_normals.append(w[2])

  # construct mesh
  counts=array(counts,dtype=int32)
  vertices=array(vertices,dtype=int32)-1
  mesh=PolygonSoup(counts,vertices)

  # add vertex properties
  props={}
  props['X']=array(X,dtype=float)
  props['normals']=array(normals,dtype=float)
  props['texcoords']=array(texcoords,dtype=float)

  # add extra face properties
  if face_normals:
    assert len(vertices)==len(face_normals)
    props['face_normals']=array(face_normals,dtype=int)-1
  if face_texcoords:
    assert len(vertices)==len(face_texcoords)
    props['face_texcoords']=array(face_texcoords,dtype=int)-1

  # done
  return mesh,props

def write_obj(file,mesh,X=None):

  assert X is not None or isinstance(mesh, MutableTriangleTopology)

  if X is None:
    X = mesh.vertex_field(vertex_position_id)
    mesh = mesh.face_soup()[0]

  """Write a simple obj file.
  For now, only mesh and positions are supported
  """

  output=open(file,"w")

  # write format
  print>>output,'''\
# simple obj file format:
#   # vertex at coordinates (x,y,z)
#   v x y z
#   # triangle [quad] with vertices a,b,c[,d]
#   f a b c [d]
#   # vertices are indexed starting from 1
'''

  # write vertices
  for x in X:
    output.write("v %.5g %.5g %.5g\n"%tuple(x))

  # write polygons
  if isinstance(mesh,TriangleSoup):
    triangles=mesh.elements+1
    for t in triangles:
      print>>output,'f',' '.join(map(str,t))
  elif isinstance(mesh,PolygonSoup):
    vertices=mesh.vertices+1
    offset=0
    for c in mesh.counts:
      print>>output,'f',' '.join(map(str,vertices[offset:offset+c]))
      offset+=c
  else:
    raise TypeError('unknown mesh type %s'%type(mesh))

def merge_meshes(surfaces):
  if len(surfaces)==1:
    (m,x), = surfaces
    return (m if isinstance(m,TriangleSoup) else TriangleSoup(m)),x
  tris = []
  X = []
  total = 0
  for m,x in surfaces:
    tris.append(total+(m.elements if isinstance(m,TriangleSoup) else asarray(m)))
    x = asarray(x)
    X.append(x)
    total += len(x)
  return TriangleSoup(concatenate(tris).astype(int32)),concatenate(X)

# convenience functions operating directly on MutableTriangleTopology, assuming
# vertex positions are stored in the default location.

# make a MutableTriangleTopology from a soupy or meshy thing and a position array
def meshify(mesh,X):
  if isinstance(mesh, TriangleSoup):
    mesh = TriangleTopology(mesh)
  if isinstance(mesh, CTriangleTopology):
    mesh = mesh.mutate()

  # it is possible that a mesh does not reference the last few vertices. Add as
  # many isolated vertices to the end as necessary (to store all of X)
  n_isolated_vertices = len(X) - mesh.all_vertices().__len__()
  if n_isolated_vertices > 0:
    mesh.add_vertices(n_isolated_vertices);

  mesh.add_vertex_field('3d', vertex_position_id)
  copyto(mesh.vertex_field(vertex_position_id), X)
  return mesh

def mesh_lower_hull(mesh, up, offset, draft_angle = 0., division_angle = 30./180.*pi):
  return meshify(*lower_hull(mesh.face_soup()[0], mesh.vertex_field(vertex_position_id), up, offset, draft_angle, division_angle))

def mesh_offset(mesh, offset):
  return meshify(*rough_offset_mesh(mesh, mesh.vertex_field(vertex_position_id), offset))

def decimate(mesh,X,distance,max_angle=pi/2,min_vertices=-1,boundary_distance=0):
  return geode_wrap.decimate(mesh,X,distance,max_angle,min_vertices,boundary_distance)
