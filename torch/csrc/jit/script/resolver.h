#pragma once

#include <ATen/core/jit_type.h>
#include <ATen/core/qualified_name.h>
#include <torch/csrc/jit/script/sugared_value.h>

namespace torch {
namespace jit {
namespace script {

struct Resolver;
using ResolverPtr = std::shared_ptr<Resolver>;

/**
 * class Resolver
 *
 * Represents an "outer environment" in which we an look up names and return
 * a corresponding SugaredValue. This is used during compilation to resolve
 * references to names which are not defined internal to the graph.
 *
 * Example: PythonResolver looks at the enclosing Python scope for `name`.
 *
 * NOTE: When adding methods, keep this an abstract class (i.e. all new methods
 * should be purely virtual). Resist the urge to provide a default
 * implementation; you should explicitly think about how each resolver would
 * handle the method.
 */
struct Resolver {
  virtual ~Resolver() {}

  // Resolve a given name to a SugaredValue. This takes the method `m` that the
  // caller is currently constructing, since we may need to insert nodes into
  // the graph to create a value.
  virtual std::shared_ptr<SugaredValue> resolveValue(
      const std::string& name,
      Function& m,
      const SourceRange& loc) const = 0;

  // Resolve `name` to a TypePtr.
  virtual TypePtr resolveType(const std::string& name) const = 0;
};

// A resolver that only understands "torch.foo()" lookups.
struct NativeResolver : public Resolver {
  std::shared_ptr<SugaredValue> resolveValue(
      const std::string& name,
      Function& m,
      const SourceRange& loc) const override {
    if (name == "torch") {
      return std::make_shared<BuiltinModule>("aten");
    }
    return nullptr;
  }

  TypePtr resolveType(const std::string& name) const override {
    return ClassType::get(c10::QualifiedName(name));
  }
};

inline std::shared_ptr<NativeResolver> nativeResolver() {
  return std::make_shared<NativeResolver>();
}
} // namespace script
} // namespace jit
} // namespace torch
