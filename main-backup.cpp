#include <Geode/Enums.hpp>
#include <Geode/Geode.hpp>
#include <Geode/binding/CheckpointObject.hpp>
#include <Geode/binding/PlayerCheckpoint.hpp>
#include <Geode/loader/SettingV3.hpp>

#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJGameLevel.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

#include <Geode/binding/PlayerObject.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <arc/time/Interval.hpp>

using namespace geode::prelude;

struct Settings {
  bool enabled = true;
};
static Settings settings;

struct Input {
  int frame;
  bool pressed;
};

struct Pathfinder {
  PlayerObject* ghost;

  int ghostFrame = 0;
  int playerFrame = 0;
  
  bool jump = false;
  bool isDead = false;

  std::vector<float> inputs;
  std::vector<CCPoint> history;
};
static Pathfinder pathfinder;

class $modify(MyPlayLayer, PlayLayer) {
  bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
    if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

    pathfinder.ghostFrame = 0;
    pathfinder.playerFrame = 0;

    pathfinder.jump = false;
    pathfinder.isDead = false;
    
    pathfinder.inputs.clear();
    pathfinder.history.clear();
    
    pathfinder.ghost = nullptr;
    if (settings.enabled) createGhost();

    return true;
  }

  bool simulate() {
    auto* ghost = pathfinder.ghost;
    if (!ghost || !m_player1 || m_player1->m_isDead) return false;

    for (int i = 0; i < 1; i++) {
      ghost->m_collisionLogTop->removeAllObjects();
      ghost->m_collisionLogBottom->removeAllObjects();

      ghost->m_collisionLogLeft->removeAllObjects();
      ghost->m_collisionLogRight->removeAllObjects();

      ghost->update(0.5f);
      checkCollisions(ghost, 0.5f, false);

      ghost->updateRotation(0.5f);
      ghost->updatePlayerScale();

      if (pathfinder.isDead) break;
    }

    return pathfinder.isDead;
  }

  void pathfind() {
    auto* ghost = pathfinder.ghost;
    bool result = simulate();

    if (!result) {
      ghost->releaseButton(PlayerButton::Jump);

      CCPoint pos = ghost->getPosition();
      pathfinder.history.push_back(pos);

      if (pathfinder.jump) {
        pathfinder.jump = false;
        pathfinder.inputs.push_back(ghost->getPositionX());
      }
    } else {
      pathfinder.isDead = false;

      int index = pathfinder.history.size();
      if (index == 0) return;

      ghost->setPosition(pathfinder.history.at(index - 1));
      ghost->pushButton(PlayerButton::Jump);

      pathfinder.history.pop_back();
      pathfinder.jump = true;
    }
  }

  void createGhost() {
    if (pathfinder.ghost) return;

    pathfinder.ghost = PlayerObject::create(0, 0, this, this->m_objectLayer, false);
    pathfinder.ghost->setVisible(true);

    pathfinder.ghost->copyAttributes(m_player1);
    m_objectLayer->addChild(pathfinder.ghost);
  }

  void postUpdate(float dt) {
    PlayLayer::postUpdate(dt);

    if (settings.enabled) {
      m_player1->releaseButton(PlayerButton::Jump);
      pathfind();

      if (pathfinder.inputs.size() > 0) {
        if (m_player1->getPositionX() >= pathfinder.inputs[0]) {
          pathfinder.inputs.erase(pathfinder.inputs.begin());
          m_player1->pushButton(PlayerButton::Jump);
        }
      }
    }
  }

  void destroyPlayer(PlayerObject* player, GameObject* object) {
    if (player == pathfinder.ghost) {
      pathfinder.isDead = true;
      return;
    }

    PlayLayer::destroyPlayer(player, object);
    pathfinder.ghost->copyAttributes(m_player1);
  }
};

$on_mod(Loaded) {
  settings.enabled = Mod::get()->getSettingValue<bool>("enabled");

  listenForSettingChanges<bool>("enabled", [](bool value) {
    settings.enabled = value;
  });
}