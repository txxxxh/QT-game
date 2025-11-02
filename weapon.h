#ifndef WEAPON_H
#define WEAPON_H

#include <QObject>
#include "projectile.h"

enum class WeaponType {
    FIST,
    KNIFE,
    BALL,
    RIFLE,
    SNIPER
};

class Weapon : public QObject
{
    Q_OBJECT

public:
    Weapon(WeaponType type);
    bool isAmmoEmpty() const { return ammo == 0; }

    Projectile* fire(qreal x, qreal y, bool facingRight, int ownerID, bool* ammoEmpty = nullptr);
    WeaponType getType() const { return type; }
    int getAmmo() const { return ammo; }

private:
    WeaponType type;
    int ammo;
    int damage;
    int cooldown;
    int lastFireTime;
};

#endif // WEAPON_H
