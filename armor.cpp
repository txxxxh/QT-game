#include "armor.h"

Armor::Armor(ArmorType type) : type(type)
{
    // 设置初始耐久度
    switch (type) {
    case ArmorType::LIGHT:
        durability = 100; // 轻甲不消耗耐久度
        break;
    case ArmorType::BULLETPROOF:
        durability = 100;
        break;
    default:
        durability = 0;
        break;
    }
}

int Armor::absorbDamage(int damage, ProjectileType projectileType)
{
    int remainingDamage = damage;

    if (type == ArmorType::LIGHT) {
        // 轻甲防御
        if (projectileType == ProjectileType::MELEE) {
            // 完全免疫拳头伤害
            if (damage <= 5) {
                remainingDamage = 0;
            } else {
                // 小刀伤害减半
                remainingDamage = damage / 2;
            }
        }
    }
    else if (type == ArmorType::BULLETPROOF) {
        // 防弹衣
        if (projectileType == ProjectileType::BULLET) {
            // 防弹衣吸收70%的子弹伤害
            int absorbedDamage = damage * 0.7;
            remainingDamage = damage - absorbedDamage;

            // 降低耐久度
            durability -= absorbedDamage;
            if (durability <= 0) {
                durability = 0;
                // 确保isExpired()返回true
            }
        }
    }

    return remainingDamage;
}

QString Armor::getName() const
{
    switch (type) {
    case ArmorType::LIGHT:
        return "轻甲";
    case ArmorType::BULLETPROOF:
        return QString("防弹衣(%1)").arg(durability);
    default:
        return "无";
    }
}
