#include "bathread.h"
#include <QDebug>
#include <QAbstractEventDispatcher>

BAThread::BAThread()
    : QObject(nullptr)
    , mIsRunning(false)
{
}

BAThread::~BAThread() {
    stop();
    //moveToThread(QThread::currentThread());
}

void BAThread::start() {
    if(mIsRunning) {
        return;
    }

    shouldStop = false;

    moveToThread(&mThread);
    connect(&mThread, SIGNAL(started()), this, SLOT(outerLoop()));
    mThread.start();
}

void BAThread::exitThreadLoop()
{
    shouldStop = true;
    if(mThread.eventDispatcher())
        mThread.eventDispatcher()->wakeUp();
}

void BAThread::stop() {
    if(!mIsRunning){
        return;
    }

    exitThreadLoop();

    mThread.wait();
}

bool BAThread::isRunning() const
{
    return mIsRunning;
}

void BAThread::setIsRunning(bool isRunning)
{
    if (mIsRunning == isRunning)
        return;

    mIsRunning = isRunning;
    emit isRunningChanged(mIsRunning);
}

void BAThread::outerLoop() {
    setIsRunning(true);

    mainLoop();

    mThread.quit();
    disconnect(&mThread, SIGNAL(started()), this, SLOT(outerLoop()));

    setIsRunning(false);
}

bool BAThread::getShouldStop() const
{
    return shouldStop;
}
