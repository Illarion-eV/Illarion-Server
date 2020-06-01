#include <utility>



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

template<typename clock_type>
Task<clock_type>::Task(std::function<void()> task, typename clock_type::time_point start_point, std::chrono::nanoseconds interval, std::string  name) : _task(std::move(task)), _next(start_point), _interval(interval), _name(std::move(name)) { }

template<typename clock_type>
auto Task<clock_type>::run() -> bool {
	_task();

	if (_interval > std::chrono::nanoseconds::zero()) {
		_next += std::chrono::duration_cast<typename clock_type::duration>(_interval);
		return true;
	}

	return false;
}

template<typename clock_type>
void ClockBasedScheduler<clock_type>::addOneshotTask(std::function<void()> task, std::chrono::nanoseconds delay, const std::string& taskname) {
	std::unique_lock<std::mutex> lock(_container_mutex);
	typename clock_type::time_point start_time = clock_type::now() + std::chrono::duration_cast<typename clock_type::duration>(delay);
	_tasks.emplace(task, start_time, std::chrono::nanoseconds::zero(), taskname);
}

template<typename clock_type>
void ClockBasedScheduler<clock_type>::addRecurringTask(std::function<void()> task, std::chrono::nanoseconds interval, const std::string& taskname, bool start_immediately) {
	std::unique_lock<std::mutex> lock(_container_mutex);
	typename clock_type::time_point start_time = clock_type::now();
	if (!start_immediately) {
		start_time += std::chrono::duration_cast<typename clock_type::duration>(interval);
    }

	_tasks.emplace(task, start_time, interval, taskname);
}

template<typename clock_type>
void ClockBasedScheduler<clock_type>::addRecurringTask(std::function<void()> task, std::chrono::nanoseconds interval, typename clock_type::time_point first_time, const std::string& taskname) {
	std::unique_lock<std::mutex> lock(_container_mutex);
	_tasks.emplace(task, first_time, interval, taskname);
}

template<typename clock_type>
void ClockBasedScheduler<clock_type>::signalNewPlayerAction() {
	std::unique_lock<std::mutex> lock(_new_action_signal_mutex);
	_new_action_available_cond.notify_all();
}

template<typename clock_type>
void ClockBasedScheduler<clock_type>::run_once(std::chrono::nanoseconds max_timeout) {
	auto next_action_time = getNextTaskTime();
	if (next_action_time > max_timeout) {
		next_action_time = max_timeout;

    }

	{
		std::unique_lock<std::mutex> lock(_new_action_signal_mutex);
		_new_action_available_cond.wait_for(lock, next_action_time);
	}

	execute_tasks();
}

template<typename clock_type>
auto ClockBasedScheduler<clock_type>::getNextTaskTime() -> std::chrono::nanoseconds {
	std::unique_lock<std::mutex> lock(_container_mutex);
	if (_tasks.empty()) {
		return std::chrono::nanoseconds::max();
    }

	return _tasks.top().getNextTime() - clock_type::now();
}

template<typename clock_type>
void ClockBasedScheduler<clock_type>::execute_tasks() {
	auto now = clock_type::now();

	std::unique_lock<std::mutex> lock(_container_mutex);
	while (!_tasks.empty() && now >= _tasks.top().getNextTime()) {
		auto task = _tasks.top();
		_tasks.pop();
		lock.unlock();

		bool runResult = task.run();

		lock.lock();
		if (runResult) {
			_tasks.push(task);
        }
	}
}

