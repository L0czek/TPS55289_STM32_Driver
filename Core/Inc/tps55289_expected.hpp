/**
 * @file tps55289_expected.hpp
 * @brief std::expected alias for the tps55289 driver.
 *
 * The driver is built as C++23 (same standard as the firmware that links it),
 * so the alias always resolves to std::expected. A C++17 fallback used to live
 * here; it was removed because mixing it with C++23 callers gave the same name
 * two different layouts (ODR violation) and it relied on exceptions.
 */
#ifndef __TPS55289_EXPECTED_HPP
#define __TPS55289_EXPECTED_HPP

#if __cplusplus < 202302L
#error "The tps55289 driver requires C++23 (std::expected)"
#endif

#include <expected>
#include <system_error>

namespace tps55289 {
template<typename T, typename E = std::error_code>
using expected = std::expected<T, E>;

} // namespace tps55289

#endif // __TPS55289_EXPECTED_HPP
