/* Copyright (c) 2016 PaddlePaddle Authors. All Rights Reserve.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#pragma once

#include <map>
#include <vector>
#include "BufferArg.h"
#include "paddle/math/Matrix.h"
#include "paddle/utils/ClassRegistrar.h"

namespace paddle {

/**
 * Function Configuration.
 * The argument type of Function::init.
 * Follow-up will consider moving this data structure to Proto inside.
 */
class FuncConfig {
public:
  union value {
    size_t s;
    real r;
    int i;
    bool b;
  };

  template <typename T>
  T get(const std::string& key) const;

  template <typename T>
  FuncConfig& set(const std::string& key, T v);

protected:
  std::map<std::string, value> valueMap_;
};

/**
 * Argument type for Function::calc().
 * A BufferArgs contains a set of BufferArg,
 * because Function can have multiple inputs, outputs and inouts.
 */
class BufferArgs {
public:
  BufferArgs() {}
  size_t size() const { return args_.size(); }

  // add argument into BufferArgss
  template <typename Tensor>
  void addArg(const Tensor& arg) {
    args_.push_back(std::make_shared<BufferArg>(arg));
  }

  void addArg(const Matrix& arg, const TensorShape& shape);

  void addArg(const CpuSparseMatrix& arg);
  void addArg(const GpuSparseMatrix& arg);

  // get argument
  const BufferArg& operator[](size_t num) const {
    CHECK_LT(num, args_.size());
    return *args_[num];
  }

private:
  std::vector<BufferArgPtr> args_;
};

/**
 * Base class for Function.
 * The basic Function implementation requires override init and calc interfaces.
 * Need to pay attention to the inouts argument. For the input argument
 * that will be modified, it needs to be passed through inouts.
 */
class FunctionBase {
public:
  virtual ~FunctionBase() {}

  virtual void init(const FuncConfig& config) {}

  virtual void calc(const BufferArgs& inputs,
                    const BufferArgs& outputs,
                    const BufferArgs& inouts) {}

  static ClassRegistrar<FunctionBase> funcRegistrar_;
};

#define FUNC_NAME(typeName, deviceName) #typeName "-" #deviceName

#define REGISTER_TYPED_FUNC(typeName, deviceName, className)   \
  static InitFunction __reg_type_##typeName##deviceName([]() { \
    FunctionBase::funcRegistrar_                               \
        .registerClass<className<DEVICE_TYPE_##deviceName>>(   \
            FUNC_NAME(typeName, deviceName));                  \
  })

}  // namespace paddle
