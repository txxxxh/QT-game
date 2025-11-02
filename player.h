#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsRectItem>
#include <QObject>
#include <QGraphicsScene>
#include <QColor>
#include <QTimer>
#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QDir>
#include "platform.h"
#include "item.h"
#include "weapon.h"
#include "projectile.h"

// 前向声明，避免循环引用
class Armor;

class Player : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

public:
    Player(qreal x, qreal y, QColor color, int playerID);
    ~Player();

    void moveLeft();
    void moveRight();
    void stopMoving();
    void jump();
    void crouch(bool isCrouching);
    void fire(QList<Projectile *> &projectiles, QGraphicsScene *scene);
    void applyGravity();
    void move();
    void checkPlatformCollision(Platform *platform);
    void pickupItem(Item *item);
    void takeDamage(int damage, ProjectileType projectileType = ProjectileType::BULLET);
    void updateEffects();
    void setOnGround(bool ground) { onGround = ground; }

    // 新增方法
    void equipArmor(Armor *newArmor);
    Armor *getArmor() const { return armor; }
    void setFacingRight(bool facing) { facingRight = facing; }
    bool isOnGround() const { return onGround; }
    bool hasWeapon() const { return weapon != nullptr; }

    int getHealth() const { return health; }
    int getPlayerID() const { return playerID; }
    bool isCrouching() const { return crouching; }
    QString getWeaponName() const;
    QString getArmorName() const;

    // 在Player类的public部分添加:
    qreal getXVelocity() const { return xVelocity; }
    qreal getYVelocity() const { return yVelocity; }
    void setXVelocity(qreal velocity) { xVelocity = velocity; }
    void setYVelocity(qreal velocity) { yVelocity = velocity; }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void setPlayerImage(const QString &imagePath);

private:
    int playerID;
    int health;
    qreal xVelocity;
    qreal yVelocity;
    qreal speed;
    qreal jumpForce;
    bool onGround;
    bool facingRight;
    bool crouching;
    bool hidden;

    QColor color;
    Weapon *weapon;
    Armor *armor; // 新增护甲
    PlatformType currentPlatform;

    // 状态效果
    bool hasAdrenaline;
    QTimer *adrenalineTimer;
    QTimer *adrenalineHealTimer;

    // 尺寸常量
    const qreal PLAYER_WIDTH = 40;
    const qreal PLAYER_HEIGHT = 80;
    const qreal CROUCHING_HEIGHT = 40;
    const qreal GRAVITY = 0.5;
    const qreal ICE_SPEED_MULTIPLIER = 1.5;

    QPixmap playerImage;
    bool useImage;

private slots:
    void endAdrenalineEffect();
    void adrenalineHeal();
};

#endif // PLAYER_H
