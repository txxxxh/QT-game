#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QRandomGenerator>
#include "player.h"
#include "platform.h"
#include "item.h"
#include "projectile.h"

// 前置声明，避免头文件引用警告
class Weapon;
class AI;

// 游戏模式枚举
enum class GameMode {
    PLAYER_VS_PLAYER,
    PLAYER_VS_AI
};

class GameWindow : public QMainWindow
{
    Q_OBJECT

public:
    GameWindow(QWidget *parent = nullptr);
    ~GameWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void updateGame();
    void spawnItems();
    void startGame();
    void startAIGame();  // 新增 - 开始AI对战
    void gameOver(Player* winner);

private:
    void setupScene();
    void createPlayers();
    void createPlatforms();
    void createControls();
    void spawnItem();
    void checkCollisions();
    void renderInfo();

    QGraphicsScene *scene;
    QGraphicsView *view;
    QTimer *gameTimer;
    QTimer *itemTimer;
    QLabel *player1HealthLabel;
    QLabel *player2HealthLabel;
    QLabel *player1WeaponLabel;
    QLabel *player2WeaponLabel;
    QLabel *player1ArmorLabel;  // 新增 - 护甲显示
    QLabel *player2ArmorLabel;  // 新增 - 护甲显示
    QLabel *gameOverLabel;
    QPushButton *startButton;
    QPushButton *aiButton;      // 新增 - AI对战按钮

    Player *player1;
    Player *player2;
    AI *ai;                     // 新增 - AI控制器
    QList<Platform*> platforms;
    QList<Item*> items;
    QList<Projectile*> projectiles;

    int gameWidth;
    int gameHeight;
    bool gameRunning;
    GameMode gameMode;          // 新增 - 游戏模式

    // 用于记录按键状态
    bool keys[10] = {false};

    // 键盘映射
    const Qt::Key player1Keys[5] = {Qt::Key_A, Qt::Key_D, Qt::Key_W, Qt::Key_S, Qt::Key_Space};
    const Qt::Key player2Keys[5] = {Qt::Key_Left, Qt::Key_Right, Qt::Key_Up, Qt::Key_Down, Qt::Key_Return};
};

#endif // GAMEWINDOW_H
