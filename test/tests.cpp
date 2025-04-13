#include "example.h"

#include <gtest/gtest.h>

static_assert(false, "Implement tests");

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
