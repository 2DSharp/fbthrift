/*
 * Copyright 2004-present Facebook, Inc.
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

#ifndef THRIFT_THRIFT_H_
#define THRIFT_THRIFT_H_

#include <folly/Range.h>
#include <folly/Traits.h>
#include <folly/lang/Bits.h>
#include <folly/portability/Time.h>

#include <thrift/lib/cpp/thrift_config.h>

#ifdef THRIFT_PLATFORM_CONFIG
# include THRIFT_PLATFORM_CONFIG
#endif

#include <assert.h>
#include <sys/types.h>
#if __has_include(<inttypes.h>)
#include <inttypes.h>
#endif
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <exception>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <thrift/lib/cpp/TLogging.h>

namespace apache { namespace thrift {

struct ltstr {
  bool operator()(const char* s1, const char* s2) const {
    return strcmp(s1, s2) < 0;
  }
};

/**
 * Specialization fwd-decl in _types.h.
 * Specialization defn in _data.h.
 */
template <typename T>
struct TEnumDataStorage {
  static_assert(sizeof(T) == ~0ull, "invalid use of base template");
};

/**
 * Specialization defn in _types.h.
 * Specialization member defns in _types.cpp.
 */
template <typename T>
struct TEnumTraits {
  //  When instantiated with an enum type T, includes:
  //
  //      using type = T;
  //
  //      static constexpr std::size_t const size = /*...*/;
  //      static folly::Range<type const*> const values;
  //      static folly::Range<folly::StringPiece const*> const names;
  //
  //      static char const* findName(type value);
  //      static bool findValue(char const* name, type* out);
  //
  //  When instantiated with an enum type T which is not empty, includes:
  //
  //      static constexpr type min() { /*...*/ }
  //      static constexpr type min() { /*...*/ }
  //
  //  To test at compile time whether a given enum T is generated by thrift, the
  //  presence of TEnumTraits<T>::type may be tested. If it is present, then T
  //  is generated by thrift; if absent, then T is not generated by thrift.

  static char const* findName(T) {
    static_assert(sizeof(T) == ~0ull, "invalid use of base template");
    return nullptr;
  }
  static bool findValue(char const*, T*) {
    static_assert(sizeof(T) == ~0ull, "invalid use of base template");
    return false;
  }
};

namespace detail {

template <typename EnumTypeT, typename ValueTypeT>
struct TEnumMapFactory {
  using EnumType = EnumTypeT;
  using ValueType = ValueTypeT;
  using ValuesToNamesMapType = std::map<ValueType, const char*>;
  using NamesToValuesMapType = std::map<const char*, ValueType, ltstr>;
  using Traits = TEnumTraits<EnumType>;

  static ValuesToNamesMapType makeValuesToNamesMap() {
    ValuesToNamesMapType _return;
    for (size_t i = 0; i < Traits::size; ++i) {
      _return.emplace(ValueType(Traits::values[i]), Traits::names[i].data());
    }
    return _return;
  }
  static NamesToValuesMapType makeNamesToValuesMap() {
    NamesToValuesMapType _return;
    for (size_t i = 0; i < Traits::size; ++i) {
      _return.emplace(Traits::names[i].data(), ValueType(Traits::values[i]));
    }
    return _return;
  }
};

}

class TOutput {
 public:
  TOutput() : f_(&errorTimeWrapper) {}

  inline void setOutputFunction(void (*function)(const char *)){
    f_ = function;
  }

  inline void operator()(const char *message){
    f_(message);
  }

  // It is important to have a const char* overload here instead of
  // just the string version, otherwise errno could be corrupted
  // if there is some problem allocating memory when constructing
  // the string.
  void perror(const char *message, int errno_copy);
  inline void perror(const std::string &message, int errno_copy) {
    perror(message.c_str(), errno_copy);
  }

  void printf(const char *message, ...);

  inline static void errorTimeWrapper(const char* msg) {
    time_t now;
    char dbgtime[26];
    time(&now);
    ctime_r(&now, dbgtime);
    dbgtime[24] = 0;
    fprintf(stderr, "Thrift: %s %s\n", dbgtime, msg);
  }

  /** Just like strerror_r but returns a C++ string object. */
  static std::string strerror_s(int errno_copy);

 private:
  void (*f_)(const char *);
};

extern TOutput GlobalOutput;

/**
 * Base class for all Thrift exceptions.
 * Should never be instantiated, only caught.
 */
class TException : public std::exception {
public:
  TException() {}
  TException(TException&&) noexcept {}
  TException(const TException&) {}
  TException& operator=(const TException&) { return *this; }
  TException& operator=(TException&&) { return *this; }
};

/**
 * Base class for exceptions from the Thrift library, and occasionally
 * from the generated code.  This class should not be thrown by user code.
 * Instances of this class are not meant to be serialized.
 */
class TLibraryException : public TException {
 public:
  TLibraryException() {}

  explicit TLibraryException(const std::string& message) :
    message_(message) {}

  TLibraryException(const char* message, int errnoValue);

  ~TLibraryException() throw() override {}

  const char* what() const throw() override {
    if (message_.empty()) {
      return "Default TLibraryException.";
    } else {
      return message_.c_str();
    }
  }

 protected:
  std::string message_;

};

#if T_GLOBAL_DEBUG_VIRTUAL > 1
void profile_virtual_call(const std::type_info& info);
void profile_generic_protocol(const std::type_info& template_type,
                              const std::type_info& prot_type);
void profile_print_info(FILE *f);
void profile_print_info();
void profile_write_pprof(FILE* gen_calls_f, FILE* virtual_calls_f);
#endif

}} // apache::thrift

#endif // #ifndef THRIFT_THRIFT_H_
