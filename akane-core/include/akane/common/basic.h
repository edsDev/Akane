#pragma once
#include <stdexcept>
#include <fmt/format.h>

namespace akane
{
    template <typename TFmt, typename... TArgs> void Throw(const TFmt& fmt, const TArgs& ... args)
    {
        throw std::runtime_error(fmt::format(fmt, args...));
    }

    template <typename TFmt, typename... TArgs> void Warn(const TFmt& fmt, const TArgs& ... args)
    {
        fmt::print(fmt, args...);
    }
} // namespace akane

#define AKANE_DISABLE_COPY(KLASS)
#define AKANE_DISABLE_COPY_AND_MOVE(KLASS)

#define AKANE_STRINGIFY_AUX(X) #X
#define AKANE_STRINGIFY(X) AKANE_STRINGIFY_AUX(X)
#define AKANE_ERROR_MESSAGE(LEVEL, MSG)                                                            \
    ("[" __FILE__ ":" AKANE_STRINGIFY(__LINE__) "] " LEVEL " (" MSG ") FAILED\n")

#define AKANE_DEBUG_MODE
#ifdef AKANE_DEBUG_MODE
#define AKANE_ASSERT(CONDITION)                                                                    \
    static_cast<void>(!!(CONDITION) ||                                                             \
                      (::akane::Throw(AKANE_ERROR_MESSAGE("ASSERT", #CONDITION)), 0))

#define AKANE_CHECK(CONDITION)                                                                     \
    static_cast<void>(!!(CONDITION) || (::akane::Warn(AKANE_ERROR_MESSAGE("CHECK", #CONDITION)), 0))
#else
#define AKANE_ASSERT(CONDITION)
#define AKANE_CHECK(CONDITION)
#endif // AKANE_DEBUG_MODE

#define AKANE_REQUIRE(CONDITION)                                                                   \
    static_cast<void>(!!(CONDITION) ||                                                             \
                      (::akane::Throw(AKANE_ERROR_MESSAGE("REQUIRE", #CONDITION)), 0))

#define AKANE_NO_IMPL() (::akane::Throw("no impl"))