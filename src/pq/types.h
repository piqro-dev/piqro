#pragma once

#include <base/common.h>

#include <base/string.h>

#include <base/arena.h>

//
// value
//

typedef enum : uint8_t
{
	VALUE_NULL,
	VALUE_NUMBER,
	VALUE_BOOLEAN,
	VALUE_STRING,
	VALUE_ARRAY,
} PQ_ValueType;

typedef struct PQ_Value PQ_Value;

struct PQ_Value 
{
	PQ_ValueType type;

	union
	{
		float n;
		bool b; 
		String s;
		
		struct 
		{
			struct PQ_Value* elements;
			uint16_t count; // immutable
		} a;
	};
};

#define pq_value_array(arena, N) ((PQ_Value){ VALUE_ARRAY, .a = { .elements = arena_push_array((arena), PQ_Value, (N)), .count = (N) } })

#define pq_value_null()          ((PQ_Value){ VALUE_NULL })

#define pq_value_number(v)       ((PQ_Value){ VALUE_NUMBER, .n = (float)(v) })

#define pq_value_boolean(v)      ((PQ_Value){ VALUE_BOOLEAN, .b = (bool)(v) })

#define pq_value_string(v)       ((PQ_Value){ VALUE_STRING, .s = v })

static inline const char* pq_value_to_c_str(const PQ_ValueType type)
{
	switch (type) 
	{
		case VALUE_NULL:    return "null";
		case VALUE_NUMBER:  return "number"; 
		case VALUE_STRING:  return "string"; 
		case VALUE_BOOLEAN: return "boolean";
		case VALUE_ARRAY:   return "array"; 

		default: return "unknown";
	}
}

static inline float pq_value_as_number(const PQ_Value v)
{
	switch (v.type) 
	{
		case VALUE_NULL:    return __builtin_nanf("");
		case VALUE_NUMBER:  return v.n;
		case VALUE_STRING:  return 0.0f;
		case VALUE_BOOLEAN: return (float)v.b;
		case VALUE_ARRAY:   return 0.0f;

		default: return 0.0f;
	}
}

static inline bool pq_value_as_boolean(const PQ_Value v)
{
	switch (v.type) 
	{
		case VALUE_NULL:    return false;
		case VALUE_NUMBER:  return (bool)v.n;
		case VALUE_STRING:  return false;
		case VALUE_BOOLEAN: return v.b;
		case VALUE_ARRAY:   return false;

		default: return false;
	}
}

static inline String pq_value_as_string(Arena* arena, const PQ_Value v)
{
	switch (v.type) 
	{
		case VALUE_NULL:    return str_copy(arena, s("null"));
		case VALUE_NUMBER:  return str_format(arena, "%f", v.n);
		case VALUE_STRING:  return str_copy_from_to(arena, v.s, 1, v.s.length - 1);
		case VALUE_BOOLEAN: return str_format(arena, "%s", v.b ? "true" : "false");
		case VALUE_ARRAY:   return (String){};

		default: return (String){};
	}
}

static inline bool pq_value_can_be_number(PQ_Value l)
{
	switch (l.type)
	{
		case VALUE_BOOLEAN: return true;
		case VALUE_NULL:    return true;
		case VALUE_NUMBER:  return true;

		default: return false;
	}
}

#define DEFINE_VALUE_OPERATIONS \
	OP(add, number, number, +) \
	OP(sub, number, number, -) \
	OP(div, number, number, /) \
	OP(mul, number, number, *) \
	\
	OP(and,     boolean, number, &&) \
	OP(or,      boolean, number, ||) \
	OP(greater, boolean, number, >) \
	OP(less,    boolean, number, <) \
	OP(gt,      boolean, number, >=) \
	OP(lt,      boolean, number, <=)

#define OP(name, ret_type, type, op) \
	static inline PQ_Value pq_value_##name(PQ_Value l, PQ_Value r) { return pq_value_##ret_type(pq_value_as_##type(l) op pq_value_as_##type(r)); }

DEFINE_VALUE_OPERATIONS

static inline PQ_Value pq_value_mod(PQ_Value l, PQ_Value r)
{
	return pq_value_number(__builtin_fmodf(pq_value_as_number(l), pq_value_as_number(r)));
}

static inline PQ_Value pq_value_equals(PQ_Value l, PQ_Value r)
{
	if (l.type == VALUE_STRING && r.type == VALUE_STRING)
	{
		return pq_value_boolean(str_equals(l.s, r.s));
	}

	if ((l.type == VALUE_STRING && r.type != VALUE_STRING) || (l.type != VALUE_STRING && r.type == VALUE_STRING))
	{
		return pq_value_boolean(false);
	}

	if (pq_value_can_be_number(l) && pq_value_can_be_number(r))
	{
		return pq_value_boolean(pq_value_as_number(l) == pq_value_as_number(r));
	}

	return pq_value_boolean(false);
}

