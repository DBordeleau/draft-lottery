#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_dsbTeamCount_valueChanged(double arg1);
    void on_btnDoLottery_clicked();

private:
    Ui::MainWindow *ui;

private:
    void updateTeamInputs(int count);
    void updateTotalOdds();
    void showWinnerAnimation(const QString &winnerName, int winnerOdds);
    void startEliminationSequence(const QVector<QPair<QString, int>> &teams, int winnerIndex);
    void showTeamElimination(const QString &teamName, int position, int odds, std::function<void()> onComplete);
    void eliminateNextTeam(const QVector<QPair<QString, int>> &teams, int currentIndex, int totalTeams, const QString &winnerName, int winnerOdds);
    bool eventFilter(QObject *watched, QEvent *event) override;
    QList<QLineEdit *> oddsInputs;
    QLabel *totalOddsLabel;
};
#endif // MAINWINDOW_H
