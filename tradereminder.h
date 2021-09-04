#ifndef TRADEREMINDER_H
#define TRADEREMINDER_H

#include <QMainWindow>

namespace Ui {
class tradeReminder;
}

class tradeReminder : public QMainWindow
{
    Q_OBJECT

public:
    explicit tradeReminder(QWidget *parent = 0);
    ~tradeReminder();

private slots:
    void useUser(); //done
    void importData(); //done
    void exportData(); //done
    void doRefresh(); //done
    void copyRes(); //done
    void showAgents(); //done
    void addToLib(); //done

private:
    Ui::tradeReminder *ui;
    QString currentDir;
    QString userAgentFile;
    QString currentUser;
    int n_agents;
    void do_login(QString &username, QString &userAgentFile); //done
};

#endif // TRADEREMINDER_H
