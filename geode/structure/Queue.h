//#####################################################################
// Class Queue
//#####################################################################
#pragma once

#include <geode/array/Array.h>
namespace geode {

template<class T>
class Queue {
public:
  Array<T> array;
  int front,back;

  explicit Queue(const int size)
     : array(size+1), front(0), back(0)
  {}

  void enqueue(const T& element) { // resizes array on overflow
    array(back) = element;
    if (++back>=array.size()) back = 0;
    if (back==front) {
      Array<T> new_array(4*array.size()/3+2);
      back = 0;
      for (int index=front;index<array.size();index++) new_array[back++] = array[index];
      for (int index=0;index<front;index++) new_array[back++] = array[index];
      front=0;
      array.swap(new_array);
    }
  }

  T dequeue() {
    assert(!empty());
    int index = front;
    if (++front>=array.size())
      front = 0;
    return array[index];
  }

  const T& peek() const {
    return array[front];
  }

  const T& operator[](const int i) {
    assert(i<size());
    int index = front+i;
    if (index>=array.size())
      index = index-array.size();
    return array[index];
  }

  int size() const {
    return back<front?back-front+array.size():back-front;
  }

  bool empty() const {
    return back==front;
  }

  bool full() const {
    return size()==array.size()-1;
  }

  void clear() {
    front=0;back=0;
  }
};
}
