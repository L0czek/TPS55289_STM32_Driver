/**
 * @file tps55289_expected.hpp
 * @brief C++23 std::expected support for TPS55289 driver
 *
 * This header provides a type alias to std::expected (C++23) or
 * falls back to a simple expected implementation for C++17 compatibility.
 */

#ifndef __TPS55289_EXPECTED_HPP
#define __TPS55289_EXPECTED_HPP

#if __cplusplus >= 202302L
    // C++23: Use std::expected directly
    #include <system_error>
    #include <expected>
    namespace tps55289 {
        template<typename T, typename E = std::error_code>
        using expected = std::expected<T, E>;
    }
#else
    // C++17: Use simple expected implementation
    #include <optional>
    #include <stdexcept>
    #include <utility>
    #include <string>
    #include <type_traits>
    #include <system_error>
    #include <cstdint>

    namespace tps55289 {

    /**
     * @brief Simple expected implementation for C++17
     * @tparam T Value type
     * @tparam E Error type
     */
    template<typename T, typename E = std::error_code>
    class expected {
    public:
        using value_type = T;
        using error_type = E;
        using expected_type = expected<T, E>;

        // Default constructor
        expected() : val_(), err_(), has_val_(true) {}

        // Value constructor
        expected(const T& val) : val_(val), has_val_(true) {}
        expected(T&& val) : val_(std::move(val)), has_val_(true) {}

        // Error constructor
        expected(const E& err) : err_(err), has_val_(false) {}
        expected(E&& err) : err_(std::move(err)), has_val_(false) {}

        // Copy constructor
        expected(const expected& other) : has_val_(other.has_val_) {
            if (has_val_) {
                val_ = other.val_;
            } else {
                err_ = other.err_;
            }
        }

        // Move constructor
        expected(expected&& other) noexcept : has_val_(other.has_val_) {
            if (has_val_) {
                val_ = std::move(other.val_);
            } else {
                err_ = std::move(other.err_);
            }
        }

        // Copy assignment
        expected& operator=(const expected& other) {
            if (this != &other) {
                if (has_val_ && other.has_val_) {
                    val_ = other.val_;
                } else if (!has_val_ && !other.has_val_) {
                    err_ = other.err_;
                } else if (has_val_ && !other.has_val_) {
                    val_ = T();
                    err_ = other.err_;
                    has_val_ = false;
                } else {
                    err_ = other.err_;
                    val_ = T();
                    has_val_ = true;
                }
            }
            return *this;
        }

        // Move assignment
        expected& operator=(expected&& other) noexcept {
            if (this != &other) {
                if (has_val_ && other.has_val_) {
                    val_ = std::move(other.val_);
                } else if (!has_val_ && !other.has_val_) {
                    err_ = std::move(other.err_);
                } else if (has_val_ && !other.has_val_) {
                    val_ = T();
                    err_ = std::move(other.err_);
                    has_val_ = false;
                } else {
                    err_ = std::move(other.err_);
                    val_ = T();
                    has_val_ = true;
                }
            }
            return *this;
        }

        // Destructor
        ~expected() = default;

        // Check if has value
        explicit operator bool() const noexcept { return has_val_; }
        bool has_value() const noexcept { return has_val_; }

        // Access value
        const T& value() const {
            if (!has_val_) {
                throw std::runtime_error("bad_expected_access: no value");
            }
            return val_;
        }

        T& value() {
            if (!has_val_) {
                throw std::runtime_error("bad_expected_access: no value");
            }
            return val_;
        }

        // Access error
        const E& error() const {
            if (has_val_) {
                throw std::runtime_error("bad_expected_access: no error");
            }
            return err_;
        }

        // Get value or default
        T value_or(const T& default_val) const noexcept {
            if (has_val_) {
                return val_;
            }
            return default_val;
        }

    private:
        union {
            T val_;
            E err_;
        };
        bool has_val_;
    };

    // Specialization for void value type
    template<typename E>
    class expected<void, E> {
    public:
        using value_type = void;
        using error_type = E;
        using expected_type = expected<void, E>;

        // Default constructor (success)
        expected() : err_(), has_val_(true) {}

        // Error constructor
        expected(const E& err) : err_(err), has_val_(false) {}
        expected(E&& err) : err_(std::move(err)), has_val_(false) {}

        // Copy constructor
        expected(const expected& other) : has_val_(other.has_val_) {
            if (!has_val_) {
                err_ = other.err_;
            }
        }

        // Move constructor
        expected(expected&& other) noexcept : has_val_(other.has_val_) {
            if (!has_val_) {
                err_ = std::move(other.err_);
            }
        }

        // Copy assignment
        expected& operator=(const expected& other) {
            if (this != &other) {
                if (has_val_ && !other.has_val_) {
                    err_ = other.err_;
                    has_val_ = false;
                } else if (!has_val_ && other.has_val_) {
                    err_ = E();
                    has_val_ = true;
                } else if (!has_val_ && !other.has_val_) {
                    err_ = other.err_;
                }
            }
            return *this;
        }

        // Move assignment
        expected& operator=(expected&& other) noexcept {
            if (this != &other) {
                if (has_val_ && !other.has_val_) {
                    err_ = std::move(other.err_);
                    has_val_ = false;
                } else if (!has_val_ && other.has_val_) {
                    err_ = E();
                    has_val_ = true;
                } else if (!has_val_ && !other.has_val_) {
                    err_ = std::move(other.err_);
                }
            }
            return *this;
        }

        // Destructor
        ~expected() = default;

        // Check if has value
        explicit operator bool() const noexcept { return has_val_; }
        bool has_value() const noexcept { return has_val_; }

        // Access error
        const E& error() const {
            if (has_val_) {
                throw std::runtime_error("bad_expected_access: no error");
            }
            return err_;
        }

    private:
        union {
            char dummy_;
            E err_;
        };
        bool has_val_;
    };

    } // namespace tps55289
#endif

#endif // __TPS55289_EXPECTED_HPP