static inline PQ_Value pq_value_not(PQ_Value l)
{
	return pq_value_boolean(!pq_value_as_boolean(l));
}

static inline PQ_Value pq_value_bw_or(PQ_Value l, PQ_Value r)
{
	return pq_value_number((float)((uint32_t)pq_value_as_number(l) | (uint32_t)pq_value_as_number(r)));
}

static inline PQ_Value pq_value_bw_and(PQ_Value l, PQ_Value r)
{
	return pq_value_number((float)((uint32_t)pq_value_as_number(l) & (uint32_t)pq_value_as_number(r)));
}

static inline PQ_Value pq_value_bw_xor(PQ_Value l, PQ_Value r)
{
	return pq_value_number((float)((uint32_t)pq_value_as_number(l) ^ (uint32_t)pq_value_as_number(r)));
}

static inline PQ_Value pq_value_bw_left_shift(PQ_Value l, PQ_Value r)
{
	return pq_value_number((float)((uint32_t)pq_value_as_number(l) << (uint32_t)pq_value_as_number(r)));
}

static inline PQ_Value pq_value_bw_right_shift(PQ_Value l, PQ_Value r)
{
	return pq_value_number((float)((uint32_t)pq_value_as_number(l) >> (uint32_t)pq_value_as_number(r)));
}

#undef OP
#undef DEFINE_VALUE_OPERATIONS

//
// instructions
//

#define DEFINE_INSTRUCTIONS \
	INST(CALL) \
	INST(LOAD_IMMEDIATE) \
	INST(LOAD_LOCAL) \
	INST(STORE_LOCAL) \
	INST(LOAD_GLOBAL) \
	INST(STORE_GLOBAL) \
	INST(LOAD_LOCAL_SUBSCRIPT) \
	INST(STORE_LOCAL_SUBSCRIPT) \
	INST(LOAD_GLOBAL_SUBSCRIPT) \
	INST(STORE_GLOBAL_SUBSCRIPT) \
	INST(LOAD_ARRAY) \
	INST(JUMP) \
	INST(JUMP_COND) \
	INST(LOAD_NULL) \
	INST(ADD) \
	INST(SUB) \
	INST(DIV) \
	INST(MUL) \
	INST(MOD) \
	INST(AND) \
	INST(OR) \
	INST(GREATER_THAN) \
	INST(LESS_THAN) \
	INST(EQUALS) \
	INST(GREATER) \
	INST(LESS) \
	INST(NOT) \
	INST(NEGATE) \
	INST(BW_OR) \
	INST(BW_AND) \
	INST(BW_XOR) \
	INST(BW_LEFT_SHIFT) \
	INST(BW_RIGHT_SHIFT) \
	INST(RETURN) \
	INST(HALT)

#define INST(name) INST_##name,

typedef enum : uint8_t
{
	DEFINE_INSTRUCTIONS
} PQ_InstructionType;

typedef struct
{
	PQ_InstructionType type;
	uint16_t arg;
} PQ_Instruction;

#undef INST
#define INST(name) case INST_##name: return #name;

static inline const char* pq_inst_to_c_str(const PQ_InstructionType type)
{
	switch (type)
	{
		DEFINE_INSTRUCTIONS
	}
	
	return "unknown";
}

#undef INST
#undef DEFINE_INSTRUCTIONS

// the DEFINE_INSTRUCTIONS macro above is sorted in that way so this check is very easily done
static inline bool pq_inst_needs_arg(const PQ_InstructionType type)
{
	return type >= INST_CALL && type <= INST_JUMP_COND;
}

//
// tokens
//

