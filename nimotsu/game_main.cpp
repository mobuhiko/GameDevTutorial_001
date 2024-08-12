#define _CRT_SECURE_NO_WARNINGS  // 跳过scanf的警告

#include <cstdio>
#include <cstdint>
#include <array>
#include <vector>

namespace MyMacro {
  constexpr uint32_t STAGE_WIDTH = 8;
  constexpr uint32_t STAGE_HEIGHT = 5;

  namespace Pic {
    constexpr char WALL = '#';
    constexpr char PLAYER = 'p';
    constexpr char BOX = 'o';
    constexpr char TARGET = '.';
  }

  namespace Operation {
    constexpr char UP_KEY = 'w';
    constexpr char LEFT_KEY = 'a';
    constexpr char RIGHT_KEY = 'd';
    constexpr char DOWN_KEY = 's';
    constexpr char QUIT_KEY = 'q';
  }
};  // namespace MyMacro

struct Pos {
  bool operator==(const Pos& cmp) const = default; // 好像是c++20的新功能

  int32_t x{0};
  int32_t y{0};
};

void change_pos(const char input, Pos& pos) {
  switch (input) {
    case MyMacro::Operation::DOWN_KEY:
    {
      ++pos.y;
    } break;
    case MyMacro::Operation::UP_KEY:
    {
      --pos.y;
    } break;
    case MyMacro::Operation::LEFT_KEY:
    {
      --pos.x;
    } break;
    case MyMacro::Operation::RIGHT_KEY:
    {
      ++pos.x;
    } break;
    default:
      break;
  }
}

struct MyElement
{
  char pic{' '};
  Pos pos{};
};

MyElement create_wall_element(const int32_t x, const int32_t y) {
  return MyElement{
      .pic = MyMacro::Pic::WALL, .pos = Pos{x, y}}; // c++20的新功能
}

MyElement create_player_element(const int32_t x, const int32_t y) {
  return MyElement{
      .pic = MyMacro::Pic::PLAYER, .pos = Pos{x, y}};
}

MyElement create_box_element(const int32_t x, const int32_t y) {
  return MyElement{
      .pic = MyMacro::Pic::BOX, .pos = Pos{x, y}};
}

MyElement create_target_element(const int32_t x, const int32_t y) {
  return MyElement{
      .pic = MyMacro::Pic::TARGET, .pos = Pos{x, y}};
}

// 因为BOX和PLAYER可以跑在TARGET的上面
// 所以用2个表直接优先画ObjectVec
using StageVec = std::vector<MyElement>;
using ObjectVec = std::vector<MyElement>;

char get_input() {
  char input = '\0';
  scanf("%c", &input);
  return input;
}

bool update_game(const char input, const StageVec& stage_vec, ObjectVec& object_vec, bool& is_quit) {
  auto get_wall = [](const Pos& pos, const StageVec& stg_vec) {
    auto find_it = std::find_if(stg_vec.cbegin(), stg_vec.cend(),
                                [pos](const auto& tmp) {
      return tmp.pic == MyMacro::Pic::WALL && tmp.pos == pos;
    }
    );

    return find_it;
  };

  auto get_box = [](const Pos& pos, ObjectVec& obj_vec) {
    auto find_it = std::find_if(obj_vec.begin(), obj_vec.end(),
                                [pos](const auto& tmp) {
      return tmp.pic == MyMacro::Pic::BOX && tmp.pos == pos;
    }
    );

    return find_it;
  };

  auto get_target = [](const Pos& pos, const StageVec& stg_vec) {
    auto find_it = std::find_if(stg_vec.begin(), stg_vec.end(),
                                [pos](const auto& tmp) {
      return tmp.pic == MyMacro::Pic::TARGET && tmp.pos == pos;
    }
    );

    return find_it;
  };

  is_quit = false;

  if (input == MyMacro::Operation::QUIT_KEY) {
    is_quit = true;
    return false;
  }

  // 因为scanf包括回车键什么都能接收到, 在这里把不要的输入全过滤掉
  if (input != MyMacro::Operation::DOWN_KEY
      && input != MyMacro::Operation::LEFT_KEY
      && input != MyMacro::Operation::RIGHT_KEY
      && input != MyMacro::Operation::UP_KEY) {
    return false;
  }

  auto find_player_it = std::find_if(object_vec.begin(), object_vec.end(),
                                     [](const auto& tmp) {
    return tmp.pic == MyMacro::Pic::PLAYER;
  }
  );

  if (find_player_it == object_vec.end()) {
    is_quit = true;
    return false;
  }

  const Pos before_player_pos = find_player_it->pos;
  Pos tmp_player_pos = before_player_pos;

  change_pos(input, tmp_player_pos);

  if (auto wall = get_wall(tmp_player_pos, stage_vec);
      wall != stage_vec.cend()) {
    return true;
  }

  if (auto box = get_box(tmp_player_pos, object_vec);
      box != object_vec.end()) {
    Pos tmp_box_pos = box->pos;
    change_pos(input, tmp_box_pos);

    bool is_push_available = true;
    if (auto wall = get_wall(tmp_box_pos, stage_vec);
        wall != stage_vec.cend()) {
      is_push_available = false;
    }
    if (auto another_box = get_box(tmp_box_pos, object_vec);
        another_box != object_vec.end()) {
      is_push_available = false;
    }

    if (is_push_available) {
      box->pos = tmp_box_pos;
      find_player_it->pos = tmp_player_pos;
    }
  }
  else {
    find_player_it->pos = tmp_player_pos;
  }

  bool is_all_clear = true;
  for (const auto& it : object_vec) {
    if (it.pic == MyMacro::Pic::BOX) {
      const Pos box_pos = it.pos;
      const bool is_met_target = (get_target(box_pos, stage_vec) != stage_vec.cend());
      if (!is_met_target) {
        is_all_clear = false;
        break;
      }
    }
  }

  if (is_all_clear) {
    printf("CONGRATULATION !!!\n");
    is_quit = true;
  }

  return true;
}

