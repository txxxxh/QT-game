#include "ai.h"
#include <QRandomGenerator>
#include <QtMath>
#include <QDebug>

AI::AI(Player *controlledPlayer) : player(controlledPlayer), currentState(AIState::FIND_WEAPON),
    stateTimer(0), shootCooldown(0)
{
    targetPosition = player->pos();
}

void AI::update(Player *targetPlayer, QList<Item*> &items, QList<Platform*> &platforms,
                QList<Projectile*> &projectiles, QGraphicsScene *scene)
{
    // 减少计时器
    stateTimer--;
    shootCooldown--;

    // 当前状态处理
    switch (currentState) {
    case AIState::FIND_WEAPON:
        if (!player->hasWeapon() || player->getWeaponName() == "拳头") {
            findWeapon(items, platforms);
        } else {
            // 已有武器，转向寻找护甲或玩家
            if (!player->getArmor() && QRandomGenerator::global()->bounded(100) < 40) {
                currentState = AIState::FIND_ARMOR;
                stateTimer = 100;
            } else {
                currentState = AIState::SEEK_PLAYER;
                stateTimer = 100;
            }
        }
        break;

    case AIState::FIND_ARMOR:
        if (!player->getArmor()) {
            findArmor(items, platforms);
        } else {
            // 已有护甲，转向寻找玩家
            currentState = AIState::SEEK_PLAYER;
            stateTimer = 100;
        }
        break;

    case AIState::SEEK_PLAYER:
        // 如果生命值低且没有武器，可能会寻找武器或逃跑
        if (player->getHealth() < 30 && (!player->hasWeapon() || player->getWeaponName() == "拳头")) {
            if (QRandomGenerator::global()->bounded(100) < 70) {
                currentState = AIState::FIND_WEAPON;
                stateTimer = 150;
            } else {
                currentState = AIState::RETREAT;
                stateTimer = 100;
            }
        }
        // 如果生命值低，可能会寻找护甲
        else if (player->getHealth() < 50 && !player->getArmor()) {
            if (QRandomGenerator::global()->bounded(100) < 50) {
                currentState = AIState::FIND_ARMOR;
                stateTimer = 120;
            }
        }
        // 正常寻找玩家
        else {
            seekPlayer(targetPlayer, platforms);

            // 如果已经足够接近玩家，且有武器，转为攻击状态
            if (canAttackFrom(player->pos(), targetPlayer->pos()) && player->hasWeapon()) {
                currentState = AIState::ATTACK;
                stateTimer = 50;
            }
        }
        break;

    case AIState::ATTACK:
        attack(targetPlayer, projectiles, scene);

        // 随机决定是否继续攻击或转入其他状态
        if (stateTimer <= 0) {
            int decision = QRandomGenerator::global()->bounded(100);
            if (decision < 30) {
                currentState = AIState::RETREAT;
                stateTimer = 60;
            } else if (decision < 60) {
                currentState = AIState::SEEK_PLAYER;
                stateTimer = 80;
            } else {
                // 继续攻击
                stateTimer = 50;
            }
        }
        break;

    case AIState::RETREAT:
        retreat(targetPlayer, platforms);

        // 撤退一段时间后，转向其他行为
        if (stateTimer <= 0) {
            int decision = QRandomGenerator::global()->bounded(100);
            if (decision < 40) {
                currentState = AIState::FIND_WEAPON;
                stateTimer = 100;
            } else {
                currentState = AIState::SEEK_PLAYER;
                stateTimer = 80;
            }
        }
        break;

    case AIState::IDLE:
        // 闲置状态结束后转向其他行为
        if (stateTimer <= 0) {
            int decision = QRandomGenerator::global()->bounded(100);
            if (decision < 30) {
                currentState = AIState::FIND_WEAPON;
                stateTimer = 80;
            } else if (decision < 50) {
                currentState = AIState::FIND_ARMOR;
                stateTimer = 70;
            } else {
                currentState = AIState::SEEK_PLAYER;
                stateTimer = 100;
            }
        }
        break;
    }

    // 移动到目标位置
    moveToTarget();
}

void AI::findWeapon(QList<Item*> &items, QList<Platform*> &platforms)
{
    Item* bestItem = findBestItem(items);

    if (bestItem) {
        targetPosition = bestItem->pos();
    } else if (stateTimer <= 0) {
        // 找不到武器或时间到，转向寻找玩家
        currentState = AIState::SEEK_PLAYER;
        stateTimer = 80;
    }
}

void AI::findArmor(QList<Item*> &items, QList<Platform*> &platforms)
{
    // 寻找护甲类物品
    Item* armorItem = nullptr;
    for (Item* item : items) {
        if (item->getType() == ItemType::LIGHT_ARMOR || item->getType() == ItemType::BULLETPROOF_VEST) {
            armorItem = item;
            break;
        }
    }

    if (armorItem) {
        targetPosition = armorItem->pos();
    } else if (stateTimer <= 0) {
        // 找不到护甲或时间到，转向寻找玩家
        currentState = AIState::SEEK_PLAYER;
        stateTimer = 80;
    }
}

void AI::seekPlayer(Player *targetPlayer, QList<Platform*> &platforms)
{
    QPointF playerPos = targetPlayer->pos();
    targetPosition = playerPos;

    // 计算到玩家的理想路径
    targetPosition = findPath(player->pos(), playerPos, platforms);
}

