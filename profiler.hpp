#ifndef _PROFILER_HPP_
#define _PROFILER_HPP_

#include <chrono>
#include <limits>
#include <string>
#include <stdexcept>

#include "delegate.hpp"

namespace profiler_utility
{
	// manage report frequency...
	// time duration between each two report must be greater than 'report_interval_us'
	class report_manager
	{
	public:
		static constexpr std::size_t permission_denied = std::numeric_limits<std::size_t>::max();

		report_manager() = delete;

		inline report_manager(const std::size_t & _report_interval_us) :
			report_interval_us_{ _report_interval_us },
			last_report_time_{ std::chrono::high_resolution_clock::now() },
			next_report_time_{ last_report_time_ + report_interval_us_ }
		{
			if (_report_interval_us <= 0)
				throw std::invalid_argument("inline report_manager(const std::size_t & _report_interval_us) : _report_interval_us <= 0");
		}

		inline std::size_t make_report_permission(const std::chrono::high_resolution_clock::time_point & _now) noexcept
		{
			if (_now <= next_report_time_)
				return permission_denied;

			using namespace std::chrono;
			auto duration_us = duration_cast<duration<std::size_t, std::micro>>(_now - last_report_time_).count();
			last_report_time_ = _now;
			next_report_time_ = _now + report_interval_us_;

			return duration_us;
		}

	private:
		const std::chrono::microseconds report_interval_us_;

		std::chrono::high_resolution_clock::time_point last_report_time_;
		std::chrono::high_resolution_clock::time_point next_report_time_;
	};
}

class event_profiler
{
public:
	using report_type_t = delegate_any_t<void,
		const std::size_t& /* n_event */,
		const std::size_t& /* duration_us */>;

	inline event_profiler(const report_type_t & _report, const std::size_t & _report_interval_us = 1'500'000) :
		report_{ _report },
		manager_{ _report_interval_us },
		n_event_{ 0 }
	{ }

	inline void log_event() noexcept
	{
		++n_event_;

		auto duration_us = manager_.make_report_permission(std::chrono::high_resolution_clock::now());
		if (duration_us == profiler_utility::report_manager::permission_denied)
			return;

		report_(n_event_, duration_us);
		n_event_ = 0;
	}

	inline static std::string report(const std::size_t & n_event, const std::size_t & duration_us)
	{
		return std::to_string((n_event * 1'000'000) / (double)duration_us) + " event/sec in " +
			std::to_string(duration_us / 1000.) + " ms.";
	}

private:
	const report_type_t report_;

	profiler_utility::report_manager manager_;
	std::size_t n_event_;
};

class data_profiler
{
public:
	using report_type_t = delegate_any_t<void,
		const std::size_t& /* data_amount */,
		const std::size_t& /* duration_us */>;

	inline data_profiler(const report_type_t & _report, const std::size_t & _report_interval_us = 1'500'000) :
		report_{ _report },
		manager_{ _report_interval_us },
		data_amount_{ 0 }
	{ }

	inline void log_data(const std::size_t _data_size) noexcept
	{
		data_amount_ += _data_size;

		auto duration_us = manager_.make_report_permission(std::chrono::high_resolution_clock::now());
		if (duration_us == profiler_utility::report_manager::permission_denied)
			return;

		report_(data_amount_, duration_us);
		data_amount_ = 0;
	}

	inline static std::string report(const std::size_t & data_amount, const std::size_t & duration_us)
	{
		return std::to_string((data_amount * 1'000'000) / (double)duration_us) + " data/sec in " +
			std::to_string(duration_us / 1000.) + " ms.";
	}

private:
	const report_type_t report_;

	profiler_utility::report_manager manager_;
	std::size_t data_amount_;
};

class method_profiler
{
public:
	using report_type_t = delegate_any_t<void,
		const std::size_t& /* busy_time_us */,
		const std::size_t& /* idle_time_us */,
		const std::size_t& /* n_call */,
		const std::size_t& /* duration_us */>;

	inline method_profiler(const report_type_t & _report, const std::size_t & _report_interval_us = 1'500'000) :
		report_{ _report },
		manager_{ _report_interval_us },
		busy_{ false },
		last_action_time_{ std::chrono::high_resolution_clock::now() },
		busy_time_us_{ 0 },
		idle_time_us_{ 0 },
		n_call_{ 0 }
	{ }

	inline void log_enter()
	{
		if (busy_ == true)
			throw std::runtime_error("void log_enter() : call twice.");

		using namespace std::chrono;
		auto now = high_resolution_clock::now();
		idle_time_us_ += duration_cast<duration<std::size_t, std::micro>>(now - last_action_time_).count();
		++n_call_;

		last_action_time_ = now;
		busy_ = true;
	}

	inline void log_leave()
	{
		if (busy_ == false)
			throw std::runtime_error("void log_leave() : call without log_enter().");

		using namespace std::chrono;
		auto now = high_resolution_clock::now();
		busy_time_us_ += duration_cast<duration<std::size_t, std::micro>>(now - last_action_time_).count();

		last_action_time_ = now;
		busy_ = false;

		auto duration_us = manager_.make_report_permission(now);
		if (duration_us == profiler_utility::report_manager::permission_denied)
			return;

		report_(busy_time_us_, idle_time_us_, n_call_, duration_us);
		busy_time_us_ = 0;
		idle_time_us_ = 0;
		n_call_ = 0;
	}

	inline static std::string report(const std::size_t & _busy_time_us, const std::size_t & _idle_time_us,
		const std::size_t & _n_call, const std::size_t & duration_us)
	{
		return std::to_string((_busy_time_us * 100) / (double)duration_us) +
			"% usage in " + std::to_string(duration_us / 1000.) + " ms.";
	}

private:
	const report_type_t report_;

	bool busy_;
	std::chrono::high_resolution_clock::time_point last_action_time_;

	profiler_utility::report_manager manager_;
	std::size_t busy_time_us_;
	std::size_t idle_time_us_;
	std::size_t n_call_;
};

#endif // !_PROFILER_HPP_
