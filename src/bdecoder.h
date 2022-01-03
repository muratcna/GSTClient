#ifndef BDECODER_H
#define BDECODER_H

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

#include <string>
#include <thread>
#include <mutex>

#include <QObject>

// TODO: is it necesary to be qobject?
class BDecoder
{
public:
    BDecoder();
    ~BDecoder();

    void start(QString decoder, QString protocol, QString network, QString ip, int port);
    void stop();

    bool getIsRunning() const;

private:
    GstElement* pipeline;
    GstAppSink* appsink;
    gsize mBufferSize;

    std::mutex appSinkMutex;
    std::thread t;

    bool isRunning;
    int width;
    int height;
    int fps;

};

#endif // BDECODER_H
