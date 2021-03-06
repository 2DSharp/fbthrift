/*
 * Copyright 2019-present Facebook, Inc.
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

#include <folly/io/Cursor.h>
#include <folly/portability/GFlags.h>
#include <thrift/lib/cpp/protocol/TProtocol.h>
#include <thrift/lib/cpp/util/BitwiseCast.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/protocol/ProtocolReaderWireTypeInfo.h>
#include <thrift/lib/cpp2/protocol/nimble/Decoder.h>
#include <thrift/lib/cpp2/protocol/nimble/Encoder.h>
#include <thrift/lib/cpp2/protocol/nimble/NimbleTypes.h>

DECLARE_int32(thrift_cpp2_protocol_reader_string_limit);
DECLARE_int32(thrift_cpp2_protocol_reader_container_limit);

namespace apache {
namespace thrift {

using folly::io::Cursor;

class NimbleProtocolReader;

class NimbleProtocolWriter {
 public:
  using ProtocolReader = NimbleProtocolReader;

  uint32_t writeMessageBegin(
      const std::string& name,
      MessageType messageType,
      int32_t seqid);
  uint32_t writeMessageEnd();
  uint32_t writeStructBegin(const char* name);
  uint32_t writeStructEnd();
  uint32_t writeFieldBegin(const char* name, TType fieldType, int16_t fieldId);
  uint32_t writeFieldEnd();
  uint32_t writeFieldStop();
  uint32_t writeMapBegin(TType keyType, TType valType, uint32_t size);
  uint32_t writeMapEnd();
  uint32_t writeCollectionBegin(TType elemType, uint32_t size);
  uint32_t writeListBegin(TType elemType, uint32_t size);
  uint32_t writeListEnd();
  uint32_t writeSetBegin(TType elemType, uint32_t size);
  uint32_t writeSetEnd();
  uint32_t writeBool(bool value);
  uint32_t writeByte(int8_t byte);
  uint32_t writeI16(int16_t i16);
  uint32_t writeI32(int32_t i32);
  uint32_t writeI64(int64_t i64);
  uint32_t writeDouble(double dub);
  uint32_t writeFloat(float flt);
  uint32_t writeString(folly::StringPiece str);
  uint32_t writeBinary(folly::StringPiece str);
  uint32_t writeBinary(folly::ByteRange str);
  uint32_t writeBinary(const std::unique_ptr<folly::IOBuf>& str);
  uint32_t writeBinary(const folly::IOBuf& str);
  uint32_t writeSerializedData(const std::unique_ptr<folly::IOBuf>& /*data*/);

  /**
   * Functions that return the serialized size
   */

  uint32_t serializedMessageSize(const std::string& name) const;
  uint32_t
  serializedFieldSize(const char* name, TType fieldType, int16_t fieldId) const;
  uint32_t serializedStructSize(const char* name) const;
  uint32_t serializedSizeMapBegin(TType keyType, TType valType, uint32_t size)
      const;
  uint32_t serializedSizeMapEnd() const;
  uint32_t serializedSizeListBegin(TType elemType, uint32_t size) const;
  uint32_t serializedSizeListEnd() const;
  uint32_t serializedSizeSetBegin(TType elemType, uint32_t size) const;
  uint32_t serializedSizeSetEnd() const;
  uint32_t serializedSizeStop() const;
  uint32_t serializedSizeBool(bool = false) const;
  uint32_t serializedSizeByte(int8_t = 0) const;
  uint32_t serializedSizeI16(int16_t = 0) const;
  uint32_t serializedSizeI32(int32_t = 0) const;
  uint32_t serializedSizeI64(int64_t = 0) const;
  uint32_t serializedSizeDouble(double = 0.0) const;
  uint32_t serializedSizeFloat(float = 0) const;
  uint32_t serializedSizeString(folly::StringPiece str) const;
  uint32_t serializedSizeBinary(folly::StringPiece str) const;
  uint32_t serializedSizeBinary(folly::ByteRange v) const;
  uint32_t serializedSizeBinary(std::unique_ptr<folly::IOBuf> const& v) const;
  uint32_t serializedSizeBinary(folly::IOBuf const& v) const;
  uint32_t serializedSizeZCBinary(folly::StringPiece str) const;
  uint32_t serializedSizeZCBinary(folly::ByteRange v) const;
  uint32_t serializedSizeZCBinary(
      std::unique_ptr<folly::IOBuf> const& /*v*/) const;
  uint32_t serializedSizeZCBinary(folly::IOBuf const& /*v*/) const;
  uint32_t serializedSizeSerializedData(
      std::unique_ptr<folly::IOBuf> const& /*data*/) const;

  std::unique_ptr<folly::IOBuf> finalize() {
    return encoder_.finalize();
  }

 private:
  /*
   * The encoder that manipulates the underlying field and content streams.
   */
  detail::Encoder encoder_;
};

class NimbleProtocolReader {
  using NimbleType = detail::nimble::NimbleType;

 public:
  using ProtocolWriter = NimbleProtocolWriter;

  explicit NimbleProtocolReader(
      int32_t string_limit = FLAGS_thrift_cpp2_protocol_reader_string_limit,
      int32_t container_limit =
          FLAGS_thrift_cpp2_protocol_reader_container_limit)
      : string_limit_(string_limit), container_limit_(container_limit) {
    if (string_limit_ <= 0) {
      string_limit_ = INT32_MAX;
    }
    if (container_limit_ <= 0) {
      container_limit_ = INT32_MAX;
    }
  }

  static constexpr bool kUsesFieldNames() {
    return false;
  }

  static constexpr bool kOmitsContainerSizes() {
    return false;
  }

  void setInput(const folly::io::Cursor& cursor) {
    decoder_.setInput(cursor);
  }

