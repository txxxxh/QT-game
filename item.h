#ifndef ITEM_H
#define ITEM_H

#include <QGraphicsRectItem>
#include <QColor>
#include "platform.h"

enum class ItemType {
    KNIFE,
    BALL,
    RIFLE,
    SNIPER,
    BANDAGE,
    MEDKIT,
    ADRENALINE,
    LIGHT_ARMOR,    // 新增 - 轻甲
    BULLETPROOF_VEST // 新增 - 防弹衣
};

class Item : public QGraphicsRectItem
{
public:
    Item(qreal x, qreal y, ItemType type);

    void applyGravity();
    void move();
    void checkPlatformCollision(Platform *platform);

    ItemType getType() const { return type; }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    ItemType type;
    qreal yVelocity;
    bool onGround;

    const qreal GRAVITY = 0.2; // 物品下落速度较慢
};

#endif // ITEM_H
