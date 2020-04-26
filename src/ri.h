#pragma once

#include "ri-common.h"
#include "ri-ast.h"

typedef enum RiErrorKind RiErrorKind;
typedef struct RiError RiError;

typedef struct RiModule RiModule;
typedef struct Ri Ri;

//
//
//

// TODO: Separate by phase.
enum RiErrorKind {
    RiError_None,
    RiError_ModuleNotFound,
    RiError_UnexpectedCharacter,
    RiError_UnexpectedToken,
    RiError_UnexpectedType,
    RiError_UnexpectedStatement,
    RiError_UnexpectedExpression,
    RiError_UnexpectedValue,
    RiError_UnterminatedString,
    RiError_Declared,
    RiError_NotDeclared,
    RiError_CyclicDeclaration,
    RiError_CyclicType,
    RiError_Type,
    RiError_UnknownType,
    RiError_Argument
};

struct RiError {
    RiErrorKind kind;
    RiPos pos;
    CoCharArray message;
    CoCharArray path;
};

//
//
//

struct RiModule
{
    // Path from which we're loading the module.
    CoString path;
    // Storage for all module's data.
    CoArena arena;
    // Root node of the module.
    RiNode* node;

    // State:

    // Current func. Used during resolving.
    RiNode* func;
    // CoArray for functions to resolve next.
    RiNodeArray pending;
};

typedef CoSlice(RiModule*) RiModuleSlice;
typedef coarray_from_slice(RiModuleSlice) RiModuleArray;

//
//
//

struct Ri
{
    // Paths for loading modules.
    CoArray(CoString) paths;

    // Storage for globals
    CoArena arena_;
    // Storage for all ids.
    CoIntern intern;
    // Tracks all types.
    RiNodeArray types;

    // Error that caused the load to fail.
    RiError error;
    // Root scope.
    RiNode* scope;
    // Currently loaded module.
    RiModule* module;

    // Maps id to RiModule*.
    CoMap modules_map;
    // Tracks all modules.
    RiModuleArray modules;

    bool debug_tokens;

    const char* id_func;
    const char* id_var;
    const char* id_const;
    const char* id_type;
    const char* id_struct;
    const char* id_union;
    const char* id_enum;

    const char* id_import;

    const char* id_return;
    const char* id_if;
    const char* id_else;
    const char* id_for;
    const char* id_switch;
    const char* id_case;
    const char* id_default;
    const char* id_break;
    const char* id_continue;
    const char* id_fallthrough;

    const char* id_true;
    const char* id_false;
    const char* id_nil;

    const char* id_underscore;

    RiNode* nodes[RiNode_COUNT__];
};

//
//
//

bool ri_scope_set_(Ri* ri, RiNode* scope, RiNode* decl);
CoString ri_make_id_r_(Ri* ri, char* start, char* end);
CoString ri_make_id_(Ri* ri, CoString string);
void ri_error_set_(Ri* ri, RiErrorKind kind, RiPos pos, const char* format, ...);
void ri_error_format_(Ri* ri, CoCharArray* buffer);

//
//
//

void ri_init(Ri* ri, void* host);
void ri_purge(Ri* ri);

// Loads a file into buffer, stores path where it has been found to `path`.
bool ri_load(Ri* ri, CoString rel, CoCharArray* o_path, CoByteSlice* o_stream);
// Allocates and initializes a module.
RiModule* ri_add(Ri* ri, CoString path);
// Parses the module AST from `stream` passed.
bool ri_parse(Ri* ri, RiModule* module, CoByteSlice stream);
// Resolves module.
bool ri_resolve(Ri* ri, RiModule* module);

// Loads, initializes, parses and resolves the module.
RiModule* ri_import(Ri* ri, CoString rel);

// If error has happened it'll print the error out.
void ri_error_log(Ri* ri);