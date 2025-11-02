#include "player.h"
#include <QPainter>
#include <QBrush>
#include <QDebug> // 添加这行
#include <QDir>   // 添加这行
#include "armor.h"

Player::Player(qreal x, qreal y, QColor color, int playerID)
    : playerID(playerID), health(100), xVelocity(0), yVelocity(0),
      speed(5), jumpForce(-15), onGround(false), facingRight(playerID == 1),
      crouching(false), hidden(false), color(color),
      currentPlatform(PlatformType::GROUND), hasAdrenaline(false),
      armor(nullptr) // 初始化护甲为空
{
    // 设置玩家矩形
    setRect(0, 0, PLAYER_WIDTH, PLAYER_HEIGHT);
    setPos(x, y);
    setBrush(QBrush(color));

    // 创建默认武器（拳头）
    weapon = new Weapon(WeaponType::FIST);

    // 创建肾上腺素计时器
    adrenalineTimer = new QTimer(this);
    connect(adrenalineTimer, &QTimer::timeout, this, &Player::endAdrenalineEffect);

    adrenalineHealTimer = new QTimer(this);
    connect(adrenalineHealTimer, &QTimer::timeout, this, &Player::adrenalineHeal);

    useImage = false;
}

Player::~Player()
{
    delete weapon;
    delete adrenalineTimer;
    delete adrenalineHealTimer;
    if (armor)
        delete armor;
}

void Player::moveLeft()
{
    if (crouching)
        return;

    facingRight = false;

    // 在冰面上移动更快，增加差异性
    if (currentPlatform == PlatformType::ICE)
    {
        xVelocity = -speed * 1.8; // 增加冰面速度加成，更明显
    }
    // 肾上腺素状态下移动更快
    else if (hasAdrenaline)
    {
        xVelocity = -speed * 1.5;
    }
    else
    {
        xVelocity = -speed;
    }
}

void Player::moveRight()
{
    if (crouching)
        return;

    facingRight = true;

    // 在冰面上移动更快，增加差异性
    if (currentPlatform == PlatformType::ICE)
    {
        xVelocity = speed * 1.8; // 增加冰面速度加成，更明显
    }
    // 肾上腺素状态下移动更快
    else if (hasAdrenaline)
    {
        xVelocity = speed * 1.5;
    }
    else
    {
        xVelocity = speed;
    }
}

void Player::stopMoving()
{
    // 立即停止水平移动
    xVelocity = 0;
}

void Player::crouch(bool isCrouching)
{
    if (isCrouching && !crouching)
    {
        // 进入下蹲状态
        crouching = true;

        // 记住当前底部位置，用于保持玩家"脚"的位置不变
        qreal bottomY = y() + rect().height();

        // 修改碰撞箱为下蹲状态
        setRect(0, 0, PLAYER_WIDTH, CROUCHING_HEIGHT);

        // 调整Y坐标，使玩家底部位置保持不变
        setY(bottomY - CROUCHING_HEIGHT);

        // 在草地上下蹲时隐身效果更明显
        if (currentPlatform == PlatformType::GRASS)
        {
            hidden = true;
            setOpacity(0.3); // 更明显的透明效果
        }
        else
        {
            hidden = false;
            setOpacity(1.0);
        }
    }
    else if (!isCrouching && crouching)
    {
        // 离开下蹲状态
        crouching = false;
        hidden = false;
        setOpacity(1.0);

        // 记住当前底部位置，用于保持玩家"脚"的位置不变
        qreal bottomY = y() + rect().height();

        // 恢复正常高度
        setRect(0, 0, PLAYER_WIDTH, PLAYER_HEIGHT);

        // 调整Y坐标，使玩家底部位置保持不变
        setY(bottomY - PLAYER_HEIGHT);
    }
}

