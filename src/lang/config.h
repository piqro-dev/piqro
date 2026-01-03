#pragma once

#include <base/common.h>

// Generator related
static constexpr uint16_t MAX_IDENTIFIER_NAME_LENGTH = 128;
static constexpr uint16_t MAX_PROCEDURES = 64;
static constexpr uint16_t MAX_SCOPES = 32;

// VM related
static constexpr uint16_t MAX_CALL_FRAMES = MAX_SCOPES;
static constexpr uint16_t MAX_IMMEDIATES = 128;
static constexpr uint16_t MAX_VARIABLES = 128;
static constexpr uint16_t MAX_STACK_SIZE = 128;