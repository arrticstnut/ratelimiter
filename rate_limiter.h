/**
* @file:   rate_limiter.h
* @author: 
* @date:   
**/

#ifndef _RATE_LIMITER_H_
#define _RATE_LIMITER_H_
#include <chrono>
#include <mutex>

namespace cc {
// limit permits at a configurable rate
// based on rate limiter in guava:
// https://github.com/google/guava/blob/master/guava/src/com/google/common/util/concurrent/RateLimiter.java
class RateLimiter {
private:
    using Clock = std::chrono::steady_clock;
    using Microseconds = std::chrono::microseconds;
public:
    RateLimiter(double rate, double burst_size)
        : _stored_tokens(0.0)
	, _per_token(1000000.0 / rate)
        , _burst_size(burst_size)
        , _next_time(0.0) {}
    ~RateLimiter() {}
    // set limiter rate
    void set_rate(double rate);
    // acquire one token in bucket
    double acquire();
    // acquire given tokens in bucket, return wait time in seconds
    double acquire(double tokens);
    // try acquire one token in bucket
    bool try_acquire(int timeout_ms);
    // try acquire tokens in bucket
    // return false immediately if there is not enough tokens before timeout
    // reture true if tokens can be obtained before timeout
    bool try_acquire(double tokens, int timeout_ms);
    // put new tokens into bucket due to now and _next_time
    void resync(uint64_t now);
    // reserve given tokens for future use
    // return wait time until the reservation can be consumed
    Microseconds reserve(double tokens);
private:
    uint64_t _now();
    // tokens in bucket
    double _stored_tokens;
    // time per token to consume
    double _per_token;
    // max size of tokens
    double _burst_size;
    // the time when new token is available
    double _next_time;
    std::mutex _mutex;
};
} // end of namespace
#endif