void Player::fire(QList<Projectile *> &projectiles, QGraphicsScene *scene)
{
    if (weapon)
    {
        Projectile *proj = weapon->fire(x() + rect().width() / 2, y() + rect().height() / 2, facingRight, playerID);

        // 检查武器弹药是否用光
        if (weapon->isAmmoEmpty())
        {
            // 删除当前武器，切换回拳头
            delete weapon;
            weapon = new Weapon(WeaponType::FIST);
        }

        if (proj)
        {
            projectiles.append(proj);
            scene->addItem(proj);
        }
    }
}

void Player::applyGravity()
{
    if (!onGround)
    {
        yVelocity += GRAVITY;
    }
}

void Player::move()
{
    // 保存当前位置以检查碰撞
    qreal oldX = x();
    qreal oldY = y();

    // 应用速度限制，防止速度过高导致的穿墙问题
    const qreal MAX_VELOCITY = 20.0;
    if (xVelocity > MAX_VELOCITY)
        xVelocity = MAX_VELOCITY;
    if (xVelocity < -MAX_VELOCITY)
        xVelocity = -MAX_VELOCITY;
    if (yVelocity > MAX_VELOCITY)
        yVelocity = MAX_VELOCITY;
    if (yVelocity < -MAX_VELOCITY)
        yVelocity = -MAX_VELOCITY;

    // 应用移动
    setPos(x() + xVelocity, y() + yVelocity);

    // 边界检查
    if (x() < 0)
    {
        setX(0);
    }
    else if (x() + rect().width() > 1200)
    {
        setX(1200 - rect().width());
    }

    // 添加顶部和底部边界检查
    if (y() < 0)
    {
        setY(0);
        yVelocity = 0; // 防止继续向上移动
    }
    else if (y() + rect().height() > 800)
    {
        setY(800 - rect().height());
        yVelocity = 0;
        onGround = true; // 着陆在底部边界
    }
}

void Player::checkPlatformCollision(Platform *platform)
{
    if (collidesWithItem(platform))
    {
        QRectF playerRect = sceneBoundingRect();
        QRectF platformRect = platform->sceneBoundingRect();

        // 计算上一帧位置
        qreal prevBottom = playerRect.bottom() - yVelocity;
        qreal prevTop = playerRect.top() - yVelocity;
        qreal prevRight = playerRect.right() - xVelocity;
        qreal prevLeft = playerRect.left() - xVelocity;

        // 检查是否从上方着陆 - 改进的检测逻辑
        if ((prevBottom <= platformRect.top() ||
             (playerRect.bottom() >= platformRect.top() &&
              playerRect.bottom() <= platformRect.top() + 10)) &&
            yVelocity >= 0)
        { // 确保玩家正在下落或静止

            // 着陆在平台上
            setY(platformRect.top() - rect().height());
            yVelocity = 0;
            onGround = true;
            currentPlatform = platform->getType();
            return; // 如果已确定站在平台上，提前返回
        }
        // 检查头部碰撞
        else if (prevTop >= platformRect.bottom() &&
                 playerRect.top() <= platformRect.bottom())
        {
            // 头部碰到平台
            setY(platformRect.bottom());
            yVelocity = 0;
        }
        // 检查水平碰撞 - 右侧
        else if (prevRight <= platformRect.left() &&
                 playerRect.right() >= platformRect.left())
        {
            // 右侧碰撞
            setX(platformRect.left() - rect().width());
            xVelocity = 0;
        }
        // 检查水平碰撞 - 左侧
        else if (prevLeft >= platformRect.right() &&
                 playerRect.left() <= platformRect.right())
        {
            // 左侧碰撞
            setX(platformRect.right());
            xVelocity = 0;
        }
    }
}

