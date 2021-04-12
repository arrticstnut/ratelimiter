/**
* @file:   rate_limiter.cpp
* @author: 
* @date:  
**/

#include <algorithm>
#include <thread>
#include "rate_limiter.h"

namespace cc {

void RateLimiter::set_rate(double rate) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (std::abs(rate) < 1.0) {
        rate = 1.0;
    }
    _per_token = 1000000.0 / rate;
}

double RateLimiter::acquire() {
    return acquire(1);
}

double RateLimiter::acquire(double tokens) {
    auto wait_time = reserve(tokens);
    std::this_thread::sleep_for(wait_time);
    return wait_time.count() / 1000000.0;
}

bool RateLimiter::try_acquire(int timeout_ms) {
    return try_acquire(1, timeout_ms);
}

bool RateLimiter::try_acquire(double tokens, int timeout_ms) {
    uint64_t now = _now();
    if (_next_time > now + timeout_ms * 1000) {
        return false;
    }
    acquire(tokens);
    return true;
}

void RateLimiter::resync(uint64_t now) {
    if (now > _next_time) {
        if (std::abs(_per_token) < 1.0) {
            _per_token = 1.0;
        }
        _stored_tokens = std::min(_stored_tokens + (now - _next_time) / _per_token, _burst_size);
        _next_time = now;
    }
}

RateLimiter::Microseconds RateLimiter::reserve(double tokens) {
    std::lock_guard<std::mutex> lock(_mutex);
    uint64_t now = _now();
    resync(now);
    uint64_t wait_time = _next_time - now;
    double old_tokens = std::min(tokens, _stored_tokens);
    double new_tokens = tokens - old_tokens;
    _next_time += new_tokens * _per_token;
    _stored_tokens -= old_tokens;
    return Microseconds(wait_time);
}

uint64_t RateLimiter::_now() {
    return std::chrono::duration_cast<Microseconds>(
        Clock::now().time_since_epoch()
    ).count();
}

} // end of namespace
