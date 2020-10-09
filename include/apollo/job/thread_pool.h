#ifndef APOLLO_JOB_THREAD_POOL_H
#define APOLLO_JOB_THREAD_POOL_H

#include <vector>
#include <functional>
#include <future>
#include <memory>
#include <xenium/ramalhete_queue.hpp>
#include <xenium/reclamation/generic_epoch_based.hpp>

namespace apollo
{
	class thread_pool
	{
	public:
		thread_pool(size_t);
		template<class F, class... Args>
		std::future<std::invoke_result_t<F, Args...>> enqueue(F&& f, Args&&... args);
		~thread_pool();
	private:
		bool m_stop;
		std::vector<std::thread> workers;
		xenium::ramalhete_queue<
			std::unique_ptr<std::function<void()>>,
			xenium::policy::reclaimer<xenium::reclamation::epoch_based<>>,
			xenium::policy::entries_per_node<2048>
		> tasks;
	};

	inline thread_pool::thread_pool(size_t threads)
		: m_stop(false)
	{
		for (size_t i = 0; i < threads; ++i)
			workers.emplace_back(
				[this]
				{
					for (;;)
					{
						if (m_stop)
							return;
						std::unique_ptr<std::function<void()>> task;
						if (tasks.try_pop(task)) {
							(*task)();
						}
					}
				}
				);
	}

	template<class F, class... Args>
	std::future<std::invoke_result_t<F, Args...>> thread_pool::enqueue(F&& f, Args&&... args)
	{
		using return_type = std::invoke_result_t<F, Args...>;

		auto task = std::make_shared<std::packaged_task<return_type()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);

		std::future<return_type> res = task->get_future();

		tasks.push(std::make_unique<std::function<void()>>([task]() { (*task)(); }));
		return res;
	}

	inline thread_pool::~thread_pool()
	{
		m_stop = true;
		for (std::thread& worker : workers)
			worker.join();
	}
}
#endif // !APOLLO_JOB_THREAD_POOL_H
