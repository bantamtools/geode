//#####################################################################
// Class Array<T,2>
//#####################################################################
//
// A shareable, two-dimensional array.
//
//#####################################################################
#pragma once

#include <geode/array/ArrayNdBase.h>
#include <geode/array/Subarray.h>
#include <geode/geometry/Box.h>
#include <geode/python/exceptions.h>
#include <geode/vector/Vector.h>
namespace geode {

template<class T_>
class Array<T_,2> : public ArrayNdBase<T_,Array<T_,2> >
{
  typedef T_ T;
public:
  enum Workaround1 {dimension=2};
  enum Workaround2 {d=dimension};
  typedef typename boost::remove_const<T>::type Element;
  typedef ArrayNdBase<T,Array> Base;

  using Base::flat; // one-dimensional data storage
  using Base::data;

private:
  struct Unusable{};
  typedef typename mpl::if_<boost::is_const<T>,Array<Element,d>,Unusable>::type MutableSelf;
public:

  int m,n; // sizes

  Array()
    : m(0), n(0) {}

  Array(const Vector<int,d>& counts, const bool initialize=true)
    : Base(counts.x*counts.y,initialize), m(counts.x), n(counts.y) {
    assert(m>=0 && n>=0);
  }

  Array(const int m, const int n, const bool initialize=true)
    : Base(m*n,initialize),m(m),n(n) {
    assert(m>=0 && n>=0);
  }

  Array(const int m, const int n, T* data, PyObject* owner)
    : m(m), n(n) {
    assert(m>=0 && n>=0);
    flat = Array<T>(m*n,data,owner);
  }

  Array(const Vector<int,d>& counts, T* data, PyObject* owner)
    : m(counts.x), n(counts.y) {
    assert(m>=0 && n>=0);
    flat = Array<T>(m*n,data,owner);
  }

  Array(const Array& source)
    : m(source.m), n(source.n) {
    flat = source.flat;
  }

  // conversion from mutable to const
  Array(const MutableSelf& source)
    : m(source.m), n(source.n) {
    flat = source.flat;
  }

  explicit Array(const NdArray<T>& array) {
    GEODE_ASSERT(array.rank()==2);
    m = array.shape[0];
    n = array.shape[1];
    flat = array.flat;
  }

  RawArray<T,2> raw() const {
    return RawArray<T,2>(m,n,data());
  }

  Array& operator=(const Array& source) {
    flat = source.flat;
    m = source.m;
    n = source.n;
    return *this;
  }

  // Conversion from mutable to const
  Array& operator=(const MutableSelf& source) {
    flat = source.flat;
    m = source.m;
    n = source.n;
    return *this;
  }

  template<class TArray> void copy(const TArray& source) {
    resize(source.sizes(),false);
    const_cast<const Array*>(this)->copy(source);
  }

  template<class TArray> void copy(const TArray& source) const {
    assert(sizes()==source.sizes());
    if (boost::is_same<TArray,Array<Element,2> >::value || boost::is_same<TArray,Array<const Element,2> >::value)
      memcpy(flat.data(),source.data(),flat.size()*sizeof(T));
    else
      for (int i=0;i<m;i++) for (int j=0;j<n;j++)
        flat[i*n+j] = source(i,j);
  }

  Array<Element,d> copy() const {
    Array<Element,d> result;
    result.copy(*this);
    return result;
  }

  Vector<int,d> sizes() const {
    return Vector<int,d>(m,n);
  }

  int total_size() const {
    return m*n;
  }

  void clean_memory() {
    Array empty;
    swap(empty);
  }

  T& operator()(const int i,const int j) const {
    assert(unsigned(i)<unsigned(m) && unsigned(j)<unsigned(n));
    return flat[i*n+j];
  }

  T& operator()(const Vector<int,d>& index) const {
    assert(unsigned(index.x)<unsigned(m) && unsigned(index.y)<unsigned(n));
    return flat[index.x*n+index.y];
  }

  T& operator[](const Vector<int,d>& index) const {
    assert(unsigned(index.x)<unsigned(m) && unsigned(index.y)<unsigned(n));
    return flat[index.x*n+index.y];
  }

  const RawArray<T,d-1> operator[](const int i) const {
    assert(unsigned(i)<unsigned(m));
    return RawArray<T,d-1>(n,data()+i*n);
  }

  const RawArray<T,d-1> operator()(const int i) const {
    return operator[](i);
  }

  const RawArray<T> row(const int i) const {
    return operator[](i);
  }

  const Subarray<T> column(const int j) const {
    return Subarray<T>(flat,j,flat.size(),n);
  }

  bool valid(const Vector<int,d>& index) const {
    return unsigned(index.x)<unsigned(m) && unsigned(index.y)<unsigned(n);
  }

