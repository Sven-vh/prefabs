#pragma once
#include "pch.h"
#include "CppUnitTest.h"
#include "svh/serializer.hpp"

#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <memory>
#include <set>
#include <unordered_set>
#include <array>
#include <deque>
#include <list>
#include <initializer_list>
#include <tuple>
#include <optional>
#include <variant>
#include <utility>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace compare_tests {

	static std::wstring to_wstring(const std::string& s) {
		return std::wstring(s.begin(), s.end());
	}

	template<typename T>
	void CheckCompare(const T& left, const T& right, const svh::json& result) {
		std::string resultDump;
		try {
			resultDump = svh::Compare::GetChanges(left, right).dump();
		} catch (const std::exception& ex) {
			Assert::Fail(to_wstring(std::string("Exception thrown: ") + ex.what()).c_str());
		}
		// compare wstring’d JSON
		std::wstring wResult = to_wstring("\n" + resultDump);
		std::string expectedDump = result.dump();
		std::wstring wExpected = to_wstring("\n" + expectedDump);
		Assert::AreEqual(wExpected, wResult, L"\nCompare output did not match expected JSON");
		// put result in the log
		Logger::WriteMessage(wResult.c_str());
	}
	/* Primitive Types */
	TEST_CLASS(DefaultTypes) {
public:
	//TEST_METHOD(int) { CheckSerialization(2, svh::json(2)); }
	TEST_METHOD(Int) {
		int left = 1;
		int right = 2;
		CheckCompare(left, right, svh::json(2));
	}
	//TEST_METHOD(Double) { CheckSerialization(3.14, svh::json(3.14)); }
	TEST_METHOD(Double) {
		double left = 3.14;
		double right = 2.71;
		CheckCompare(left, right, svh::json(2.71));
	}
	//TEST_METHOD(Float) { CheckSerialization(1.0f, svh::json(1.0f)); }
	TEST_METHOD(Float) {
		float left = 1.0f;
		float right = 2.0f;
		CheckCompare(left, right, svh::json(2.0f));
	}
	//TEST_METHOD(Bool) { CheckSerialization(true, svh::json(true)); }
	TEST_METHOD(Bool) {
		bool left = true;
		bool right = false;
		CheckCompare(left, right, svh::json(false));
	}
	//TEST_METHOD(Char) { CheckSerialization('a', svh::json('a')); }
	TEST_METHOD(Char) {
		char left = 'a';
		char right = 'b';
		CheckCompare(left, right, svh::json('b'));
	}
	//TEST_METHOD(CString) {
	//	const char* cstr = "hello";
	//	CheckSerialization(cstr, svh::json("hello"));
	//}
	TEST_METHOD(CString) {
		const char* left = "hello";
		const char* right = "world";
		CheckCompare(left, right, svh::json("world"));
	}
	//TEST_METHOD(String) {
	//	std::string s = "hello";
	//	CheckSerialization(s, svh::json("hello"));
	//}
	TEST_METHOD(String) {
		std::string left = "hello";
		std::string right = "world";
		CheckCompare(left, right, svh::json("world"));
	}
	//TEST_METHOD(StringEscaping) {
	//	std::string s = "He said: \"\\Hello\n\"";
	//	CheckSerialization(s, svh::json("He said: \"\\Hello\n\""));
	//}
	TEST_METHOD(StringEscaping) {
		std::string left = "He said: \"\\Hello\n\"";
		std::string right = "He said: \"\\World\n\"";
		CheckCompare(left, right, svh::json("He said: \"\\World\n\""));
	}
	//TEST_METHOD(UnicodeString) {
	//	std::string s = u8"π≈3.14";
	//	CheckSerialization(s, svh::json(u8"π≈3.14"));
	//}
	TEST_METHOD(UnicodeString) {
		std::string left = u8"π≈3.14";
		std::string right = u8"π≈2.71";
		CheckCompare(left, right, svh::json(u8"π≈2.71"));
	}
	//TEST_METHOD(StringView) {
	//	std::string_view sv = "viewed";
	//	CheckSerialization(sv, svh::json("viewed"));
	//}
	TEST_METHOD(StringView) {
		std::string_view left = "viewed";
		std::string_view right = "seen";
		CheckCompare(left, right, svh::json("seen"));
	}
	};

	/* Enum Types */
	TEST_CLASS(EnumTypes) {
public:
	//	TEST_METHOD(ColorEnum) {
	//		Color c = Color::Green;
	//		CheckSerialization(c, svh::json("Green"));
	//	}
	TEST_METHOD(ColorEnum) {
		Color left = Color::Green;
		Color right = Color::Blue;
		CheckCompare(left, right, svh::json("Blue"));
	}
	//	TEST_METHOD(DirectionEnum) {
	//		Direction d = Direction::South;
	//		CheckSerialization(d, svh::json("South"));
	//	}
	TEST_METHOD(DirectionEnum) {
		Direction left = Direction::South;
		Direction right = Direction::North;
		CheckCompare(left, right, svh::json("North"));
	}
	//	TEST_METHOD(EnumClassAsInt) {
	//		int i = static_cast<int>(Color::Red);
	//		CheckSerialization(i, svh::json(0));
	//	}
	TEST_METHOD(EnumClassAsInt) {
		int left = static_cast<int>(Color::Red);
		int right = static_cast<int>(Color::Green);
		CheckCompare(left, right, svh::json(1));
	}
	};

	/* Sequence containers: vector, array, set, unordered_set, deque, list, initializer_list, c-style array */
	TEST_CLASS(Containers) {
public:
	/* Vector 1-depth */
	TEST_METHOD(Vector) {
		std::vector<int> left{ 1,2,3 };
		std::vector<int> right{ 1,2,3 };
		CheckCompare(left, right, svh::json());
	}
	TEST_METHOD(ChangedVector) {
		std::vector<int> left{ 1,2,3 };
		std::vector<int> right{ 4,5,6 };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				0,1,2
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, 0 },
					  { svh::VALUE, 4 }
				  }),
				  svh::json({
					  { svh::INDEX, 1 },
					  { svh::VALUE, 5 }
				  }),
				  svh::json({
					  { svh::INDEX, 2 },
					  { svh::VALUE, 6 }
				  })
			  })
		}
		};
		CheckCompare(left, right, expected);
	}
	TEST_METHOD(AddedVector) {
		std::vector<int> left{ 1,2,3 };
		std::vector<int> right{ 1,2,3,4 };

		svh::json expected = {
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, 3 },
					  { svh::VALUE, 4 }
				  })
			  })
			}
		};

		CheckCompare(left, right, expected);
	}

	/* Vector 2-depth */
	TEST_METHOD(VectorOfVector_Unchanged) {
		std::vector<std::vector<int>> left{ {1, 2}, {3, 4} };
		std::vector<std::vector<int>> right{ {1, 2}, {3, 4} };
		CheckCompare(left, right, svh::json());
	}

	TEST_METHOD(VectorOfVector_ChangedInner) {
		std::vector<std::vector<int>> left{ {1, 2}, {3, 4} };
		std::vector<std::vector<int>> right{ {1, 5}, {3, 4} };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  svh::json::array({0, 1})
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, svh::json::array({0, 1}) },
					  { svh::VALUE, 5 }
				  })
			  })
			}
		};
		CheckCompare(left, right, expected);
	}

	TEST_METHOD(VectorOfVector_AddedRow) {
		std::vector<std::vector<int>> left{ {1, 2}, {3, 4} };
		std::vector<std::vector<int>> right{ {1, 2}, {3, 4}, {5, 6} };
		svh::json expected = {
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, 2 },
					  { svh::VALUE, svh::json::array({5, 6}) }
				  })
			  })
			}
		};
		CheckCompare(left, right, expected);
	}

	TEST_METHOD(VectorOfVector_RemovedRow) {
		std::vector<std::vector<int>> left{ {1, 2}, {3, 4}, {5, 6} };
		std::vector<std::vector<int>> right{ {1, 2}, {3, 4} };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  svh::json(2)
			  })
			}
		};
		CheckCompare(left, right, expected);
	}

	/* Vector 3-depth */
	// 1) No change at all
	TEST_METHOD(Vector3_Unchanged) {
		std::vector<std::vector<std::vector<int>>> left{
			{{1,2}, {3,4}},
			{{5,6}, {7,8}}
		};
		auto right = left;
		CheckCompare(left, right, svh::json());
	}

	// 2) Change at the deepest level
	TEST_METHOD(Vector3_ChangedDeepest) {
		std::vector<std::vector<std::vector<int>>> left{ {{1,2}} };
		std::vector<std::vector<std::vector<int>>> right{ {{1,3}} };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  svh::json::array({0, 0, 1})
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, svh::json::array({0, 0, 1}) },
					  { svh::VALUE, 3 }
				  })
			  })
			}
		};
		CheckCompare(left, right, expected);
	}
	TEST_METHOD(Vector3_ChangedMid) {
		// Outer has two 2‑deep “slices”.
		// In the second slice (outer INDEX 1), we change its first inner vector
		// from {5,6} to {9,10}.
		std::vector<std::vector<std::vector<int>>> left{
			{{1,2}, {3,4}},
			{{5,6}, {7,8}}
		};
		std::vector<std::vector<std::vector<int>>> right{
			{{1,2}, {3,4}},
			{{9,10}, {7,8}}
		};

		svh::json expected = {
			// We expect two removals: 5 at [1,0,0] and 6 at [1,0,1]
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  svh::json::array({1, 0, 0}),
				  svh::json::array({1, 0, 1})
			  })
			},
			// And two adds:  9 at [1,0,0] and 10 at [1,0,1]
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, svh::json::array({1, 0, 0}) },
					  { svh::VALUE, 9 }
				  }),
				  svh::json({
					  { svh::INDEX, svh::json::array({1, 0, 1}) },
					  { svh::VALUE, 10 }
				  })
			  })
			}
		};

		CheckCompare(left, right, expected);
	}


	// 3) Add a new element at the deepest level
	TEST_METHOD(Vector3_AddedDeepest) {
		std::vector<std::vector<std::vector<int>>> left{ {{1,2}} };
		std::vector<std::vector<std::vector<int>>> right{ {{1,2,3}} };
		svh::json expected = {
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, svh::json::array({0, 0, 2}) },
					  { svh::VALUE, 3 }
				  })
			  })
			}
		};
		CheckCompare(left, right, expected);
	}

	// 4) Remove an element at the deepest level
	TEST_METHOD(Vector3_RemovedDeepest) {
		std::vector<std::vector<std::vector<int>>> left{ {{1,2,3}} };
		std::vector<std::vector<std::vector<int>>> right{ {{1,2}} };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  svh::json::array({0, 0, 2})
			  })
			}
		};
		CheckCompare(left, right, expected);
	}

	// 5) Add a new “row” at the mid (2nd) level
	TEST_METHOD(Vector3_AddedMid) {
		// Outer[0] initially has 1 inner vector; we’re inserting a second inner vector {2}
		std::vector<std::vector<std::vector<int>>> left{ {{1}} };
		std::vector<std::vector<std::vector<int>>> right{ {{1}, {2}} };
		svh::json expected = {
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, svh::json::array({0, 1}) },
					  { svh::VALUE, svh::json::array({2}) }
				  })
			  })
			}
		};
		CheckCompare(left, right, expected);
	}

	// 6) Remove a “row” at the mid level
	TEST_METHOD(Vector3_RemovedMid) {
		std::vector<std::vector<std::vector<int>>> left{ {{1}, {2}} };
		std::vector<std::vector<std::vector<int>>> right{ {{1}} };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  svh::json::array({0, 1})
			  })
			}
		};
		CheckCompare(left, right, expected);
	}

	// 7) Add a new “slice” at the outer level
	TEST_METHOD(Vector3_AddedOuter) {
		// Outer initially has one element; we’re inserting a whole new mid‐vector {{{2}}}
		std::vector<std::vector<std::vector<int>>> left{ {{1}} };
		std::vector<std::vector<std::vector<int>>> right{ {{1}}, {{2}} };
		svh::json expected = {
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, 1 },
					  { svh::VALUE, svh::json::array({
						  svh::json::array({2})
					  }) }
				  })
			  })
			}
		};
		CheckCompare(left, right, expected);
	}

	// 8) Remove a “slice” at the outer level
	TEST_METHOD(Vector3_RemovedOuter) {
		std::vector<std::vector<std::vector<int>>> left{ {{1}}, {{2}} };
		std::vector<std::vector<std::vector<int>>> right{ {{1}} };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  svh::json(1)
			  })
			}
		};
		CheckCompare(left, right, expected);
	}

	/* Vector of bool */
	TEST_METHOD(VectorOfBool_Unchanged) {
		std::vector<bool> left{ true, false, true };
		std::vector<bool> right{ true, false, true };
		CheckCompare(left, right, svh::json());
	}

	TEST_METHOD(VectorOfBool_Changed) {
		std::vector<bool> left{ true, false, true };
		std::vector<bool> right{ false, false, true };
		svh::json expected = {
		  { svh::REMOVED_INDICES, svh::json::array({ 0 }) },
		  { svh::ADDED_VALUES, svh::json::array({
			  svh::json({
				{ svh::INDEX, 1 },
				{ svh::VALUE, false }
			  })
		  })}
		};
		CheckCompare(left, right, expected);
	}

	TEST_METHOD(VectorOfBool_Added) {
		std::vector<bool> left{ true, false };
		std::vector<bool> right{ true, false, true };
		svh::json expected = {
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, 2 },
					  { svh::VALUE, true }
				  })
			  })
			}
		};
		CheckCompare(left, right, expected);
	}

	TEST_METHOD(VectorOfBool_Removed) {
		std::vector<bool> left{ true, false, true };
		std::vector<bool> right{ true, false };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({ 2 })
			}
		};
		CheckCompare(left, right, expected);
	}
	/* Arrays */
	TEST_METHOD(Array) {
		std::array<float, 3> a{ {1.0f,2.0f,3.0f} };
		std::array<float, 3> b{ {1.0f,2.0f,3.0f} };
		CheckCompare(a, b, svh::json());
	}
	TEST_METHOD(ChangedArray) {
		std::array<float, 3> a{ {1.0f,2.0f,3.0f} };
		std::array<float, 3> b{ {4.0f,5.0f,6.0f} };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  0,1,2
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, 0 },
					  { svh::VALUE, 4.0f }
				  }),
				  svh::json({
					  { svh::INDEX, 1 },
					  { svh::VALUE, 5.0f }
				  }),
				  svh::json({
					  { svh::INDEX, 2 },
					  { svh::VALUE, 6.0f }
				  })
			  })
			}
		};
		CheckCompare(a, b, expected);
	}
	/* Array 2-depth */
	TEST_METHOD(ArrayOfArrays) {
		std::array<std::array<int, 2>, 2> a{ { {1,2}, {3,4} } };
		std::array<std::array<int, 2>, 2> b{ { {1,2}, {3,4} } };
		CheckCompare(a, b, svh::json());
	}
	TEST_METHOD(ChangedArrayOfArrays) {
		std::array<std::array<int, 2>, 2> a{ { {1,2}, {3,4} } };
		std::array<std::array<int, 2>, 2> b{ { {1,5}, {3,4} } };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  svh::json::array({0, 1})
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, svh::json::array({0, 1}) },
					  { svh::VALUE, 5 }
				  })
			  })
			}
		};
		CheckCompare(a, b, expected);
	}
	/* Array 3-depth */
	TEST_METHOD(ArrayOfArraysOfArrays) {
		std::array<std::array<std::array<int, 2>, 2>, 2> a{ {{{{{1, 2}}, {{3, 4}}}}, {{{{5, 6}}, {{7, 8}}}}} };
		std::array<std::array<std::array<int, 2>, 2>, 2> b{ {{{{{1, 2}}, {{3, 4}}}}, {{{{5, 6}}, {{7, 8}}}}} };
		CheckCompare(a, b, svh::json());
	}
	TEST_METHOD(ChangedArrayOfArraysOfArrays) {
		std::array<std::array<std::array<int, 2>, 2>, 2> a{ {{{{{1, 2}}, {{3, 4}}}}, {{{{5, 6}}, {{7, 8}}}}} };
		std::array<std::array<std::array<int, 2>, 2>, 2> b{ {{{{{1, 2}}, {{3, 4}}}}, {{{{5, 9}}, {{7, 8}}}}} };
		svh::json expected = {
		  { svh::REMOVED_INDICES,
			svh::json::array({
			  svh::json::array({1, 0, 1})
			})
		  },
		  { svh::ADDED_VALUES,
			svh::json::array({
			  svh::json({
				{ svh::INDEX, svh::json::array({1, 0, 1}) },
				{ svh::VALUE, 9 }
			  })
			})
		  }
		};
		CheckCompare(a, b, expected);
	}

	//
	//	/* Sets */
	//	TEST_METHOD(Set) {
	//		std::set<int> s{ 5,6,7 };
	//		CheckSerialization(s, svh::json::array({ 5,6,7 }));
	//	}
	//	TEST_METHOD(EmptySet) {
	//		std::set<std::string> s;
	//		CheckSerialization(s, svh::json::array({}));
	//	}
	//	TEST_METHOD(SetOfSets) {
	//		std::set<std::set<int>> ss{ {1,2}, {3,4} };
	//		CheckSerialization(ss, svh::json::array({ {1,2}, {3,4} }));
	//	}
	//	/* Unordered sets */
	//	TEST_METHOD(UnorderedSet) {
	//		std::unordered_set<int> us{ 8,9,10 };
	//		CheckSerialization(us, svh::json::array({ 8,9,10 }));
	//	}
	//	TEST_METHOD(EmptyUnorderedSet) {
	//		std::unordered_set<std::string> us;
	//		CheckSerialization(us, svh::json::array({}));
	//	}
	//	/* No default hash function for unordered_set of unordered_set */
	//
	//	/* Multisets */
	//	TEST_METHOD(Multiset) {
	//		std::multiset<int> ms{ 1,2,2,3 };
	//		CheckSerialization(ms, svh::json::array({ 1,2,2,3 }));
	//	}
	//	TEST_METHOD(UnorderedMultiset) {
	//		std::unordered_multiset<std::string> ums{ "a","b","a" };
	//		CheckSerialization(ums, svh::json::array({ "a","a","b" })); // order not guaranteed
	//	}
	//
	//	/* Deques */
	//	TEST_METHOD(Deque) {
	//		std::deque<int> d{ 11,12,13 };
	//		CheckSerialization(d, svh::json::array({ 11,12,13 }));
	//	}
	//	TEST_METHOD(EmptyDeque) {
	//		std::deque<std::string> d;
	//		CheckSerialization(d, svh::json::array({}));
	//	}
	//	TEST_METHOD(DequeOfDeques) {
	//		std::deque<std::deque<int>> dd{ {1,2}, {3,4} };
	//		CheckSerialization(dd, svh::json::array({ {1,2}, {3,4} }));
	//	}
	//
	//	/* Lists */
	//	TEST_METHOD(List) {
	//		std::list<int> l{ 14,15,16 };
	//		CheckSerialization(l, svh::json::array({ 14,15,16 }));
	//	}
	//	TEST_METHOD(EmptyList) {
	//		std::list<std::string> l;
	//		CheckSerialization(l, svh::json::array({}));
	//	}
	//	TEST_METHOD(ListOfLists) {
	//		std::list<std::list<int>> ll{ {1,2}, {3,4} };
	//		CheckSerialization(ll, svh::json::array({ {1,2}, {3,4} }));
	//	}
	//
	//	/* Initializer lists */
	//	TEST_METHOD(InitializerList) {
	//		std::initializer_list<int> il{ 17,18,19 };
	//		CheckSerialization(il, svh::json::array({ 17,18,19 }));
	//	}
	//	TEST_METHOD(EmptyInitializerList) {
	//		std::initializer_list<std::string> il{};
	//		CheckSerialization(il, svh::json::array({}));
	//	}
	//	TEST_METHOD(InitializerListOfLists) {
	//		std::initializer_list<std::initializer_list<int>> ill{ {1,2}, {3,4} };
	//		CheckSerialization(ill, svh::json::array({ {1,2}, {3,4} }));
	//	}
	//
	//	/* C-style arrays */
	//	TEST_METHOD(CStyleArray) {
	//		int arr[] = { 20,21,22 };
	//		CheckSerialization(arr, svh::json::array({ 20,21,22 }));
	//	}
	//	/* Not possible to make en empty C-style array */
	//	TEST_METHOD(CStyleArrayOfArrays) {
	//		int arr[2][2] = { {1,2}, {3,4} };
	//		CheckSerialization(arr, svh::json::array({ {1,2}, {3,4} }));
	//	}
	//
	//	/* Forward lists */
	//	TEST_METHOD(ForwardList) {
	//		std::forward_list<int> fl{ 1,2,3 };
	//		CheckSerialization(fl, svh::json::array({ 1,2,3 }));
	//	}
	//	TEST_METHOD(EmptyForwardList) {
	//		std::forward_list<std::string> fl;
	//		CheckSerialization(fl, svh::json::array({}));
	//	}
	//	TEST_METHOD(ForwardListOfLists) {
	//		std::forward_list<std::forward_list<int>> fll{ {1,2}, {3,4} };
	//		CheckSerialization(fll, svh::json::array({ {1,2}, {3,4} }));
	//	}
	//
	//	/* Map */
	//	TEST_METHOD(Map) {
	//		std::map<std::string, int> m{ {"one",1},{"two",2} };
	//		CheckSerialization(m, svh::json::object({ {"one",1},{"two",2} }));
	//	}
	//	TEST_METHOD(FloatMap) {
	//		std::map<float, int> m{ {1.0f,1},{2.0f,2} };
	//		//auto json_object = svh::json::object({ {1.0f,1},{2.0f,2} }); // isn't valid JSON
	//		//CheckSerialization(m, json_object);
	//		auto result = svh::Serializer::ToJson(m);
	//		Logger::WriteMessage(result.dump().c_str());
	//	}
	//	TEST_METHOD(EmptyMap) {
	//		std::map<std::string, int> m;
	//		CheckSerialization(m, svh::json::object({}));
	//	}
	//
	//	TEST_METHOD(MapOfMaps) {
	//		std::map<std::string, std::map<std::string, int>> mm{
	//			{"first", {{"a",1},{"b",2}}},
	//			{"second",{{"c",3},{"d",4}}}
	//		};
	//		CheckSerialization(mm,
	//			svh::json::object({
	//				{"first",  svh::json::object({{"a",1},{"b",2}})},
	//				{"second", svh::json::object({{"c",3},{"d",4}})}
	//				})
	//		);
	//	}
	//
	//	/* Unordered map*/
	//	TEST_METHOD(UnorderedMap_SingleElement) {
	//		std::unordered_map<std::string, int> um{ {"solo",42} };
	//		CheckSerialization(um, svh::json::object({ {"solo",42} }));
	//	}
	//	TEST_METHOD(EmptyUnorderedMap) {
	//		std::unordered_map<std::string, int> um;
	//		CheckSerialization(um, svh::json::object({}));
	//	}
	//	TEST_METHOD(UnorderedMapOfMaps) {
	//		std::unordered_map<std::string, std::unordered_map<std::string, int>> umm{
	//			{"first", {{"a",1},{"b",2}}},
	//			{"second",{{"c",3},{"d",4}}}
	//		};
	//		CheckSerialization(umm,
	//			svh::json::object({
	//				{"first",  svh::json::object({{"a",1},{"b",2}})},
	//				{"second", svh::json::object({{"c",3},{"d",4}})}
	//				})
	//		);
	//	}
	//
	//	/* Multimap */
	//	TEST_METHOD(Multimap) {
	//		std::multimap<std::string, int> mm{ {"one",1},{"one",2} };
	//		CheckSerialization(mm, svh::json::object({ {"one",svh::json::array({1,2})} }));
	//	}
	//	TEST_METHOD(EmptyMultimap) {
	//		std::multimap<std::string, int> mm;
	//		CheckSerialization(mm, svh::json::object({}));
	//	}
	//	TEST_METHOD(MultimapOfMaps) {
	//		std::multimap<std::string, std::multimap<std::string, int>> mmm{
	//			{"first", {{"a",1},{"b",2}}},
	//			{"second",{{"c",3},{"d",4}}}
	//		};
	//		CheckSerialization(mmm,
	//			svh::json::object({
	//				{"first",  svh::json::object({{"a",1},{"b",2}})},
	//				{"second", svh::json::object({{"c",3},{"d",4}})}
	//				})
	//		);
	//	}
	//
	//	/* Unordered multimap */
	//	TEST_METHOD(UnorderedMultimap) {
	//		std::unordered_multimap<std::string, int> umm{ {"one",1},{"one",2} };
	//		CheckSerialization(umm, svh::json::object({ {"one",svh::json::array({1,2})} }));
	//	}
	//	TEST_METHOD(EmptyUnorderedMultimap) {
	//		std::unordered_multimap<std::string, int> umm;
	//		CheckSerialization(umm, svh::json::object({}));
	//	}
	//	TEST_METHOD(UnorderedMultimapOfMaps) {
	//		std::unordered_multimap<std::string, std::unordered_multimap<std::string, int>> ummm{
	//			{"first", {{"a",1},{"b",2}}},
	//			{"second",{{"c",3},{"d",4}}}
	//		};
	//		CheckSerialization(ummm,
	//			svh::json::object({
	//				{"first",  svh::json::object({{"a",1},{"b",2}})},
	//				{"second", svh::json::object({{"c",3},{"d",4}})}
	//				})
	//		);
	//	}
	};
	//
	//	/* Test pair and tuple types */
	//	TEST_CLASS(PairTupleTypes) {
	//public:
	//	TEST_METHOD(Pair) {
	//		std::pair<int, std::string> p{ 7, "seven" };
	//		CheckSerialization(p, svh::json::array({ 7,"seven" }));
	//	}
	//
	//	TEST_METHOD(Tuple) {
	//		std::tuple<int, float, bool> t{ 8, 2.5f, false };
	//		CheckSerialization(t, svh::json::array({ 8,2.5f,false }));
	//	}
	//	TEST_METHOD(EmptyTuple) {
	//		std::tuple<> t;
	//		CheckSerialization(t, svh::json::array({}));
	//	}
	//
	//	};
	//
	//	/* Optional and variant */
	//	TEST_CLASS(OptionalVariantTypes) {
	//public:
	//	/* With int*/
	//	TEST_METHOD(Optional) {
	//		std::optional<int> opt = 99;
	//		CheckSerialization(opt, svh::json(99));
	//	}
	//	TEST_METHOD(Optional_Null) {
	//		std::optional<int> opt;
	//		CheckSerialization(opt, svh::json(nullptr));
	//	}
	//
	//	/* With string */
	//	TEST_METHOD(Variant) {
	//		std::variant<int, std::string> v = 100;
	//		CheckSerialization(v, svh::json(100));
	//	}
	//	TEST_METHOD(VariantString) {
	//		std::variant<int, std::string> v = "hello";
	//		CheckSerialization(v, svh::json("hello"));
	//	}
	//
	//	/* With struct */
	//	TEST_METHOD(VariantWithStruct) {
	//		std::variant<int, Transform> v = Transform{ {1.0f,2.0f,3.0f}, {1.0f,0.0f,0.0f,0.0f}, {1.0f,1.0f,1.0f} };
	//		CheckSerialization(v,
	//			svh::json::object({
	//				{"position", svh::json::array({ 1.0f,2.0f,3.0f })},
	//				{"rotation", svh::json::array({ 0.0f,0.0f,0.0f,1.0f })},
	//				{"scale",    svh::json::array({ 1.0f,1.0f,1.0f })},
	//				})
	//				);
	//	}
	//	};
	//
	//	/* Test raw pointers */ /* Currently doesn't support raw pointers */
	//	//TEST_CLASS(RawPointerTypes) {
	//	//	TEST_METHOD(RawPointer_NonNull) {
	//	//		int x = 5;
	//	//		int* p = &x;
	//	//		CheckSerialization(p, svh::json(5));
	//	//	}
	//	//	TEST_METHOD(RawPointer_Null) {
	//	//		int* p = nullptr;
	//	//		CheckSerialization(p, svh::json(nullptr));
	//	//	}
	//	//};
	//
	//	/* Test smart pointers */
	//	TEST_CLASS(PointerTypes) {
	//public:
	//	/* Unique pointers */
	//	TEST_METHOD(UniquePointer_NonNull) {
	//		auto up = std::make_unique<int>(99);
	//		CheckSerialization(up, svh::json(99));
	//	}
	//
	//	TEST_METHOD(UniquePointer_Null) {
	//		std::unique_ptr<int> up;
	//		CheckSerialization(up, svh::json(nullptr));
	//	}
	//
	//	/* Shared pointers */
	//	TEST_METHOD(SharedPointer_NonNull) {
	//		auto sp = std::make_shared<int>(123);
	//		CheckSerialization(sp, svh::json(123));
	//	}
	//
	//	TEST_METHOD(SharedPointer_Null) {
	//		std::shared_ptr<int> sp;
	//		CheckSerialization(sp, svh::json(nullptr));
	//	}
	//
	//	TEST_METHOD(SharedPointerOfVector) {
	//		auto spv = std::make_shared<std::vector<int>>(std::vector<int>{4, 5, 6});
	//		CheckSerialization(spv, svh::json::array({ 4,5,6 }));
	//	}
	//
	//	/* Weak pointers */
	//	TEST_METHOD(WeakPointer_Locked) {
	//		auto sp = std::make_shared<int>(42);
	//		std::weak_ptr<int> wp = sp;
	//		CheckSerialization(wp, svh::json(42));
	//	}
	//	TEST_METHOD(WeakPointer_Expired) {
	//		std::weak_ptr<int> wp;
	//		CheckSerialization(wp, svh::json(nullptr));
	//	}
	//	};
	//
		/* Test visitable structs */
	TEST_CLASS(VisitableStructs) {
public:
	TEST_METHOD(TransformStruct) {
		Transform A;
		A.position = { 1.0f, 2.0f, 3.0f };
		A.rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
		A.scale = { 1.0f, 1.0f, 1.0f };

		CheckCompare(A, A, svh::json());
	}

	TEST_METHOD(TransformStruct_Changed) {
		Transform A;
		A.position = { 1.0f, 2.0f, 3.0f };
		A.rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
		A.scale = { 1.0f, 1.0f, 1.0f };
		Transform B;
		B.position = { 4.0f, 5.0f, 6.0f };
		B.rotation = { 1.0f, 0.0f, 0.0f, 0.0f }; // rotation unchanged
		B.scale = { 11.0f, 12.0f, 13.0f };
		CheckCompare(A, B,
			svh::json::object({
				{"position", svh::json::array({4.0f,5.0f,6.0f})},
				{"scale",    svh::json::array({11.0f,12.0f,13.0f})}
				})
		);
	}

	TEST_METHOD(ItemHolderStruct_Changed) {
		//ItemHolder ih;
		//ih.item_count = 3;
		//ih.items = { 1, 2, 3 };
		//CheckSerialization(ih,
		//	svh::json::object({
		//		{"item_count", 3},
		//		{"items", svh::json::array({1,2,3})}
		//		})
		//);
		ItemHolder A;
		A.item_count = 3;
		A.items = { 1, 2, 3 };
		ItemHolder B;
		B.item_count = 4;
		B.items = { 1, 2, 3, 4 };
		CheckCompare(A, B,
			svh::json::object({
				{"item_count", 4},
				{"items", svh::Compare::GetChanges(A.items, B.items)}
				})
		);
	}

	TEST_METHOD(GameEntityStruct) {
		//GameEntity ge;
		//ge.name = "entity";
		//ge.transform = std::make_shared<Transform>();
		//ge.item_holder = std::make_shared<ItemHolder>();
		//CheckSerialization(ge,
		//	svh::json::object({
		//		{"name", "entity"},
		//		{"transform", svh::Serializer::ToJson(ge.transform)},
		//		{"item_holder", svh::Serializer::ToJson(ge.item_holder)}
		//		})
		//);
		GameEntity A;
		A.name = "entity";
		A.transform = std::make_shared<Transform>();
		A.item_holder = std::make_shared<ItemHolder>();
		GameEntity B;
		B.name = "entity";
		B.transform = std::make_shared<Transform>();
		B.item_holder = std::make_shared<ItemHolder>();
		CheckCompare(A, B,
			svh::json::object({
				{"transform", svh::Serializer::ToJson(A.transform)},
				{"item_holder", svh::Serializer::ToJson(A.item_holder)}
				})
		);
	}
	};
	//
	//	TEST_CLASS(Inheritance) {
	//public:
	//	TEST_METHOD(InheritedComponent) {
	//		DerivedComponent mc;
	//		mc.name = "Test";
	//		mc.value = 42;
	//		mc.id = 0;
	//		CheckSerialization(mc,
	//			svh::json::object({
	//				{"name", "Test"},
	//				{"value",42},
	//				{"id", 0}
	//				})
	//		);
	//	}
	//
	//	TEST_METHOD(BaseComponentTest) {
	//		BaseComponent* bc = new DerivedComponent();
	//		bc->id = 1;
	//		CheckSerialization(*bc,
	//			svh::json::object({
	//				{"id", 1}
	//				})
	//		);
	//	}
	//	};
	//
	//	/* Test larger integer types */
	//	TEST_CLASS(LargerIntegerTypes) {
	//public:
	//	TEST_METHOD(LongLong) {
	//		long long ll = 123456789012345LL;
	//		CheckSerialization(ll, svh::json(123456789012345LL));
	//	}
	//	TEST_METHOD(LongLongMin) {
	//		long long ll = std::numeric_limits<long long>::min();
	//		CheckSerialization(ll, svh::json(std::numeric_limits<long long>::min()));
	//	}
	//	TEST_METHOD(LongLongMax) {
	//		long long ll = std::numeric_limits<long long>::max();
	//		CheckSerialization(ll, svh::json(std::numeric_limits<long long>::max()));
	//	}
	//
	//	// unsigned long long
	//	TEST_METHOD(UnsignedLongLong) {
	//		unsigned long long ull = 123456789012345ULL;
	//		CheckSerialization(ull, svh::json(123456789012345ULL));
	//	}
	//	TEST_METHOD(UnsignedLongLongMax) {
	//		unsigned long long ull = std::numeric_limits<unsigned long long>::max();
	//		CheckSerialization(ull, svh::json(std::numeric_limits<unsigned long long>::max()));
	//	}
	//
	//	// size_t
	//	TEST_METHOD(SizeT) {
	//		std::size_t sz = 1234567890ULL;
	//		CheckSerialization(sz, svh::json(1234567890ULL));
	//	}
	//	TEST_METHOD(SizeTMax) {
	//		std::size_t sz = std::numeric_limits<std::size_t>::max();
	//		CheckSerialization(sz, svh::json(std::numeric_limits<std::size_t>::max()));
	//	}
	//	};
	//
	//	/* Test complex nested structures */
	//	TEST_CLASS(ComplexNestedTest) {
	//public:
	//	TEST_METHOD(PlayerEntitySerialization) {
	//		// Build a deeply nested object
	//		auto transform = std::make_shared<Transform>();
	//		transform->position = { 1.0f, 2.0f, 3.0f };
	//		transform->rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
	//		transform->scale = { 1.0f, 1.0f, 1.0f };
	//
	//		Inventory inv;
	//		inv.items = { "potion", "elixir" };
	//		inv.ammo = { {"arrows", 20} };
	//		inv.misc = { "key", "map" };
	//
	//		// Create weapons using moves
	//		auto w1 = std::make_unique<Weapon>();
	//		w1->name = "Sword"; w1->damage = 10;
	//		auto w2 = std::make_unique<Weapon>();
	//		w2->name = "Bow";   w2->damage = 7;
	//
	//		// Create armors
	//		auto a1 = std::make_shared<Armor>();
	//		a1->name = "Helmet"; a1->defense = 5;
	//		auto a2 = std::make_shared<Armor>();
	//		a2->name = "Armor";  a2->defense = 15;
	//
	//		// Setup skill tree
	//		Skill s1; s1.name = "Fireball"; s1.level = 3;
	//		Skill s2; s2.name = "IceShard"; s2.level = 2;
	//		s2.subskills.push_back(Skill{ "Freeze", 1, {} });
	//		SkillTree st; st.skills = { s1, s2 };
	//
	//		PlayerEntity player;
	//		player.id = "player1";
	//		player.transform = transform;
	//		player.inventory = inv;
	//
	//		// Assign weapons explicitly (unique_ptr cannot be copied)
	//		player.weapons.clear();
	//		player.weapons.push_back(std::move(w1));
	//		player.weapons.push_back(std::move(w2));
	//
	//		player.armors = { { "head", a1 }, { "body", a2 } };
	//		player.skill_tree = st;
	//
	//		// Expected JSON
	//		svh::json expected = svh::json::object({
	//			{ "id", "player1" },
	//			{ "transform", svh::json::object({
	//				{ "position", svh::json::array({1.0f,2.0f,3.0f}) },
	//				{ "rotation", svh::json::array({0.0f,0.0f,0.0f,1.0f}) },
	//				{ "scale",    svh::json::array({1.0f,1.0f,1.0f}) }
	//			})},
	//			{ "inventory", svh::json::object({
	//				{ "items", svh::json::array({"potion","elixir"}) },
	//				{ "ammo",  svh::json::object({{"arrows",20}}) },
	//				{ "misc",  svh::json::array({"key","map"}) }
	//			})},
	//			{ "weapons", svh::json::array({
	//				svh::json::object({{"name","Sword"},{"damage",10}}),
	//				svh::json::object({{"name","Bow"},{"damage",7}})
	//			})},
	//			{ "armors", svh::json::object({
	//				{ "body", svh::json::object({{"name","Armor"},{"defense",15}}) },
	//				{ "head", svh::json::object({{"name","Helmet"},{"defense",5}}) }
	//			})},
	//			{ "skill_tree", svh::json::object({
	//				{ "skills", svh::json::array({
	//					svh::json::object({
	//						{"name","Fireball"},
	//						{"level",3},
	//						{"subskills", svh::json::array({})}
	//					}),
	//					svh::json::object({
	//						{"name","IceShard"},
	//						{"level",2},
	//						{"subskills", svh::json::array({
	//							svh::json::object({{"name","Freeze"},{"level",1},{"subskills",svh::json::array({})}})
	//						})}
	//					})
	//				})}
	//			})}
	//			});
	//
	//		CheckSerialization(player, expected);
	//	}
	//	};

} // namespace prefabstests