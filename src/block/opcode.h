#pragma once

#include <core/core.h>

#include <block/value.h>

#define DEFINE_OPCODES \
	OPCODE(PRINT) \
	OPCODE(IF) \
	OPCODE(ELSE) \
	OPCODE(REPEAT) \
	OPCODE(PROC) \
	OPCODE(VALUE) \
	OPCODE(ADD) \
	OPCODE(SUBTRACT) \
	OPCODE(MULTIPLY) \
	OPCODE(DIVIDE) \
	OPCODE(EQUALS) \
	OPCODE(AND) \
	OPCODE(OR) \
	OPCODE(LESS) \
	OPCODE(GREATER) \
	OPCODE(LESS_THAN) \
	OPCODE(GREATER_THAN) \
	OPCODE(NOT) \
	OPCODE(SET_VAR) \
	OPCODE(GET_VAR) \
	OPCODE(PEN_DOWN) \
	OPCODE(PEN_UP) \
	OPCODE(PEN_GO_TO)

#define OPCODE(name) name,

struct Opcode
{
	enum OpcodeType : uint8_t
	{
		DEFINE_OPCODES
	};

	inline Opcode() = default;

	inline Opcode(OpcodeType type) 
		: m_type(type) {}

	inline Opcode(OpcodeType type, const Value& value) 
		: m_type(type), m_value(value) {}

	#undef OPCODE
	#define OPCODE(name) case OpcodeType::name: return #name; break;

	inline const char* to_string() const
	{
		switch (m_type) 
		{
			DEFINE_OPCODES
		}
	
		return "Unknown";
	}

	inline bool is_operation() const
	{
		return m_type >= Opcode::ADD && m_type <= Opcode::DIVIDE;
	}

	inline bool is_expression() const
	{ 
		return m_type >= Opcode::VALUE && m_type <= Opcode::GET_VAR;
	}
	
	inline bool is_value() const   
	{ 
		return m_type == Opcode::VALUE; 
	}
	
	inline bool is_number() const
	{
		return is_value() && m_value.type() == Value::NUMBER;
	}
	
	inline bool is_boolean() const
	{
		return is_value() && m_value.type() == Value::BOOLEAN;
	}
	
	inline bool is_string() const
	{
		return is_value() && m_value.type() == Value::STRING;
	}

	inline OpcodeType type() const
	{
		return m_type;
	}

	inline Value& value()
	{
		return m_value;
	}

private:
	OpcodeType m_type;
	Value m_value;
};
