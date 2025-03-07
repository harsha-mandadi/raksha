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
//-----------------------------------------------------------------------------
#ifndef SRC_IR_PROTO_SCHEMA_H_
#define SRC_IR_PROTO_SCHEMA_H_

#include "src/ir/types/schema.h"
#include "third_party/arcs/proto/manifest.pb.h"

namespace raksha::ir::types::proto {

// Decodes the given `schema_proto` and returns the resulting Schema.
Schema decode(const arcs::SchemaProto& schema_proto);

// Encodes the given `schema` as an arcs::SchemaProto.
arcs::SchemaProto encode(const Schema& schema);

}  // namespace raksha::ir::types::proto

#endif  // SRC_IR_PROTO_SCHEMA_H_
