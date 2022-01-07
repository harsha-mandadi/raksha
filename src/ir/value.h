//-----------------------------------------------------------------------------
// Copyright 2021 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//----------------------------------------------------------------------------
#ifndef SRC_IR_VALUE_H_
#define SRC_IR_VALUE_H_

#include <vector>

#include "absl/container/flat_hash_map.h"
#include "src/ir/storage.h"
#include "src/ir/types/type.h"

namespace raksha::ir {

class Operation;
class Operator;
class Value;

namespace value {
// TODO: Add constructors and make fields private.
// Also validate fields.


// A value that is an argument to an operation.
class OperationArgument {
 public:
  OperationArgument(const Operation* op, std::string name)
      : operation_(op), operand_name_(name) {}

 private:
  const Operation* operation_;
  std::string operand_name_;
};

// A value that is the result of an operation.
class OperationResult {
 public:
  OperationResult(const Operation* op, std::string name)
      : operation_(op), operand_name_(name) {}

 private:
  const Operation* operation_;
  std::string operand_name_;
};

// Argument to an operator (not operation). This is typically used in the
// implementation of an operator.
class OperatorArgument {
 public:
  OperatorArgument(const Operator* op, std::string name)
      : op_(op), operand_name_(name) {}

 private:
  const Operator* op_;
  std::string operand_name_;
};

// Result of an operator (not operation). This is typically used in the
// implementation of an operator.
class OperatorResult {
 public:
  const Operator* op_;
  std::string operand_name_;
};

// A field within another value.
class Field {
 public:
  Field(Value* parent, std::string field_name)
      : parent_(parent), field_name_(std::move(field_name)) {}

 private:
  // TODO: Memory management and copying.
  Value* parent_;
  std::string field_name_;
};

// Indicates the value in a storage.
class Store {
 public:
  Store(const Storage& storage) : storage_(&storage) {}

 private:
  const Storage* storage_;
};

class Constant {
  // int, string, etc.
};

class Predicate {
 public:
  Predicate() {}

 private:
  // TODO: we should store the actual predicate.
  // std::unique_ptr<ir::Predicate> predicate_;
};

// A special value to indicate that it can be anything.
class Any {};

}  // namespace value

// A class that represents a data value.
class Value {
 public:
  // TODO:
  const types::Type* type() const { return nullptr; }
  // TODO: make the variable private.
  Value(value::OperatorArgument arg): value_(std::move(arg)) {}
  Value(value::OperatorResult arg): value_(std::move(arg)) {}
  Value(value::OperationArgument arg): value_(std::move(arg)) {}
  Value(value::OperationResult arg): value_(std::move(arg)) {}
  Value(value::Field arg): value_(std::move(arg)) {}
  Value(value::Predicate arg) : value_(std::move(arg)) {}
  Value(value::Any arg) : value_(std::move(arg)) {}
  Value(value::Store arg) : value_(std::move(arg)) {}

 private:
  std::variant<value::OperationResult, value::OperationArgument,
               value::OperatorResult, value::OperatorArgument, value::Field,
               value::Store, value::Constant, value::Predicate, value::Any>
      value_;
};

using ValueList = std::vector<Value>;
using NamedValueMap = absl::flat_hash_map<std::string, Value>;
using NamedValueListMap = absl::flat_hash_map<std::string, ValueList>;

}  // namespace raksha::ir

#endif  // SRC_IR_VALUE_H_