void draw(const StageVec& stage_vec, const ObjectVec& obj_vec) {
  for (int32_t y = 0; y < MyMacro::STAGE_HEIGHT; ++y) {
    for (int32_t x = 0; x < MyMacro::STAGE_WIDTH; ++x) {
      char draw_target = ' ';
      if (auto find_obj_it = std::find_if(
        obj_vec.cbegin(), obj_vec.cend(), [x, y](const auto& tmp) {
        return tmp.pos.x == x && tmp.pos.y == y;
      });
      find_obj_it != obj_vec.cend()) {
        draw_target = find_obj_it->pic;
      }
      else {
        if (auto find_stg_it =
            std::find_if(stage_vec.cbegin(), stage_vec.cend(),
            [x, y](const auto& tmp) {
          return tmp.pos.x == x &&
            tmp.pos.y == y;
        });
        find_stg_it != stage_vec.cend()) {
          draw_target = find_stg_it->pic;
        }
      }

      printf("%c", draw_target);
    }
    printf("\n");
  }

  printf("\n");
}

bool initialize(StageVec& stage_vec, ObjectVec& obj_vec) {
  /*
   01234567
  0########
  1# .. p #
  2# oo   #
  3#      #
  4########
  */

  auto create_wall = [](StageVec& list) {
    for (int32_t y = 0; y < MyMacro::STAGE_HEIGHT; ++y) {
      for (int32_t x = 0; x < MyMacro::STAGE_WIDTH; ++x) {
        if (y == 0) {
          MyElement wall = create_wall_element(x, y);
          list.emplace_back(wall);
        }
        else if (y == MyMacro::STAGE_HEIGHT - 1) {
          MyElement wall = create_wall_element(x, y);
          list.emplace_back(wall);
        }
        else {
          if (x == 0 || x == MyMacro::STAGE_WIDTH - 1) {
            MyElement wall = create_wall_element(x, y);
            list.emplace_back(wall);
          }
        }
      }
    }
  };

  auto create_target = [](StageVec& list) {
    MyElement target_1 = create_target_element(2, 1);
    list.emplace_back(target_1);
    MyElement target_2 = create_target_element(3, 1);
    list.emplace_back(target_2);
  };

  auto create_player = [](ObjectVec& list) {
    MyElement player = create_player_element(5, 1);
    list.emplace_back(player);
  };

  auto create_box = [](ObjectVec& list) {
    MyElement box_1 = create_box_element(2, 2);
    list.emplace_back(box_1);
    MyElement box_2 = create_box_element(3, 2);
    list.emplace_back(box_2);
  };

  StageVec wall_list{};
  StageVec target_list{};
  create_wall(wall_list);
  create_target(target_list);

  ObjectVec player_list{};
  ObjectVec box_list{};
  create_box(box_list);
  create_player(player_list);

  bool is_fail = false;
  for (const auto& it : wall_list) {
    stage_vec.emplace_back(it);
  }

  for (const auto& it : target_list) {
    if (auto find_it = std::find_if(stage_vec.cbegin(), stage_vec.cend(),
        [it](const auto& tmp) {
      return tmp.pic == MyMacro::Pic::WALL &&
        tmp.pos == it.pos;
    });
    find_it != stage_vec.cend()) {
      is_fail = true;
    }
    else {
      stage_vec.emplace_back(it);
    }
  }

  for (const auto& it : box_list) {
    if (auto find_it = std::find_if(stage_vec.cbegin(), stage_vec.cend(),
        [it](const auto& tmp) {
      return tmp.pic == MyMacro::Pic::WALL &&
        tmp.pos == it.pos;
    });
    find_it != stage_vec.cend()) {
      is_fail = true;
    }
    else {
      obj_vec.emplace_back(it);
    }
  }

  for (const auto& it : player_list) {
    if (auto find_stage_it = std::find_if(stage_vec.cbegin(), stage_vec.cend(),
        [it](const auto& tmp) {
      return tmp.pic == MyMacro::Pic::WALL &&
        tmp.pos == it.pos;
    });
    find_stage_it != stage_vec.cend()) {
      is_fail = true;
    }
    else {
      if (auto find_obj_it = std::find_if(obj_vec.cbegin(), obj_vec.cend(),
          [it](const auto& tmp) {
        return tmp.pic == MyMacro::Pic::BOX &&
          tmp.pos == it.pos;
      });
      find_obj_it != obj_vec.cend()) {
        is_fail = true;
      }
      else {
        obj_vec.emplace_back(it);
      }
    }
  }

  draw(stage_vec, obj_vec);

  return !is_fail;
}


int main() {
  bool is_quit{false};
  StageVec stage_list{};
  ObjectVec obj_list{};
  initialize(stage_list, obj_list);

  while (true) {
    const char input_key = get_input();
    if (update_game(input_key, stage_list, obj_list, is_quit)) {
      draw(stage_list, obj_list);
    }

    if (is_quit) {
      break;
    }
  }

  return 0;
}