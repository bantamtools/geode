//#####################################################################
// Class Action
//#####################################################################
#pragma once

#include <geode/value/Value.h>
#include <geode/utility/move.h>
#include <boost/type_traits/remove_reference.hpp>
namespace geode {

using boost::remove_reference;

class GEODE_CORE_CLASS_EXPORT Action {
private:
  template<class T> friend void set_value_and_dependencies(ValueRef<T> &, T const &, vector<ValueBase const*> const &);
  friend class ValueBase;
  mutable ValueBase::Link* inputs_; // linked list of inputs we depend on
  GEODE_CORE_EXPORT static const Action* current; // if nonzero, pulled values link themselves to this automatically
  mutable bool executing; // are we in the middle of execution?
protected:
  class Executing {
    bool& executing;
    const Action* const parent;
  public:
    GEODE_CORE_EXPORT Executing(); // Values pulled during execution won't be registered to any action
    GEODE_CORE_EXPORT Executing(const Action& self);
    ~Executing() {
      stop(0);
    }
    // Convenience function to stop execution in the middle of a statement
    template<class T> typename remove_reference<T>::type&& stop(T&& x) {
      executing = false;
      Action::current = parent;
      return geode::move(x);
    }
  };
  friend class Executing;
public:

  GEODE_CORE_EXPORT Action();
  GEODE_CORE_EXPORT virtual ~Action();

  // Count inputs
  int inputs() const;

  GEODE_CORE_EXPORT void dump_dependencies(int indent) const;
  GEODE_CORE_EXPORT vector<Ref<const ValueBase>> dependencies() const;

protected:
  GEODE_CORE_EXPORT void clear_dependencies() const;
  GEODE_CORE_EXPORT void depend_on(const ValueBase& value) const;
private:
  virtual void input_changed() const = 0;
};


// Set the ValueRef to the given value, and set its dependencies manually (instead of a cache() or other
// automatic way of determining them. You should know what you're doing. This only works if the ValueRef
// passed in points to an object which inherits from both Value<T> and Action (such as Compute<T>).
template<class T>
void set_value_and_dependencies(ValueRef<T> &value, T const &v, vector<ValueBase const*> const &dependencies) {

  // Get the action
  Action const*action = 0;
  try {
    action = dynamic_cast<Action const*>(&*value);
  } catch (std::bad_cast) {
    throw std::runtime_error("set_value_and_dependencies only works on objects derived from both Action and Value");
  }

  // Set new dependencies
  action->clear_dependencies();
  for (ValueBase const *dep : dependencies)
    action->depend_on(*dep);

  // Set the value
  value->set_value(v);
}

}
