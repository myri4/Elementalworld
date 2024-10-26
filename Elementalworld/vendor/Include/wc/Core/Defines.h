#pragma once

#define WC_IGNORE_WARNING_PUSH #pragma warning(push, 0)
#define WC_IGNORE_WARNING_POP #pragma warning(pop)
//#define WC_IGNORE_WARNING(x)

#define WC_NO_COPY(CLASS_NAME) CLASS_NAME() = default; \
CLASS_NAME& operator = (const CLASS_NAME&) = delete; \
CLASS_NAME(const CLASS_NAME&) = delete;