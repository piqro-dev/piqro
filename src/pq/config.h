#pragma once

#include <base/common.h>

static constexpr uint16_t PQ_MAX_TOKENS = 1 << 12;
static constexpr uint16_t PQ_MAX_INSTRUCTIONS = 1 << 12;
static constexpr uint16_t PQ_MAX_BLOB_SIZE = 2953;

static constexpr uint16_t PQ_MAX_PROCEDURES = 256;
static constexpr uint16_t PQ_MAX_SCOPES = 256;

static constexpr uint16_t PQ_MAX_LOCALS = 512;
static constexpr uint16_t PQ_MAX_GLOBALS = 512;

static constexpr uint16_t PQ_MAX_IMMEDIATES = 256;
static constexpr uint16_t PQ_MAX_CALL_FRAMES = PQ_MAX_SCOPES;
static constexpr uint16_t PQ_MAX_STACK_SIZE = 256;