void AI::attack(Player *targetPlayer, QList<Projectile*> &projectiles, QGraphicsScene *scene)
{
    QPointF playerPos = targetPlayer->pos();

    // 调整面向
    if (playerPos.x() < player->x()) {
        player->setFacingRight(false);
    } else {
        player->setFacingRight(true);
    }

    // 射击
    if (shootCooldown <= 0) {
        player->fire(projectiles, scene);
        shootCooldown = 30; // 设置射击冷却时间
    }

    // 随机移动以避免被击中
    if (stateTimer % 30 == 0) {
        int moveDirection = QRandomGenerator::global()->bounded(3) - 1; // -1, 0, 1
        targetPosition = player->pos() + QPointF(moveDirection * 50, 0);
    }
}

void AI::retreat(Player *targetPlayer, QList<Platform*> &platforms)
{
    QPointF playerPos = targetPlayer->pos();
    QPointF retreatDir;

    // 往远离玩家的方向撤退
    if (player->x() < playerPos.x()) {
        retreatDir = QPointF(-200, 0); // 向左撤退
    } else {
        retreatDir = QPointF(200, 0); // 向右撤退
    }

    // 设置撤退目标位置
    QPointF retreatPos = player->pos() + retreatDir;
    targetPosition = findPath(player->pos(), retreatPos, platforms);
}

QPointF AI::findPath(QPointF start, QPointF end, QList<Platform*> &platforms)
{
    // 简化版路径规划
    // 首先检查目标是否直接可达
    if (canReachPosition(end, platforms)) {
        return end;
    }

    // 如果不能直接到达，尝试找到最近的平台
    Platform* nearestPlatform = findNearestPlatform(end, platforms);
    if (nearestPlatform) {
        // 返回平台上方的位置
        return QPointF(end.x(), nearestPlatform->y() - 30);
    }

    // 无法找到路径，返回原始目标
    return end;
}

bool AI::canReachPosition(QPointF position, QList<Platform*> &platforms)
{
    // 检查位置下方是否有平台支撑
    return isOnPlatform(position, platforms);
}

bool AI::isOnPlatform(QPointF position, QList<Platform*> &platforms)
{
    for (Platform* platform : platforms) {
        QRectF platformRect = platform->sceneBoundingRect();
        if (position.x() >= platformRect.left() &&
            position.x() <= platformRect.right() &&
            qAbs(position.y() + 30 - platformRect.top()) < 20) {
            return true;
        }
    }
    return false;
}

Platform* AI::findNearestPlatform(QPointF position, QList<Platform*> &platforms)
{
    Platform* nearest = nullptr;
    qreal minDist = 1000000;

    for (Platform* platform : platforms) {
        QPointF platformCenter = platform->sceneBoundingRect().center();
        qreal dist = QLineF(position, platformCenter).length();

        if (dist < minDist) {
            minDist = dist;
            nearest = platform;
        }
    }

    return nearest;
}

Item* AI::findBestItem(QList<Item*> &items)
{
    // 寻找最优物品
    Item* bestItem = nullptr;
    int bestScore = -1;

    for (Item* item : items) {
        int score = 0;

        // 基于物品类型评分
        switch (item->getType()) {
        case ItemType::RIFLE:
            score = 80;
            break;
        case ItemType::SNIPER:
            score = 90;
            break;
        case ItemType::KNIFE:
            score = 30;
            break;
        case ItemType::BALL:
            score = 60;
            break;
        case ItemType::LIGHT_ARMOR:
            score = 50;
            break;
        case ItemType::BULLETPROOF_VEST:
            score = 70;
            break;
        case ItemType::BANDAGE:
            if (player->getHealth() < 50) score = 40;
            else score = 20;
            break;
        case ItemType::MEDKIT:
            if (player->getHealth() < 30) score = 85;
            else score = 40;
            break;
        case ItemType::ADRENALINE:
            score = 65;
            break;
        }

        // 考虑距离因素
        qreal dist = QLineF(player->pos(), item->pos()).length();
        score = score - dist / 10;

        if (score > bestScore) {
            bestScore = score;
            bestItem = item;
        }
    }

    return bestItem;
}

bool AI::canAttackFrom(QPointF position, QPointF targetPosition)
{
    // 检查是否在攻击范围内
    qreal dist = QLineF(position, targetPosition).length();

    // 基于武器类型确定攻击范围
    int attackRange = 100; // 默认范围

    QString weaponName = player->getWeaponName();
    if (weaponName == "拳头") {
        attackRange = 50;
    } else if (weaponName == "小刀") {
        attackRange = 70;
    } else if (weaponName == "步枪") {
        attackRange = 300;
    } else if (weaponName == "狙击枪") {
        attackRange = 500;
    } else if (weaponName == "实心球") {
        attackRange = 200;
    }

    return dist <= attackRange;
}

void AI::moveToTarget()
{
    // 移动到目标位置
    QPointF currentPos = player->pos();

    // 计算移动方向
    if (targetPosition.x() < currentPos.x() - 5) {
        player->moveLeft();
    } else if (targetPosition.x() > currentPos.x() + 5) {
        player->moveRight();
    } else {
        player->stopMoving();
    }

    // 跳跃逻辑
    if (targetPosition.y() < currentPos.y() - 50 && player->isOnGround()) {
        player->jump();
    }

    // 下蹲逻辑 - 在目标位置下方或需要拾取物品时下蹲
    if (targetPosition.y() > currentPos.y() + 30) {
        player->crouch(true);
    } else {
        player->crouch(false);
    }
}
