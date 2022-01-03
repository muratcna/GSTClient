#include "bwidget.h"

#include <QVBoxLayout>
#include <QDebug>
#include <QTimer>
#include <QFileDialog>
#include <QScreen>

BWidget::BWidget(QWidget *parent)
    : QWidget(parent)
    , mDecoder(nullptr)
{

    this->setFixedSize(350, 150);

    mLblIP = new QLabel("Ip");
    mLblPort = new QLabel("Port");
    mIpText = new QLineEdit("224.0.0.1");
    mPortText = new QLineEdit("5000");

    mLblNetworkProtocol = new QLabel("Protocol");
    mLblDecoder = new QLabel("Decoder");
    mLblCast = new QLabel("Cast");

    mCbNetworkProtocol = new QComboBox;
    mCbDecoder = new QComboBox;
    mCbCast = new QComboBox;

    {
        mCbNetworkProtocol->addItem("RTP/UDP");
        mCbNetworkProtocol->addItem("MPEGTS");
    }
    {
        mCbDecoder->addItem("H264");
        mCbDecoder->addItem("MJPEG");
    }
    {
        mCbCast->addItem("UNICAST");
        mCbCast->addItem("MULTICAST");
        mCbCast->addItem("BROADCAST");
    }

    mBtnStart = new QPushButton("Start");

    mBtnStop = new QPushButton("Stop");
    mBtnStop->setEnabled(false);

    QVBoxLayout *tVb = new QVBoxLayout;
    {
        {//1.line
            QHBoxLayout *tHb = new QHBoxLayout;
            {
                tHb->addWidget(mLblIP);
                tHb->addWidget(mIpText);
                tHb->addWidget(mLblPort);
                tHb->addWidget(mPortText);
                tVb->addLayout(tHb);
            }
        }
        {//2.line
            QHBoxLayout *tHb = new QHBoxLayout;
            {
                tHb->addWidget(mLblNetworkProtocol);
                tHb->addWidget(mLblDecoder);
                tHb->addWidget(mLblCast);
                tVb->addLayout(tHb);
            }
        }
        {//3.line
            QHBoxLayout *tHb = new QHBoxLayout;
            {
                tHb->addWidget(mCbNetworkProtocol);
                tHb->addWidget(mCbDecoder);
                tHb->addWidget(mCbCast);
                tVb->addLayout(tHb);
            }
        }
        {//4.line
            QHBoxLayout *tHb = new QHBoxLayout;
            {
                tHb->addWidget(mBtnStart);
                tHb->addWidget(mBtnStop);
                tVb->addLayout(tHb);
            }
        }
    }
    setLayout(tVb);

    connect(mBtnStart, &QPushButton::clicked, this, &BWidget::onClickBtnStart,Qt::QueuedConnection);
    connect(mBtnStop, &QPushButton::clicked, this, &BWidget::onClickBtnStop,Qt::QueuedConnection);

    QTimer *tTimer = new QTimer;
    connect(tTimer, &QTimer::timeout, this, [this](){
        if(!mDecoder->getIsRunning()) {
            mBtnStop->setEnabled(false);
        }

    });
    tTimer->start(100);

    mDecoder = new BDecoder;
}

BWidget::~BWidget()
{
    qDebug() << "~BWidget";
}

void BWidget::onClickBtnStart()
{
    mBtnStart->setEnabled(false);
    mBtnStop->setEnabled(true);

    qDebug() << "Protocol:" << mCbNetworkProtocol->currentText();
    qDebug() << "Decoder:" << mCbDecoder->currentText();
    qDebug() << "Cast:" << mCbCast->currentText();
    qDebug() << "Ip:" << mIpText->text();
    qDebug() << "Port:" << mPortText->text();

    mDecoder->start(mCbDecoder->currentText(), mCbNetworkProtocol->currentText(), mCbCast->currentText(),
                    mIpText->text(), mPortText->text().toInt());
}

void BWidget::onClickBtnStop()
{
    mBtnStart->setEnabled(true);
    mBtnStop->setEnabled(false);

    mDecoder->waitForStop();
    mDecoder->signalStop();
}
