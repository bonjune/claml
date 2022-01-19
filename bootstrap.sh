LLVM_CONFIG=llvm-config-12
LLVM_HOME=$($LLVM_CONFIG --prefix)
OPERATION_KIND_FILE=$LLVM_HOME/include/clang/AST/OperationKinds.def
BUILTIN_TYPES_FILE=$LLVM_HOME/include/clang/AST/BuiltinTypes.def
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
  # TODO: other types
  TARGET=src/builtinTypeKind.ml
  echo $HEADER >$TARGET
  echo "type t =" >>$TARGET
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

gen_config
gen_binary_operator_kind
gen_unary_operator_kind
gen_builtin_type_kind
gen_stmt_kind
