#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <svh/defines.hpp>

namespace glm {
	inline svh::json SerializeImpl(const glm::vec3& v) {
		return svh::json::array({ v.x, v.y, v.z });
	}

	inline svh::json SerializeImpl(const glm::vec4& v) {
		return svh::json::array({ v.x, v.y, v.z, v.w });
	}

	inline svh::json SerializeImpl(const glm::quat& q) {
		return svh::json::array({ q.x, q.y, q.z, q.w });
	}

	inline void DeserializeImpl(const svh::json& j, glm::vec3& v) {
		if (j.is_array() && j.size() == 3) {
			v.x = j[0].get<float>();
			v.y = j[1].get<float>();
			v.z = j[2].get<float>();
		}
	}

	inline void DeserializeImpl(const svh::json& j, glm::vec4& v) {
		if (j.is_array() && j.size() == 4) {
			v.x = j[0].get<float>();
			v.y = j[1].get<float>();
			v.z = j[2].get<float>();
			v.w = j[3].get<float>();
		}
	}

	inline void DeserializeImpl(const svh::json& j, glm::quat& q) {
		if (j.is_array() && j.size() == 4) {
			q.x = j[0].get<float>();
			q.y = j[1].get<float>();
			q.z = j[2].get<float>();
			q.w = j[3].get<float>();
		}
	}
}

struct Transform {
	glm::vec3 position = glm::vec3(0.0f);
	glm::quat rotation = glm::quat();
	glm::vec3 scale = glm::vec3(1.0f);
};

VISITABLE_STRUCT(Transform, position, rotation, scale);

struct ItemHolder {
	int item_count = 3;
	std::vector<int> items = { 1, 2, 3 };
};

VISITABLE_STRUCT(ItemHolder, item_count, items);

struct GameEntity {
	std::string name = "test";
	std::shared_ptr<Transform> transform;
	std::shared_ptr<ItemHolder> item_holder;
};;

VISITABLE_STRUCT(GameEntity, name, transform, item_holder);

struct BaseComponent {
	int id = 0;
};

VISITABLE_STRUCT(BaseComponent, id);

struct DerivedComponent : public BaseComponent {
	std::string name = "Test";
	int value = 42;
};

VISITABLE_STRUCT(DerivedComponent, name, value, id);

enum class Color { Red, Green, Blue };
enum class Direction { North, South, East, West };



inline svh::json SerializeImpl(const Color& c) {
	switch (c) {
	case Color::Red: return svh::json("Red");
	case Color::Green: return svh::json("Green");
	case Color::Blue: return svh::json("Blue");
	default: return svh::json("Unknown");
	}
}

inline void DeserializeImpl(const svh::json& j, Color& c) {
	if (j.is_string()) {
		std::string color_str = j.get<std::string>();
		if (color_str == "Red") c = Color::Red;
		else if (color_str == "Green") c = Color::Green;
		else if (color_str == "Blue") c = Color::Blue;
		else c = Color::Red; // Default case
	}
}

inline svh::json SerializeImpl(const Direction& d) {
	switch (d) {
	case Direction::North: return svh::json("North");
	case Direction::South: return svh::json("South");
	case Direction::East: return svh::json("East");
	case Direction::West: return svh::json("West");
	default: return svh::json("Unknown");
	}
}

inline void DeserializeImpl(const svh::json& j, Direction& d) {
	if (j.is_string()) {
		std::string direction_str = j.get<std::string>();
		if (direction_str == "North") d = Direction::North;
		else if (direction_str == "South") d = Direction::South;
		else if (direction_str == "East") d = Direction::East;
		else if (direction_str == "West") d = Direction::West;
		else d = Direction::North; // Default case
	}
}

struct Weapon {
	std::string name;
	int damage;
};
VISITABLE_STRUCT(Weapon, name, damage);

struct Armor {
	std::string name;
	int defense;
};
VISITABLE_STRUCT(Armor, name, defense);

struct Skill {
	std::string name;
	int level;
	std::vector<Skill> subskills;
};
VISITABLE_STRUCT(Skill, name, level, subskills);

struct SkillTree {
	std::vector<Skill> skills;
};
VISITABLE_STRUCT(SkillTree, skills);

struct Inventory {
	std::vector<std::string> items;
	std::map<std::string, int> ammo;
	std::unordered_set<std::string> misc;
};
VISITABLE_STRUCT(Inventory, items, ammo, misc);

struct PlayerEntity {
	std::string id;
	std::shared_ptr<Transform> transform;
	Inventory inventory;
	std::vector<std::unique_ptr<Weapon>> weapons;
	std::map<std::string, std::shared_ptr<Armor>> armors;
	std::optional<SkillTree> skill_tree;
};
VISITABLE_STRUCT(PlayerEntity, id, transform, inventory, weapons, armors, skill_tree);