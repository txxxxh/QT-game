#ifndef ARMOR_H
#define ARMOR_H

#include <QObject>
#include "projectile.h"

enum class ArmorType {
    NONE,
    LIGHT,  // 轻甲
    BULLETPROOF  // 防弹衣
};

class Armor : public QObject
{
    Q_OBJECT

public:
    Armor(ArmorType type);

    ArmorType getType() const { return type; }
    int getDurability() const { return durability; }
    bool isExpired() const { return type == ArmorType::BULLETPROOF && durability <= 0; }

    // 处理伤害
    int absorbDamage(int damage, ProjectileType projectileType);

    // 获取显示名称
    QString getName() const;

private:
    ArmorType type;
    int durability;
};

#endif // ARMOR_H
