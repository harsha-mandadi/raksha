#include "src/common/testing/gtest.h"
#include "src/ir/context.h"
#include "src/ir/operation.h"
#include "src/ir/types/primitive_type.h"
#include "src/ir/value.h"

namespace raksha::ir {

TEST(NewIrTest, TrySomeStuff) {
  Context context;

  // particle P1
  //  .bar: reads Bar {}
  //   foo: writes Foo {}
  //   claim foo.a is userSelection
  //   claim foo.x is derived from bar.y
  //

  // Using PrimitiveType for illustrative purposes.
  // In reality we will have to create the correct types.
  // For generic operators, we should also create a TypeVariable type.

  const Operator* select_op = context.RegisterOperator(
      OperatorBuilder("sql.select")
          .AddInput("input", std::make_unique<types::PrimitiveType>())
          .AddOutput("output", std::make_unique<types::PrimitiveType>())
          .build());

  const Operator* tag_claim_op = context.RegisterOperator(
      OperatorBuilder("arcs.tag_claim")
          .AddInput("input", std::make_unique<types::PrimitiveType>())
          .AddOutput("output", std::make_unique<types::PrimitiveType>())
          .build());

  // TODO: This is not quite right.
  const Operator* derives_from_op = context.RegisterOperator(
      OperatorBuilder("arcs.derives_from")
          .AddInput("input", std::make_unique<types::PrimitiveType>())
          .AddOutput("output", std::make_unique<types::PrimitiveType>())
          .build());

  // TODO:
  //  - Add Implementation that includes tag_claim and derives_from.
  //  - Add results that map the outputs to results of the operation.
  const Operator* particle_p1 = context.RegisterOperator(
      OperatorBuilder("arcs.particle.P1")
          .AddInput("foo", std::make_unique<types::PrimitiveType>())
          .AddOutput("bar", std::make_unique<types::PrimitiveType>())
          .AddImplementation([tag_claim_op, derives_from_op](
                                 OperatorBuilder& builder, Operator* op) {
            builder.AddOperation(
                tag_claim_op,
                {{"input", Value(value::OperatorArgument(op, "foo"))}});
          })
          .build());

  ASSERT_NE(select_op, nullptr);
  ASSERT_NE(tag_claim_op, nullptr);
  ASSERT_NE(derives_from_op, nullptr);
  ASSERT_NE(particle_p1, nullptr);
}

}  // namespace raksha::ir
