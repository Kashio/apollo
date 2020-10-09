#ifndef APOLLO_JOB_JOB_HANLDE_H
#define APOLLO_JOB_JOB_HANLDE_H

#include <future>

namespace apollo
{
	class job_handle
	{
	private:
		std::shared_future<void> m_future;
	public:
		job_handle() = default;

		job_handle(std::shared_future<void>& future)
			: m_future(std::move(future)) {}

		void complete() const
		{
			m_future.wait();
		}

		bool valid() const
		{
			return m_future.valid();
		}
	};
}

#endif // !APOLLO_JOB_JOB_HANLDE_H
