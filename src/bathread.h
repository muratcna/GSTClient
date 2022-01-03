#ifndef BATHREAD_H
#define BATHREAD_H

#include <QObject>
#include <QThread>

class BAThread : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isRunning READ isRunning WRITE setIsRunning NOTIFY isRunningChanged)
public:
    BAThread();
    virtual ~BAThread();

    void start();
    void stop();

    bool isRunning() const; // status check

public slots:

signals:
    void isRunningChanged(bool isRunning);

protected:
    // derived must use this for mainLoop!
    virtual void mainLoop() = 0;
    // use this for exiting mainloop, like: while(!getShouldStop()) ...
    bool getShouldStop() const;
    // may be used for self-stop
    void exitThreadLoop();

private slots:
    void setIsRunning(bool isRunning);
    void outerLoop();

private:
    QThread mThread;
    bool mIsRunning = false;
    bool shouldStop = false;
};

#endif // BATHREAD_H
