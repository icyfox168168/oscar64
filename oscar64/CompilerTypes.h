#pragma once

#include "MachineTypes.h"

static const uint64 COPT_OPTIMIZE_BASIC = 0x00000001;
static const uint64 COPT_OPTIMIZE_INLINE = 0x00000002;

static const uint64 COPT_OPTIMIZE_AUTO_INLINE = 0x00000010;
static const uint64 COPT_OPTIMIZE_AUTO_INLINE_ALL = 0x00000020;

static const uint64 COPT_TARGET_PRG = 0x100000000ULL;
static const uint64 COPT_TARGET_CRT16 = 0x200000000ULL;
static const uint64 COPT_TARGET_CRT512 = 0x400000000ULL;
static const uint64 COPT_TARGET_COPY = 0x800000000ULL;

static const uint64 COPT_NATIVE = 0x01000000;

static const uint64 COPT_DEFAULT = COPT_OPTIMIZE_BASIC | COPT_OPTIMIZE_INLINE;

static const uint64 COPT_OPTIMIZE_DEFAULT = COPT_OPTIMIZE_BASIC | COPT_OPTIMIZE_INLINE;

static const uint64 COPT_OPTIMIZE_SIZE = COPT_OPTIMIZE_BASIC | COPT_OPTIMIZE_INLINE;

static const uint64 COPT_OPTIMIZE_SPEED = COPT_OPTIMIZE_BASIC | COPT_OPTIMIZE_INLINE | COPT_OPTIMIZE_AUTO_INLINE;

static const uint64 COPT_OPTIMIZE_ALL = COPT_OPTIMIZE_BASIC | COPT_OPTIMIZE_INLINE | COPT_OPTIMIZE_AUTO_INLINE | COPT_OPTIMIZE_AUTO_INLINE_ALL;

struct CompilerSettings
{
	uint64		mCompilerFlags;
	uint8		mRegWork;
	uint8		mRegFParam;
	uint8		mRegIP;
	uint8		mRegAccu;
	uint8		mRegAddr;
	uint8		mRegStack;
	uint8		mRegLocals;
	uint8		mRegTmp;
	uint8		mRegTmpSaved;
};
