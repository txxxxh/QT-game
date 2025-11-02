#include "weapon.h"
#include <QTime>

Weapon::Weapon(WeaponType type)
    : type(type), lastFireTime(0)
{
    // 根据武器类型设置属性
    switch (type) {
    case WeaponType::FIST:
        ammo = -1; // 无限
        damage = 5;
        cooldown = 500; // 毫秒
        break;
    case WeaponType::KNIFE:
        ammo = -1; // 无限
        damage = 10;
        cooldown = 300; // 毫秒
        break;
    case WeaponType::BALL:
        ammo = 5;
        damage = 15;
        cooldown = 1000; // 毫秒
        break;
    case WeaponType::RIFLE:
        ammo = 20;
        damage = 18;
        cooldown = 200; // 毫秒
        break;
    case WeaponType::SNIPER:
        ammo = 5;
        damage = 30;
        cooldown = 1500; // 毫秒
        break;
    }
}

Projectile* Weapon::fire(qreal x, qreal y, bool facingRight, int ownerID, bool* ammoEmpty)
{
    int currentTime = QTime::currentTime().msecsSinceStartOfDay();

    // 检查冷却时间
    if (currentTime - lastFireTime < cooldown) {
        if (ammoEmpty) *ammoEmpty = false;
        return nullptr;
    }

    // 检查弹药
    if (ammo == 0) {
        if (ammoEmpty) *ammoEmpty = true;
        return nullptr;
    }

    // 消耗弹药
    if (ammo > 0) {
        ammo--;
        // 检查弹药是否耗尽
        if (ammo == 0 && ammoEmpty) *ammoEmpty = true;
    } else {
        if (ammoEmpty) *ammoEmpty = false;
    }

    // 更新发射时间
    lastFireTime = currentTime;

    // 创建投射物
    ProjectileType projType;
    qreal speed;
    int projLifespan;

    switch (type) {
    case WeaponType::FIST:
        projType = ProjectileType::MELEE;
        speed = 10;
        projLifespan = 20;
        break;
    case WeaponType::KNIFE:
        projType = ProjectileType::MELEE;
        speed = 12;
        projLifespan = 20;
        break;
    case WeaponType::BALL:
        projType = ProjectileType::BALL;
        speed = 8;
        projLifespan = 100;
        break;
    case WeaponType::RIFLE:
        projType = ProjectileType::BULLET;
        speed = 15;
        projLifespan = 80;
        break;
    case WeaponType::SNIPER:
        projType = ProjectileType::BULLET;
        speed = 20;
        projLifespan = 80;
        break;
    default:
        projType = ProjectileType::MELEE;
        speed = 10;
        projLifespan = 20;
    }

    // 近战武器（拳头和小刀）的攻击范围有限
    return new Projectile(x, y, facingRight, projType, damage, speed, ownerID, projLifespan);
}
