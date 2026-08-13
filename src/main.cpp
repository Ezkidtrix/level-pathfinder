#include <Geode/Enums.hpp>
#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>

#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJGameLevel.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

#include <Geode/binding/PlayerObject.hpp>
#include <Geode/modify/PlayerObject.hpp>

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
  bool hold = false;
  PlayerObject* ghost;
};
static Pathfinder pathfinder;

class $modify(MyPlayLayer, PlayLayer) {
  bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
    if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

    pathfinder.hold = false;
    pathfinder.ghost = nullptr;

    if (settings.enabled) createGhost();
    return true;
  }

  void simulate(bool hold) {
    auto* ghost = pathfinder.ghost;
    if (!ghost || !m_player1 || m_player1->m_isDead) return;

    ghost->releaseButton(PlayerButton::Jump);
    if (hold) ghost->pushButton(PlayerButton::Jump);

    for (int i = 0; i < 1; i++) {
      ghost->m_collisionLogTop->removeAllObjects();
      ghost->m_collisionLogBottom->removeAllObjects();

      ghost->m_collisionLogLeft->removeAllObjects();
      ghost->m_collisionLogRight->removeAllObjects();

      ghost->update(0.5f);
      checkCollisions(ghost, 0.5f, false);

      ghost->updateRotation(0.5f);
      ghost->updatePlayerScale();

      if (ghost->m_isDead || ghost->m_maybeIsColliding) break;
    }
  }

  void pathfind() {
    auto* ghost = pathfinder.ghost;
    simulate(true);

    if (ghost->m_isDead) {
      pathfinder.hold = !pathfinder.hold;
      pathfinder.ghost->m_isDead = false;

      m_player1->releaseButton(PlayerButton::Jump);
      if (pathfinder.hold) m_player1->pushButton(PlayerButton::Jump);
    }

    // if (pathfinder.hold) {
    //   pathfinder.hold = false;
    //   m_player1->releaseButton(PlayerButton::Jump);
    // }
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
    if (settings.enabled) pathfind();
  }

  void destroyPlayer(PlayerObject* player, GameObject* object) {
    if (player == pathfinder.ghost) {
      pathfinder.ghost->m_isDead = true;
      pathfinder.ghost->copyAttributes(m_player1);
      
      return;
    }
    
    PlayLayer::destroyPlayer(player, object);
  }
};

$on_mod(Loaded) {
  settings.enabled = Mod::get()->getSettingValue<bool>("enabled");

  listenForSettingChanges<bool>("enabled", [](bool value) {
    settings.enabled = value;
  });
}