  /**
   * Reading functions
   */
  void
  readMessageBegin(std::string& name, MessageType& messageType, int32_t& seqid);
  void readMessageEnd();
  void readStructBegin(const std::string& name);
  void readStructEnd();
  void
  readFieldBegin(std::string& name, NimbleType& fieldType, int16_t& fieldId);
  void readFieldEnd();
  void readMapBegin(NimbleType& keyType, NimbleType& valType, uint32_t& size);
  void readMapEnd();
  void readListBegin(NimbleType& elemType, uint32_t& size);
  void readListEnd();
  void readSetBegin(NimbleType& elemType, uint32_t& size);
  void readSetEnd();
  void readBool(bool& value);
  void readBool(std::vector<bool>::reference value);
  void readByte(int8_t& byte);
  void readI16(int16_t& i16);
  void readI32(int32_t& i32);
  void readI64(int64_t& i64);
  void readDouble(double& dub);
  void readFloat(float& flt);
  template <typename StrType>
  void readString(StrType& str);
  template <typename StrType>
  void readBinary(StrType& str);
  void readBinary(std::unique_ptr<folly::IOBuf>& str);
  void readBinary(folly::IOBuf& str);
  void skip(NimbleType /*type*/) {}
  bool peekMap() {
    return false;
  }
  bool peekSet() {
    return false;
  }
  bool peekList() {
    return false;
  }

  const Cursor& getCursor() const {
    throw std::logic_error(
        "NimbleProtocolReader doesn't expose the underlying cursor.");
  }

  size_t getCursorPosition() const {
    return 0;
  }

  struct StructReadState {
    int16_t fieldId;
    detail::nimble::NimbleType nimbleType;

    void readStructBegin(NimbleProtocolReader* iprot) {
      iprot->readStructBegin("");
    }

    void readStructEnd(NimbleProtocolReader* /*iprot*/) {}

    void readFieldBegin(NimbleProtocolReader* iprot) {
      iprot->advanceToNextFieldSlow(*this);
    }

    FOLLY_NOINLINE void readFieldBeginNoInline(
        NimbleProtocolReader* /*iprot*/) {}

    void readFieldEnd(NimbleProtocolReader* /*iprot*/) {}

    FOLLY_ALWAYS_INLINE bool advanceToNextField(
        NimbleProtocolReader* iprot,
        int32_t currFieldId,
        int32_t nextFieldId,
        TType nextFieldType) {
      return iprot->advanceToNextField(
          currFieldId, nextFieldId, nextFieldType, *this);
    }

    void afterAdvanceFailure(NimbleProtocolReader* iprot) {
      iprot->advanceToNextFieldSlow(*this);
    }

    bool atStop() {
      // Note that this might not correspond to a fieldID of 0 (and in fact,
      // never should); it's only safe to look at the type in this instance.
      // This makes advanceToNextFieldSlow slightly simpler, since it lets it
      // unconditionally set the field ID to the adjusted field ID indicated by
      // the high bits.
      return nimbleType == detail::nimble::NimbleType::STOP;
    }

    FOLLY_ALWAYS_INLINE bool isCompatibleWithType(
        NimbleProtocolReader* iprot,
        TType expectedFieldType) {
      return iprot->isCompatibleWithType(expectedFieldType, *this);
    }

    [[noreturn]] inline void skip(NimbleProtocolReader* iprot) {
      iprot->skip(*this);
    }

    std::string& fieldName() {
      throw std::logic_error("NimbleProtocol doesn't support field names");
    }

    template <typename StructTraits>
    void fillFieldTraitsFromName() {
      throw std::logic_error("NimbleProtocol doesn't support field names");
    }
  };

 protected:
  FOLLY_ALWAYS_INLINE bool advanceToNextField(
      int32_t currFieldId,
      int32_t nextFieldId,
      TType type,
      StructReadState& state);

  void advanceToNextFieldSlow(StructReadState& state);

  FOLLY_ALWAYS_INLINE bool isCompatibleWithType(
      TType expectedFieldType,
      StructReadState& state);
  /* The actual method that does skip when there is a mismatch */
  [[noreturn]] void skip(StructReadState& state);

 private:
  detail::Decoder decoder_;
  int32_t string_limit_;
  int32_t container_limit_;
};

namespace detail {

template <class Protocol>
struct ProtocolReaderStructReadState;

template <>
struct ProtocolReaderStructReadState<NimbleProtocolReader>
    : NimbleProtocolReader::StructReadState {};

template <>
struct ProtocolReaderWireTypeInfo<NimbleProtocolReader> {
  using WireType = nimble::NimbleType;

  static WireType fromTType(apache::thrift::protocol::TType ttype) {
    return nimble::ttypeToNimbleType(ttype);
  }

  static WireType defaultValue() {
    return nimble::NimbleType::STOP;
  }
};

} // namespace detail

template <>
inline void skip<NimbleProtocolReader, detail::nimble::NimbleType>(
    NimbleProtocolReader& /* prot */,
    detail::nimble::NimbleType /* arg_type */) {
  // Fool the noreturn warning; we can't add the annotation without breaking
  // interface compatibility.
  volatile bool b = true;
  if (b) {
    throw std::runtime_error("Not implemented yet");
  }
}

template <>
inline void skip_n<NimbleProtocolReader, detail::nimble::NimbleType>(
    NimbleProtocolReader& /* prot */,
    std::uint32_t /* n */,
    std::initializer_list<detail::nimble::NimbleType> /* types*/) {
  volatile bool b = true;
  if (b) {
    throw std::runtime_error("Not implemented yet");
  }
}

} // namespace thrift
} // namespace apache

#include <thrift/lib/cpp2/protocol/NimbleProtocol.tcc>
