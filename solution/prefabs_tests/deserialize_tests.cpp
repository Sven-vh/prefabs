﻿#pragma once
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

namespace deserialize_tests {

	static std::wstring to_wstring(const std::string& s) {
		return std::wstring(s.begin(), s.end());
	}

	template<typename T>
	void CheckDeserialization(
		const T& expectedObj,
		const svh::json& inputJson,
		const wchar_t* message = L"Output did not match expected JSON"
	) {
		T resultObj;
		try {
			svh::Deserializer::FromJson<T>(inputJson, resultObj);
		} catch (const std::exception& ex) {
			Assert::Fail(to_wstring(std::string("Exception thrown: ") + ex.what()).c_str());
		}

		// we serialize the result to check if it matches the input
		std::string resultDump;
		try {
			resultDump = svh::Serializer::ToJson(resultObj).dump();
		} catch (const std::exception& ex) {
			Assert::Fail(to_wstring(std::string("Exception thrown: ") + ex.what()).c_str());
		}
		// compare wstring’d JSON
		std::wstring wResult = to_wstring("\n"+resultDump+ "\n");
		std::string expectedDump = inputJson.dump();
		std::wstring wExpected = to_wstring("\n" +expectedDump+ "\n");
		Assert::AreEqual(wExpected, wResult, message);
		// put result in the log
		Logger::WriteMessage(wResult.c_str());
	}

	/* Primitive Types */
	TEST_CLASS(DefaultTypes) {
public:
	TEST_METHOD(Int) { CheckDeserialization(1, svh::json(1)); }
	TEST_METHOD(Double) { CheckDeserialization(3.14, svh::json(3.14)); }
	TEST_METHOD(Float) { CheckDeserialization(1.0f, svh::json(1.0f)); }
	TEST_METHOD(Bool) { CheckDeserialization(true, svh::json(true)); }
	TEST_METHOD(Char) { CheckDeserialization('a', svh::json('a')); }
	//TEST_METHOD(CString) {
	//	const char* cstr = "hello";
	//	CheckDeserialization(cstr, svh::json("hello"));
	//}
	TEST_METHOD(String) {
		std::string s = "hello";
		CheckDeserialization(s, svh::json("hello"));
	}
	TEST_METHOD(StringEscaping) {
		std::string s = "He said: \"\\Hello\n\"";
		CheckDeserialization(s, svh::json("He said: \"\\Hello\n\""));
	}
	TEST_METHOD(UnicodeString) {
		std::string s = u8"π≈3.14";
		CheckDeserialization(s, svh::json(u8"π≈3.14"));
	}
	TEST_METHOD(StringView) {
		std::string_view sv = "viewed";
		CheckDeserialization(sv, svh::json("viewed"));
	}
	};

	/* Enum Types */
	TEST_CLASS(EnumTypes) {
public:
	TEST_METHOD(ColorEnum) {
		Color c = Color::Green;
		CheckDeserialization(c, svh::json("Green"));
	}
	TEST_METHOD(DirectionEnum) {
		Direction d = Direction::South;
		CheckDeserialization(d, svh::json("South"));
	}
	TEST_METHOD(EnumClassAsInt) {
		int i = static_cast<int>(Color::Red);
		CheckDeserialization(i, svh::json(0));
	}
	};

