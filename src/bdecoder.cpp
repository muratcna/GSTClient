#include "bdecoder.h"

#include <thread>

#include <QDebug>

using namespace std;

static gboolean
bus_message (GstBus * bus, GstMessage * message, GMainLoop *loop)
{
    GST_DEBUG ("got message %s",
               gst_message_type_get_name (GST_MESSAGE_TYPE (message)));

    switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR: {
        GError *err = NULL;
        gchar *dbg_info = NULL;

        gst_message_parse_error (message, &err, &dbg_info);
        g_printerr ("ERROR from element %s: %s\n",
                    GST_OBJECT_NAME (message->src), err->message);
        g_printerr ("Debugging info: %s\n", (dbg_info) ? dbg_info : "none");
        g_error_free (err);
        g_free (dbg_info);
        g_main_loop_quit (loop);
        break;
    }
    case GST_MESSAGE_EOS:
        g_message("eos arrived\n");
        g_main_loop_quit (loop);
        break;
    default:
        break;
    }
    return TRUE;
}

BDecoder::BDecoder(): pipeline(nullptr), isRunning(false)
{
    if( !gst_is_initialized() ) {
        gst_init(nullptr, nullptr);
    }
}

BDecoder::~BDecoder() {
    if(isRunning) {
        stop();
    }
}

void BDecoder::start(QString decoder, QString protocol, QString network, QString ip, int port)
{
    string portStr = std::to_string(port);
    string ipStr = ip.toStdString();
    string pipdesc;

    if( decoder.compare("MJPEG") == 0 )
    {
        if( protocol.compare("RTP/UDP") == 0 )
        {
            if(( network.compare("UNICAST") == 0 || network.compare("MULTICAST") == 0 || network.compare("BROADCAST") == 0 ))
            {
                pipdesc = "udpsrc address=" + ipStr + " port=" + portStr + " ! rtpjpegdepay ! decodebin ! videoconvert ! autovideosink sync=false";
            }
            else
            {
                pipdesc = "dxgiscreencapsrc ! videoconvert ! openjpegenc ! rtpjpegpay ! udpsink host=" + ipStr + " port=" + portStr;
            }
        }
        else if( protocol.compare("MPEGTS") == 0 )
        {
            if(( network.compare("UNICAST") == 0 || network.compare("MULTICAST") == 0 || network.compare("BROADCAST") == 0 ))
            {
                pipdesc = "filesrc location= ! decodebin ! videoconvert ! openjpegenc ! mpegtsmux ! udpsink host=" + ipStr + " port=" + portStr;
            }
            else
            {
                pipdesc = "dxgiscreencapsrc ! videoconvert ! openjpegenc ! mpegtsmux ! udpsink host=" + ipStr + " port=" + portStr;
            }
        }
    }
    else if( decoder.compare("H264") == 0 )
    {
        if( protocol.compare("RTP/UDP") == 0 )
        {
            if(( network.compare("UNICAST") == 0 || network.compare("MULTICAST") == 0 || network.compare("BROADCAST") == 0 ))
            {
                pipdesc = "udpsrc address=" + ipStr + " port=" + portStr + " ! decodebin ! videoconvert ! autovideosink sync=false";
            }
        }
        else if( protocol.compare("MPEGTS") == 0 )
        {
            if(( network.compare("UNICAST") == 0 || network.compare("MULTICAST") == 0 || network.compare("BROADCAST") == 0 ))
            {
                pipdesc = "filesrc location= ! decodebin ! videoconvert ! x264enc tune=zerolatency ! mpegtsmux ! udpsink host=" + ipStr + " port=" + portStr;
            }
            else
            {
                pipdesc = "dxgiscreencapsrc ! videoconvert ! x264enc tune=zerolatency ! mpegtsmux ! udpsink host=" + ipStr + " port=" + portStr;
            }
        }
    }

    qDebug() << "decoding  pipeline string:" << pipdesc.c_str();

    t = std::thread([this, pipdesc]() { // capture this only for pipeline

        isRunning = true;

        pipeline = gst_parse_launch(pipdesc.c_str(), nullptr);
        //GstElement* pipeline_self = (GstElement*)gst_object_ref(pipeline);

        GMainLoop* loop = g_main_loop_new (NULL, FALSE);
        GstBus* bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
        gst_bus_add_watch(bus, (GstBusFunc)bus_message, loop);
        appsink = (GstAppSink*) gst_bin_get_by_name(GST_BIN(pipeline),"appsinkname");
        gst_element_set_state (pipeline, GST_STATE_PLAYING);


        while(isRunning) {
            GstSample *sample = gst_app_sink_try_pull_sample(appsink, 100000000);

            if(sample){
                GstBuffer *buffer = gst_sample_get_buffer(sample);

                int num, denom;
                GstCaps *caps = gst_sample_get_caps(sample);
                GstStructure *structure = gst_caps_get_structure(caps, 0);
                gst_structure_get_int (structure, "width", &width);
                gst_structure_get_int (structure, "height", &height);
                gst_structure_get_fraction (structure, "framerate", &num, &denom);
                fps = float(num)/float(denom);
                qDebug() <<"fps:" <<fps  <<"mRealWidth:" <<width  <<"mRealHeight:" <<height;


                gsize bufferSize = gst_buffer_get_size(buffer);
                if(bufferSize != mBufferSize) {
                    mBufferSize = bufferSize;

                    gst_sample_unref(sample);
                }

                GstMapInfo mapInfo;
                gst_buffer_map(buffer, &mapInfo, GST_MAP_READ);

                gst_buffer_ref(buffer);
                pImage = QImage(mapInfo.data, mRealWidth, mRealHeight, QImage::Format_RGB888, gstBufferCleanupHandler, buffer);

                gst_buffer_unmap(buffer, &mapInfo);


                gst_sample_unref(sample);
            } else if(gst_app_sink_is_eos(appsink)) {
                qDebug() << "BAVideoImageSource::mainLoop eos arrived";
                break;
            }

        }
        /*qDebug() << "decoder main loop started";
        g_main_loop_run (loop);
        qDebug() << "exited decoder main loop";*/

        // cleanup
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(bus);
        g_main_loop_unref(loop);
        //gst_object_unref(GST_OBJECT(pipeline_self));
        gst_object_unref(GST_OBJECT(pipeline));

        isRunning = false;
    });
    g_assert(appsink);

}

void BDecoder::stop()
{
    if(isRunning) {
        gst_element_send_event(pipeline, gst_event_new_eos());
        t.join();

        std::unique_lock<std::mutex> lock(appSinkMutex);
        gst_object_unref(GST_OBJECT(appsink));
        appsink = nullptr;
        lock.unlock();

        gst_object_unref (GST_OBJECT(pipeline));

    }
}

bool BDecoder::getIsRunning() const
{
    return isRunning;
}
