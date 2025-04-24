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
	TEST_METHOD(Vector_Changed) {
		std::vector<int> left{ 1,2,3 };
		std::vector<int> right{ 4,5,6 };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				{0},{1},{2}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0 } },
					  { svh::VALUE, 4 }
				  }),
				  svh::json({
					  { svh::INDEX, { 1 } },
					  { svh::VALUE, 5 }
				  }),
				  svh::json({
					  { svh::INDEX, { 2 } },
					  { svh::VALUE, 6 }
				  })
			  })
		}
		};
		CheckCompare(left, right, expected);
	}
	TEST_METHOD(Vector_Added) {
		std::vector<int> left{ 1,2,3 };
		std::vector<int> right{ 1,2,3,4 };

		svh::json expected = {
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 3 } },
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
					  { svh::INDEX, { 2 } },
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
				  svh::json({2})
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
					  { svh::INDEX, { 1 } },
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
				  svh::json({1})
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
		  { svh::REMOVED_INDICES, svh::json::array({ {0} }) },
		  { svh::ADDED_VALUES, svh::json::array({
			  svh::json({
				{ svh::INDEX, { 1 } },
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
					  { svh::INDEX, { 2 } },
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
			  svh::json::array({ {2} })
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
		// {"removed indices":[[0],[1],[2]],"added":[{"index":[0],"value":4.0},{"index":[1],"value":5.0},{"index":[2],"value":6.0}]}
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0},{1},{2}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, {0} },
					  { svh::VALUE, 4.0 }
				  }),
				  svh::json({
					  { svh::INDEX, {1} },
					  { svh::VALUE, 5.0 }
				  }),
				  svh::json({
					  { svh::INDEX, {2} },
					  { svh::VALUE, 6.0 }
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

	/* Sets */
	TEST_METHOD(Set_Unchanged) {
		/*std::set<int> s{ 5,6,7 };
		CheckSerialization(s, svh::json::array({ 5,6,7 }));*/
		std::set<int> A{ 5,6,7 };
		std::set<int> B{ 5,6,7 };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(Set_Changed) {
		std::set<int> A{ 5,6,7 };
		std::set<int> B{ 8,9,10 };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0},{1},{2}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0 } },
					  { svh::VALUE, 8 }
				  }),
				  svh::json({
					  { svh::INDEX, { 1 } },
					  { svh::VALUE, 9 }
				  }),
				  svh::json({
					  { svh::INDEX, { 2 } },
					  { svh::VALUE, 10 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	//TEST_METHOD(SetOfSets) {
	//	std::set<std::set<int>> ss{ {1,2}, {3,4} };
	//	CheckSerialization(ss, svh::json::array({ {1,2}, {3,4} }));
	//}
	TEST_METHOD(SetOfSets_Unchanged) {
		std::set<std::set<int>> A{ {1,2}, {3,4} };
		std::set<std::set<int>> B{ {1,2}, {3,4} };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(SetOfSets_Changed) {
		std::set<std::set<int>> A{ {1,2}, {3,4} };
		std::set<std::set<int>> B{ {5,6}, {7,8} };
		//{"removed indices":[[0,0],[0,1],[1,0],[1,1]],"added":[{"index":[0,0],"value":5},{"index":[0,1],"value":6},{"index":[1,0],"value":7},{"index":[1,1],"value":8}]}
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  svh::json::array({0, 0}),
				  svh::json::array({0, 1}),
				  svh::json::array({1, 0}),
				  svh::json::array({1, 1})
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0, 0 } },
					  { svh::VALUE, 5 }
				  }),
				  svh::json({
					  { svh::INDEX, { 0, 1 } },
					  { svh::VALUE, 6 }
				  }),
				  svh::json({
					  { svh::INDEX, { 1, 0 } },
					  { svh::VALUE, 7 }
				  }),
				  svh::json({
					  { svh::INDEX, { 1, 1 } },
					  { svh::VALUE, 8 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	///* Unordered sets */
	//TEST_METHOD(UnorderedSet) {
	//	std::unordered_set<int> us{ 8,9,10 };
	//	CheckSerialization(us, svh::json::array({ 8,9,10 }));
	//}
	TEST_METHOD(UnorderedSet_Unchanged) {
		std::unordered_set<int> A{ 8,9,10 };
		std::unordered_set<int> B{ 8,9,10 };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(UnorderedSet_Changed) {
		std::unordered_set<int> A{ 8,9,10 };
		std::unordered_set<int> B{ 11,12,13 };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0},{1},{2}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0 } },
					  { svh::VALUE, 11 }
				  }),
				  svh::json({
					  { svh::INDEX, { 1 } },
					  { svh::VALUE, 12 }
				  }),
				  svh::json({
					  { svh::INDEX, { 2 } },
					  { svh::VALUE, 13 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	// 
	//TEST_METHOD(EmptyUnorderedSet) {
	//	std::unordered_set<std::string> us;
	//	CheckSerialization(us, svh::json::array({}));
	//}
	///* No default hash function for unordered_set of unordered_set */

	///* Multisets */
	//TEST_METHOD(Multiset) {
	//	std::multiset<int> ms{ 1,2,2,3 };
	//	CheckSerialization(ms, svh::json::array({ 1,2,2,3 }));
	//}
	TEST_METHOD(Multiset_Unchanged) {
		std::multiset<int> A{ 1,2,2,3 };
		std::multiset<int> B{ 1,2,2,3 };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(Multiset_Changed) {
		std::multiset<int> A{ 1,2,2,3 };
		std::multiset<int> B{ 4,5,6 };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0},{1},{2},{3}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0 } },
					  { svh::VALUE, 4 }
				  }),
				  svh::json({
					  { svh::INDEX, { 1 } },
					  { svh::VALUE, 5 }
				  }),
				  svh::json({
					  { svh::INDEX, { 2 } },
					  { svh::VALUE, 6 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(MultisetOfMultisets_Changed) {
		std::multiset<std::multiset<int>> A{ {1,2}, {3,4} };
		std::multiset<std::multiset<int>> B{ {5,6}, {7,8} };
		//{"removed indices":[[0,0],[0,1],[1,0],[1,1]],"added":[{"index":[0,0],"value":5},{"index":[0,1],"value":6},{"index":[1,0],"value":7},{"index":[1,1],"value":8}]}
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  svh::json::array({0, 0}),
				  svh::json::array({0, 1}),
				  svh::json::array({1, 0}),
				  svh::json::array({1, 1})
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0, 0 } },
					  { svh::VALUE, 5 }
				  }),
				  svh::json({
					  { svh::INDEX, { 0, 1 } },
					  { svh::VALUE, 6 }
				  }),
				  svh::json({
					  { svh::INDEX, { 1, 0 } },
					  { svh::VALUE, 7 }
				  }),
				  svh::json({
					  { svh::INDEX, { 1, 1 } },
					  { svh::VALUE, 8 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	//TEST_METHOD(UnorderedMultiset) {
	//	std::unordered_multiset<std::string> ums{ "a","b","a" };
	//	CheckSerialization(ums, svh::json::array({ "a","a","b" })); // order not guaranteed
	//}
	TEST_METHOD(UnorderedMultiset_Unchanged) {
		std::unordered_multiset<std::string> A{ "a","b","a" };
		std::unordered_multiset<std::string> B{ "a","b","a" };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(UnorderedMultiset_Changed) {
		std::unordered_multiset<std::string> A{ "a","b","a" };
		std::unordered_multiset<std::string> B{ "c","d","e" };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0},{1},{2}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0 } },
					  { svh::VALUE, "c" }
				  }),
				  svh::json({
					  { svh::INDEX, { 1 } },
					  { svh::VALUE, "d" }
				  }),
				  svh::json({
					  { svh::INDEX, { 2 } },
					  { svh::VALUE, "e" }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	//
	//	/* Deques */
	//	TEST_METHOD(Deque) {
	//		std::deque<int> d{ 11,12,13 };
	//		CheckSerialization(d, svh::json::array({ 11,12,13 }));
	//	}
	TEST_METHOD(Deque_Unchanged) {
		std::deque<int> A{ 11,12,13 };
		std::deque<int> B{ 11,12,13 };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(Deque_Changed) {
		std::deque<int> A{ 11,12,13 };
		std::deque<int> B{ 14,15,16 };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0},{1},{2}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0 } },
					  { svh::VALUE, 14 }
				  }),
				  svh::json({
					  { svh::INDEX, { 1 } },
					  { svh::VALUE, 15 }
				  }),
				  svh::json({
					  { svh::INDEX, { 2 } },
					  { svh::VALUE, 16 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(Deque_Added) {
		std::deque<int> A{ 11,12,13 };
		std::deque<int> B{ 11,12,13,14 };
		svh::json expected = {
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 3 } },
					  { svh::VALUE, 14 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(Deque_Removed) {
		std::deque<int> A{ 11,12,13 };
		std::deque<int> B{ 11,12 };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({{ 2 }})
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(DequeOfDeques_Unchanged) {
		std::deque<std::deque<int>> A{ {1,2}, {3,4} };
		std::deque<std::deque<int>> B{ {1,2}, {3,4} };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(DequeOfDeques_Changed) {
		std::deque<std::deque<int>> A{ {1,2}, {3,4} };
		std::deque<std::deque<int>> B{ {5,6}, {7,8} };
		//{"removed indices":[[0,0],[0,1],[1,0],[1,1]],"added":[{"index":[0,0],"value":5},{"index":[0,1],"value":6},{"index":[1,0],"value":7},{"index":[1,1],"value":8}]}
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  svh::json::array({0, 0}),
				  svh::json::array({0, 1}),
				  svh::json::array({1, 0}),
				  svh::json::array({1, 1})
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, svh::json::array({0, 0}) },
					  { svh::VALUE, 5 }
				  }),
				  svh::json({
					  { svh::INDEX, svh::json::array({0, 1}) },
					  { svh::VALUE, 6 }
				  }),
				  svh::json({
					  { svh::INDEX, svh::json::array({1, 0}) },
					  { svh::VALUE, 7 }
				  }),
				  svh::json({
					  { svh::INDEX, svh::json::array({1, 1}) },
					  { svh::VALUE, 8 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	//	/* Lists */
	//	TEST_METHOD(List) {
	//		std::list<int> l{ 14,15,16 };
	//		CheckSerialization(l, svh::json::array({ 14,15,16 }));
	//	}
	TEST_METHOD(List_Unchanged) {
		std::list<int> A{ 14,15,16 };
		std::list<int> B{ 14,15,16 };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(List_Changed) {
		std::list<int> A{ 14,15,16 };
		std::list<int> B{ 17,18,19 };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0},{1},{2}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0 } },
					  { svh::VALUE, 17 }
				  }),
				  svh::json({
					  { svh::INDEX, { 1 } },
					  { svh::VALUE, 18 }
				  }),
				  svh::json({
					  { svh::INDEX, { 2 } },
					  { svh::VALUE, 19 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(List_Added) {
		std::list<int> A{ 14,15,16 };
		std::list<int> B{ 14,15,16,17 };
		svh::json expected = {
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 3 } },
					  { svh::VALUE, 17 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(List_Removed) {
		std::list<int> A{ 14,15,16 };
		std::list<int> B{ 14,15 };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({ {2} })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(ListOfLists_Unchanged) {
		std::list<std::list<int>> A{ {1,2}, {3,4} };
		std::list<std::list<int>> B{ {1,2}, {3,4} };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(ListOfLists_Changed) {
		std::list<std::list<int>> A{ {1,2}, {3,4} };
		std::list<std::list<int>> B{ {5,6}, {7,8} };
		//{"removed indices":[[0,0],[0,1],[1,0],[1,1]],"added":[{"index":[0,0],"value":5},{"index":[0,1],"value":6},{"index":[1,0],"value":7},{"index":[1,1],"value":8}]}
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  svh::json::array({0, 0}),
				  svh::json::array({0, 1}),
				  svh::json::array({1, 0}),
				  svh::json::array({1, 1})
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, svh::json::array({0, 0}) },
					  { svh::VALUE, 5 }
				  }),
				  svh::json({
					  { svh::INDEX, svh::json::array({0, 1}) },
					  { svh::VALUE, 6 }
				  }),
				  svh::json({
					  { svh::INDEX, svh::json::array({1, 0}) },
					  { svh::VALUE, 7 }
				  }),
				  svh::json({
					  { svh::INDEX, svh::json::array({1, 1}) },
					  { svh::VALUE, 8 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}

	/* Initializer lists */
	TEST_METHOD(InitializerList_Unchanged) {
		std::initializer_list<int> A{ 17,18,19 };
		std::initializer_list<int> B{ 17,18,19 };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(InitializerList_Changed) {
		std::initializer_list<int> A{ 17,18,19 };
		std::initializer_list<int> B{ 20,21,22 };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0},{1},{2}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0 } },
					  { svh::VALUE, 20 }
				  }),
				  svh::json({
					  { svh::INDEX, { 1 } },
					  { svh::VALUE, 21 }
				  }),
				  svh::json({
					  { svh::INDEX, { 2 } },
					  { svh::VALUE, 22 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(InitializerList_Added) {
		std::initializer_list<int> A{ 17,18,19 };
		std::initializer_list<int> B{ 17,18,19,20 };
		svh::json expected = {
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 3 } },
					  { svh::VALUE, 20 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(InitializerList_Removed) {
		std::initializer_list<int> A{ 17,18,19 };
		std::initializer_list<int> B{ 17,18 };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({ {2} })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(InitializerListOfLists_Unchanged) {
		std::initializer_list<std::initializer_list<int>> A{ {1,2}, {3,4} };
		std::initializer_list<std::initializer_list<int>> B{ {1,2}, {3,4} };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(InitializerListOfLists_Changed) {
		std::initializer_list<std::initializer_list<int>> A{ {1,2}, {3,4} };
		std::initializer_list<std::initializer_list<int>> B{ {5,6}, {7,8} };
		//{"removed indices":[[0,0],[0,1],[1,0],[1,1]],"added":[{"index":[0,0],"value":5},{"index":[0,1],"value":6},{"index":[1,0],"value":7},{"index":[1,1],"value":8}]}
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  svh::json::array({0, 0}),
				  svh::json::array({0, 1}),
				  svh::json::array({1, 0}),
				  svh::json::array({1, 1})
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, svh::json::array({0, 0}) },
					  { svh::VALUE, 5 }
				  }),
				  svh::json({
					  { svh::INDEX, svh::json::array({0, 1}) },
					  { svh::VALUE, 6 }
				  }),
				  svh::json({
					  { svh::INDEX, svh::json::array({1, 0}) },
					  { svh::VALUE, 7 }
				  }),
				  svh::json({
					  { svh::INDEX, svh::json::array({1, 1}) },
					  { svh::VALUE, 8 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	//	/* C-style arrays */
	//	TEST_METHOD(CStyleArray) {
	//		int arr[] = { 20,21,22 };
	//		CheckSerialization(arr, svh::json::array({ 20,21,22 }));
	//	}
	TEST_METHOD(CStyleArray_Unchanged) {
		int arr[] = { 20,21,22 };
		int arr2[] = { 20,21,22 };
		CheckCompare(arr, arr2, svh::json());
	}
	TEST_METHOD(CStyleArray_Changed) {
		int arr[] = { 20,21,22 };
		int arr2[] = { 23,24,25 };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0},{1},{2}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, {0} },
					  { svh::VALUE, 23 }
				  }),
				  svh::json({
					  { svh::INDEX, {1} },
					  { svh::VALUE, 24 }
				  }),
				  svh::json({
					  { svh::INDEX, {2} },
					  { svh::VALUE, 25 }
				  })
			  })
			}
		};
		CheckCompare(arr, arr2, expected);
	}
	/* Added or Removed is maybe possible to compare but out of scope for this project*/

	//	/* Not possible to make en empty C-style array */
	//	TEST_METHOD(CStyleArrayOfArrays) {
	//		int arr[2][2] = { {1,2}, {3,4} };
	//		CheckSerialization(arr, svh::json::array({ {1,2}, {3,4} }));
	//	}
	TEST_METHOD(CStyleArrayOfArrays_Unchanged) {
		int arr[2][2] = { {1,2}, {3,4} };
		int arr2[2][2] = { {1,2}, {3,4} };
		CheckCompare(arr, arr2, svh::json());
	}
	TEST_METHOD(CStyleArrayOfArrays_Changed) {
		int arr[2][2] = { {1,2}, {3,4} };
		int arr2[2][2] = { {5,6}, {7,8} };
		//{"removed indices":[[0,0],[0,1],[1,0],[1,1]],"added":[{"index":[0,0],"value":5},{"index":[0,1],"value":6},{"index":[1,0],"value":7},{"index":[1,1],"value":8}]}
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0,0},{0,1},{1,0},{1,1}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, {0,0} },
					  { svh::VALUE, 5 }
				  }),
				  svh::json({
					  { svh::INDEX, {0,1} },
					  { svh::VALUE, 6 }
				  }),
				  svh::json({
					  { svh::INDEX, {1,0} },
					  { svh::VALUE, 7 }
				  }),
				  svh::json({
					  { svh::INDEX, {1,1} },
					  { svh::VALUE, 8 }
				  })
			  })
			}
		};
		CheckCompare(arr, arr2, expected);
	}
	//	/* Forward lists */
	//	TEST_METHOD(ForwardList) {
	//		std::forward_list<int> fl{ 1,2,3 };
	//		CheckSerialization(fl, svh::json::array({ 1,2,3 }));
	//	}
	TEST_METHOD(ForwardsList_Unchanged) {
		std::forward_list<int> A{ 1,2,3 };
		std::forward_list<int> B{ 1,2,3 };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(ForwardsList_Changed) {
		std::forward_list<int> A{ 1,2,3 };
		std::forward_list<int> B{ 4,5,6 };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0},{1},{2}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0 } },
					  { svh::VALUE, 4 }
				  }),
				  svh::json({
					  { svh::INDEX, { 1 } },
					  { svh::VALUE, 5 }
				  }),
				  svh::json({
					  { svh::INDEX, { 2 } },
					  { svh::VALUE, 6 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(ForwardsList_Added) {
		std::forward_list<int> A{ 1,2,3 };
		std::forward_list<int> B{ 1,2,3,4 };
		svh::json expected = {
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 3 } },
					  { svh::VALUE, 4 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(ForwardsList_Removed) {
		std::forward_list<int> A{ 1,2,3 };
		std::forward_list<int> B{ 1,2 };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({ {2} })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(ForwardsListOfLists_Unchanged) {
		std::forward_list<std::forward_list<int>> A{ {1,2}, {3,4} };
		std::forward_list<std::forward_list<int>> B{ {1,2}, {3,4} };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(ForwardsListOfLists_Changed) {
		std::forward_list<std::forward_list<int>> A{ {1,2}, {3,4} };
		std::forward_list<std::forward_list<int>> B{ {5,6}, {7,8} };
		//{"removed indices":[[0,0],[0,1],[1,0],[1,1]],"added":[{"index":[0,0],"value":5},{"index":[0,1],"value":6},{"index":[1,0],"value":7},{"index":[1,1],"value":8}]}
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  svh::json::array({0, 0}),
				  svh::json::array({0, 1}),
				  svh::json::array({1, 0}),
				  svh::json::array({1, 1})
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, svh::json::array({0, 0}) },
					  { svh::VALUE, 5 }
				  }),
				  svh::json({
					  { svh::INDEX, svh::json::array({0, 1}) },
					  { svh::VALUE, 6 }
				  }),
				  svh::json({
					  { svh::INDEX, svh::json::array({1, 0}) },
					  { svh::VALUE, 7 }
				  }),
				  svh::json({
					  { svh::INDEX, svh::json::array({1, 1}) },
					  { svh::VALUE, 8 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}

	/* Map */
	/* WARNING, maps are ordered based on the key, not the order in which you put them */
	TEST_METHOD(Map_Unchanged) {
		std::map<std::string, int> A{ {"one",1},{"two",2} };
		std::map<std::string, int> B{ {"one",1},{"two",2} };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(Map_Changed) {
		std::map<std::string, int> A{ {"one",1},{"two",2} };
		std::map<std::string, int> B{ {"three",3},{"four",4} };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0},{1}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0 } },
					  { svh::VALUE, {"four",4} }
				  }),
				  svh::json({
					  { svh::INDEX, { 1 } },
					  { svh::VALUE, {"three",3} }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(Map_Added) {
		std::map<std::string, int> A{ {"one",1},{"two",2} };
		std::map<std::string, int> B{ {"one",1},{"two",2},{"three",3} };
		svh::json expected = {
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 1 } }, // index 1 since th becomes before tw. 
					  { svh::VALUE, {"three",3} }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}

	TEST_METHOD(Map_Removed) {
		std::map<std::string, int> A{ {"one",1},{"two",2} };
		std::map<std::string, int> B{ {"one",1} };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({ {1} })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(MapOfMaps_Unchanged) {
		std::map<std::string, std::map<std::string, int>> mm{
			{"first", {{"a",1},{"b",2}}},
			{"second",{{"c",3},{"d",4}}}
		};
		std::map<std::string, std::map<std::string, int>> mm2{
			{"first", {{"a",1},{"b",2}}},
			{"second",{{"c",3},{"d",4}}}
		};
		CheckCompare(mm, mm2, svh::json());
	}
	TEST_METHOD(MapOfMaps_Changed) {
		std::map<std::string, std::map<std::string, int>> mm{
			{"first", {{"a",1},{"b",2}}},
			{"second",{{"c",3},{"d",4}}}
		};
		std::map<std::string, std::map<std::string, int>> mm2{
			{"first", {{"a",5},{"b",6}}},
			{"second",{{"c",7},{"d",8}}}
		};
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0},{1}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0 } },
					  { svh::VALUE, {"first",svh::json::object({{"a",5},{"b",6}})} }
				  }),
				  svh::json({
					  { svh::INDEX, { 1 } },
					  { svh::VALUE, {"second",svh::json::object({{"c",7},{"d",8}})} }
				  })
			  })
			}
		};
		CheckCompare(mm, mm2, expected);
	}

	/* Unordered map*/
	TEST_METHOD(UnorderedMap_Unchanged) {
		std::unordered_map<std::string, int> A{ {"a",1},{"b",2} };
		std::unordered_map<std::string, int> B{ {"a",1},{"b",2} };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(UnorderedMap_Changed) {
		std::unordered_map<std::string, int> A{ {"a",1},{"b",2} };
		std::unordered_map<std::string, int> B{ {"c",3},{"d",4} };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0},{1}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0 } },
					  { svh::VALUE, {"c",3} }
				  }),
				  svh::json({
					  { svh::INDEX, { 1 } },
					  { svh::VALUE, {"d",4} }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(UnorderedMap_Added) {
		std::unordered_map<std::string, int> A{ {"a",1},{"b",2} };
		std::unordered_map<std::string, int> B{ {"a",1},{"b",2},{"c",3} };
		svh::json expected = {
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 2 } },
					  { svh::VALUE, {"c",3} }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(UnorderedMap_Removed) {
		std::unordered_map<std::string, int> A{ {"a",1},{"b",2} };
		std::unordered_map<std::string, int> B{ {"a",1} };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({{ 1 }})
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(UnorderedMapOfMaps_Unchanged) {
		std::unordered_map<std::string, std::unordered_map<std::string, int>> umm{
			{"first", {{"a",1},{"b",2}}},
			{"second",{{"c",3},{"d",4}}}
		};
		std::unordered_map<std::string, std::unordered_map<std::string, int>> umm2{
			{"first", {{"a",1},{"b",2}}},
			{"second",{{"c",3},{"d",4}}}
		};
		CheckCompare(umm, umm2, svh::json());
	}
	TEST_METHOD(UnorderedMapOfMaps_Changed) {
		std::unordered_map<std::string, std::unordered_map<std::string, int>> umm{
			{"first", {{"a",1},{"b",2}}},
			{"second",{{"c",3},{"d",4}}}
		};
		std::unordered_map<std::string, std::unordered_map<std::string, int>> umm2{
			{"first", {{"a",5},{"b",6}}},
			{"second",{{"c",7},{"d",8}}}
		};
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0},{1}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0 } },
					  { svh::VALUE, {"first",svh::json::object({{"a",5},{"b",6}})} }
				  }),
				  svh::json({
					  { svh::INDEX, { 1 } },
					  { svh::VALUE, {"second",svh::json::object({{"c",7},{"d",8}})} }
				  })
			  })
			}
		};
		CheckCompare(umm, umm2, expected);
	}

	/* Multimap */
	TEST_METHOD(Multimap_Unchanged) {
		std::multimap<std::string, int> A{ {"a",1},{"b",2} };
		std::multimap<std::string, int> B{ {"a",1},{"b",2} };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(Multimap_Changed) {
		std::multimap<std::string, int> A{ {"a",1},{"b",2} };
		std::multimap<std::string, int> B{ {"c",3},{"d",4} };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0},{1}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0 } },
					  { svh::VALUE, {"c",3} }
				  }),
				  svh::json({
					  { svh::INDEX, { 1 } },
					  { svh::VALUE, {"d",4} }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(Multimap_Added) {
		std::multimap<std::string, int> A{ {"a",1},{"b",2} };
		std::multimap<std::string, int> B{ {"a",1},{"b",2},{"c",3} };
		svh::json expected = {
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 2 } },
					  { svh::VALUE, {"c",3} }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(Multimap_Removed) {
		std::multimap<std::string, int> A{ {"a",1},{"b",2} };
		std::multimap<std::string, int> B{ {"a",1} };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({{ 1 }})
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(MultimapOfMaps_Unchanged) {
		std::multimap<std::string, std::multimap<std::string, int>> mmm{
			{"first", {{"a",1},{"b",2}}},
			{"second",{{"c",3},{"d",4}}}
		};
		std::multimap<std::string, std::multimap<std::string, int>> mmm2{
			{"first", {{"a",1},{"b",2}}},
			{"second",{{"c",3},{"d",4}}}
		};
		CheckCompare(mmm, mmm2, svh::json());
	}
	TEST_METHOD(MultimapOfMaps_Changed) {
		std::multimap<std::string, std::multimap<std::string, int>> mmm{
			{"first", {{"a",1},{"b",2}}},
			{"second",{{"c",3},{"d",4}}}
		};
		std::multimap<std::string, std::multimap<std::string, int>> mmm2{
			{"first", {{"a",5},{"b",6}}},
			{"second",{{"c",7},{"d",8}}}
		};
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0},{1}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0 } },
					  { svh::VALUE, {"first",svh::json::object({{"a",5},{"b",6}})} }
				  }),
				  svh::json({
					  { svh::INDEX, { 1 } },
					  { svh::VALUE, {"second",svh::json::object({{"c",7},{"d",8}})} }
				  })
			  })
			}
		};
		CheckCompare(mmm, mmm2, expected);
	}

	/* Unordered multimap */
	TEST_METHOD(UnorderedMultimap_Unchanged) {
		std::unordered_multimap<std::string, int> A{ {"a",1},{"b",2} };
		std::unordered_multimap<std::string, int> B{ {"a",1},{"b",2} };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(UnorderedMultimap_Changed) {
		std::unordered_multimap<std::string, int> A{ {"a",1},{"b",2} };
		std::unordered_multimap<std::string, int> B{ {"c",3},{"d",4} };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0},{1}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0 } },
					  { svh::VALUE, {"c",3} }
				  }),
				  svh::json({
					  { svh::INDEX, { 1 } },
					  { svh::VALUE, {"d",4} }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(UnorderedMultimap_Added) {
		std::unordered_multimap<std::string, int> A{ {"a",1},{"b",2} };
		std::unordered_multimap<std::string, int> B{ {"a",1},{"b",2},{"c",3} };
		svh::json expected = {
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 2 } },
					  { svh::VALUE, {"c",3} }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(UnorderedMultimap_Removed) {
		std::unordered_multimap<std::string, int> A{ {"a",1},{"b",2} };
		std::unordered_multimap<std::string, int> B{ {"a",1} };
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({{ 1 }})
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(UnorderedMultimapOfMaps_Unchanged) {
		std::unordered_multimap<std::string, std::unordered_multimap<std::string, int>> ummm{
			{"first", {{"a",1},{"b",2}}},
			{"second",{{"c",3},{"d",4}}}
		};
		std::unordered_multimap<std::string, std::unordered_multimap<std::string, int>> ummm2{
			{"first", {{"a",1},{"b",2}}},
			{"second",{{"c",3},{"d",4}}}
		};
		CheckCompare(ummm, ummm2, svh::json());
	}
	TEST_METHOD(UnorderedMultimapOfMaps_Changed) {
		std::unordered_multimap<std::string, std::unordered_multimap<std::string, int>> ummm{
			{"first", {{"a",1},{"b",2}}},
			{"second",{{"c",3},{"d",4}}}
		};
		std::unordered_multimap<std::string, std::unordered_multimap<std::string, int>> ummm2{
			{"first", {{"a",5},{"b",6}}},
			{"second",{{"c",7},{"d",8}}}
		};
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({
				  {0},{1}
			  })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0 } },
					  { svh::VALUE, {"first",svh::json::object({{"a",5},{"b",6}})} }
				  }),
				  svh::json({
					  { svh::INDEX, { 1 } },
					  { svh::VALUE, {"second",svh::json::object({{"c",7},{"d",8}})} }
				  })
			  })
			}
		};
		CheckCompare(ummm, ummm2, expected);
	}
	};

	/* Test pair and tuple types */
	TEST_CLASS(PairTupleTypes) {
public:
	/* Pairs */
	TEST_METHOD(Pair_Unchanged) {
		std::pair<int, int> A{ 1,2 };
		std::pair<int, int> B{ 1,2 };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(Pair_Changed) {
		std::pair<int, int> A{ 1,2 };
		std::pair<int, int> B{ 1,4 };
		//{"second":4}
		svh::json expected = svh::json({ { svh::SECOND, 4 } });
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(PairOfPairs_Unchanged) {
		std::pair<std::pair<int, int>, std::pair<int, int>> A{ {1,2}, {3,4} };
		std::pair<std::pair<int, int>, std::pair<int, int>> B{ {1,2}, {3,4} };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(PairOfPairs_Changed) {
		std::pair<std::pair<int, int>, std::pair<int, int>> A{ {1,2}, {3,4} };
		std::pair<std::pair<int, int>, std::pair<int, int>> B{ {5,2}, {7,8} };
		//{"first":{"first":5,"second":6},"second":{"first":7,"second":8}}
		svh::json expected = svh::json({ { svh::FIRST, { { svh::FIRST, 5 } } }, { svh::SECOND, { { svh::FIRST, 7 }, { svh::SECOND, 8 } } } });
		CheckCompare(A, B, expected);
	}

	/* Tuple */
	TEST_METHOD(Tuple_Unchanged) {
		std::tuple<int, int> A{ 1,2 };
		std::tuple<int, int> B{ 1,2 };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(Tuple_Changed) {
		std::tuple<int, int> A{ 1,2 };
		std::tuple<int, int> B{ 1,4 };
		//{"removed indices":[[1]],"added":[{"index":[1],"value":4}]}
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({ {1} })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 1 } },
					  { svh::VALUE, 4 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(TupleOfTuples_Unchanged) {
		std::tuple<std::tuple<int, int>, std::tuple<int, int>> A{ {1,2}, {3,4} };
		std::tuple<std::tuple<int, int>, std::tuple<int, int>> B{ {1,2}, {3,4} };
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(TupleOfTuples_Changed) {
		std::tuple<std::tuple<int, int>, std::tuple<int, int>> A{ {1,2}, {3,4} };
		std::tuple<std::tuple<int, int>, std::tuple<int, int>> B{ {1,6}, {3,4} };
		//{"removed indices":[[0,1]],"added":[{"index":[0,1],"value":6}]}
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({ { 0,1 } })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0,1 } },
					  { svh::VALUE, 6 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}

	TEST_METHOD(TupleOfTuples_Changed_Multiple) {
		std::tuple<std::tuple<int, int>, std::tuple<int, int>> A{ {1,2}, {3,4} };
		std::tuple<std::tuple<int, int>, std::tuple<int, int>> B{ {1,6}, {3,8} };
		//{"removed indices":[[0],[1]],"added":[{"index":[0],"value":[1,6]},{"index":[1],"value":[3,8]}]}
		svh::json expected = {
			{ svh::REMOVED_INDICES,
			  svh::json::array({ { 0,1 },{ 1,1 } })
			},
			{ svh::ADDED_VALUES,
			  svh::json::array({
				  svh::json({
					  { svh::INDEX, { 0,1 } },
					  { svh::VALUE, 6 }
				  }),
				  svh::json({
					  { svh::INDEX, { 1,1 } },
					  { svh::VALUE, 8 }
				  })
			  })
			}
		};
		CheckCompare(A, B, expected);
	}

	};

	/* Optional and variant */
	TEST_CLASS(OptionalVariantTypes) {
		//public:
		TEST_METHOD(Optional_Unchanged) {
			std::optional<int> opt = 99;
			std::optional<int> opt2 = 99;
			CheckCompare(opt, opt2, svh::json());
		}
		TEST_METHOD(Optional_Changed) {
			std::optional<int> opt = 99;
			std::optional<int> opt2 = 100;
			svh::json expected = svh::json(100);
			CheckCompare(opt, opt2, expected);
		}
		TEST_METHOD(OptionalOfOptional_Unchanged) {
			std::optional<std::optional<int>> opt = 99;
			std::optional<std::optional<int>> opt2 = 99;
			CheckCompare(opt, opt2, svh::json());
		}
		TEST_METHOD(OptionalOfOptional_Changed) {
			std::optional<std::optional<int>> opt = 99;
			std::optional<std::optional<int>> opt2 = 100;
			svh::json expected = svh::json(100);
			CheckCompare(opt, opt2, expected);
		}
	};

	/* Test raw pointers */
	TEST_CLASS(RawPointerTypes) {
public:
	TEST_METHOD(RawPointer_UnChanged) {
		int* A = new int(42);
		int* B = new int(42);
		CheckCompare(*A, *B, svh::json());
		delete A;
		delete B;
	}
	TEST_METHOD(RawPointer_Changed) {
		int* A = new int(42);
		int* B = new int(43);
		svh::json expected = svh::json(43);
		CheckCompare(*A, *B, expected);
		delete A;
		delete B;
	}
	};
	//
		/* Test smart pointers */
	TEST_CLASS(PointerTypes) {
public:
	/* Unique pointers */
	TEST_METHOD(UniquePointer_Unchanged) {
		std::unique_ptr<int> A = std::make_unique<int>(42);
		std::unique_ptr<int> B = std::make_unique<int>(42);
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(UniquePointer_Changed) {
		std::unique_ptr<int> A = std::make_unique<int>(42);
		std::unique_ptr<int> B = std::make_unique<int>(43);
		svh::json expected = svh::json(43);
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(UniquePointerOfUniquePointer_Unchanged) {
		std::unique_ptr<std::unique_ptr<int>> A = std::make_unique<std::unique_ptr<int>>(std::make_unique<int>(42));
		std::unique_ptr<std::unique_ptr<int>> B = std::make_unique<std::unique_ptr<int>>(std::make_unique<int>(42));
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(UniquePointerOfUniquePointer_Changed) {
		std::unique_ptr<std::unique_ptr<int>> A = std::make_unique<std::unique_ptr<int>>(std::make_unique<int>(42));
		std::unique_ptr<std::unique_ptr<int>> B = std::make_unique<std::unique_ptr<int>>(std::make_unique<int>(43));
		svh::json expected = svh::json(43);
		CheckCompare(A, B, expected);
	}

	/* Shared pointers */
	TEST_METHOD(SharedPointer_Unchanged) {
		std::shared_ptr<int> A = std::make_shared<int>(42);
		std::shared_ptr<int> B = std::make_shared<int>(42);
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(SharedPointer_Changed) {
		std::shared_ptr<int> A = std::make_shared<int>(42);
		std::shared_ptr<int> B = std::make_shared<int>(43);
		svh::json expected = svh::json(43);
		CheckCompare(A, B, expected);
	}
	TEST_METHOD(SharedPointerOfSharedPointer_Unchanged) {
		std::shared_ptr<std::shared_ptr<int>> A = std::make_shared<std::shared_ptr<int>>(std::make_shared<int>(42));
		std::shared_ptr<std::shared_ptr<int>> B = std::make_shared<std::shared_ptr<int>>(std::make_shared<int>(42));
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(SharedPointerOfSharedPointer_Changed) {
		std::shared_ptr<std::shared_ptr<int>> A = std::make_shared<std::shared_ptr<int>>(std::make_shared<int>(42));
		std::shared_ptr<std::shared_ptr<int>> B = std::make_shared<std::shared_ptr<int>>(std::make_shared<int>(43));
		svh::json expected = svh::json(43);
		CheckCompare(A, B, expected);
	}

	/* Weak pointers */
	TEST_METHOD(WeakPointer_Unchanged) {
		std::shared_ptr<int> A_SP = std::make_shared<int>(42);
		std::weak_ptr<int> A = A_SP;
		std::shared_ptr<int> B_SP = std::make_shared<int>(42);
		std::weak_ptr<int> B = B_SP;
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(WeakPointer_Changed) {
		std::shared_ptr<int> A_SP = std::make_shared<int>(42);
		std::weak_ptr<int> A = A_SP;
		std::shared_ptr<int> B_SP = std::make_shared<int>(43);
		std::weak_ptr<int> B = B_SP;
		svh::json expected = svh::json(43);
		CheckCompare(A, B, expected);
	}
	};
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

	TEST_METHOD(GameEntityStruct_Unchanged) {
		GameEntity A;
		A.name = "entity";
		A.transform = std::make_shared<Transform>();
		A.item_holder = std::make_shared<ItemHolder>();
		GameEntity B;
		B.name = "entity";
		B.transform = std::make_shared<Transform>();
		B.item_holder = std::make_shared<ItemHolder>();
		CheckCompare(A, B, svh::json()); // returns empty since the values inside the pointers are the same
	}

	TEST_METHOD(GameEntityStruct_Changed) {
		GameEntity A;
		A.name = "entity";
		A.transform = std::make_shared<Transform>();
		A.item_holder = std::make_shared<ItemHolder>();
		GameEntity B;
		B.name = "entity";
		B.transform = std::make_shared<Transform>();
		B.transform->position = { 1.0f, 2.0f, 3.0f };
		B.item_holder = std::make_shared<ItemHolder>();
		B.item_holder->item_count = 69;
		CheckCompare(A, B,
			svh::json::object({
				{"transform", svh::json::object({
					{"position", svh::json::array({1.0f,2.0f,3.0f})}
					})},
				{"item_holder", svh::json::object({
					{"item_count", 69}
					})}
				})
		);
	}
	};
	//
	TEST_CLASS(Inheritance) {
public:
	TEST_METHOD(InheritedComponent_Unchanged) {
		DerivedComponent A;
		A.name = "Test";
		A.value = 42;
		A.id = 0;
		DerivedComponent B;
		B.name = "Test";
		B.value = 42;
		B.id = 0;
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(InheritedComponent_Changed) {
		DerivedComponent A;
		A.name = "Test";
		A.value = 42;
		A.id = 0;
		DerivedComponent B;
		B.name = "Test";
		B.value = 43; // changed
		B.id = 0;
		CheckCompare(A, B,
			svh::json::object({
				{"value", 43}
				})
		);
	}

	TEST_METHOD(BaseComponentTest_Unchanged) {
		BaseComponent* A = new DerivedComponent();
		A->id = 1;
		BaseComponent* B = new DerivedComponent();
		B->id = 1;

		CheckCompare(*A, *B, svh::json());
		delete A;
		delete B;
	}

	TEST_METHOD(BaseComponentTest_Changed) {
		BaseComponent* A = new DerivedComponent();
		A->id = 1;
		BaseComponent* B = new DerivedComponent();
		B->id = 2;
		CheckCompare(*A, *B,
			svh::json::object({
				{"id", 2}
				})
		);
		delete A;
		delete B;
	}
	};
	//
		/* Test larger integer types */
	TEST_CLASS(LargerIntegerTypes) {
public:
	TEST_METHOD(LongLong_Unchanged) {
		long long A = 123456789012345LL;
		long long B = 123456789012345LL;
		CheckCompare(A, B, svh::json());
	}
	TEST_METHOD(LongLong_Changed) {
		long long A = 123456789012345LL;
		long long B = 123456789012346LL;
		CheckCompare(A, B, svh::json(123456789012346LL));
	}

	// unsigned long long
	TEST_METHOD(UnsignedLongLong_Unchanged) {
		unsigned long long ull = 123456789012345ULL;
		unsigned long long ull2 = 123456789012345ULL;
		CheckCompare(ull, ull2, svh::json());
	}
	TEST_METHOD(UnsignedLongLong_Changed) {
		unsigned long long ull = 123456789012345ULL;
		unsigned long long ull2 = 123456789012346ULL;
		CheckCompare(ull, ull2, svh::json(123456789012346ULL));
	}

	// size_t
	TEST_METHOD(SizeT_Unchanged) {
		std::size_t sz = 123456789012345ULL;
		std::size_t sz2 = 123456789012345ULL;
		CheckCompare(sz, sz2, svh::json());
	}
	TEST_METHOD(SizeT_Changed) {
		std::size_t sz = 123456789012345ULL;
		std::size_t sz2 = 123456789012346ULL;
		CheckCompare(sz, sz2, svh::json(123456789012346ULL));
	}
	};

	PlayerEntity CreateComplexNestedObject() {
		auto transform = std::make_shared<Transform>();
		transform->position = { 1.0f, 2.0f, 3.0f };
		transform->rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
		transform->scale = { 1.0f, 1.0f, 1.0f };
		Inventory inv;
		inv.items = { "potion", "elixir" };
		inv.ammo = { {"arrows", 20} };
		inv.misc = { "key", "map" };
		auto w1 = std::make_unique<Weapon>();
		w1->name = "Sword"; w1->damage = 10;
		auto w2 = std::make_unique<Weapon>();
		w2->name = "Bow";   w2->damage = 7;
		auto a1 = std::make_shared<Armor>();
		a1->name = "Helmet"; a1->defense = 5;
		auto a2 = std::make_shared<Armor>();
		a2->name = "Armor";  a2->defense = 15;
		Skill s1; s1.name = "Fireball"; s1.level = 3;
		Skill s2; s2.name = "IceShard"; s2.level = 2;
		s2.subskills.push_back(Skill{ "Freeze", 1, {} });
		SkillTree st; st.skills = { s1, s2 };
		PlayerEntity player;
		player.id = "player1";
		player.transform = transform;
		player.inventory = inv;
		player.weapons.clear();
		player.weapons.push_back(std::move(w1));
		player.weapons.push_back(std::move(w2));
		player.armors = { { "body", a1 }, { "head", a2 } };
		player.skill_tree = st;
		return player;
	}
	/* Test complex nested structures */
	TEST_CLASS(ComplexNestedTest) {
public:
	TEST_METHOD(PlayerEntitySerialization_UnChanged) {
		PlayerEntity A = CreateComplexNestedObject();
		PlayerEntity B = CreateComplexNestedObject();
		CheckCompare(A, B, svh::json());
	}

	TEST_METHOD(PlayerEntitySerialization_Changed) {
		auto A = CreateComplexNestedObject();
		auto B = CreateComplexNestedObject();
		B.weapons[0]->damage = 20;
		//{"weapons":{"removed indices":[[0]],"added":[{"index":[0],"value":{"name":"Sword","damage":20}}]}}
		svh::json expected = svh::json::object({
			{ "weapons", svh::json::object({
				{ svh::REMOVED_INDICES,
				  svh::json::array({ { 0 } })
				},
				{ svh::ADDED_VALUES,
				  svh::json::array({
					  svh::json({
						  { svh::INDEX, { 0 } },
						  { svh::VALUE, svh::json::object({ {"name", "Sword"}, {"damage", 20} }) }
					  })
				  })
				}
			}) }
			});
		CheckCompare(A, B, expected);
	}
	};

} // namespace prefabstests