	/* Sequence containers: vector, array, set, unordered_set, deque, list, initializer_list, c-style array */
	TEST_CLASS(Containers) {
public:
	/* Vectors */
	TEST_METHOD(Vector) {
		std::vector<int> v{ 1,2,3 };
		CheckDeserialization(v, svh::json::array({ 1,2,3 }));
	}
	TEST_METHOD(EmptyVector) {
		std::vector<int> v;
		CheckDeserialization(v, svh::json::array({}));
	}
	TEST_METHOD(VectorOfVectors) {
		std::vector<std::vector<int>> v{ {1,2},{3,4} };
		CheckDeserialization(v, svh::json::array({ {1,2},{3,4} }));
	}
	TEST_METHOD(VectorOfBool) { // vector<bool> is a proxy-reference container
		std::vector<bool> vb{ true, false, true };
		CheckDeserialization(vb, svh::json::array({ true, false, true }));
	}

	/* Arrays */
	TEST_METHOD(Array) {
		std::array<float, 3> a{ {1.0f,2.0f,3.0f} };
		CheckDeserialization(a, svh::json::array({ 1.0f,2.0f,3.0f }));
	}
	TEST_METHOD(EmptyArray) {
		std::array<int, 0> a{ {} };
		CheckDeserialization(a, svh::json::array({}));
	}
	TEST_METHOD(ArrayOfArrays) {
		std::array<std::array<int, 2>, 2> a{ { {1,2}, {3,4} } };
		CheckDeserialization(a, svh::json::array({ {1,2}, {3,4} }));
	}

	/* Sets */
	TEST_METHOD(Set) {
		std::set<int> s{ 5,6,7 };
		CheckDeserialization(s, svh::json::array({ 5,6,7 }));
	}
	TEST_METHOD(EmptySet) {
		std::set<std::string> s;
		CheckDeserialization(s, svh::json::array({}));
	}
	TEST_METHOD(SetOfSets) {
		std::set<std::set<int>> ss{ {1,2}, {3,4} };
		CheckDeserialization(ss, svh::json::array({ {1,2}, {3,4} }));
	}
	/* Unordered sets */
	TEST_METHOD(UnorderedSet) {
		std::unordered_set<int> us{ 8,9,10 };
		CheckDeserialization(us, svh::json::array({ 8,9,10 }));
	}
	TEST_METHOD(EmptyUnorderedSet) {
		std::unordered_set<std::string> us;
		CheckDeserialization(us, svh::json::array({}));
	}
	/* No default hash function for unordered_set of unordered_set* /

	/* Multisets */
	TEST_METHOD(Multiset) {
		std::multiset<int> ms{ 1,2,2,3 };
		CheckDeserialization(ms, svh::json::array({ 1,2,2,3 }));
	}
	TEST_METHOD(UnorderedMultiset) {
		std::unordered_multiset<std::string> ums{ "a","b","a" };
		CheckDeserialization(ums, svh::json::array({ "a","a","b" })); // order not guaranteed
	}

	/* Deques */
	TEST_METHOD(Deque) {
		std::deque<int> d{ 11,12,13 };
		CheckDeserialization(d, svh::json::array({ 11,12,13 }));
	}
	TEST_METHOD(EmptyDeque) {
		std::deque<std::string> d;
		CheckDeserialization(d, svh::json::array({}));
	}
	TEST_METHOD(DequeOfDeques) {
		std::deque<std::deque<int>> dd{ {1,2}, {3,4} };
		CheckDeserialization(dd, svh::json::array({ {1,2}, {3,4} }));
	}

	/* Lists */
	TEST_METHOD(List) {
		std::list<int> l{ 14,15,16 };
		CheckDeserialization(l, svh::json::array({ 14,15,16 }));
	}
	TEST_METHOD(EmptyList) {
		std::list<std::string> l;
		CheckDeserialization(l, svh::json::array({}));
	}
	TEST_METHOD(ListOfLists) {
		std::list<std::list<int>> ll{ {1,2}, {3,4} };
		CheckDeserialization(ll, svh::json::array({ {1,2}, {3,4} }));
	}

	/* Initializer lists */
	/* Can't use initializer_list for deserialization */

	///* C-style arrays */
	TEST_METHOD(CStyleArray) {
		int arr[] = { 20,21,22 };
		CheckDeserialization(arr, svh::json::array({ 20,21,22 }));
	}
	/* Not possible to make en empty C-style array */
	TEST_METHOD(CStyleArrayOfArrays) {
		int arr[2][2] = { {1,2}, {3,4} };
		CheckDeserialization(arr, svh::json::array({ {1,2}, {3,4} }));
	}

	/* Forward lists */
	TEST_METHOD(ForwardList) {
		std::forward_list<int> fl{ 1,2,3 };
		CheckDeserialization(fl, svh::json::array({ 1,2,3 }));
	}
	TEST_METHOD(EmptyForwardList) {
		std::forward_list<std::string> fl;
		CheckDeserialization(fl, svh::json::array({}));
	}
	TEST_METHOD(ForwardListOfLists) {
		std::forward_list<std::forward_list<int>> fll{ {1,2}, {3,4} };
		CheckDeserialization(fll, svh::json::array({ {1,2}, {3,4} }));
	}

	/* Map */
	TEST_METHOD(Map) {
		std::map<std::string, int> m{ {"one",1},{"two",2} };
		CheckDeserialization(m, svh::json::object({ {"one",1},{"two",2} }));
	}
	TEST_METHOD(EmptyMap) {
		std::map<std::string, int> m;
		CheckDeserialization(m, svh::json::object({}));
	}
	TEST_METHOD(FloatMap) {
		std::map<float, int> m{ {1.0f,1},{2.0f,2} };
		CheckDeserialization(m, svh::json::object({ {"1.0",1},{"2.0",2} }));
	}

	TEST_METHOD(MapOfMaps) {
		std::map<std::string, std::map<std::string, int>> mm{
			{"first", {{"a",1},{"b",2}}},
			{"second",{{"c",3},{"d",4}}}
		};
		CheckDeserialization(mm,
			svh::json::object({
				{"first",  svh::json::object({{"a",1},{"b",2}})},
				{"second", svh::json::object({{"c",3},{"d",4}})}
				})
		);
	}

	/* Unordered map*/
	TEST_METHOD(UnorderedMap_SingleElement) {
		std::unordered_map<std::string, int> um{ {"solo",42} };
		CheckDeserialization(um, svh::json::object({ {"solo",42} }));
	}
	TEST_METHOD(EmptyUnorderedMap) {
		std::unordered_map<std::string, int> um;
		CheckDeserialization(um, svh::json::object({}));
	}
	TEST_METHOD(UnorderedMapOfMaps) {
		std::unordered_map<std::string, std::unordered_map<std::string, int>> umm{
			{"first", {{"a",1},{"b",2}}},
			{"second",{{"c",3},{"d",4}}}
		};
		CheckDeserialization(umm,
			svh::json::object({
				{"first",  svh::json::object({{"a",1},{"b",2}})},
				{"second", svh::json::object({{"c",3},{"d",4}})}
				})
		);
	}

	/* Multimap */
	TEST_METHOD(Multimap) {
		std::multimap<std::string, int> mm{ {"one",1},{"one",2} };
		CheckDeserialization(mm, svh::json::object({ {"one",svh::json::array({1,2})} }));
	}
	TEST_METHOD(EmptyMultimap) {
		std::multimap<std::string, int> mm;
		CheckDeserialization(mm, svh::json::object({}));
	}
	TEST_METHOD(MultimapOfMaps) {
		std::multimap<std::string, std::multimap<std::string, int>> mmm{
			{"first", {{"a",1},{"b",2}}},
			{"second",{{"c",3},{"d",4}}}
		};
		CheckDeserialization(mmm,
			svh::json::object({
				{"first",  svh::json::object({{"a",1},{"b",2}})},
				{"second", svh::json::object({{"c",3},{"d",4}})}
				})
		);
	}

	/* Unordered multimap */
	TEST_METHOD(UnorderedMultimap) {
		std::unordered_multimap<std::string, int> umm{ {"one",1},{"one",2} };
		CheckDeserialization(umm, svh::json::object({ {"one",svh::json::array({1,2})} }));
	}
	TEST_METHOD(EmptyUnorderedMultimap) {
		std::unordered_multimap<std::string, int> umm;
		CheckDeserialization(umm, svh::json::object({}));
	}
	TEST_METHOD(UnorderedMultimapOfMaps) {
		std::unordered_multimap<std::string, std::unordered_multimap<std::string, int>> ummm{
			{"first", {{"a",1},{"b",2}}},
			{"second",{{"c",3},{"d",4}}}
		};
		CheckDeserialization(ummm,
			svh::json::object({
				{"first",  svh::json::object({{"a",1},{"b",2}})},
				{"second", svh::json::object({{"c",3},{"d",4}})}
				})
		);
	}
	};

