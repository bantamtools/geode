//#####################################################################
// Class IndirectArray
//#####################################################################
#pragma once

#include <geode/array/ArrayExpression.h>
#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/add_reference.hpp>
namespace geode {

template<class TArray,class TIndices> struct IsArray<IndirectArray<TArray,TIndices> >:public mpl::true_{};
template<class TArray,class TIndices> struct HasCheapCopy<IndirectArray<TArray,TIndices> >:public mpl::true_{};

template<class TArray,class TIndices> // TIndices=RawArray<const int>
class IndirectArray : public ArrayExpression<typename TArray::Element,IndirectArray<TArray,TIndices>,TIndices> {
  typedef typename boost::remove_reference<TIndices>::type TIndicesNoReference;
  typedef typename mpl::if_<boost::is_reference<TIndices>,const TIndicesNoReference&,const TIndicesNoReference>::type ConstTIndices;
  BOOST_MPL_ASSERT((mpl::not_<boost::is_const<TIndicesNoReference> >));
  typedef typename TArray::Element T;
  typedef ArrayBase<T,IndirectArray<TArray,TIndices> > Base;
  struct Unusable{};
public:
  typedef T Element;
  typedef decltype(boost::declval<TArray>()[0]) result_type;

  TArray& array;
  ConstTIndices indices;

  template<class TOtherArray>
  IndirectArray(TOtherArray& array, typename boost::add_reference<ConstTIndices>::type indices)
    : array(array), indices(indices) {
    BOOST_MPL_ASSERT((boost::is_base_of<TArray,TOtherArray>)); // avoid grabbing reference to temporary
  }

  IndirectArray(const IndirectArray<typename boost::remove_const<TArray>::type,TIndices>& indirect)
    : array(indirect.array), indices(indirect.indices) {}

  int size() const {
    return indices.size();
  }

  result_type operator[](const int i) const {
    return array[indices[i]];
  }

  IndirectArray& operator=(const IndirectArray& source) {
    return Base::operator=(source);
  }

  template<class TOtherArray> IndirectArray& operator=(const TOtherArray& source) {
    return Base::operator=(source);
  }

  typename boost::remove_reference<result_type>::type* data() const {
    return array.data()+offset_if_contiguous(indices);
  }

private:
  static int offset_if_contiguous(const ARange& indices) { // for contiguous indices, we can extract an offset
    return 0;
  }

  template<class TIndices2> static int offset_if_contiguous(const ArrayPlusScalar<int,TIndices2>& indices) {
    return indices.c+offset_if_contiguous(indices.array);
  }
};
}
