#pragma once

#include <base/common.h>

static constexpr uint16_t PQ_MAX_TOKENS = 1 << 12;
static constexpr uint16_t PQ_MAX_INSTRUCTIONS = 1 << 12;
static constexpr uint16_t PQ_MAX_BLOB_SIZE = 2953;

static constexpr uint8_t PQ_MAX_PROCEDURES = 128;
static constexpr uint8_t PQ_MAX_SCOPES = 128;
static constexpr uint8_t PQ_MAX_VARIABLES = 128;

static constexpr uint8_t PQ_MAX_IMMEDIATES = 128;
static constexpr uint8_t PQ_MAX_CALL_FRAMES = PQ_MAX_SCOPES;
static constexpr uint8_t PQ_MAX_STACK_SIZE = 128;
static constexpr uint8_t PQ_MAX_LOCALS = PQ_MAX_VARIABLES;
static constexpr uint8_t PQ_MAX_GLOBALS = PQ_MAX_VARIABLES;