	/* Test pair and tuple types */
	TEST_CLASS(PairTupleTypes) {
public:
	TEST_METHOD(Pair) {
		std::pair<int, std::string> p{ 7, "seven" };
		CheckDeserialization(p, svh::json::object({ { "7","seven" } }));
	}

	TEST_METHOD(Tuple) {
		std::tuple<int, float, bool> t{ 8, 2.5f, false };
		CheckDeserialization(t, svh::json::array({ 8,2.5f,false }));
	}
	TEST_METHOD(EmptyTuple) {
		std::tuple<> t;
		CheckDeserialization(t, svh::json::array({}));
	}

	};

	/* Optional and variant */
	TEST_CLASS(OptionalVariantTypes) {
public:
	/* With int*/
	TEST_METHOD(Optional) {
		std::optional<int> opt = 99;
		CheckDeserialization(opt, svh::json(99));
	}
	TEST_METHOD(Optional_Null) {
		std::optional<int> opt;
		CheckDeserialization(opt, svh::json(nullptr));
	}

	/* With string */
	TEST_METHOD(Variant) {
		std::variant<int, std::string> v = 100;
		CheckDeserialization(v, svh::json(100));
	}
	TEST_METHOD(VariantString) {
		std::variant<int, std::string> v = "hello";
		CheckDeserialization(v, svh::json("hello"));
	}

	/* With struct */
	TEST_METHOD(VariantWithStruct) {
		std::variant<int, Transform> v = Transform{ {1.0f,2.0f,3.0f}, {1.0f,0.0f,0.0f,0.0f}, {1.0f,1.0f,1.0f} };
		CheckDeserialization(v,
			svh::json::object({
				{"position", svh::json::array({ 1.0f,2.0f,3.0f })},
				{"rotation", svh::json::array({ 0.0f,0.0f,0.0f,1.0f })},
				{"scale",    svh::json::array({ 1.0f,1.0f,1.0f })},
				})
				);
	}
	};

