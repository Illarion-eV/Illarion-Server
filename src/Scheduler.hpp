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

/**
 * @brief Represents a scheduled task that can be executed once or repeatedly.
 * @tparam clock_type The clock type to use for timing (e.g., std::chrono::steady_clock).
 *
 * Tasks can be one-shot (interval of zero) or recurring (positive interval).
 * Recurring tasks automatically reschedule themselves after execution.
 */
template <typename clock_type> class Task {
public:
    /**
     * @brief Constructs a task with specified timing parameters.
     * @param task The function to execute.
     * @param start_point The time point when the task should first execute.
     * @param interval The interval for recurring tasks (zero for one-shot tasks).
     * @param name A human-readable name for debugging and logging.
     */
    Task(std::function<void()> task, typename clock_type::time_point start_point, std::chrono::nanoseconds interval,
         std::string name);

    /**
     * @brief Comparison operator for priority queue ordering (earlier tasks have higher priority).
     * @param other The other task to compare against.
     * @return true if this task should execute after the other task.
     */
    inline auto operator<(const Task &other) const -> bool { return other._next < _next; }

    /**
     * @brief Executes the task and reschedules if recurring.
     * @return true if the task should remain in the queue (recurring), false if it should be removed (one-shot).
     */
    auto run() -> bool;

    /**
     * @brief Gets the task's name.
     * @return The task name.
     */
    [[nodiscard]] inline auto getName() const -> std::string { return _name; }

    /**
     * @brief Gets the next scheduled execution time.
     * @return The time point when this task should next execute.
     */
    [[nodiscard]] inline auto getNextTime() const -> typename clock_type::time_point { return _next; }

private:
    std::function<void()> _task; ///< The function to execute.
    typename clock_type::time_point _next; ///< The next scheduled execution time.
    std::chrono::nanoseconds _interval; ///< The interval between executions (zero for one-shot).
    std::string _name; ///< Human-readable task name for debugging.
};

/**
 * @brief A thread-safe task scheduler for executing functions at specific times or intervals.
 * @tparam clock_type The clock type to use for timing (e.g., std::chrono::steady_clock).
 *
 * This scheduler manages a priority queue of tasks and executes them at their scheduled times.
 * It supports both one-shot and recurring tasks, and can be signaled to wake up for new player actions.
 */
template <typename clock_type> class ClockBasedScheduler {
public:
    /**
     * @brief Adds a task that executes once after a delay.
     * @param task The function to execute.
     * @param delay The delay before execution.
     * @param taskname A name for the task (for debugging).
     */
    void addOneshotTask(std::function<void()> task, std::chrono::nanoseconds delay, const std::string &taskname);

    /**
     * @brief Adds a task that executes repeatedly at a fixed interval.
     * @param task The function to execute.
     * @param interval The time between executions.
     * @param taskname A name for the task (for debugging).
     * @param start_immediately If true, execute immediately then repeat; if false, wait one interval before first execution.
     */
    void addRecurringTask(std::function<void()> task, std::chrono::nanoseconds interval, const std::string &taskname,
                          bool start_immediately = false);

    /**
     * @brief Adds a recurring task with a specific first execution time.
     * @param task The function to execute.
     * @param interval The time between executions.
     * @param first_time The time point for the first execution.
     * @param taskname A name for the task (for debugging).
     */
    void addRecurringTask(std::function<void()> task, std::chrono::nanoseconds interval,
                          typename clock_type::time_point first_time, const std::string &taskname);

    /**
     * @brief Signals the scheduler that a new player action is available.
     *
     * This wakes up the scheduler if it's waiting, allowing it to process player actions immediately.
     */
    void signalNewPlayerAction();

    /**
     * @brief Runs the scheduler for one iteration, executing due tasks.
     * @param max_timeout Maximum time to wait for tasks or signals before returning.
     */
    void run_once(std::chrono::nanoseconds max_timeout);

private:
    /**
     * @brief Gets the time until the next scheduled task.
     * @return The duration until the next task, or a large value if no tasks are scheduled.
     */
    auto getNextTaskTime() -> std::chrono::nanoseconds;

    /**
     * @brief Executes all tasks that are due.
     */
    void execute_tasks();

    std::mutex _new_action_signal_mutex; ///< Mutex for the condition variable.
    std::condition_variable _new_action_available_cond; ///< Condition variable for signaling new actions.

    using task_container_t = std::priority_queue<Task<std::chrono::steady_clock>>; ///< Priority queue type for tasks.
    task_container_t _tasks; ///< The queue of scheduled tasks.
    std::mutex _container_mutex; ///< Mutex protecting the task queue.
};

#include "Scheduler.tcc"

#endif
