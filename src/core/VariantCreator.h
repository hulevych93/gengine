#pragma once

#include <boost/mpl/at.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/int.hpp>
#include <boost/variant.hpp>

namespace Gengine {

template <class V>
using creator_ = std::function<void(V&)>;
template <class V>
using constructors_ = std::vector<creator_<V>>;

template <class T, class V>
void construct_variant_(V& v) {
  v = T{};
}

template <class V>
void default_construct_variant_(V& v) {}

template <class V>
struct build_constructors_ {
  build_constructors_(constructors_<V>& impl) : impl(impl) {}

  creator_<V> operator[](std::size_t index) const {
    if (index < impl.size()) {
      return impl[index];
    }

    return default_construct_variant_<V>;
  }

  template <class T>
  void operator()(T) {
    impl.push_back(&construct_variant_<T, V>);
  }

 private:
  constructors_<V>& impl;
};

template <class... T>
void create_variant(std::int32_t which, boost::variant<T...>& object) {
  using variant_ = boost::variant<T...>;
  using types_ = typename variant_::types;
  using builder_ = build_constructors_<variant_>;

  constructors_<variant_> constructors;
  builder_ builder = {constructors};
  boost::mpl::for_each<types_>(builder);
  builder[which](object);
}
}  // namespace Gengine
