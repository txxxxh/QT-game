#ifndef AI_H
#define AI_H

#include <QObject>
#include <QPointF>
#include "player.h"
#include "platform.h"
#include "item.h"

enum class AIState {
    FIND_WEAPON,
    FIND_ARMOR,
    SEEK_PLAYER,
    ATTACK,
    RETREAT,
    IDLE
};

class AI : public QObject
{
    Q_OBJECT

public:
    AI(Player *controlledPlayer);

    void update(Player *targetPlayer, QList<Item*> &items, QList<Platform*> &platforms,
                QList<Projectile*> &projectiles, QGraphicsScene *scene);

private:
    Player *player;
    AIState currentState;
    QPointF targetPosition;
    int stateTimer;
    int shootCooldown;

    // AI行为方法
    void findWeapon(QList<Item*> &items, QList<Platform*> &platforms);
    void findArmor(QList<Item*> &items, QList<Platform*> &platforms);
    void seekPlayer(Player *targetPlayer, QList<Platform*> &platforms);
    void attack(Player *targetPlayer, QList<Projectile*> &projectiles, QGraphicsScene *scene);
    void retreat(Player *targetPlayer, QList<Platform*> &platforms);

    // 辅助方法
    QPointF findPath(QPointF start, QPointF end, QList<Platform*> &platforms);
    bool canReachPosition(QPointF position, QList<Platform*> &platforms);
    bool isOnPlatform(QPointF position, QList<Platform*> &platforms);
    Platform* findNearestPlatform(QPointF position, QList<Platform*> &platforms);
    Item* findBestItem(QList<Item*> &items);
    bool canAttackFrom(QPointF position, QPointF targetPosition);
    void moveToTarget();
};

#endif // AI_H
