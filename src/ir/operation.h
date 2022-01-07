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
#ifndef SRC_IR_OPERATION_H_
#define SRC_IR_OPERATION_H_

#include <vector>

#include "absl/container/flat_hash_map.h"
#include "src/ir/types/type.h"
#include "src/ir/value.h"

namespace raksha::ir {

// Signature of an operator. An operator could be simple operators (e.g., `+`,
// `-`, `sql.select`, `sql.groupby`) or complex operators (e.g., particle,
// function) that is compose of other operations.
class Operator {
 public:
  // Declaration of an input or ouptut of the operation.
  struct DataDecl {
    std::string name;
    std::unique_ptr<types::Type> type;
  };

  // The type for a collection of `DataDecl` entities.
  // using DataDeclList = std::vector<DataDecl>;
  using DataDeclMap =
      absl::flat_hash_map<std::string, std::unique_ptr<types::Type>>;

  // The implementation of an operator as a collection of operations.
  class Impl {
   public:
    // The type for a collection of `Operation` instances.
    using OperationList = std::vector<std::unique_ptr<Operation>>;

    const OperationList& operations() const { return operations_; }
    const Operator* parent() const { return parent_; }
    const NamedValueListMap results() const { return results_; }

   private:
    friend class OperatorBuilder;

    Impl(const Operator* parent) : parent_(parent) {}

    // The operator for which we are defining the implementation.
    const Operator* parent_;
    OperationList operations_;
    // Maps the outputs of the operator to the corresponding value.
    // Note that a result can have more than one value which is used
    // to represent non-determinism.
    NamedValueListMap results_;
  };

  const std::string& name() const { return name_; }
  const DataDeclMap& inputs() const { return inputs_; }
  const DataDeclMap& outputs() const { return outputs_; }
  const Impl* impl() const { return impl_.get(); }

 private:
  friend class OperatorBuilder;

  Operator(std::string name) : name_(std::move(name)), impl_(nullptr) {}

  std::string name_;
  DataDeclMap inputs_;
  DataDeclMap outputs_;
  // If the implementation is nullptr, then this is one of the basic operators
  // like `+`, `-`, etc., that cannot be split up any further.
  std::unique_ptr<Impl> impl_;
};

// An Operation represents a unit of execution.
class Operation {
 public:
  Operation(const Operator* op, NamedValueMap inputs)
      : op_(op), inputs_(std::move(inputs)) {
    CHECK(op != nullptr);
    CHECK(op->inputs().size() == inputs_.size());
  }

  const Operator& op() const { return *op_; }

 private:
  const Operator* op_;
  NamedValueMap inputs_;
};

class OperatorBuilder {
 public:
  OperatorBuilder(std::string name)
      : operator_(new Operator(std::move(name))) {}

  OperatorBuilder& AddInput(std::string name,
                            std::unique_ptr<types::Type> type) {
    auto insertion_result =
        operator_->inputs_.insert({std::move(name), std::move(type)});
    CHECK(insertion_result.second);
    return *this;
  }

  OperatorBuilder& AddOutput(std::string name,
                             std::unique_ptr<types::Type> type) {
    auto insertion_result =
        operator_->outputs_.insert({std::move(name), std::move(type)});
    CHECK(insertion_result.second);
    return *this;
  }

  const Operation* AddOperation(const Operator* op, NamedValueMap inputs) {
    std::unique_ptr<Operation> operation(new Operation(op, std::move(inputs)));
    const Operation* result = operation.get();
    GetImpl()->operations_.push_back(std::move(operation));
    return result;
  }

  OperatorBuilder& AddImplementation(
      std::function<void(OperatorBuilder&, Operator*)> builder) {
    builder(*this, operator_.get());
    return *this;
  }

  OperatorBuilder& AddResult(absl::string_view name, Value output) {
    CHECK(operator_->outputs_.find(name) != operator_->outputs_.end());
    GetImpl()->results_[name].push_back(output);
    return *this;
  }

  std::unique_ptr<Operator> build() { return std::move(operator_); }

 private:
  Operator::Impl* GetImpl() {
    if (operator_->impl_ == nullptr) {
      operator_->impl_ =
          std::unique_ptr<Operator::Impl>(new Operator::Impl(operator_.get()));
    }
    return operator_->impl_.get();
  }

  std::unique_ptr<Operator> operator_;
};

}  // namespace raksha::ir


#endif  // SRC_IR_OPERATION_H_
