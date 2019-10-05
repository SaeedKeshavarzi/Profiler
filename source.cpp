#include <stdio.h>
#include <iostream>
#include <thread>

#include "profiler.hpp"

class reporter
{
public:
	reporter(const std::string & _name) :
		name_{ _name }
	{ }

	void operator()(const std::size_t & _busy_us, const std::size_t & _idle_us, const std::size_t & _n_call, 
		const std::size_t & _duration_us)
	{
		static int endl_cnt_ = 0;

		std::cout << name_ + ": " + method_profiler::report(_busy_us, _idle_us, _n_call, _duration_us)
			<< std::endl;

		if (++endl_cnt_ == 2)
		{
			endl_cnt_ = 0;
			std::cout << std::endl;
		}
	}

	std::string name_;
};

static void f1(int & n)
{
	using namespace std::literals::string_literals;
	using namespace std::literals::chrono_literals;

	static reporter f1_reporter("f1"s);
	static method_profiler f1_profiler(f1_reporter);

	f1_profiler.log_enter();

	n = 1 + ((n - 1) * n) % 13;
	std::this_thread::sleep_for(2ms);

	f1_profiler.log_leave();
}

static void f2(int & n)
{
	using namespace std::literals::string_literals;
	using namespace std::literals::chrono_literals;

	static reporter f2_reporter("f2"s);
	static method_profiler f2_profiler(f2_reporter);

	f2_profiler.log_enter();

	n = 1 + ((n - 1) * 13) % n;
	std::this_thread::sleep_for(6ms);

	f2_profiler.log_leave();
}

int main()
{
	int n = 12;

	for (int i = 0; i < 1'000; ++i)
	{
		f1(n);
		f2(n);
	}
	
	std::cout << "\npress enter to exit... ";
	getchar();
	return 0;
}
