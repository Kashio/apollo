#ifndef TEST_MOVE_SYSTEM_H
#define TEST_MOVE_SYSTEM_H

#include <apollo/system.h>
#include <apollo/core/common.h>
#include <apollo/command/destroy_command.h>
#include <thread>
#include <iostream>
#include <sstream>
#include <chrono>

class move_system : public apollo::system
{
public:
	move_system(apollo::registry& registry)
		: apollo::system(registry) {}

	void update() override
	{
		//std::cout << "[move_system] main thread id: " << std::this_thread::get_id() << '\n';
		//apollo::command_buffer cb = registry.create_command_buffer();
		auto& j1 = for_each([this](apollo::entity& entity, transform& t) {
			std::stringstream s;
			s << "[print_transform_job] worker thread id: " << std::this_thread::get_id() << "\n\t";
			s << "job[" << this->m_dependency.m_id << "]\n\t";
			s << "x: " << t.m_x << " y: " << t.m_y << " z: " << t.m_z << '\n';
			if (t.m_x != 0.0f)
			{
				t.m_x = t.m_y = t.m_z = 0.0f;
			}
			else
			{
				t.m_x = t.m_y = t.m_z = 100.0f;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
			std::cout << s.str();
		});
		j1.schedule();
		//auto j2 = registry.for_each([&cb](apollo::entity& entity, transform& t) {
		//	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		//	t.m_x++;
		//	//cb.add_command<apollo::destroy_command>(entity);
		//	std::stringstream s;
		//	s << "[move_job] worker thread id: " << std::this_thread::get_id() << "\n\n";
		//	std::cout << s.str();
		//});
		//apollo::job_handle& jh2 = j2.schedule();
		//apollo::job_handle& jh1 = j1.schedule(jh2);
		//jh.complete();
		//cb.execute();
	}
};

#endif // !TEST_MOVE_SYSTEM_H
