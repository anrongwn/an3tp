#pragma once

// anNoCopyable public class
struct anNoCopyable {
    anNoCopyable() = default;
    anNoCopyable &operator=(const anNoCopyable &) = delete;
    anNoCopyable(const anNoCopyable &) = delete;
};
