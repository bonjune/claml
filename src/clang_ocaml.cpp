#include "clang/AST/DeclBase.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/CompilerInstance.h"

#include "caml/alloc.h"
#include "caml/callback.h"
#include "caml/custom.h"
#include "caml/fail.h"
#include "caml/memory.h"
#include "caml/mlvalues.h"

#include "utils.h"

extern "C" {
value clang_parse_file(value args) {
  CAMLparam1(args);
  CAMLlocal1(v);
  int num_args = Wosize_val(args);
  int i;
  char const **clang_args =
      (char const **)malloc(num_args * sizeof(const char *const));
  for (i = 0; i < num_args; i++) {
    clang_args[i] = String_val(Field(args, i));
  }
  clang::IntrusiveRefCntPtr<clang::DiagnosticsEngine> Diags =
      clang::CompilerInstance::createDiagnostics(
          new clang::DiagnosticOptions());
  clang::FileSystemOptions FileSystemOpts;
  llvm::SmallVector<const char *, 4> Args;
  Args.push_back("clang");
  Args.push_back(clang_args[0]);
  std::shared_ptr<clang::PCHContainerOperations> PCHContainerOps =
      std::make_shared<clang::PCHContainerOperations>();
  clang::ASTUnit *Unit(clang::ASTUnit::LoadFromCommandLine(
      Args.data(), Args.data() + Args.size(), PCHContainerOps, Diags, ""));
  clang::TranslationUnitDecl *TU =
      Unit->getASTContext().getTranslationUnitDecl();
  v = caml_alloc(0, Abstract_tag);
  *((clang::TranslationUnitDecl **)Data_abstract_val(v)) = TU;
  CAMLreturn(v);
}

void clang_dump_translation_unit(value TU) {
  CAMLparam1(TU);
  clang::TranslationUnitDecl *TTU =
      *((clang::TranslationUnitDecl **)Data_abstract_val(TU));
  TTU->dump();
  CAMLreturn0;
}

value clang_decls_begin(value TU) {
  CAMLparam1(TU);
  CAMLlocal1(v);
  clang::TranslationUnitDecl *TTU =
      *((clang::TranslationUnitDecl **)Data_abstract_val(TU));
  v = caml_alloc(1, Abstract_tag);
  clang::Decl *D = *(TTU->decls_begin());
  if (D) {
    *((clang::Decl **)Data_abstract_val(v)) = D;
    CAMLreturn(caml_alloc_some(v));
  } else {
    CAMLreturn(Val_none);
  }
}

value clang_decls_succ(value Decl) {
  CAMLparam1(Decl);
  CAMLlocal1(v);
  clang::Decl *TTU = *((clang::Decl **)Data_abstract_val(Decl));
  v = caml_alloc(1, Abstract_tag);
  clang::Decl *Next = TTU->getNextDeclInContext();
  if (Next) {
    *((clang::Decl **)Data_abstract_val(v)) = Next;
    CAMLreturn(caml_alloc_some(v));
  } else {
    CAMLreturn(Val_none);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Begin Decl
////////////////////////////////////////////////////////////////////////////////

WRAPPER_INT(clang_decl_get_kind, Decl, getKind)

value clang_decl_get_kind_name(value Decl) {
  CAMLparam1(Decl);
  clang::Decl *D = *((clang::Decl **)Data_abstract_val(Decl));
  CAMLreturn(clang_to_string(D->getDeclKindName()));
}

void clang_decl_dump(value TU) {
  CAMLparam1(TU);
  clang::Decl *TTU = *((clang::Decl **)Data_abstract_val(TU));
  TTU->dump();
  CAMLreturn0;
}

value clang_decl_is_implicit(value Decl) {
  CAMLparam1(Decl);
  CAMLlocal1(R);
  clang::Decl *D = *((clang::Decl **)Data_abstract_val(Decl));
  CAMLreturn(Val_bool(D->isImplicit()));
}

WRAPPER_LIST_WITH_IDX(clang_function_decl_get_params, FunctionDecl, ParmVarDecl,
                      getNumParams, getParamDecl)

value clang_function_decl_return_type(value Decl) {
  CAMLparam1(Decl);
  clang::Decl *D = *((clang::Decl **)Data_abstract_val(Decl));
  if (clang::FunctionDecl *FD = llvm::dyn_cast<clang::FunctionDecl>(D)) {
    CAMLreturn(clang_to_qual_type(FD->getReturnType()));
  } else {
    assert(false);
  }
}

value clang_decl_is_value_decl(value Decl) {
  CAMLparam1(Decl);
  clang::Decl *D = *((clang::Decl **)Data_abstract_val(Decl));
  if (clang::ValueDecl *VD = llvm::dyn_cast<clang::ValueDecl>(D)) {
    CAMLreturn(Val_int(1));
  } else {
    CAMLreturn(Val_int(0));
  }
}

value clang_function_decl_has_body(value Decl) {
  CAMLparam1(Decl);
  clang::Decl *D = *((clang::Decl **)Data_abstract_val(Decl));
  if (clang::FunctionDecl *FD = llvm::dyn_cast<clang::FunctionDecl>(D)) {
    CAMLreturn(Val_bool(FD->hasBody()));
  } else {
    assert(false);
  }
}

value clang_function_decl_get_body(value Decl) {
  CAMLparam1(Decl);
  CAMLlocal1(R);
  clang::Decl *D = *((clang::Decl **)Data_abstract_val(Decl));
  if (clang::FunctionDecl *FD = llvm::dyn_cast<clang::FunctionDecl>(D)) {
    R = caml_alloc(1, Abstract_tag);
    *((const clang::Stmt **)Data_abstract_val(R)) = FD->getBody();
    CAMLreturn(R);
  } else {
    assert(false);
  }
}

value clang_record_decl_is_anonymous(value Decl) {
  CAMLparam1(Decl);
  clang::RecordDecl *D = *((clang::RecordDecl **)Data_abstract_val(Decl));
  CAMLreturn(Val_bool(D->isAnonymousStructOrUnion()));
}

value clang_record_decl_is_struct(value Decl) {
  CAMLparam1(Decl);
  clang::RecordDecl *D = *((clang::RecordDecl **)Data_abstract_val(Decl));
  CAMLreturn(Val_bool(D->isStruct()));
}

value clang_record_decl_is_union(value Decl) {
  CAMLparam1(Decl);
  clang::RecordDecl *D = *((clang::RecordDecl **)Data_abstract_val(Decl));
  CAMLreturn(Val_bool(D->isUnion()));
}

value clang_record_decl_field_begin(value Decl) {
  CAMLparam1(Decl);
  CAMLlocal1(R);
  clang::RecordDecl *RD = *((clang::RecordDecl **)Data_abstract_val(Decl));
  if (RD->field_empty())
    CAMLreturn(Val_none);
  clang::FieldDecl *FD = *(RD->field_begin());
  if (FD) {
    R = caml_alloc(1, Abstract_tag);
    *((clang::FieldDecl **)Data_abstract_val(R)) = FD;
    CAMLreturn(caml_alloc_some(R));
  } else {
    CAMLreturn(Val_none);
  }
}

value clang_record_decl_field_list_internal(value Decl) {
  CAMLparam1(Decl);
  CAMLlocal3(Hd, Tl, R);
  clang::RecordDecl *RD = *((clang::RecordDecl **)Data_abstract_val(Decl));
  R = caml_alloc(1, Abstract_tag);
  Tl = Val_int(0);
  for (auto i = RD->field_begin(); i != RD->field_end(); i++) {
    Hd = caml_alloc(1, Abstract_tag);
    *((clang::FieldDecl **)Data_abstract_val(Hd)) = *i;
    value Tmp = caml_alloc(2, Abstract_tag);
    Field(Tmp, 0) = Hd;
    Field(Tmp, 1) = Tl;
    Tl = Tmp;
  }
  CAMLreturn(Tl);
}

value clang_decl_get_name(value Decl) {
  CAMLparam1(Decl);
  clang::Decl *D = *((clang::Decl **)Data_abstract_val(Decl));
  if (clang::NamedDecl *VD = llvm::dyn_cast<clang::NamedDecl>(D)) {
    CAMLreturn(clang_to_string(VD->getName().data()));
  } else {
    assert(false);
  }
}

value clang_decl_get_type(value Decl) {
  CAMLparam1(Decl);
  clang::Decl *D = *((clang::Decl **)Data_abstract_val(Decl));
  if (clang::ValueDecl *VD = llvm::dyn_cast<clang::ValueDecl>(D)) {
    CAMLreturn(clang_to_qual_type(VD->getType()));
  } else {
    assert(false);
  }
}

value clang_source_location(value decl) {
  CAMLparam1(decl);
  CAMLlocal1(result);
  clang::Decl *d = *((clang::Decl **)Data_abstract_val(decl));
  const clang::PresumedLoc &loc =
      d->getASTContext().getSourceManager().getPresumedLoc(d->getLocation());
  if (loc.isValid()) {
    result = caml_alloc(3, 0);
    Store_field(result, 0, clang_to_string(loc.getFilename()));
    Store_field(result, 1, Val_int(loc.getLine()));
    Store_field(result, 2, Val_int(loc.getColumn()));
    CAMLreturn(caml_alloc_some(result));
  } else {
    CAMLreturn(Val_none);
  }
}

value clang_decl_storage_class(value Decl) {
  CAMLparam1(Decl);
  clang::Decl *D = *((clang::Decl **)Data_abstract_val(Decl));
  if (clang::FunctionDecl *FD = llvm::dyn_cast<clang::FunctionDecl>(D)) {
    CAMLreturn(Val_int(FD->getStorageClass()));
  } else if (clang::VarDecl *VD = llvm::dyn_cast<clang::VarDecl>(D)) {
    CAMLreturn(Val_int(VD->getStorageClass()));
  } else {
    CAMLreturn(Val_int(clang::SC_None));
  }
}

value clang_vardecl_has_init(value VarDecl) {
  CAMLparam1(VarDecl);
  clang::VarDecl *VD = *((clang::VarDecl **)Data_abstract_val(VarDecl));
  CAMLreturn(Val_bool(VD->hasInit()));
}

value clang_vardecl_get_init(value VarDecl) {
  CAMLparam1(VarDecl);
  CAMLlocal1(R);
  clang::VarDecl *VD = *((clang::VarDecl **)Data_abstract_val(VarDecl));
  R = caml_alloc(1, Abstract_tag);
  if (VD->hasInit()) {
    *((clang::Expr **)Data_abstract_val(R)) = VD->getInit();
    CAMLreturn(caml_alloc_some(R));
  } else {
    CAMLreturn(Val_none);
  }
}

WRAPPER_PTR(clang_goto_stmt_get_label, GotoStmt, LabelDecl, getLabel)

WRAPPER_PTR(clang_if_stmt_get_cond, IfStmt, Expr, getCond)

WRAPPER_PTR(clang_if_stmt_get_then, IfStmt, Stmt, getThen)

WRAPPER_PTR(clang_if_stmt_get_else, IfStmt, Stmt, getElse)

value clang_if_stmt_has_else_storage(value Stmt) {
  LOG("begin clang_if_has_else_storage");
  CAMLparam1(Stmt);
  clang::IfStmt *S = *((clang::IfStmt **)Data_abstract_val(Stmt));
  LOG("end clang_if_has_else_storage");
  CAMLreturn(Val_bool(S->hasElseStorage()));
}

value clang_label_stmt_get_name(value Stmt) {
  CAMLparam1(Stmt);
  clang::LabelStmt *S = *((clang::LabelStmt **)Data_abstract_val(Stmt));
  CAMLreturn(clang_to_string(S->getName()));
}

WRAPPER_PTR(clang_label_stmt_get_sub_stmt, LabelStmt, Stmt, getSubStmt)

WRAPPER_PTR(clang_while_stmt_get_cond, WhileStmt, Expr, getCond)

WRAPPER_PTR(clang_while_stmt_get_body, WhileStmt, Stmt, getBody)

value clang_typedef_decl_get_underlying_type(value Decl) {
  CAMLparam1(Decl);
  clang::TypedefDecl *TD = *((clang::TypedefDecl **)Data_abstract_val(Decl));
  CAMLreturn(clang_to_qual_type(TD->getUnderlyingType()));
}

value clang_enum_decl_get_enums(value T) {
  CAMLparam1(T);
  CAMLlocal4(Hd, Tl, AT, PT);
  clang::EnumDecl *D = *((clang::EnumDecl **)Data_abstract_val(T));
  Tl = Val_int(0);
  for (auto i = D->enumerator_begin(); i != D->enumerator_end(); i++) {
    Hd = caml_alloc(1, Abstract_tag);
    *((const clang::EnumConstantDecl **)Data_abstract_val(Hd)) = *i;
    value Tmp = caml_alloc(2, Abstract_tag);
    Field(Tmp, 0) = Hd;
    Field(Tmp, 1) = Tl;
    Tl = Tmp;
  }
  CAMLreturn(Tl);
}
////////////////////////////////////////////////////////////////////////////////
// End Decl
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Expr
////////////////////////////////////////////////////////////////////////////////

WRAPPER_INT(clang_stmt_get_kind, Stmt, getStmtClass)

value clang_stmt_get_kind_name(value Expr) {
  CAMLparam1(Expr);
  clang::Expr *E = *((clang::Expr **)Data_abstract_val(Expr));
  CAMLreturn(clang_to_string(E->getStmtClassName()));
}

value clang_integer_literal_to_int(value Expr) {
  CAMLparam1(Expr);
  clang::IntegerLiteral *E =
      *((clang::IntegerLiteral **)Data_abstract_val(Expr));
  llvm::APInt V = E->getValue();
  unsigned int Bit = V.getBitWidth();
  if (V.isSignedIntN(Bit)) {
    CAMLreturn(caml_copy_int64(V.getSExtValue()));
  } else {
    CAMLreturn(caml_copy_int64(V.getZExtValue()));
  }
}

WRAPPER_INT(clang_character_literal_get_kind, CharacterLiteral, getKind)

value clang_string_literal_get_string(value Expr) {
  CAMLparam1(Expr);
  clang::StringLiteral *E = *((clang::StringLiteral **)Data_abstract_val(Expr));
  CAMLreturn(clang_to_string(E->getString().data()));
}

WRAPPER_INT(clang_character_literal_get_value, CharacterLiteral, getValue)

value clang_floating_literal_to_float(value Expr) {
  CAMLparam1(Expr);
  clang::FloatingLiteral *E =
      *((clang::FloatingLiteral **)Data_abstract_val(Expr));
  llvm::APFloat V = E->getValue();
  CAMLreturn(caml_copy_double(V.convertToDouble()));
}

WRAPPER_INT(clang_cast_kind, CastExpr, getCastKind)

value clang_cast_kind_name(value Expr) {
  CAMLparam1(Expr);
  clang::CastExpr *E = *((clang::CastExpr **)Data_abstract_val(Expr));
  CAMLreturn(clang_to_string(E->getCastKindName()));
}

WRAPPER_PTR(clang_cast_sub_expr, CastExpr, Expr, getSubExpr)

value clang_expr_get_type(value Expr) {
  CAMLparam1(Expr);
  LOG("begin clang_expr_get_type");
  clang::Expr *E = *((clang::ExplicitCastExpr **)Data_abstract_val(Expr));
  LOG("end clang_expr_get_type");
  CAMLreturn(clang_to_qual_type(E->getType()));
}

value clang_compound_stmt_body_begin(value Stmt) {
  CAMLparam1(Stmt);
  CAMLlocal1(R);
  clang::CompoundStmt *S = *((clang::CompoundStmt **)Data_abstract_val(Stmt));
  R = caml_alloc(1, Abstract_tag);
  clang::Stmt *D = *(S->body_begin());
  if (D) {
    *((clang::Stmt **)Data_abstract_val(R)) = D;
    CAMLreturn(caml_alloc_some(R));
  } else {
    CAMLreturn(Val_none);
  }
}

value clang_compound_stmt_body_succ(value Stmt) {
  /* TODO */
  CAMLparam1(Stmt);
  CAMLreturn(Val_none);
}

WRAPPER_LIST_WITH_REV_ITER(clang_compound_stmt_body_list, CompoundStmt, Stmt,
                           body_rbegin, body_rend)

WRAPPER_LIST_WITH_REV_ITER(clang_decl_stmt_decl_list, DeclStmt, Decl,
                           decl_rbegin, decl_rend)

value clang_return_stmt_get_ret_value(value Stmt) {
  CAMLparam1(Stmt);
  CAMLlocal1(R);
  clang::ReturnStmt *S = *((clang::ReturnStmt **)Data_abstract_val(Stmt));
  R = caml_alloc(1, Abstract_tag);
  clang::Expr *D = S->getRetValue();
  if (D) {
    *((clang::Stmt **)Data_abstract_val(R)) = D;
    CAMLreturn(caml_alloc_some(R));
  } else {
    CAMLreturn(Val_none);
  }
}

WRAPPER_INT(clang_binary_operator_kind, BinaryOperator, getOpcode)

value clang_binary_operator_kind_name(value Expr) {
  CAMLparam1(Expr);
  clang::BinaryOperator *E =
      *((clang::BinaryOperator **)Data_abstract_val(Expr));
  CAMLreturn(clang_to_string(E->getOpcodeStr().data()));
}

WRAPPER_PTR(clang_binary_operator_get_lhs, BinaryOperator, Expr, getLHS)

WRAPPER_PTR(clang_binary_operator_get_rhs, BinaryOperator, Expr, getRHS)

WRAPPER_INT(clang_unary_operator_kind, UnaryOperator, getOpcode)

WRAPPER_PTR(clang_unary_operator_get_sub_expr, UnaryOperator, Expr, getSubExpr)

WRAPPER_PTR(clang_decl_ref_get_decl, DeclRefExpr, ValueDecl, getDecl)

WRAPPER_INT(clang_unary_expr_or_type_trait_expr_get_kind,
            UnaryExprOrTypeTraitExpr, getKind)

value clang_unary_expr_or_type_trait_expr_is_argument_type(value Expr) {
  CAMLparam1(Expr);
  CAMLlocal1(R);
  clang::UnaryExprOrTypeTraitExpr *E =
      *((clang::UnaryExprOrTypeTraitExpr **)Data_abstract_val(Expr));
  CAMLreturn(Val_bool(E->isArgumentType()));
}

WRAPPER_PTR(clang_unary_expr_or_type_trait_expr_get_argument_expr,
            UnaryExprOrTypeTraitExpr, Expr, getArgumentExpr)

value clang_unary_expr_or_type_trait_expr_get_argument_type(value Expr) {
  CAMLparam1(Expr);
  clang::UnaryExprOrTypeTraitExpr *S =
      *((clang::UnaryExprOrTypeTraitExpr **)Data_abstract_val(Expr));
  CAMLreturn(clang_to_qual_type(S->getArgumentType()));
}

WRAPPER_PTR(clang_member_expr_get_base, MemberExpr, Expr, getBase)

WRAPPER_PTR(clang_member_expr_get_member_decl, MemberExpr, ValueDecl,
            getMemberDecl)

value clang_member_expr_is_arrow(value Expr) {
  CAMLparam1(Expr);
  CAMLlocal1(R);
  clang::MemberExpr *E = *((clang::MemberExpr **)Data_abstract_val(Expr));
  CAMLreturn(Val_bool(E->isArrow()));
}

WRAPPER_PTR(clang_paren_expr_get_sub_expr, ParenExpr, Expr, getSubExpr)

WRAPPER_PTR(clang_call_expr_get_callee, CallExpr, Expr, getCallee)

WRAPPER_LIST_WITH_IDX(clang_call_expr_get_args, CallExpr, Expr, getNumArgs,
                      getArg)

WRAPPER_PTR(clang_array_subscript_expr_get_base, ArraySubscriptExpr, Expr,
            getBase)

WRAPPER_PTR(clang_array_subscript_expr_get_idx, ArraySubscriptExpr, Expr,
            getIdx)

WRAPPER_PTR(clang_va_arg_expr_get_sub_expr, VAArgExpr, Expr, getSubExpr)

WRAPPER_LIST_WITH_IDX(clang_init_list_expr_get_inits, InitListExpr, Expr,
                      getNumInits, getInit)

#define CLANG_DECL_KIND(Kind)                                                  \
  value clang_decl_kind_##Kind() {                                             \
    CAMLparam0();                                                              \
    CAMLreturn(Val_int(clang::Decl::Kind));                                    \
  }

CLANG_DECL_KIND(AccessSpec)
CLANG_DECL_KIND(Block)
CLANG_DECL_KIND(Typedef)
}
