#include <Geode/Enums.hpp>
#include <Geode/Geode.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/loader/SettingV3.hpp>

#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GJGameLevel.hpp>

using namespace geode::prelude;

struct Input {
  int frame = 0;

  bool held = false;
  bool pressed = false;
};

struct Pathfinder {
  std::vector<Input> inputs;
  bool pathfinding = false;

  int index = 0;
  int frame = 0;
};
static Pathfinder pathfinder;

struct Settings {
  bool enabled = true;
};
static Settings settings;

class $modify(MyPlayLayer, PlayLayer) {
  bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
    if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
    if (!settings.enabled) return true;
    
    pathfinder.index = 0;
    pathfinder.inputs.clear();

    pathfinder.inputs.push_back({ 0, false, false });
    pathfinder.pathfinding = true;

    return true;
  }

  void postUpdate(float dt) {
    PlayLayer::postUpdate(dt);

    if (!settings.enabled) return;
    if (pathfinder.pathfinding) this->pathfind(pathfinder);
  }

  void levelComplete() {
    PlayLayer::levelComplete();
    if (settings.enabled) pathfinder.pathfinding = false;
  }

  void onQuit() {
    PlayLayer::onQuit();
    pathfinder.pathfinding = false;
  }

  void pathfind(Pathfinder& pf) {
    if (!pf.pathfinding) return;

    log::info("Input Data: {} {}", pf.index, pf.inputs.size());
    pf.frame++;

    if (pf.index < pf.inputs.size()) {
      auto& input = pf.inputs[pf.index];

      if (input.frame == pf.frame) {
        m_player1->pushButton(PlayerButton::Jump);
        if (input.pressed) m_player1->releaseButton(PlayerButton::Jump);

        pf.index++;
        log::info("input frame = pathfinder frame");
      }
    }

    if (m_player1 && m_player1->m_isDead) {
      log::info("player died!");

      if (pf.inputs.empty()) {
        pf.pathfinding = false;
        return;
      }

      auto& last = pf.inputs.back();

      if (!last.held) {
        last.held = true;
        last.pressed = !last.pressed;

        resetPath();
      } else {
        pf.inputs.pop_back();

        if (pf.inputs.empty()) {
          pf.pathfinding = false;
          return;
        }

        resetPath();
      }

      return;
    }

    pf.inputs.push_back({ pf.frame, false, true });
  }

  void resetPath() {
    pathfinder.index = 0;
    pathfinder.frame = 0;
  }
};

$on_mod(Loaded) {
  settings.enabled = Mod::get()->getSettingValue<bool>("enabled");

  listenForSettingChanges<bool>("enabled", [](bool value) {
    settings.enabled = value;
  });
}