#define DEFINE_TOKENS \
	TOKEN(UNKNOWN,            "unknown") \
	\
	TOKEN(STRING,             "string literal") \
	TOKEN(NUMBER,             "number literal") \
	TOKEN(IDENTIFIER,         "identifier") \
	TOKEN(TRUE,               "`true`") \
	TOKEN(FALSE,              "`false`") \
	TOKEN(NULL,               "`null`") \
	\
	TOKEN(VAR,                "keyword `var`") \
	TOKEN(FOREVER,            "keyword `forever`") \
	TOKEN(REPEAT,             "keyword `repeat`") \
	TOKEN(UNTIL,              "keyword `until`") \
	TOKEN(DEFINE,             "keyword `define`") \
	TOKEN(RETURN,             "keyword `return`") \
	TOKEN(IF,                 "keyword `if`") \
	TOKEN(ELSE,               "keyword `else`") \
	TOKEN(BREAK,              "keyword `break`") \
	TOKEN(FOREIGN,            "keyword `foreign`") \
	\
	TOKEN(LESS,               "`<`") \
	TOKEN(GREATER,            "`>`") \
	TOKEN(LESS_THAN,          "`<=`") \
	TOKEN(GREATER_THAN,       "`>=`") \
	TOKEN(DOUBLE_EQUALS,      "`==`") \
	TOKEN(EXCLAMATION,        "`!`") \
	TOKEN(NOT_EQUALS,         "`!=`") \
	TOKEN(DOUBLE_AND,         "`&&`") \
	TOKEN(DOUBLE_PIPE,        "`||`") \
	\
	TOKEN(PERCENT,            "`%`") \
	TOKEN(PLUS,               "`+`") \
	TOKEN(DASH,               "`-`") \
	TOKEN(SLASH,              "`/`") \
	TOKEN(STAR,               "`*`") \
	TOKEN(LEFT_SHIFT,         "`<<`") \
	TOKEN(RIGHT_SHIFT,        "`>>") \
	TOKEN(PIPE,               "`|`") \
	TOKEN(AND,                "`&`") \
	TOKEN(CARET,              "`^`") \
	\
	TOKEN(EQUALS,             "`=`") \
	TOKEN(PERCENT_EQUALS,     "`%=`") \
	TOKEN(PLUS_EQUALS,        "`+=`") \
	TOKEN(DASH_EQUALS,        "`-=`") \
	TOKEN(SLASH_EQUALS,       "`/=`") \
	TOKEN(STAR_EQUALS,        "`*=`") \
	TOKEN(LEFT_SHIFT_EQUALS,  "`<<=`") \
	TOKEN(RIGHT_SHIFT_EQUALS, "`>>=`") \
	TOKEN(PIPE_EQUALS,        "`|=`") \
	TOKEN(AND_EQUALS,         "`&=`") \
	TOKEN(CARET_EQUALS,       "`^=`") \
	\
	TOKEN(OPEN_BRACE,         "`{`") \
	TOKEN(CLOSE_BRACE,        "`}`") \
	TOKEN(OPEN_PAREN,         "`(`") \
	TOKEN(CLOSE_PAREN,        "`)`") \
	TOKEN(OPEN_BOX,           "`[`") \
	TOKEN(CLOSE_BOX,          "`]`") \
	TOKEN(COMMA,              "`,`")

#define TOKEN(name, fancy_name) TOKEN_##name,

typedef enum : uint8_t
{
	DEFINE_TOKENS
} PQ_TokenType;

typedef struct
{
	PQ_TokenType type;

	uint16_t line; 
	uint32_t start;
	uint32_t end;  
} PQ_Token;

#undef TOKEN
#define TOKEN(name, fancy_name) case TOKEN_##name: return fancy_name;

static inline const char* pq_token_to_c_str(const PQ_TokenType type)
{
	switch (type)
	{
		DEFINE_TOKENS
	}
	
	return "unknown";
}

#undef TOKEN
#undef DEFINE_TOKENS

static inline String pq_token_as_string(Arena* arena, const PQ_Token t, String source)
{
	return str_copy_from_to(arena, source, t.start, t.end);
}

static inline bool pq_token_is_binary_op(const PQ_TokenType type)
{
	return type >= TOKEN_LESS && type <= TOKEN_CARET;
}

static inline bool pq_token_is_assign_op(const PQ_TokenType type)
{	
	return type >= TOKEN_EQUALS && type <= TOKEN_CARET_EQUALS; 
}

// the precedences mostly follow C's.
// see: https://en.cppreference.com/w/c/language/operator_precedence.html
// goes from lowest from highest level of precedence
static inline int8_t pq_token_precedence_of(const PQ_TokenType type)
{
	switch (type)
	{
		case TOKEN_DOUBLE_AND: return 0;
		case TOKEN_DOUBLE_PIPE: return 1;

		case TOKEN_PIPE: return 2;
		case TOKEN_CARET: return 3;
		case TOKEN_AND: return 4;

		case TOKEN_NOT_EQUALS:
		case TOKEN_DOUBLE_EQUALS: return 5;  

		case TOKEN_LESS:
		case TOKEN_GREATER:
		case TOKEN_LESS_THAN:
		case TOKEN_GREATER_THAN: return 6;

		case TOKEN_LEFT_SHIFT: 
		case TOKEN_RIGHT_SHIFT: return 7;

		case TOKEN_PLUS:
		case TOKEN_DASH: return 8;

		case TOKEN_PERCENT:
		case TOKEN_SLASH:
		case TOKEN_STAR: return 9;

		default: {}
	}

	return -1;
}

//
// compiled blob
//

typedef struct 
{
	uint8_t* buffer;
	uint16_t size;
} PQ_CompiledBlob;