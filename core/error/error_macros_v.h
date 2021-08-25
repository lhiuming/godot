#pragma once

/**
 * vformat version for error macros.
 *
 * Put them in a separate file to avoid slowing down compiling of too much files.
 */

#include "core/variant/variant.h"
#include "error_macros.h"

/**
 * Prints `m_msgf` with formatted arguments.
 */
#define WARN_PRINTF(m_msgf, ...) WARN_PRINT(vformat(m_msgf, __VA_ARGS__))


