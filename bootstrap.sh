LLVM_CONFIG=llvm-config-12
LLVM_HOME=$($LLVM_CONFIG --prefix)
OPERATION_KIND_FILE=$LLVM_HOME/include/clang/AST/OperationKinds.def
OPENCL_IMAGE_TYPES_FILE=$LLVM_HOME/include/clang/Basic/OpenCLImageTypes.def
OPENCL_EXTENSION_TYPES_FILE=$LLVM_HOME/include/clang/Basic/OpenCLExtensionTypes.def
SVE_TYPES_FILE=$LLVM_HOME/include/clang/Basic/AArch64SVEACLETypes.def
PPC_TYPES_FILE=$LLVM_HOME/include/clang/Basic/PPCTypes.def
BUILTIN_TYPES_FILE=$LLVM_HOME/include/clang/AST/BuiltinTypes.def
ATTR_LIST_FILE=$LLVM_HOME/include/clang/Basic/AttrList.inc
STMT_NODES_FILE=$LLVM_HOME/include/clang/AST/StmtNodes.inc

HEADER="(* This file is automatically generated by bootstrap.sh *)"

LLVM_LIBS="-lclangFrontend -lclangDriver -lclangSerialization -lclangParse -lclangSema -lclangAnalysis -lclangARCMigrate -lclangRewrite -lclangEdit -lclangAST -lclangLex -lclangBasic"

gen_config() {
  mkdir -p config
  printf "(%s)" "$($LLVM_CONFIG --cflags)" >config/cflags.sexp
  printf "(%s)" "$($LLVM_CONFIG --ldflags)" >config/ldflags.sexp
  printf "(%s %s)" "$LLVM_LIBS" "$($LLVM_CONFIG --libs)" >config/archives.sexp
}

gen_binary_operator_kind() {
  TARGET=src/binaryOperatorKind.ml
  echo $HEADER >$TARGET
  echo "type t =" >>$TARGET
  for line in $(grep "^BINARY_OPERATION" $OPERATION_KIND_FILE | cut -f 2 -d '(' | cut -f 1 -d ','); do
    echo "  | $line" >>$TARGET
  done
  echo "[@@deriving show]" >>$TARGET
}

gen_unary_operator_kind() {
  TARGET=src/unaryOperatorKind.ml
  echo $HEADER >$TARGET
  echo "type t =" >>$TARGET
  for line in $(grep "^UNARY_OPERATION" $OPERATION_KIND_FILE | cut -f 2 -d '(' | cut -f 1 -d ','); do
    echo "  | $line" >>$TARGET
  done
  echo "[@@deriving show]" >>$TARGET
}

gen_builtin_type_kind() {
  TARGET=src/builtinTypeKind.ml
  echo $HEADER >$TARGET
  echo "type t =" >>$TARGET
  for line in $(grep "^IMAGE_READ_TYPE" $OPENCL_IMAGE_TYPES_FILE | cut -d ',' -f2); do
    echo "  | ${line}RO" >>$TARGET
  done
  for line in $(grep "^IMAGE_WRITE_TYPE" $OPENCL_IMAGE_TYPES_FILE | cut -d ',' -f2); do
    echo "  | ${line}WO" >>$TARGET
  done
  for line in $(grep "^IMAGE_READ_WRITE_TYPE" $OPENCL_IMAGE_TYPES_FILE | cut -d ',' -f2); do
    echo "  | ${line}RW" >>$TARGET
  done
  for line in $(grep "^INTEL_SUBGROUP_AVC_TYPE" $OPENCL_EXTENSION_TYPES_FILE | cut -d ',' -f2 | cut -d ')' -f1); do
    echo "  | $line" >>$TARGET
  done
  for line in $(grep "^SVE_VECTOR_TYPE\|^SVE_PREDICATE_TYPE" $SVE_TYPES_FILE | cut -d ',' -f3); do
    echo "  | $line" >>$TARGET
  done
  for line in $(grep "^PPC_VECTOR_" $PPC_TYPES_FILE | cut -d ',' -f2); do
    echo "  | $line" >>$TARGET
  done
  for line in $(grep "^BUILTIN_TYPE\|^SIGNED_TYPE\|^UNSIGNED_TYPE\|^FLOATING_TYPE\|^PLACEHOLDER_TYPE\|^SHARED_SINGLETON_TYPE" $BUILTIN_TYPES_FILE | sed -e "s/SHARED_SINGLETON_TYPE(//g" | cut -d '(' -f2 | cut -d ',' -f1); do
    echo "  | $line" >>$TARGET
  done
  echo "[@@deriving show]" >>$TARGET
}

gen_stmt_kind() {
  TARGET=src/stmtKind.ml
  echo $HEADER >$TARGET
  echo "type t =" >>$TARGET
  echo "  | NoStmtClass" >>$TARGET
  for line in $(grep "^[^#^/^|^\\]" $STMT_NODES_FILE | grep -v "STMT_RANGE" | grep -v "ABSTRACT_STMT" | cut -d '(' -f2 | cut -d ',' -f1); do
    echo "  | $line" >>$TARGET
  done
  echo "[@@deriving show]" >>$TARGET
}

gen_attr_kind() {
  TARGET=src/attrKind.ml
  echo $HEADER >$TARGET
  echo "type t =" >>$TARGET
  for line in $(grep "^TYPE_ATTR\|^STMT_ATTR\|^DECL_OR_STMT_ATTR\|^INHERITABLE_ATTR\|^DECL_OR_TYPE_ATTR\|^INHERITABLE_PARAM_ATTR\|^PARAMETER_ABI_ATTR" $ATTR_LIST_FILE | cut -d '(' -f2 | cut -d ')' -f1); do
    echo "  | $line" >>$TARGET
  done
  echo "[@@deriving show]" >>$TARGET
}

gen_test() {
  TARGET=test/dune
  echo "(executable
 (name test)
 (modules test)
 (libraries clang))
 " >$TARGET
  for cfile in $(find test -name "*.c"); do
    cfile=$(basename $cfile)
    echo "(rule
 (deps $cfile)
 (targets ${cfile%%.*}.output)
 (action
  (with-stdout-to
   %{targets}
   (pipe-stdout
    (ignore-stderr
     (run ./test.exe %{deps}))
    (run clang-format)))))

(rule
 (alias runtest)
 (action
  (diff ${cfile%%.*}.expected ${cfile%%.*}.output)))
" >>$TARGET
  done
}

gen_config
gen_binary_operator_kind
gen_unary_operator_kind
gen_builtin_type_kind
gen_stmt_kind
gen_attr_kind
gen_test
