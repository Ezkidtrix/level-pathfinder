#include <Geode/Enums.hpp>
#include <Geode/Geode.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/loader/SettingV3.hpp>

#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GJGameLevel.hpp>

using namespace geode::prelude;

bool pathfinding = false;

struct Settings {
  bool enabled = true;
};
static Settings settings;

class $modify(MyGJGameLevel, GJGameLevel) {
  void savePercentage(int percentage, bool isPractice, int p2, int p3, bool p4) {
    if (settings.enabled) return; 
    GJGameLevel::savePercentage(percentage, isPractice, p2, p3, p4);
  }
};

class $modify(MyPlayLayer, PlayLayer) {
  struct Fields {
    int m_frame = 0;
    PlayerObject* m_state;
  };

  bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
    if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
    if (!settings.enabled) return true;
    
    pathfinding = true;
    m_fields->m_state = m_player1;

    return true;
  }

  void postUpdate(float dt) {
    PlayLayer::postUpdate(dt);
    if (!settings.enabled) return;

    this->pathfind(m_player1);
  }

  void levelComplete() {
    if (settings.enabled) {
      this->onQuit(); 
      return; 
    }

    PlayLayer::levelComplete();
  }

  void onQuit() {
    PlayLayer::onQuit();
    pathfinding = false;
  }

  void saveState(PlayerObject* player) {
    m_fields->m_state = player;
  }

  void loadState(PlayerObject* player) {
    player = m_fields->m_state;
    player->setPosition(m_fields->m_state->getRealPosition());
  }

  void pathfind(PlayerObject* player, bool hold = false) {
    // int frame = 0;
    // int maxFrames = 50000;

    // auto currentPos = player->getPosition();
    // auto lastPos = player->getLastPosition();

    if (hold) player->pushButton(PlayerButton::Jump);
    else player->releaseButton(PlayerButton::Jump);

    if (player->m_isDead) {
      hold = !hold;
      loadState(m_fields->m_state);

      log::info("Player Died!");
      return;
    } else {
      saveState(player);
    }
  }
};

$on_mod(Loaded) {
  settings.enabled = Mod::get()->getSettingValue<bool>("enabled");

  listenForSettingChanges<bool>("enabled", [](bool value) {
    settings.enabled = value;
  });
}