  bool valid(const int i,const int j) const {
    return unsigned(i)<unsigned(m) && unsigned(j)<unsigned(n);
  }

  void resize(int m_new, int n_new, const bool initialize_new_elements=true, const bool copy_existing_elements=true) {
    if (m_new==m && n_new==n) return;
    assert(m_new>=0 && n_new>=0);
    int new_size = m_new*n_new;
    if (n_new==n || !flat.size() || !new_size || !copy_existing_elements)
      flat.resize(new_size,initialize_new_elements,copy_existing_elements);
    else {
      Array<T> new_flat(new_size,initialize_new_elements);
      int m2 = geode::min(m,m_new),
          n2 = geode::min(n,n_new);
      for (int i=0;i<m2;i++) for (int j=0;j<n2;j++)
        new_flat(i*n_new+j) = flat(i*n+j);
      flat=new_flat;
    }
    m = m_new;
    n = n_new;
  }

  void resize(const Vector<int,d>& counts, const bool initialize_new_elements=true, const bool copy_existing_elements=true) {
    resize(counts.x,counts.y,initialize_new_elements,copy_existing_elements);
  }

  RawArray<T,1> reshape(int m_new) const {
    assert(m_new==flat.size());return flat;
  }

  RawArray<T,2> reshape(int m_new, int n_new) const {
    return flat.reshape(m_new,n_new);
  }

  RawArray<T,3> reshape(int m_new, int n_new, int mn_new) const {
    return flat.reshape(m_new,n_new,mn_new);
  }

  const Array<T,2> reshape_own(int m_new,int n_new) const {
    return flat.reshape_own(m_new,n_new);
  }

  const Array<T,3> reshape_own(int m_new,int n_new,int mn_new) const {
    return flat.reshape_own(m_new,n_new,mn_new);
  }

  void swap(Array& other) {
    flat.swap(other.flat);
    std::swap(m,other.m);
    std::swap(n,other.n);
  }

  RawArray<T,2> slice(int imin,int imax) const {
    assert(unsigned(imin)<=unsigned(imax) && unsigned(imax)<=unsigned(m));
    return RawArray<T,2>(imax-imin,n,data()+imin*n);
  }

  Array<T,2> slice_own(int imin,int imax) const {
    assert(unsigned(imin)<=unsigned(imax) && unsigned(imax)<=unsigned(m));
    return Array<T,2>(imax-imin,n,data()+imin*n,flat.borrow_owner());
  }

  const Subarray<T,2> slice(int imin,int imax,int jmin,int jmax) const {
    return Subarray<T,2>(*this,imin,imax,jmin,jmax);
  }

  const Subarray<T> diagonal() const {
    return Subarray<T>(flat,0,min(m,n)*(n+1)-n,n+1);
  }

  template<class T2> const Array<typename mpl::if_<boost::is_same<T2,Element>,T,T2>::type,2> as() const {
    return flat.template as<T2>().reshape_own(m,n);
  }

  void transpose() {
    raw().transpose();
    std::swap(m,n);
  }

  Array<Element,2> transposed() const {
    return raw().transposed();
  }

  void permute_rows(RawArray<const int> p, int direction) const { // 1 for forward (A[i] = A[p[i]]), -1 for backward (A[p[i]] = A[i])
    raw().permute_rows(p,direction);
  }

  void permute_columns(RawArray<const int> p, int direction) const { // 1 for forward (A[i] = A[p[i]]), -1 for backward (A[p[i]] = A[i])
    raw().permute_columns(p,direction);
  }

  template<class TArray> int append(const TArray& array) {
    int i = m;
    resize(m+1,n,false);
    (*this)[i]=array;
    return i;
  }
};

template<class T> static inline std::ostream& operator<<(std::ostream& output, const Array<T,2>& a) {
  return output<<Subarray<T,2>(a);
}

template<class T> static inline T frobenius_norm(const Array<T,2>& a) {
  return frobenius_norm(a.raw());
}

// Matrix infinity norm
template<class T> static inline T infinity_norm(const Array<T,2>& a) {
  return infinity_norm(a.raw());
}

GEODE_CORE_EXPORT Array<real,2> identity_matrix(int m, int n=-1);

GEODE_CORE_EXPORT Array<float,2> dot(Subarray<const float,2> A, Subarray<const float,2> B);
GEODE_CORE_EXPORT Array<double,2> dot(Subarray<const double,2> A, Subarray<const double,2> B);
GEODE_CORE_EXPORT Array<float> dot(Subarray<const float,2> A, RawArray<const float> x);
GEODE_CORE_EXPORT Array<double> dot(Subarray<const double,2> A, RawArray<const double> x);

}
