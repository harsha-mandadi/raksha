#include "absl/container/flat_hash_set.h"
#include "src/common/testing/gtest.h"
#include "src/ir/context.h"
#include "src/ir/operation.h"
#include "src/ir/types/primitive_type.h"
#include "src/ir/value.h"

namespace raksha::ir {

TEST(NewIrTest, TrySomeStuff) {
  Context context;

  // Using PrimitiveType for illustrative purposes.
  // In reality we will have to create the correct types.
  // For generic operators, we should also create a TypeVariable type.

  const Operator* select_op = context.RegisterOperator(
      OperatorBuilder("sql.select")
          .AddInput("input", std::make_unique<types::PrimitiveType>())
          .AddOutput("output", std::make_unique<types::PrimitiveType>())
          .build());
  ASSERT_NE(select_op, nullptr);

  const Operator* tag_claim_op = context.RegisterOperator(
      OperatorBuilder("arcs.tag_claim")
          .AddInput("input", std::make_unique<types::PrimitiveType>())
          .AddInput("predicate", std::make_unique<types::PrimitiveType>())
          .AddOutput("output", std::make_unique<types::PrimitiveType>())
          .build());
  ASSERT_NE(tag_claim_op, nullptr);

  // Foo {a, b}
  const Operator* make_foo_op = context.RegisterOperator(
      OperatorBuilder("core.makeFoo")
          .AddInput("a", std::make_unique<types::PrimitiveType>())
          .AddInput("b", std::make_unique<types::PrimitiveType>())
          .AddOutput("value", std::make_unique<types::PrimitiveType>())
          .build());
  ASSERT_NE(make_foo_op, nullptr);

  // Temporary hack to manage Value instances used in value::Field
  absl::flat_hash_set<Value*> values;

  // Foo {a, b}
  // Bar {x, y}
  //
  // particle P1
  //  .bar: reads Bar {}
  //   foo: writes Foo {}
  //   claim foo.a is userSelection
  //   claim foo.b is derived from bar.y
  const Operator* particle_p1 = context.RegisterOperator(
      OperatorBuilder("arcs.particle.P1")
          .AddInput("bar", std::make_unique<types::PrimitiveType>())
          .AddOutput("foo", std::make_unique<types::PrimitiveType>())
          .AddImplementation([&values, tag_claim_op, make_foo_op](
                                 OperatorBuilder& builder, Operator* op) {
            // CONCERNS TO BE ADDRESSED:
            //  - this is quite verbose
            //  - SHould an operation also have the outputs explicitly
            //  specified?

            // Create values for bar.x and bar.y
            Value* bar =
                *(values.insert(new Value(value::OperatorArgument(op, "bar")))
                      .first);
            Value bar_x = Value(value::Field(bar, "x"));
            Value bar_y = Value(value::Field(bar, "y"));

            // Build claim foo.a is "UserSelection".
            //
            // create "userSelection" predicate. TODO: Store the actual
            // ir::Predicate instance.
            Value userSelection = Value(value::Predicate());

            // The two versions of foo capturing the fact that foo.a may be
            // derived from bar.x or bar.y.  It is not ideal that we need to
            // create two different foo_a objects.  This can cause an
            // exponential blow up in the number of objects we need to create.
            // We should come up with a better alternative before we commit.
            // Perhaps, we should all for multiple inputs to an operation?
            const Operation* foo1_a = builder.AddOperation(
                tag_claim_op, {
                                  {"input", bar_x},
                                  {"predicate", userSelection},
                              });
            const Operation* foo2_a = builder.AddOperation(
                tag_claim_op, {{"input", bar_y}, {"predicate", userSelection}});

            // Create `foo`s that takes care of derives from as well as "claim
            // foo.a is userSelection".
            const Operation* foo1 = builder.AddOperation(
                make_foo_op,
                {{"a", Value(value::OperationResult(foo1_a, "output"))},
                 {"b", bar_y}});
            // foo.a <- bar.y, foo.b <- bar.y
            const Operation* foo2 = builder.AddOperation(
                make_foo_op,
                {{"a", Value(value::OperationResult(foo2_a, "output"))},
                 {"b", bar_y}});

            // Assign the two foos to be the output of this particle.
            builder.AddResult("foo",
                              Value(value::OperationResult(foo1, "value")));
            builder.AddResult("foo",
                              Value(value::OperationResult(foo2, "value")));
          })
          .build());

  ASSERT_NE(particle_p1, nullptr);
}

}  // namespace raksha::ir
