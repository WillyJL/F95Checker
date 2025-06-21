#pragma once

#include <mlib/m-string.h>

#ifdef __cplusplus
extern "C" {
#endif

bool c7zip_extract(const char* archive, const char* output, m_string_ptr error);

#ifdef __cplusplus
}
#endif
