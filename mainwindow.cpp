#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLineEdit>
#include <QHBoxLayout>
#include <QMap>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QEventLoop>
#include <QPainter>

// This class represents a single piece of confetti
class ConfettiParticle
{
public:
    QPointF position;
    QPointF velocity;
    QColor color;
    double rotation;
    double rotationSpeed;
    double size;

    ConfettiParticle(const QPointF &pos, const QPointF &vel, const QColor &col,
                     double rot, double rotSpeed, double sz)
        : position(pos), velocity(vel), color(col), rotation(rot),
          rotationSpeed(rotSpeed), size(sz) {}

    // Simulates confetti falling and spinning
    void update()
    {
        position += velocity;
        velocity.ry() += 0.1;
        rotation += rotationSpeed;
    }

    void draw(QPainter &painter) const
    {
        painter.save();
        painter.translate(position);
        painter.rotate(rotation);
        painter.setBrush(QBrush(color));
        painter.setPen(Qt::NoPen);
        painter.drawRect(-size / 2, -size / 2, size, size);
        painter.restore();
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), totalOddsLabel(nullptr)
{
    ui->setupUi(this);
    this->setFixedSize(1024, 768);
    setWindowTitle("YOFHL Draft Lottery");
    setWindowIcon(QIcon(":/resources/yofhllogo.png"));

    // Makes sure we only have one instance of the total odds label
    if (!totalOddsLabel)
    {
        totalOddsLabel = new QLabel("Total Odds: 0%");
        QFont totalFont;
        totalFont.setPointSize(12);
        totalFont.setBold(true);
        totalOddsLabel->setFont(totalFont);
        totalOddsLabel->setAlignment(Qt::AlignCenter);
        ui->verticalLayout->addWidget(totalOddsLabel, 0, Qt::AlignHCenter);
    }

    // Trying to apply this layout in designer is not working for some reason
    if (!ui->teamListWidget->layout())
    {
        QVBoxLayout *layout = new QVBoxLayout(ui->teamListWidget);
        layout->setSpacing(2);
        layout->setContentsMargins(4, 4, 4, 4);
        ui->teamListWidget->setLayout(layout);
    }

    // Initialize team inputs
    updateTeamInputs(static_cast<int>(ui->dsbTeamCount->value()));
}

// Destructor for MainWindow, cleans up the UI
MainWindow::~MainWindow()
{
    delete ui;
}

// Updates the total odds label which controls the button state
void MainWindow::updateTotalOdds()
{
    int total = 0;
    for (QLineEdit *edit : oddsInputs)
    {
        bool ok;
        int val = edit->text().toInt(&ok);
        if (ok)
            total += val;
    }

    totalOddsLabel->setText(QString("Total Odds: %1%").arg(total));

    // Green if 100%, red otherwise
    if (total == 100)
    {
        totalOddsLabel->setStyleSheet("color: green;");
    }
    else
    {
        totalOddsLabel->setStyleSheet("color: red;");
    }

    ui->btnDoLottery->setEnabled(total == 100);
}

// Called whenever the number in the double spinner box changes to update the team inputs
void MainWindow::on_dsbTeamCount_valueChanged(double arg1)
{
    updateTeamInputs(static_cast<int>(arg1));
}

// Handles custom painting for the confetti overlay
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    // Draw confetti particles when its a paint event and the watched object had the confetti property
    if (event->type() == QEvent::Paint)
    {
        QWidget *widget = qobject_cast<QWidget *>(watched);
        if (widget && widget->property("confetti").toBool())
        {
            QPainter painter(widget);
            painter.setRenderHint(QPainter::Antialiasing);

            QVariant var = widget->property("confettiParticles");
            QList<ConfettiParticle> *confetti = static_cast<QList<ConfettiParticle> *>(var.value<void *>());

            if (confetti)
            {
                for (const auto &particle : *confetti)
                {
                    particle.draw(painter);
                }
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

// Updates the team inputs based on the number of teams specified in the spinner box
void MainWindow::updateTeamInputs(int count)
{
    QLayout *layout = ui->teamListWidget->layout();

    // Number of inputs before the update
    int currentCount = oddsInputs.size();

    // Logic to preserve existing data in the inputs
    if (count > currentCount)
    {
        // Add new inputs if the team count increases
        for (int i = currentCount; i < count; ++i)
        {
            QHBoxLayout *rowLayout = new QHBoxLayout;

            QLineEdit *nameEdit = new QLineEdit;
            nameEdit->setPlaceholderText(QString("Team %1 Name").arg(i + 1));
            nameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            nameEdit->setAlignment(Qt::AlignCenter);
            nameEdit->setMinimumHeight(30);
            QFont nameFont;
            nameFont.setPointSize(12);
            nameEdit->setFont(nameFont);

            QLineEdit *oddsEdit = new QLineEdit;
            oddsEdit->setPlaceholderText("Odds");
            oddsEdit->setFixedWidth(75);
            oddsEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
            oddsEdit->setAlignment(Qt::AlignCenter);
            oddsEdit->setMinimumHeight(30);
            QFont oddsFont;
            oddsFont.setPointSize(12);
            oddsFont.setBold(true);
            oddsEdit->setFont(oddsFont);

            QLabel *percentLabel = new QLabel("%");
            percentLabel->setMinimumHeight(30);
            percentLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
            QFont percentFont;
            percentFont.setPointSize(12);
            percentFont.setBold(true);
            percentLabel->setFont(percentFont);

            connect(oddsEdit, &QLineEdit::textChanged, this, &MainWindow::updateTotalOdds);
            oddsInputs.append(oddsEdit);

            rowLayout->addWidget(nameEdit);
            rowLayout->addWidget(oddsEdit);
            rowLayout->addWidget(percentLabel);

            QWidget *container = new QWidget;
            container->setLayout(rowLayout);

            layout->addWidget(container);
        }
    }
    // Remove extra inputs if the team count decreases
    else if (count < currentCount)
    {
        for (int i = currentCount - 1; i >= count; --i)
        {
            QLayoutItem *child = layout->itemAt(i);
            if (child)
            {
                QWidget *widget = child->widget();
                if (widget)
                {
                    layout->removeItem(child);
                    delete widget;
                }
                delete child;
            }

            if (i < oddsInputs.size())
            {
                oddsInputs.removeAt(i);
            }
        }
    }

    // Retain existing input data
    for (int i = 0; i < count; ++i)
    {
        QLineEdit *oddsEdit = oddsInputs[i];
        QLineEdit *nameEdit = qobject_cast<QLineEdit *>(oddsEdit->parentWidget()->layout()->itemAt(0)->widget());

        oddsEdit->setText(oddsEdit->text());
        nameEdit->setText(nameEdit->text());
    }

    updateTotalOdds();
}

// Shows the winner animation with confetti and a message, plays after the elimination sequence
void MainWindow::showWinnerAnimation(const QString &winnerName, int winnerOdds)
{
    // Label styling
    QLabel *winLabel = new QLabel(this);
    winLabel->setText(QString("<h1 style='color:gold; font-size:36pt; text-align:center; text-shadow: 2px 2px 4px black;'>%1</h1>"
                              "<h3 style='text-align:center;'>Won with %2% odds!</h3>")
                          .arg(winnerName)
                          .arg(winnerOdds));

    winLabel->setAlignment(Qt::AlignCenter);
    winLabel->setStyleSheet("background-color: rgba(30, 30, 30, 240); border: 2px solid gold; border-radius: 15px; padding: 20px; color: white;");
    winLabel->setFixedSize(700, 300);

    // Center the label off screen
    winLabel->move((width() - winLabel->width()) / 2, -winLabel->height());
    winLabel->show();
    winLabel->raise();

    qApp->processEvents();

    // Drop the label into the center of the screen
    QPropertyAnimation *dropAnim = new QPropertyAnimation(winLabel, "geometry");
    dropAnim->setDuration(1200);
    dropAnim->setStartValue(QRect((width() - winLabel->width()) / 2,
                                  -winLabel->height(),
                                  winLabel->width(),
                                  winLabel->height()));
    dropAnim->setEndValue(QRect((width() - winLabel->width()) / 2,
                                (height() - winLabel->height()) / 2,
                                winLabel->width(),
                                winLabel->height()));
    dropAnim->setEasingCurve(QEasingCurve::OutBounce);

    // Create confetti overlay
    QWidget *confettiOverlay = new QWidget(this);
    confettiOverlay->setGeometry(0, 0, width(), height());
    confettiOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    confettiOverlay->setStyleSheet("background-color: transparent;");

    // Generate confetti particles
    QList<ConfettiParticle> *confetti = new QList<ConfettiParticle>();
    QStringList colors = {"#ffd700", "#ff0000", "#00ff00", "#0000ff", "#ff00ff", "#00ffff", "#ff8000"};
    QRandomGenerator rng = QRandomGenerator::securelySeeded();

    // Generate 150 confetti particles with random properties
    // Initially move upwards, then fall down simulating gravity
    for (int i = 0; i < 150; i++)
    {
        QPointF pos(width() / 2, height() / 3);
        double angle = rng.bounded(2 * M_PI);
        double speed = 1.0 + (rng.generateDouble() * 4.0);         // Range 1 - 5
        QPointF vel(speed * cos(angle), speed * sin(angle) - 3.0); // Initial upward velocity
        QColor color(colors.at(rng.bounded(colors.size())));
        double rotation = rng.bounded(360.0);
        double rotSpeed = -5.0 + (rng.generateDouble() * 10.0); // Range -5-5
        double size = 5.0 + (rng.generateDouble() * 10.0);      // Range 5-15

        confetti->append(ConfettiParticle(pos, vel, color, rotation, rotSpeed, size));
    }

    // Set properties to the overlay widget for the event filter
    confettiOverlay->setProperty("confetti", true);
    confettiOverlay->setProperty("confettiParticles", QVariant::fromValue(static_cast<void *>(confetti)));
    confettiOverlay->installEventFilter(this);

    // Timer for confetti animation
    QTimer *confettiTimer = new QTimer(this);
    connect(confettiTimer, &QTimer::timeout, [confettiOverlay, confetti]()
            {
       for (auto &particle : *confetti) {
           particle.update();
       }
       confettiOverlay->update(); });

    QSequentialAnimationGroup *animGroup = new QSequentialAnimationGroup(this);
    animGroup->addAnimation(dropAnim);

    // Start confetti after the drop animation finishes
    connect(dropAnim, &QPropertyAnimation::finished, [confettiOverlay, confettiTimer, confetti]()
            {
                confettiOverlay->show();
                confettiOverlay->raise();
                confettiTimer->start(16); });
    animGroup->addPause(3000);

    // Fade out the confetti
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(winLabel);
    winLabel->setGraphicsEffect(opacityEffect);
    opacityEffect->setOpacity(1.0);

    QPropertyAnimation *fadeAnim = new QPropertyAnimation(opacityEffect, "opacity");
    fadeAnim->setDuration(800);
    fadeAnim->setStartValue(1.0);
    fadeAnim->setEndValue(0.0);
    fadeAnim->setEasingCurve(QEasingCurve::InQuad);
    animGroup->addAnimation(fadeAnim);

    // Clean up when animation finishes
    connect(animGroup, &QSequentialAnimationGroup::finished, this, [winLabel, confettiOverlay, confettiTimer, confetti]()
            {
        confettiTimer->stop();
        delete confetti;
        confettiOverlay->deleteLater();
        winLabel->deleteLater(); });

    animGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

// Wliminates teams recursively, showing animations for each elimination
void MainWindow::eliminateNextTeam(const QVector<QPair<QString, int>> &teams, int currentIndex, int totalTeams, const QString &winnerName, int winnerOdds)
{
    qDebug() << "Inside eliminateNextTeam - Current index:" << currentIndex << "of" << teams.size();

    // If we've eliminated all teams, show the winner
    if (currentIndex >= teams.size())
    {
        qDebug() << "All teams eliminated, now showing winner:" << winnerName;
        showWinnerAnimation(winnerName, winnerOdds);
        return;
    }

    QString teamName = teams[currentIndex].first;
    int odds = teams[currentIndex].second;
    int position = totalTeams - currentIndex;

    qDebug() << "Eliminating team:" << teamName << "Position:" << position << "Odds:" << odds;

    showTeamElimination(teamName, position, odds, [this, teams, currentIndex, totalTeams, winnerName, winnerOdds]()
                        {
qDebug() << "Elimination animation complete for team at index:" << currentIndex;

// Slight delay before showing next team
QTimer::singleShot(800, this, [this, teams, currentIndex, totalTeams, winnerName, winnerOdds]() {
eliminateNextTeam(teams, currentIndex + 1, totalTeams, winnerName, winnerOdds);
}); });
}

// Starts elimination animation sequence after the lottery winner is determined
void MainWindow::startEliminationSequence(const QVector<QPair<QString, int>> &teams, int winnerIndex)
{
    qDebug() << "Inside startEliminationSequence - Winner index:" << winnerIndex;

    QString winnerTeamName = teams[winnerIndex].first;
    int winnerTeamOdds = teams[winnerIndex].second;

    qDebug() << "Winner in elimination:" << winnerTeamName;

    // Create a list of teams to eliminate (everyone except the winner)
    QVector<QPair<QString, int>> teamsToEliminate;
    for (int i = 0; i < teams.size(); ++i)
    {
        if (i != winnerIndex)
        {
            teamsToEliminate.append(teams[i]);
        }
    }

    qDebug() << "Teams to eliminate count:" << teamsToEliminate.size();

    // Randomize the order in which teams are eliminated
    std::random_device rd;
    QRandomGenerator rng(rd());
    for (int i = teamsToEliminate.size() - 1; i > 0; --i)
    {
        int j = rng.bounded(i + 1);
        teamsToEliminate.swapItemsAt(i, j);
    }

    qDebug() << "Randomized teams to eliminate:";
    for (const auto &team : teamsToEliminate)
    {
        qDebug() << "  -" << team.first << "(" << team.second << "%)";
    }

    QTimer::singleShot(500, this, [this, teamsToEliminate, winnerTeamName, winnerTeamOdds]() mutable
                       {
        qDebug() << "Timer fired, starting elimination for real";
        int totalTeams = teamsToEliminate.size();
        eliminateNextTeam(teamsToEliminate, 0, totalTeams, winnerTeamName, winnerTeamOdds); });
}

// Shows the elimination animation for a single team
void MainWindow::showTeamElimination(const QString &teamName, int position, int odds, std::function<void()> onComplete)
{
    qDebug() << "showTeamElimination - Team:" << teamName << "Position:" << position;

    QLabel *elimLabel = new QLabel(this);
    elimLabel->setAttribute(Qt::WA_DeleteOnClose);

    // Red text for eliminated teams
    QString styleColor = "#ff0000";
    int fontSize = 16;

    // Format: "Eliminated: [Team Name] - [Odds]% chance of winning"
    elimLabel->setText(QString("<h2 style='color:%1; font-size:%2pt; text-shadow: 2px 2px 5px black;'>ELIMINATED: %3 - %4% chance of winning</h2>")
                           .arg(styleColor)
                           .arg(fontSize)
                           .arg(teamName)
                           .arg(odds));

    // Label styling improve later
    elimLabel->setAlignment(Qt::AlignCenter);
    elimLabel->setStyleSheet("background-color: rgba(30, 30, 30, 240); border: 2px solid #555; border-radius: 10px; padding: 15px; color: white;");
    elimLabel->setFixedSize(750, 100);

    // Position the label off-screen to the left
    elimLabel->move(-elimLabel->width(), (height() - elimLabel->height()) / 2);
    elimLabel->show();
    elimLabel->raise();
    qApp->processEvents();

    // Slide in animation
    QPropertyAnimation *slideIn = new QPropertyAnimation(elimLabel, "pos");
    slideIn->setDuration(800);
    slideIn->setStartValue(QPoint(-elimLabel->width(), (height() - elimLabel->height()) / 2));
    slideIn->setEndValue(QPoint((width() - elimLabel->width()) / 2, (height() - elimLabel->height()) / 2));
    slideIn->setEasingCurve(QEasingCurve::OutCubic);

    // Pause label in middle of screen for 3 seconds
    QSequentialAnimationGroup *animGroup = new QSequentialAnimationGroup(this);
    animGroup->addAnimation(slideIn);
    animGroup->addPause(3000);

    // Slide out animation, move up slightly
    QPropertyAnimation *slideOut = new QPropertyAnimation(elimLabel, "pos");
    slideOut->setDuration(800);
    slideOut->setStartValue(QPoint((width() - elimLabel->width()) / 2, (height() - elimLabel->height()) / 2));
    slideOut->setEndValue(QPoint(width(), (height() - elimLabel->width()) / 2));
    slideOut->setEasingCurve(QEasingCurve::InCubic);

    animGroup->addAnimation(slideOut);

    connect(animGroup, &QSequentialAnimationGroup::finished, this, [elimLabel, onComplete]()
            {
        elimLabel->deleteLater();
        onComplete(); });

    // Start the animation
    animGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

// Starts the lottery process
void MainWindow::on_btnDoLottery_clicked()
{
    qDebug() << "Starting lottery process";

    QVector<QString> teamNames;
    QVector<int> teamOdds;

    // Collect team names and odds from the input fields
    for (int i = 0; i < oddsInputs.size(); ++i)
    {
        QLineEdit *oddsEdit = oddsInputs[i];
        bool ok;
        int odds = oddsEdit->text().toInt(&ok);

        if (ok && odds > 0)
        {
            QLineEdit *nameEdit = qobject_cast<QLineEdit *>(oddsEdit->parentWidget()->layout()->itemAt(0)->widget());
            QString teamName = nameEdit->text().trimmed();

            // If team name is empty, provide a default name
            if (teamName.isEmpty())
            {
                teamName = QString("Team %1").arg(i + 1);
            }

            teamNames.append(teamName);
            teamOdds.append(odds);
        }
    }

    int totalOdds = 0;
    for (int odds : teamOdds)
    {
        totalOdds += odds;
    }

    int randomValue = QRandomGenerator::global()->bounded(totalOdds);
    int cumulativeOdds = 0;
    int winnerIndex = -1;

    // Whoever occupies the range of the random value is the winner
    for (int i = 0; i < teamOdds.size(); ++i)
    {
        cumulativeOdds += teamOdds[i];
        if (randomValue < cumulativeOdds)
        {
            winnerIndex = i;
            break;
        }
    }

    if (winnerIndex < 0)
        return;

    QVector<QPair<QString, int>> teams;
    for (int i = 0; i < teamNames.size(); ++i)
    {
        teams.append(qMakePair(teamNames[i], teamOdds[i]));
    }

    QString winnerTeamName = teamNames[winnerIndex];
    int winnerTeamOdds = teamOdds[winnerIndex];

    qDebug() << "Winner determined:" << winnerTeamName << "with odds:" << winnerTeamOdds;
    qDebug() << "Total teams count:" << teams.size();

    // Make sure the lottery can't be run again during animation
    ui->btnDoLottery->setEnabled(false);

    QLabel *calculating = new QLabel("Drawing lottery...", this);
    calculating->setStyleSheet("background-color: rgba(30, 30, 30, 220); color: white; padding: 10px; border-radius: 5px;");
    calculating->setAlignment(Qt::AlignCenter);
    calculating->adjustSize();
    calculating->move((width() - calculating->width()) / 2, 50);
    calculating->raise();
    calculating->show();
    qApp->processEvents();

    // Start elimination
    QTimer::singleShot(1000, [=]()
                       {
                           calculating->hide();
                           calculating->deleteLater();
                           qDebug() << "Starting elimination sequence";
                           startEliminationSequence(teams, winnerIndex); });

    // Calculate animation duration for final message box timing
    int teamsToEliminate = teams.size() - 1;
    int animationDuration = teamsToEliminate * 5400 + 4000;

    qDebug() << "Teams to eliminate:" << teamsToEliminate;
    qDebug() << "Total animation duration:" << animationDuration;

    // Show final message only after all animations
    QTimer::singleShot(animationDuration, [this, winnerTeamName, winnerTeamOdds]()
                       {
        qDebug() << "Showing final winner message box";
        QMessageBox winnerBox(this);
        winnerBox.setWindowTitle("WE HAVE A WINNER!");

        QString winnerMessage = "<h2>Congratulations to:</h2>";
        winnerMessage += QString("<h1 style='color:gold; font-size: 24pt;'>%1!</h1>")
                             .arg(winnerTeamName);
        winnerMessage += QString("<p>who will draft first overall. They had a <b>%1% chance</b> of winning.</p>")
                             .arg(winnerTeamOdds);

        winnerBox.setText(winnerMessage);
        winnerBox.exec();

        // Re-enable lottery button
        ui->btnDoLottery->setEnabled(true); });
}