void Player::pickupItem(Item *item)
{
    switch (item->getType())
    {
    case ItemType::KNIFE:
        delete weapon;
        weapon = new Weapon(WeaponType::KNIFE);
        break;
    case ItemType::BALL:
        delete weapon;
        weapon = new Weapon(WeaponType::BALL);
        break;
    case ItemType::RIFLE:
        delete weapon;
        weapon = new Weapon(WeaponType::RIFLE);
        break;
    case ItemType::SNIPER:
        delete weapon;
        weapon = new Weapon(WeaponType::SNIPER);
        break;
    case ItemType::BANDAGE:
        health = qMin(health + 25, 100);
        break;
    case ItemType::MEDKIT:
        health = 100;
        break;
    case ItemType::ADRENALINE:
        hasAdrenaline = true;
        adrenalineTimer->start(10000);    // 10秒
        adrenalineHealTimer->start(1000); // 每秒回血
        break;
    case ItemType::LIGHT_ARMOR:
        equipArmor(new Armor(ArmorType::LIGHT));
        break;
    case ItemType::BULLETPROOF_VEST:
        equipArmor(new Armor(ArmorType::BULLETPROOF));
        break;
    }
}

void Player::takeDamage(int damage, ProjectileType projectileType)
{
    // 检查是否有护甲可以减免伤害
    if (armor)
    {
        damage = armor->absorbDamage(damage, projectileType);

        // 检查护甲是否已耗尽
        if (armor->isExpired())
        {
            delete armor;
            armor = nullptr;
        }
    }

    health -= damage;
    if (health < 0)
    {
        health = 0;
    }
}

void Player::equipArmor(Armor *newArmor)
{
    if (armor)
    {
        delete armor;
    }
    armor = newArmor;
}

void Player::updateEffects()
{
    // 检查护甲状态
    if (armor && armor->isExpired())
    {
        delete armor;
        armor = nullptr;
    }

    // 检查武器弹药状态
    if (weapon && weapon->isAmmoEmpty() && weapon->getType() != WeaponType::FIST && weapon->getType() != WeaponType::KNIFE)
    {
        delete weapon;
        weapon = new Weapon(WeaponType::FIST);
    }
}

void Player::endAdrenalineEffect()
{
    hasAdrenaline = false;
    adrenalineHealTimer->stop();
}

void Player::adrenalineHeal()
{
    health = qMin(health + 1, 100);
}

QString Player::getWeaponName() const
{
    if (!weapon)
        return "无";

    switch (weapon->getType())
    {
    case WeaponType::FIST:
        return "拳头";
    case WeaponType::KNIFE:
        return "小刀";
    case WeaponType::BALL:
        return "实心球";
    case WeaponType::RIFLE:
        return "步枪";
    case WeaponType::SNIPER:
        return "狙击枪";
    default:
        return "未知";
    }
}

QString Player::getArmorName() const
{
    if (!armor)
        return "无";
    return armor->getName();
}

void Player::jump()
{
    if (crouching || !onGround)
        return;

    yVelocity = jumpForce;
    onGround = false;
}

