/* Just so I can rebuild easily */
#include "svh/serializer.hpp"

int main() {

	svh::json test;
	auto numer = svh::Serializer::ToJson(1);
	test.push_back(numer);
	auto dump = test.dump();

	return 0;
}