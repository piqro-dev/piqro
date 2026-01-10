#pragma once

#include <base/common.h>

static constexpr uint16_t MAX_TOKENS = 1 << 12;
static constexpr uint16_t MAX_INSTRUCTIONS = 1 << 11;
static constexpr uint16_t MAX_BLOB_SIZE = 2953;

static constexpr uint8_t MAX_IDENTIFIER_NAME_LENGTH = 128;
static constexpr uint8_t MAX_PROCEDURES = 64;
static constexpr uint8_t MAX_SCOPES = 64;

static constexpr uint8_t MAX_CALL_FRAMES = MAX_SCOPES;
static constexpr uint8_t MAX_LOCALS = 128;
static constexpr uint8_t MAX_IMMEDIATES = 128;
static constexpr uint8_t MAX_STACK_SIZE = 128;