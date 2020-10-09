#ifndef APOLLO_JOB_JOB_H
#define APOLLO_JOB_JOB_H

#include "thread_pool.h"
#include "job_handle.h"
#include <memory>
#include <functional>

namespace apollo
{
	class job
	{
	private:
		inline static std::size_t ID = 0;
		thread_pool* m_thread_pool;
		std::function<void()> m_task;
	public:
		job_handle m_handle;
		std::size_t m_id;
	public:
		job()
			: m_thread_pool(nullptr), m_task(), m_handle() {}

		job(thread_pool* tp, std::function<void()> task)
			: m_thread_pool(tp), m_task(std::move(task)), m_handle(), m_id(ID++) {}

		template <typename... Handles, typename = std::enable_if_t<std::conjunction_v<std::is_same<job_handle, std::remove_cv_t<std::remove_reference_t<Handles>>>...>>>
		job_handle schedule(Handles&&... handles)
		{
			(..., handles.complete());
			std::shared_future<void> future = m_thread_pool->enqueue(m_task);
			m_handle = job_handle(future);
			return m_handle;
		}

		void run()
		{
			m_task();
		}
	};
}

#endif // !APOLLO_JOB_JOB_H
