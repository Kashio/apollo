#include <gtest/gtest.h>
#include <iostream>
#include <apollo/apollo.h>
#include "transform.h"
#include "mass.h"
#include "velocity.h"
#include "move_system.h"

TEST(Test, Test1)
{
	apollo::registry registry;

	auto callback_id = registry.on_construct<transform>().connect([](apollo::registry&, apollo::entity const & e) {
		std::cout << "entity(" << e << ") transform component was added\n";
	});

	auto callback_id2 = registry.on_destroy<mass>().connect([](apollo::registry&, apollo::entity const & e) {
		std::cout << "entity(" << e << ") mass component was removed\n";
	});

	auto callback_id3 = registry.on_update<transform>().connect([](apollo::registry&, apollo::entity const & e) {
		std::cout << "entity(" << e << ") transform component was updated\n";
	});

	apollo::entity e1 = registry.create();
	apollo::entity e2 = registry.create();
	apollo::entity e3 = registry.create();

	registry.emplace<transform>(e1, 10.0f, 50.0f, 30.0f);
	registry.emplace<mass>(e1, 15.0f);

	registry.emplace<transform>(e2, 1.0f, 1.0f, 1.0f);
	registry.emplace<mass>(e2, 12.0f);
	registry.emplace<mass>(e3, 5.3f);

	registry.remove<transform>(e2);

	registry.clear<mass>();
	registry.patch(e1, [](transform& t) {
		t.m_x = 88;
	});

	if (registry.any<transform, mass>(e1))
	{
		std::cout << "e1 has transform or mass\n";
	}

	//registry.destroy(e1);

	registry.create_system<move_system>();

	registry.update();

	registry.update();

	registry.on_construct<transform>().disconnect(callback_id);
	registry.on_destroy<mass>().disconnect(callback_id2);
	registry.on_update<transform>().disconnect(callback_id3);
}
