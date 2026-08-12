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
    
    

    return true;
  }

  void postUpdate(float dt) {
    PlayLayer::postUpdate(dt);
    if (!settings.enabled) return;
  }

  void levelComplete() {
    PlayLayer::levelComplete();
  }

  void onQuit() {
    PlayLayer::onQuit();
    pathfinder.pathfinding = false;
  }
};

$on_mod(Loaded) {
  settings.enabled = Mod::get()->getSettingValue<bool>("enabled");

  listenForSettingChanges<bool>("enabled", [](bool value) {
    settings.enabled = value;
  });
}