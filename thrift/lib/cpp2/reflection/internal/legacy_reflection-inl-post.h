/*
 * Copyright 2016-present Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

namespace apache {
namespace thrift {

namespace legacy_reflection_detail {

using id_t = legacy_reflection_id_t;
using datatype_t = reflection::DataType;
using schema_t = legacy_reflection_schema_t;
using type_t = reflection::Type;

// strings

namespace str {
FATAL_S(angle_l, "<");
FATAL_S(angle_r, ">");
FATAL_S(comma, ",");
FATAL_S(dot, ".");
FATAL_S(space, " ");
} // namespace str

// utils

template <typename Type, typename Module, typename Meta>
using get_type_full_name = fatal::
    cat<Type, str::space, typename Module::name, str::dot, typename Meta::name>;

template <typename Type, typename Value>
using get_container_name = fatal::cat<Type, str::angle_l, Value, str::angle_r>;

template <typename Type, typename Key, typename Mapped>
using get_map_container_name = fatal::
    cat<Type, str::angle_l, Key, str::comma, str::space, Mapped, str::angle_r>;

template <typename T, T... Values>
struct c_array {
  using type = T;
  static constexpr auto size = sizeof...(Values);
  static constexpr T data[size] = {Values...};
  static constexpr folly::Range<const type*> range() {
    return {data, size};
  }
};
template <typename T, T... Values>
constexpr T c_array<T, Values...>::data[c_array<T, Values...>::size];

template <typename T, T... Values>
struct to_c_array;
template <typename T, T... Values>
struct to_c_array<fatal::sequence<T, Values...>> : c_array<T, Values...> {};

extern id_t get_type_id(type_t type, folly::StringPiece name);

template <typename F>
void registering_datatype(
    schema_t& schema,
    folly::StringPiece rname,
    id_t rid,
    F&& f) {
  auto& dt = schema.dataTypes[rid];
  if (!dt.name.empty()) {
    return; // this datatype has already been registered
  }
  dt.name = rname.str();
  schema.names[dt.name] = rid;
  f(dt);
}

template <typename T>
struct deref {
  using type = T;
};
template <typename T, typename D>
struct deref<std::unique_ptr<T, D>> : deref<T> {};
template <typename T>
struct deref<std::shared_ptr<T>> : deref<T> {};
template <typename T>
struct deref<std::shared_ptr<T const>> : deref<T> {};
template <typename T>
using deref_t = folly::_t<deref<T>>;
template <typename T>
using deref_inner_t = deref_t<folly::remove_cvref_t<T>>;

// helper
//
// The impl::go functions may recurse to other impl::go functions, but only
// indirectly through legacy_reflection<T>::register_into, which calls this
// helper for all the assertions. This permits explicit template instantiations
// of legacy_reflection to reduce the overall template recursion depth.

template <typename T, typename TC, typename = void>
struct helper;

// impl
//
// The workhorse. Specialized per type-class.

template <typename T, typename TypeClass>
struct impl;

template <>
struct impl<void, type_class::nothing> {
  FATAL_S(rname, "void");
  static id_t rid() {
    return id_t(type_t::TYPE_VOID);
  }
  static void go(schema_t&) {}
};

template <>
struct impl<bool, type_class::integral> {
  FATAL_S(rname, "bool");
  static id_t rid() {
    return id_t(type_t::TYPE_BOOL);
  }
  static void go(schema_t&) {}
};

template <std::size_t I>
struct impl_integral;

template <>
struct impl_integral<1> {
  FATAL_S(rname, "byte");
  static id_t rid() {
    return id_t(type_t::TYPE_BYTE);
  }
  static void go(schema_t&) {}
};

template <>
struct impl_integral<2> {
  FATAL_S(rname, "i16");
  static id_t rid() {
    return id_t(type_t::TYPE_I16);
  }
  static void go(schema_t&) {}
};

template <>
struct impl_integral<4> {
  FATAL_S(rname, "i32");
  static id_t rid() {
    return id_t(type_t::TYPE_I32);
  }
  static void go(schema_t&) {}
};

template <>
struct impl_integral<8> {
  FATAL_S(rname, "i64");
  static id_t rid() {
    return id_t(type_t::TYPE_I64);
  }
  static void go(schema_t&) {}
};

template <typename I>
struct impl<I, type_class::integral> : impl_integral<sizeof(I)> {};

template <>
struct impl<double, type_class::floating_point> {
  FATAL_S(rname, "double");
  static id_t rid() {
    return id_t(type_t::TYPE_DOUBLE);
  }
  static void go(schema_t&) {}
};
template <>
struct impl<float, type_class::floating_point> {
  FATAL_S(rname, "float");
  static id_t rid() {
    return id_t(type_t::TYPE_FLOAT);
  }
  static void go(schema_t&) {}
};

template <typename T>
struct impl<T, type_class::binary> {
  FATAL_S(rname, "string");
  static id_t rid() {
    return id_t(type_t::TYPE_STRING);
  }
  static void go(schema_t&) {}
};

template <typename T>
struct impl<T, type_class::string> {
  FATAL_S(rname, "string");
  static id_t rid() {
    return id_t(type_t::TYPE_STRING);
  }
  static void go(schema_t&) {}
};

template <typename T>
struct impl<T, type_class::enumeration> {
  using meta = reflect_enum<T>;
  using module_meta = reflect_module<typename meta::module>;
  FATAL_S(rkind, "enum");
  using rname = get_type_full_name<rkind, module_meta, typename meta::traits>;
  static id_t rid() {
    static const auto storage =
        get_type_id(type_t::TYPE_ENUM, to_c_array<rname>::range());
    return storage;
  }
  static void go(schema_t& schema) {
    using traits = TEnumTraits<T>;
    registering_datatype(
        schema, to_c_array<rname>::range(), rid(), [&](datatype_t& dt) {
          dt.__isset.enumValues = true;
          for (size_t i = 0; i < traits::size; ++i) {
            (*dt.enumValues_ref())[traits::names[i].str()] =
                int(traits::values[i]);
          }
        });
  }
};

template <typename T>
struct impl<T, type_class::structure> {
  using meta = reflect_struct<T>;
  using module_meta = reflect_module<typename meta::module>;
  struct visitor {
    template <typename MemberInfo, size_t Index>
    void operator()(
        fatal::indexed<MemberInfo, Index>,
        schema_t& schema,
        datatype_t& dt) {
      using getter = typename MemberInfo::getter;
      using type = deref_inner_t<decltype(getter::ref(std::declval<T&>()))>;
      using type_class = typename MemberInfo::type_class;
      using type_helper = helper<type, type_class>;
      using member_name = typename MemberInfo::name;
      using member_annotations = typename MemberInfo::annotations::map;
      type_helper::register_into(schema);
      auto& f = (*dt.fields_ref())[MemberInfo::id::value];
      f.isRequired = MemberInfo::optional::value != optionality::optional;
      f.type = type_helper::id();
      f.name = fatal::to_instance<std::string, member_name>();
      f.order = Index;
      auto annotations = f.annotations_ref();
      if (fatal::size<member_annotations>() > 0) {
        annotations = {};
      } else {
        annotations.reset();
      }
      fatal::foreach<member_annotations>([&](auto tag) {
        using annotation = decltype(fatal::tag_type(tag));
        annotations->emplace(
            fatal::to_instance<std::string, typename annotation::key>(),
            fatal::to_instance<std::string, typename annotation::value>());
      });
    }
  };
  FATAL_S(rkind, "struct");
  using rname = get_type_full_name<rkind, module_meta, meta>;
  static id_t rid() {
    static const auto storage =
        get_type_id(type_t::TYPE_STRUCT, to_c_array<rname>::range());
    return storage;
  }
  static void go(schema_t& schema) {
    registering_datatype(
        schema, to_c_array<rname>::range(), rid(), [&](datatype_t& dt) {
          dt.__isset.fields = true;
          fatal::foreach<typename meta::members>(visitor(), schema, dt);
        });
  }
};

template <typename T>
struct impl<T, type_class::variant> {
  using meta = reflect_variant<T>;
  using module_meta = reflect_module<typename meta::module>;
  struct visitor {
    template <typename MemberInfo, size_t Index>
    void operator()(
        fatal::indexed<MemberInfo, Index>,
        schema_t& schema,
        datatype_t& dt) {
      typename MemberInfo::getter getter;
      using type = deref_inner_t<decltype(getter(std::declval<T&>()))>;
      using type_class = typename MemberInfo::metadata::type_class;
      using type_helper = helper<type, type_class>;
      using member_name = typename MemberInfo::metadata::name;
      type_helper::register_into(schema);
      auto& f = (*dt.fields_ref())[MemberInfo::metadata::id::value];
      f.isRequired = true;
      f.type = type_helper::id();
      f.name = fatal::to_instance<std::string, member_name>();
      f.order = Index;
    }
  };
  FATAL_S(rkind, "struct");
  using rname = get_type_full_name<rkind, module_meta, typename meta::traits>;
  static id_t rid() {
    static const auto storage =
        get_type_id(type_t::TYPE_STRUCT, to_c_array<rname>::range());
    return storage;
  }
  static void go(schema_t& schema) {
    registering_datatype(
        schema, to_c_array<rname>::range(), rid(), [&](datatype_t& dt) {
          dt.__isset.fields = true;
          fatal::foreach<typename meta::traits::descriptors>(
              visitor(), schema, dt);
        });
  }
};

template <typename T, typename ValueTypeClass>
struct impl<T, type_class::list<ValueTypeClass>> {
  using traits = thrift_list_traits<T>;
  using value_type = typename traits::value_type;
  using value_helper = helper<value_type, ValueTypeClass>;
  FATAL_S(rkind, "list");
  using rname = get_container_name<rkind, typename value_helper::name>;
  static id_t rid() {
    static const auto storage =
        get_type_id(type_t::TYPE_LIST, to_c_array<rname>::range());
    return storage;
  }
  static void go(schema_t& schema) {
    registering_datatype(
        schema, to_c_array<rname>::range(), rid(), [&](datatype_t& dt) {
          dt.valueType_ref() = value_helper::id();
          legacy_reflection<value_type>::register_into(schema);
        });
  }
};

template <typename T, typename ValueTypeClass>
struct impl<T, type_class::set<ValueTypeClass>> {
  using traits = thrift_set_traits<T>;
  using value_type = typename traits::value_type;
  using value_helper = helper<value_type, ValueTypeClass>;
  FATAL_S(rkind, "set");
  using rname = get_container_name<rkind, typename value_helper::name>;
  static id_t rid() {
    static const auto storage =
        get_type_id(type_t::TYPE_SET, to_c_array<rname>::range());
    return storage;
  }
  static void go(schema_t& schema) {
    registering_datatype(
        schema, to_c_array<rname>::range(), rid(), [&](datatype_t& dt) {
          dt.valueType_ref() = value_helper::id();
          legacy_reflection<value_type>::register_into(schema);
        });
  }
};

template <typename T, typename KeyTypeClass, typename MappedTypeClass>
struct impl<T, type_class::map<KeyTypeClass, MappedTypeClass>> {
  using traits = thrift_map_traits<T>;
  using key_type = typename traits::key_type;
  using key_helper = helper<key_type, KeyTypeClass>;
  using mapped_type = typename traits::mapped_type;
  using mapped_helper = helper<mapped_type, MappedTypeClass>;
  FATAL_S(rkind, "map");
  using rname = get_map_container_name<
      rkind,
      typename key_helper::name,
      typename mapped_helper::name>;
  static id_t rid() {
    static const auto storage =
        get_type_id(type_t::TYPE_MAP, to_c_array<rname>::range());
    return storage;
  }
  static void go(schema_t& schema) {
    registering_datatype(
        schema, to_c_array<rname>::range(), rid(), [&](datatype_t& dt) {
          dt.mapKeyType_ref() = key_helper::id();
          dt.valueType_ref() = mapped_helper::id();
          legacy_reflection<key_type>::register_into(schema);
          legacy_reflection<mapped_type>::register_into(schema);
        });
  }
};

template <typename T, typename TC>
using is_unknown = folly::bool_constant<
    std::is_same<TC, type_class::unknown>::value ||
    (std::is_same<reflect_type_class<T>, type_class::unknown>::value &&
     (std::is_same<TC, type_class::enumeration>::value ||
      std::is_same<TC, type_class::structure>::value ||
      std::is_same<TC, type_class::variant>::value))>;

template <typename T, typename TC>
using is_known = folly::bool_constant<!is_unknown<T, TC>::value>;

template <typename T, typename TC>
using is_complete = fatal::is_complete<impl<T, TC>>;

// helper

template <typename T, typename TC>
struct helper<
    T,
    TC,
    typename std::enable_if<
        is_known<T, TC>::value && is_complete<T, TC>::value>::type> {
  using type_impl = impl<T, TC>;
  static void register_into(schema_t& schema) {
    type_impl::go(schema);
  }
  using name = typename type_impl::rname;
  static id_t id() {
    return type_impl::rid();
  }
};

template <typename T, typename TC>
struct helper<
    T,
    TC,
    typename std::enable_if<!(
        is_known<T, TC>::value && is_complete<T, TC>::value)>::type> {
  static_assert(
      is_known<T, TC>::value,
      "legacy_reflection: missing reflection metadata");
  static_assert(
      !is_known<T, TC>::value || is_complete<T, TC>::value,
      "legacy_reflection: incomplete handler");

  static void register_into(schema_t&) {}
  using name = fatal::sequence<char>;
  static id_t id() {
    return {};
  }
};

} // namespace legacy_reflection_detail

// legacy_reflection

template <typename T>
void legacy_reflection<T>::register_into(legacy_reflection_schema_t& schema) {
  using TC = reflect_type_class<T>;
  legacy_reflection_detail::helper<T, TC>::register_into(schema);
}

template <typename T>
legacy_reflection_schema_t legacy_reflection<T>::schema() {
  legacy_reflection_schema_t schema;
  register_into(schema);
  return schema;
}

template <typename T>
constexpr folly::StringPiece legacy_reflection<T>::name() {
  using TC = reflect_type_class<T>;
  using name = typename legacy_reflection_detail::helper<T, TC>::name;
  return legacy_reflection_detail::to_c_array<name>::range();
}

template <typename T>
legacy_reflection_id_t legacy_reflection<T>::id() {
  using TC = reflect_type_class<T>;
  return legacy_reflection_detail::helper<T, TC>::id();
}

} // namespace thrift
} // namespace apache
