#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <QGraphicsRectItem>
#include <QColor>

enum class ProjectileType {
    MELEE,
    BALL,
    BULLET
};

class Projectile : public QGraphicsRectItem
{
public:
    Projectile(qreal x, qreal y, bool facingRight, ProjectileType type, int damage, qreal speed, int ownerID, int lifespan = -1);

    void move();

    int getDamage() const { return damage; }
    int getOwnerID() const { return ownerID; }
    ProjectileType getType() const { return type; } // 确保有这个方法

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    ProjectileType type;
    int damage;
    qreal xVelocity;
    qreal yVelocity;
    int ownerID;
    int lifeTime;
    int lifespan;

    // 对于抛射物的重力
    const qreal GRAVITY = 0.1;
};

#endif // PROJECTILE_H