void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // 如果在草地上下蹲则半透明
    if (hidden)
    {
        painter->setOpacity(0.3);
    }
    else
    {
        painter->setOpacity(1.0);
    }

    // 绘制玩家主体
    if (useImage && !playerImage.isNull())
    {
        // 绘制图片
        painter->drawPixmap(rect().toRect(), playerImage);
    }
    else
    {
        // 绘制玩家矩形
        painter->setBrush(color);
        painter->drawRect(rect());
    }

    // 绘制面部方向指示器（眼睛）
    painter->setBrush(Qt::white);
    if (facingRight)
    {
        painter->drawEllipse(rect().width() - 15, 10, 10, 10);
    }
    else
    {
        painter->drawEllipse(5, 10, 10, 10);
    }

    // 绘制护甲（如果有）
    if (armor)
    {
        QColor armorColor;
        switch (armor->getType())
        {
        case ArmorType::LIGHT:
            armorColor = QColor(150, 150, 255, 180); // 蓝色半透明
            break;
        case ArmorType::BULLETPROOF:
            armorColor = QColor(50, 150, 50, 180); // 绿色半透明
            break;
        default:
            armorColor = QColor(0, 0, 0, 0);
        }

        // 绘制护甲轮廓
        painter->setBrush(QBrush(armorColor));
        painter->setPen(QPen(Qt::black, 1));

        // 根据玩家大小调整护甲尺寸
        qreal armorPadding = 2;
        QRectF armorRect = rect().adjusted(armorPadding, armorPadding,
                                           -armorPadding, -armorPadding);
        painter->drawRect(armorRect);

        // 对于防弹衣，显示耐久度
        if (armor->getType() == ArmorType::BULLETPROOF)
        {
            painter->setPen(Qt::white);
            QFont font = painter->font();
            font.setPointSize(6);
            painter->setFont(font);
            painter->drawText(armorRect, Qt::AlignTop | Qt::AlignHCenter,
                              QString::number(armor->getDurability()));
        }
    }

    // 绘制武器 - 增加武器尺寸和特征使其更明显
    if (weapon)
    {
        QRectF weaponRect;
        if (facingRight)
        {
            weaponRect = QRectF(rect().width() - 5, rect().height() / 2 - 5, 30, 10);
        }
        else
        {
            weaponRect = QRectF(-25, rect().height() / 2 - 5, 30, 10);
        }

        switch (weapon->getType())
        {
        case WeaponType::FIST:
            painter->setBrush(QColor(200, 150, 100));
            if (facingRight)
            {
                painter->drawEllipse(rect().width() - 10, rect().height() / 2 - 5, 15, 15);
            }
            else
            {
                painter->drawEllipse(-5, rect().height() / 2 - 5, 15, 15);
            }
            break;
        case WeaponType::KNIFE:
            painter->setBrush(QColor(150, 150, 150));
            painter->drawRect(weaponRect);
            // 添加刀刃
            painter->setBrush(QColor(220, 220, 220));
            if (facingRight)
            {
                painter->drawRect(rect().width() + 15, rect().height() / 2 - 5, 10, 10);
            }
            else
            {
                painter->drawRect(-25, rect().height() / 2 - 5, 10, 10);
            }
            break;
        case WeaponType::BALL:
            painter->setBrush(QColor(50, 50, 50));
            if (facingRight)
            {
                painter->drawEllipse(rect().width(), rect().height() / 2 - 10, 20, 20);
            }
            else
            {
                painter->drawEllipse(-20, rect().height() / 2 - 10, 20, 20);
            }
            break;
        case WeaponType::RIFLE:
            painter->setBrush(QColor(100, 100, 100));
            painter->drawRect(weaponRect);
            // 添加枪口
            painter->setBrush(QColor(70, 70, 70));
            if (facingRight)
            {
                painter->drawRect(rect().width() + 25, rect().height() / 2 - 2, 8, 4);
            }
            else
            {
                painter->drawRect(-33, rect().height() / 2 - 2, 8, 4);
            }
            break;
        case WeaponType::SNIPER:
            painter->setBrush(QColor(70, 70, 70));
            painter->drawRect(weaponRect);
            // 添加狙击枪特征（枪口和瞄准镜）
            painter->setBrush(QColor(50, 50, 50));
            if (facingRight)
            {
                painter->drawRect(rect().width() + 25, rect().height() / 2 - 2, 12, 4);
                painter->drawEllipse(rect().width() + 10, rect().height() / 2 - 15, 10, 10);
            }
            else
            {
                painter->drawRect(-37, rect().height() / 2 - 2, 12, 4);
                painter->drawEllipse(-20, rect().height() / 2 - 15, 10, 10);
            }
            break;
        }
    }

    // 绘制玩家ID
    painter->setPen(Qt::white);
    painter->drawText(rect().width() / 2 - 5, rect().height() - 10, QString::number(playerID));
}
#include <QMessageBox>
void Player::setPlayerImage(const QString &imagePath)
{
    // QMessageBox::information(nullptr, "调试", "尝试加载图片: " + imagePath);

    playerImage.load(imagePath);
    if (!playerImage.isNull())
    {
        useImage = true;

        // 暂时不改变矩形大小，保持原有游戏逻辑
        // setRect(0, 0, playerImage.width(), playerImage.height());
    }
    else
    {

        useImage = false;
    }
}