	/* Test raw pointers */ /* Doesn't support raw pointers */
	//TEST_CLASS(RawPointerTypes) {
	//	TEST_METHOD(RawPointer_NonNull) {
	//		int x = 5;
	//		int* p = &x;
	//		CheckDeserialization(p, svh::json(5));
	//	}
	//	TEST_METHOD(RawPointer_Null) {
	//		int* p = nullptr;
	//		CheckDeserialization(p, svh::json(nullptr));
	//	}
	//};

	/* Test smart pointers */
	TEST_CLASS(PointerTypes) {
public:
	/* Unique pointers */
	TEST_METHOD(UniquePointer_NonNull) {
		auto up = std::make_unique<int>(99);
		CheckDeserialization(up, svh::json(99));
	}

	TEST_METHOD(UniquePointer_Null) {
		std::unique_ptr<int> up;
		CheckDeserialization(up, svh::json(nullptr));
	}

	/* Shared pointers */
	TEST_METHOD(SharedPointer_NonNull) {
		auto sp = std::make_shared<int>(123);
		CheckDeserialization(sp, svh::json(123));
	}

	TEST_METHOD(SharedPointer_Null) {
		std::shared_ptr<int> sp;
		CheckDeserialization(sp, svh::json(nullptr));
	}

	TEST_METHOD(SharedPointerOfVector) {
		auto spv = std::make_shared<std::vector<int>>(std::vector<int>{4, 5, 6});
		CheckDeserialization(spv, svh::json::array({ 4,5,6 }));
	}

	/* Weak pointers */
	/* Can't deserialize a locked pointer */
	TEST_METHOD(WeakPointer_Expired) {
		std::weak_ptr<int> wp;
		CheckDeserialization(wp, svh::json(nullptr));
	}
	};

	/* Test visitable structs */
	TEST_CLASS(VisitableStructs) {
public:
	TEST_METHOD(TransformStruct) {
		Transform t;
		t.position = glm::vec3(1.0f, 2.0f, 3.0f);
		t.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		t.scale = glm::vec3(1.0f, 1.0f, 1.0f);
		CheckDeserialization(t,
			svh::json::object({
				{"position", svh::json::array({1.0f,2.0f,3.0f})},
				{"rotation", svh::json::array({0.0f,0.0f,0.0f,1.0f})},
				{"scale",    svh::json::array({1.0f,1.0f,1.0f})}
				})
		);
	}

	TEST_METHOD(ItemHolderStruct) {
		ItemHolder ih;
		ih.item_count = 3;
		ih.items = { 1, 2, 3 };
		CheckDeserialization(ih,
			svh::json::object({
				{"item_count", 3},
				{"items", svh::json::array({1,2,3})}
				})
		);
	}

	TEST_METHOD(GameEntityStruct) {
		GameEntity ge;
		ge.name = "entity";
		ge.transform = std::make_shared<Transform>();
		ge.item_holder = std::make_shared<ItemHolder>();
		CheckDeserialization(ge,
			svh::json::object({
				{"name", "entity"},
				{"transform", svh::Serializer::ToJson(ge.transform)},
				{"item_holder", svh::Serializer::ToJson(ge.item_holder)}
				})
		);
	}
	};

	TEST_CLASS(Inheritance) {
public:
	TEST_METHOD(InheritedComponent) {
		DerivedComponent mc;
		mc.name = "Test";
		mc.value = 42;
		mc.id = 0;
		CheckDeserialization(mc,
			svh::json::object({
				{"name", "Test"},
				{"value",42},
				{"id", 0}
				})
		);
	}

	TEST_METHOD(BaseComponentTest) {
		BaseComponent* bc = new DerivedComponent();
		bc->id = 1;
		CheckDeserialization(*bc,
			svh::json::object({
				{"id", 1}
				})
		);
	}
	};

