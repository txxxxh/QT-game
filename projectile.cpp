#include "projectile.h"
#include <QPainter>
#include <QBrush>
#include <cmath>

Projectile::Projectile(qreal x, qreal y, bool facingRight, ProjectileType type, int damage, qreal speed, int ownerID, int lifespan)
    : type(type), damage(damage), ownerID(ownerID), lifeTime(0), lifespan(lifespan)
{
    // 设置投射物外观 - 增大尺寸使其更明显
    switch (type) {
    case ProjectileType::MELEE:
        setRect(0, 0, 30, 10);
        break;
    case ProjectileType::BALL:
        setRect(0, 0, 20, 20); // 增大实心球尺寸
        break;
    case ProjectileType::BULLET:
        setRect(0, 0, 15, 7); // 增大子弹尺寸
        break;
    }

    // 设置位置
    setPos(x - rect().width()/2, y - rect().height()/2);

    // 设置速度
    if (type == ProjectileType::BALL) {
        // 抛物线运动
        xVelocity = facingRight ? speed : -speed;
        yVelocity = -speed * 0.8; // 上抛
    } else {
        // 直线运动
        xVelocity = facingRight ? speed : -speed;
        yVelocity = 0;
    }
}

void Projectile::move()
{
    lifeTime++;

    // 检查生命周期
    if (lifespan > 0 && lifeTime >= lifespan) {
        // 标记为删除
        setPos(-100, -100);
        return;
    }

    // 保存当前位置
    qreal oldX = x();
    qreal oldY = y();

    // 对于实心球，应用重力
    if (type == ProjectileType::BALL) {
        yVelocity += GRAVITY;
    }

    // 应用速度限制
    const qreal MAX_PROJECTILE_SPEED = 20.0;
    if (xVelocity > MAX_PROJECTILE_SPEED) xVelocity = MAX_PROJECTILE_SPEED;
    if (xVelocity < -MAX_PROJECTILE_SPEED) xVelocity = -MAX_PROJECTILE_SPEED;
    if (yVelocity > MAX_PROJECTILE_SPEED) yVelocity = MAX_PROJECTILE_SPEED;
    if (yVelocity < -MAX_PROJECTILE_SPEED) yVelocity = -MAX_PROJECTILE_SPEED;

    // 移动投射物
    setPos(x() + xVelocity, y() + yVelocity);

    // 边界检查在GameWindow中处理
}

void Projectile::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QColor color;
    QColor borderColor;
    qreal borderWidth = 0;

    // 设置子弹/投射物颜色和边框 - 更加醒目
    switch (type) {
    case ProjectileType::MELEE:
        color = QColor(255, 255, 255, 150); // 半透明白色
        borderColor = Qt::white;
        borderWidth = 0.5;
        break;
    case ProjectileType::BALL:
        // 使用亮红色实心球，非常醒目
        color = QColor(255, 50, 50); // 鲜红色
        borderColor = QColor(255, 200, 200); // 亮红色边框
        borderWidth = 2.0;
        break;
    case ProjectileType::BULLET:
        // 使用亮金色子弹，非常醒目
        color = QColor(255, 215, 0); // 金色
        borderColor = QColor(255, 255, 150); // 亮黄色边框
        borderWidth = 1.5;
        break;
    }

    // 绘制投射物本体 - 带有明显边框
    painter->setBrush(QBrush(color));
    painter->setPen(QPen(borderColor, borderWidth));

    if (type == ProjectileType::BALL) {
        // 绘制圆形，带有发光效果
        painter->drawEllipse(rect());

        // 添加高光效果使球更立体
        QRadialGradient gradient(rect().center(), rect().width()/3);
        gradient.setColorAt(0, QColor(255, 255, 255, 100));
        gradient.setColorAt(1, QColor(255, 255, 255, 0));
        painter->setBrush(gradient);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(rect().adjusted(rect().width()/4, rect().height()/4, -rect().width()/4, -rect().height()/4));
    }
    else if (type == ProjectileType::BULLET) {
        // 绘制子弹本体，带有发光效果
        painter->drawRect(rect());

        // 添加子弹头部特效
        QColor headColor(255, 255, 150); // 亮黄色子弹头
        painter->setBrush(headColor);
        if (xVelocity > 0) {
            // 向右飞行的子弹，头部在右侧
            QRectF headRect(rect().right() - 5, rect().top(), 5, rect().height());
            painter->drawRect(headRect);
        } else {
            // 向左飞行的子弹，头部在左侧
            QRectF headRect(rect().left(), rect().top(), 5, rect().height());
            painter->drawRect(headRect);
        }
    }
    else {
        // 近战攻击
        painter->drawRect(rect());
    }
}
