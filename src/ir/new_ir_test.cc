#include "absl/container/flat_hash_set.h"
#include "src/common/testing/gtest.h"
#include "src/ir/context.h"
#include "src/ir/operation.h"
#include "src/ir/storage.h"
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

  // Write Storage
  const Operator* write_op = context.RegisterOperator(
      OperatorBuilder("core.write")
          .AddInput("src", std::make_unique<types::PrimitiveType>())
          .AddInput("tgt", std::make_unique<types::PrimitiveType>())
          .build());
  ASSERT_NE(make_foo_op, nullptr);

  // Temporary hack to manage Value instances used in value::Field
  // value::Field should manage the pointers on its own.
  absl::flat_hash_set<Value*> values;

  // particle P1 as an operator
  //
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

            // Input of tag claim operation is mapped to both bar.x and bar.y to
            // reflect the fact that foo.a may get its value from both.  We also
            // add a "Any()" as an input to indicate that foo.a would also be
            // created from something other than `bar.x` and `bar.y`.
            const Operation* foo_a = builder.AddOperation(
                tag_claim_op,
                {
                    {"input", {bar_x, bar_y, Value(value::Any())}},
                    {"predicate", {userSelection}},
                });

            // Create `foo` for the output.
            const Operation* foo = builder.AddOperation(
                make_foo_op,
                {{"a", {Value(value::OperationResult(foo_a, "output"))}},
                 {"b", {bar_y}}});

            // Assign the foo constructed from these operations to be the
            // output of this particle.
            builder.AddResult("foo",
                              Value(value::OperationResult(foo, "value")));
          })
          .build());

  ASSERT_NE(particle_p1, nullptr);

  // Instances of particle are represented as operations.
  auto input_storage =
      std::make_unique<Storage>(std::make_unique<types::PrimitiveType>());
  auto output_storage =
      std::make_unique<Storage>(std::make_unique<types::PrimitiveType>());

  // Recipe R
  // P1
  //  bar: reads h1
  //  foo: writes h2
  std::unique_ptr<Operation> particle_instance(new Operation(
      particle_p1, {{"bar", {Value(value::Store(*input_storage))}}}));
  std::unique_ptr<Operation> write_storage(new Operation(
      write_op,
      {{"src", {Value(value::OperationResult(particle_instance.get(), "foo"))}},
       {"tgt", {Value(value::Store(*output_storage))}}}));

  ASSERT_EQ(particle_instance->op().name(), "arcs.particle.P1");
  ASSERT_EQ(write_storage->op().name(), "core.write");
}

}  // namespace raksha::ir
