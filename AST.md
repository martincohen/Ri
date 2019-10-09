## Declarations

Each named declaration encountered during the parsing phase is stored into current scope's `decl` map (`String -> RiNode*`), which is then used to resolve identifiers (`RiNode_Id`) at a resolve phase. Each declaration node keeps it's resolving state, the name and a pointer to a prototype of the resolved node.

When we are resolving an identifier (`RiNode_Id`), we look into current scope (and parents if needed). If a declaration is found then we inspect it's `state` field:

- `RiDecl_Unresolved` -- The declaration's `prototype` is composed of potentially unresolved `RiNode_Id` nodes, and we need to resolve those first.
- `RiDecl_Resolving` -- The declaration is being resolved and we've re-entered the resolve function, which means the declaration is cyclic and we cannot continue.
- `RiDecl_Resolved` -- The declaration's `prototype` is resolved, and we can terminate early.

Once the field is resolved, we'll replace the `RiNode_Id` node with a shallow-copy `prototype`, but we'd keep the `pos` prooperty, so errors can show correct line/col position to the id node, and not point to the original declaration.

- `RiNode_Decl_Var`
- `RiNode_Decl_Func`
- `RiNode_Decl_Type`