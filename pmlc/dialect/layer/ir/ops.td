// Copyright 2020 Intel Corporation

include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

#ifndef OP_BASE
include "mlir/IR/OpBase.td"
#endif

def LayerDialect : Dialect {
  let name = "layer";
  let cppNamespace = "pmlc::dialect::layer";
}

class LayerOp<string mnemonic, list<Trait> traits = []>
    : Op<LayerDialect, mnemonic, traits> {
}


def BoxOp : LayerOp<"box", [IsolatedFromAbove]> {
  let summary = "layer container operation";
  let arguments = (ins
    Variadic<AnyType>:$operands,
    StrAttr:$op,
    OptionalAttr<DictionaryAttr>:$attrs);
  let results = (outs Variadic<AnyType>:$results);
  let regions = (region SizedRegion<1>:$body);

  let skipDefaultBuilders = 1;
  let builders = [
    OpBuilder<(ins
      "mlir::StringRef":$op,
      "mlir::ValueRange":$operands,
      "mlir::TypeRange":$resultTypes,
      "mlir::DictionaryAttr":$attrs
    )>
  ];

  let extraClassDeclaration = [{
    mlir::Block *getBody();
  }];
  
  let assemblyFormat = [{
    $op `(` $operands `)` attr-dict `:` type($operands) `->` type($results) $body
  }];
}

def ReturnOp : LayerOp<"return", [
  NoSideEffect,
  ReturnLike,
  Terminator,
  HasParent<"BoxOp">
]> {
  let summary = "Return results from a layer";
  let arguments = (ins Variadic<AnyType>:$operands);
  let assemblyFormat = "attr-dict ($operands^ `:` type($operands))?";
}
