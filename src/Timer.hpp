//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.

#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>

/**
 * @brief A simple interval timer for periodic event checking.
 *
 * This timer uses std::chrono::steady_clock to track time intervals and
 * determine when a specified duration has elapsed. Useful for implementing
 * periodic updates, cooldowns, or rate limiting.
 */
class Timer {
public:
    using clock = std::chrono::steady_clock; ///< The clock type used for timing.
    using timePoint = clock::time_point; ///< A point in time from the steady clock.
    using duration = clock::duration; ///< A time duration type.

    /**
     * @brief Constructs a timer with the specified interval.
     * @param interval The duration to wait between intervals.
     * @note The timer is initialized with the current time as the starting point.
     */
    explicit Timer(duration interval);

    /**
     * @brief Checks if the interval has been exceeded since the last check.
     *
     * When this method returns true, it resets the internal timer to the current
     * time, starting a new interval period.
     *
     * @return true if the interval duration has elapsed, false otherwise.
     */
    auto intervalExceeded() -> bool;

private:
    timePoint lastIntervalExceeded; ///< Timestamp when the interval was last exceeded.
    duration interval; ///< The duration of the interval to check against.
};

#endif
