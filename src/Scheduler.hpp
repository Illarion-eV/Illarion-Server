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

#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

template <typename clock_type> class Task {
public:
    Task(std::function<void()> task, typename clock_type::time_point start_point, std::chrono::nanoseconds interval,
         std::string name);

    inline auto operator<(const Task &other) const -> bool {
        return other._next < _next;
    }

    auto run() -> bool;

    [[nodiscard]] inline auto getName() const -> std::string {
        return _name;
    }

    [[nodiscard]] inline auto getNextTime() const -> typename clock_type::time_point {
        return _next;
    }

private:
    std::function<void()> _task;

    typename clock_type::time_point _next;
    std::chrono::nanoseconds _interval;
    std::string _name;
};

template <typename clock_type> class ClockBasedScheduler {
public:
    void addOneshotTask(std::function<void()> task, std::chrono::nanoseconds delay, const std::string &taskname);
    void addRecurringTask(std::function<void()> task, std::chrono::nanoseconds interval, const std::string &taskname,
                          bool start_immediately = false);
    void addRecurringTask(std::function<void()> task, std::chrono::nanoseconds interval,
                          typename clock_type::time_point first_time, const std::string &taskname);
    void signalNewPlayerAction();

    void run_once(std::chrono::nanoseconds max_timeout);

private:
    auto getNextTaskTime() -> std::chrono::nanoseconds;
    void execute_tasks();

    std::mutex _new_action_signal_mutex;
    std::condition_variable _new_action_available_cond;

    using task_container_t = std::priority_queue<Task<std::chrono::steady_clock>>;
    task_container_t _tasks;
    std::mutex _container_mutex;
};

#include "Scheduler.tcc"

#endif
