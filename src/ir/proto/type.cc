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
#include "src/ir/types/type.h"

#include <memory>

#include "src/common/logging/logging.h"
#include "src/ir/proto/entity_type.h"
#include "src/ir/proto/primitive_type.h"
#include "third_party/arcs/proto/manifest.pb.h"

namespace raksha::ir::types::proto {

std::unique_ptr<Type> Decode(const arcs::TypeProto &type_proto) {
  // Delegate to the various CreateFromProto implementations on the base types
  // depending upon which specific type is contained within the TypeProto.
  CHECK(!type_proto.optional())
      << "Optional types are currently unimplemented.";
  CHECK(!type_proto.has_refinement())
      << "Type refinements are currently unimplemented.";
  switch (type_proto.data_case()) {
    case arcs::TypeProto::DATA_NOT_SET:
      LOG(FATAL) << "Found a TypeProto with an unset specific type.";
    case arcs::TypeProto::kPrimitive:
      return std::make_unique<PrimitiveType>(decode(type_proto.primitive()));
    case arcs::TypeProto::kEntity:
      return std::make_unique<EntityType>(decode(type_proto.entity()));
    default:
      LOG(FATAL) << "Found unimplemented type. Only Primitive and Entity "
                    "types are currently implemented.";
  }
  CHECK(false) << "Unreachable!";
}

arcs::TypeProto Encode(const Type& type) {
  switch (type.kind()) {
    case Type::Kind::kPrimitive:
      return encodeAsTypeProto(static_cast<const PrimitiveType&>(type));
    case Type::Kind::kEntity:
      return encodeAsTypeProto(static_cast<const EntityType&>(type));
  }
  CHECK(false) << "Found unknown Type.";
}

}  // namespace raksha::ir::types::proto
