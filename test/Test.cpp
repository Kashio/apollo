#include <gtest/gtest.h>
#include <apollo/apollo.h>
#include "transform.h"
#include "mass.h"
#include "move_system.h"

TEST(Test, Test1)
{
	apollo::registry registry;
	apollo::entity e1 = registry.create();
	apollo::entity e2 = registry.create();
	apollo::entity e3 = registry.create();

	registry.emplace<transform>(e1, 10.0f, 50.0f, 30.0f);
	registry.emplace<mass>(e1, 15.0f);

	registry.emplace<transform>(e2, 1.0f, 1.0f, 1.0f);
	registry.emplace<mass>(e2, 12.0f);
	registry.emplace<mass>(e3, 5.3f);

	registry.remove<transform>(e2);

	auto v1 = registry.view<transform>();

	for (auto& entity : v1)
	{
		auto t = v1.get<transform>(entity);
		int x = 8;
	}

	//registry.destroy(e1);

	registry.create_system<move_system>();

	registry.update();
}