	/* Test larger integer types */
	TEST_CLASS(LargerIntegerTypes) {
public:
	TEST_METHOD(LongLong) {
		long long ll = 123456789012345LL;
		CheckDeserialization(ll, svh::json(123456789012345LL));
	}
	TEST_METHOD(LongLongMin) {
		long long ll = std::numeric_limits<long long>::min();
		CheckDeserialization(ll, svh::json(std::numeric_limits<long long>::min()));
	}
	TEST_METHOD(LongLongMax) {
		long long ll = std::numeric_limits<long long>::max();
		CheckDeserialization(ll, svh::json(std::numeric_limits<long long>::max()));
	}

	// unsigned long long
	TEST_METHOD(UnsignedLongLong) {
		unsigned long long ull = 123456789012345ULL;
		CheckDeserialization(ull, svh::json(123456789012345ULL));
	}
	TEST_METHOD(UnsignedLongLongMax) {
		unsigned long long ull = std::numeric_limits<unsigned long long>::max();
		CheckDeserialization(ull, svh::json(std::numeric_limits<unsigned long long>::max()));
	}

	// size_t
	TEST_METHOD(SizeT) {
		std::size_t sz = 1234567890ULL;
		CheckDeserialization(sz, svh::json(1234567890ULL));
	}
	TEST_METHOD(SizeTMax) {
		std::size_t sz = std::numeric_limits<std::size_t>::max();
		CheckDeserialization(sz, svh::json(std::numeric_limits<std::size_t>::max()));
	}
	};

	/* Test complex nested structures */
	TEST_CLASS(ComplexNestedTest) {
public:
	TEST_METHOD(PlayerEntitySerialization) {
		// Build a deeply nested object
		auto transform = std::make_shared<Transform>();
		transform->position = { 1.0f, 2.0f, 3.0f };
		transform->rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
		transform->scale = { 1.0f, 1.0f, 1.0f };

		Inventory inv;
		inv.items = { "potion", "elixir" };
		inv.ammo = { {"arrows", 20} };
		inv.misc = { "key", "map" };

		// Create weapons using moves
		auto w1 = std::make_unique<Weapon>();
		w1->name = "Sword"; w1->damage = 10;
		auto w2 = std::make_unique<Weapon>();
		w2->name = "Bow";   w2->damage = 7;

		// Create armors
		auto a1 = std::make_shared<Armor>();
		a1->name = "Helmet"; a1->defense = 5;
		auto a2 = std::make_shared<Armor>();
		a2->name = "Armor";  a2->defense = 15;

		// Setup skill tree
		Skill s1; s1.name = "Fireball"; s1.level = 3;
		Skill s2; s2.name = "IceShard"; s2.level = 2;
		s2.subskills.push_back(Skill{ "Freeze", 1, {} });
		SkillTree st; st.skills = { s1, s2 };

		PlayerEntity player;
		player.id = "player1";
		player.transform = transform;
		player.inventory = inv;

		// Assign weapons explicitly (unique_ptr cannot be copied)
		player.weapons.clear();
		player.weapons.push_back(std::move(w1));
		player.weapons.push_back(std::move(w2));

		player.armors = { { "head", a1 }, { "body", a2 } };
		player.skill_tree = st;

		// Expected JSON
		svh::json expected = svh::json::object({
			{ "id", "player1" },
			{ "transform", svh::json::object({
				{ "position", svh::json::array({1.0f,2.0f,3.0f}) },
				{ "rotation", svh::json::array({0.0f,0.0f,0.0f,1.0f}) },
				{ "scale",    svh::json::array({1.0f,1.0f,1.0f}) }
			})},
			{ "inventory", svh::json::object({
				{ "items", svh::json::array({"potion","elixir"}) },
				{ "ammo",  svh::json::object({{"arrows",20}}) },
				{ "misc",  svh::json::array({"key","map"}) }
			})},
			{ "weapons", svh::json::array({
				svh::json::object({{"name","Sword"},{"damage",10}}),
				svh::json::object({{"name","Bow"},{"damage",7}})
			})},
			{ "armors", svh::json::object({
				{ "body", svh::json::object({{"name","Armor"},{"defense",15}}) },
				{ "head", svh::json::object({{"name","Helmet"},{"defense",5}}) }
			})},
			{ "skill_tree", svh::json::object({
				{ "skills", svh::json::array({
					svh::json::object({
						{"name","Fireball"},
						{"level",3},
						{"subskills", svh::json::array({})}
					}),
					svh::json::object({
						{"name","IceShard"},
						{"level",2},
						{"subskills", svh::json::array({
							svh::json::object({{"name","Freeze"},{"level",1},{"subskills",svh::json::array({})}})
						})}
					})
				})}
			})}
			});

		CheckDeserialization(player, expected);
	}
	};

} // namespace prefabstests