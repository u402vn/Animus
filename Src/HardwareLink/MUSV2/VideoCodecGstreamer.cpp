#include "VideoCodecGstreamer.h"
#include "VideoCodecGstreamerVersion.h"

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

// Flags to indicate to parent thread that GstreamerThread started and finished
static volatile bool bEncThreadStarted = false;
static volatile bool bEncThreadFinished = false;
static volatile bool bDecThreadStarted = false;
static volatile bool bDecThreadFinished = false;

typedef struct {
    GstElement *pipeline;
    GstElement *appsrc;
    GstElement *encoder;
    GstElement *filter1;
    GstElement *parser;
    GstElement *filter2;
    GstElement *appsink;

    int width;
    int height;

    int bitrate;
    int fps;
    int gop_size;
    int encoder_type;

    GstClockTime timestamp;

    GMainLoop *loop;
    GstBus *bus;

} gst_enc_t;

typedef struct {
    GstElement *pipeline;
    GstElement *appsrc;
    GstElement *parser;
    GstElement *decoder;
    GstElement *appsink;

    int width;
    int height;

    int bitrate;
    int decoder_type;

    GMainLoop *loop;
    GstBus *bus;

} gst_dec_t;

// Bus messages processing, similar to all gstreamer examples
gboolean bus_enc_call(GstBus *bus, GstMessage *msg, gpointer data)
{
    GMainLoop *loop = (GMainLoop *) data;

    switch (GST_MESSAGE_TYPE (msg))
    {

    case GST_MESSAGE_EOS:
        fprintf(stderr, "End of stream\n");
        g_main_loop_quit(loop);
        break;

    case GST_MESSAGE_ERROR:
    {
        gchar *debug;
        GError *error;

        gst_message_parse_error(msg, &error, &debug);
        g_free(debug);

        g_printerr("Error: %s\n", error->message);
        g_error_free(error);

        g_main_loop_quit(loop);

        break;
    }
    default:
        break;
    }

    return TRUE;
}

gboolean bus_dec_call(GstBus *bus, GstMessage *msg, gpointer data)
{
    GMainLoop *loop = (GMainLoop *) data;

    switch (GST_MESSAGE_TYPE (msg))
    {

    case GST_MESSAGE_EOS:
        fprintf(stderr, "End of stream\n");
        g_main_loop_quit(loop);
        break;

    case GST_MESSAGE_ERROR:
    {
        gchar *debug;
        GError *error;

        gst_message_parse_error(msg, &error, &debug);
        g_free(debug);

        g_printerr("Error: %s\n", error->message);
        g_error_free(error);

        g_main_loop_quit(loop);

        break;
    }
    default:
        break;
    }

    return TRUE;
}

// Creates and sets up Gstreamer pipeline for encoding.
void* GstreamerEncThread(void* pThreadParam)
{
    gst_enc_t* enc = (gst_enc_t*)pThreadParam;
    guint bus_watch_id;

    enc->timestamp = 0;
    enc->pipeline = gst_pipeline_new("mypipeline");
    enc->appsrc = gst_element_factory_make("appsrc", "mysource");
    enc->filter1 =  gst_element_factory_make ("capsfilter", "myfilter1");

#if defined( VIDEO_CODEC_JETSON_NANO )
    if (enc->encoder_type == 0)
        enc->encoder = gst_element_factory_make("omxh264enc", "myomx");
    else if (enc->encoder_type == 1)
        enc->encoder = gst_element_factory_make("omxh265enc", "myomx");
#elif defined( VIDEO_CODEC_JETSON_NX )
#elif defined( VIDEO_CODEC_RASPBERRY_PI )
#elif defined( VIDEO_CODEC_IMX_8M )
    if (enc->encoder_type == 0)
        enc->encoder = gst_element_factory_make("vpuenc_h264", "myvpuenc");
    else if (enc->encoder_type == 1)
        enc->encoder = gst_element_factory_make("vpudec", "myvpudec");
#endif


    enc->filter2 =  gst_element_factory_make ("capsfilter", "myfilter2");
    
    if (enc->encoder_type == 0)
        enc->parser =  gst_element_factory_make ("h264parse" , "myparser");
    else if (enc->encoder_type == 1)
        enc->parser =  gst_element_factory_make ("h265parse" , "myparser");

    enc->appsink = gst_element_factory_make("appsink", "mysink");

    // Check if all elements were created
    if (!enc->pipeline || !enc->appsrc || !enc->filter1 ||
            !enc->encoder || !enc->filter2 || !enc->parser || !enc->appsink)
    {
        fprintf(stderr, "Could not gst_element_factory_make, terminating\n");
        bEncThreadStarted = bEncThreadFinished = true;
        return (void*)0xDEAD;
    }

    // blocksize is important for jpegenc to know how many data to expect from appsrc in a single frame, too
    char szTemp[64];
    sprintf(szTemp, "%d", enc->width*enc->height*3/2);
    g_object_set(G_OBJECT (enc->appsrc), "blocksize", szTemp, NULL);
    g_object_set(G_OBJECT (enc->appsrc), "max-bytes", enc->width*enc->height*3/2, NULL);
    g_object_set(G_OBJECT (enc->appsrc), "block", TRUE, NULL);
    g_object_set(G_OBJECT (enc->appsrc), "is-live", FALSE, NULL);
    g_object_set(G_OBJECT (enc->appsrc), "do-timestamp", TRUE, NULL);
    //g_object_set(G_OBJECT (appsrc), "num-buffers", frameSize, NULL);


    // appsrc should be linked to jpegenc with these caps otherwise jpegenc does not know size of incoming buffer
    GstCaps *filtercaps1; // between appsrc and jpegenc
    filtercaps1 = gst_caps_new_simple("video/x-raw", "format",
            G_TYPE_STRING, "NV12", "width", G_TYPE_INT, enc->width, "height",
            G_TYPE_INT, enc->height, "framerate", GST_TYPE_FRACTION, enc->fps, 1, NULL);
    g_object_set (G_OBJECT (enc->filter1), "caps", filtercaps1, NULL);

    if (enc->encoder_type == 0)
    {
        //g_object_set(G_OBJECT (enc->encoder), "low-latency", 1, NULL);
        g_object_set(G_OBJECT (enc->encoder), "preset-level", 0, NULL);
        g_object_set(G_OBJECT (enc->encoder), "num-B-Frames", 0, NULL);
        //g_object_set(G_OBJECT (enc->encoder), "vbv-size", 0, NULL);
        g_object_set(G_OBJECT (enc->encoder), "iframeinterval", enc->gop_size, NULL);
        g_object_set(G_OBJECT (enc->encoder), "profile", 1, NULL);
        //g_object_set(G_OBJECT (enc->encoder), "qp-range", "0,0:0,0:0,0", NULL);

        g_object_set(G_OBJECT (enc->encoder), "control-rate", 2, NULL);
#if defined( VIDEO_CODEC_JETSON_NANO )
        g_object_set(G_OBJECT (enc->encoder), "bitrate", enc->bitrate*1000, NULL);
#elif defined( VIDEO_CODEC_JETSON_NX )
#elif defined( VIDEO_CODEC_RASPBERRY_PI )
#elif defined( VIDEO_CODEC_IMX_8M )
        g_object_set(G_OBJECT (enc->encoder), "bitrate", enc->bitrate, NULL);
#endif
        //g_object_set(G_OBJECT (enc->encoder), "EnableTwopassCBR", TRUE, NULL);
        //g_object_set(G_OBJECT (enc->encoder), "EnableStringentBitrate", TRUE, NULL);
    }
    else if (enc->encoder_type == 1)
    {
        g_object_set(G_OBJECT (enc->encoder), "preset-level", 0, NULL);
        //g_object_set(G_OBJECT (enc->encoder), "num-B-Frames", 0, NULL);
        //g_object_set(G_OBJECT (enc->encoder), "vbv-size", 0, NULL);
        g_object_set(G_OBJECT (enc->encoder), "iframeinterval", enc->gop_size, NULL);
        g_object_set(G_OBJECT (enc->encoder), "profile", 1, NULL);
        //g_object_set(G_OBJECT (enc->encoder), "qp-range", "0,0:0,0:0,0", NULL);

        g_object_set(G_OBJECT (enc->encoder), "control-rate", 2, NULL);
#if defined( VIDEO_CODEC_JETSON_NANO )
        g_object_set(G_OBJECT (enc->encoder), "bitrate", enc->bitrate*1000, NULL);
#elif defined( VIDEO_CODEC_JETSON_NX )
#elif defined( VIDEO_CODEC_RASPBERRY_PI )
#elif defined( VIDEO_CODEC_IMX_8M )
        g_object_set(G_OBJECT (enc->encoder), "bitrate", enc->bitrate, NULL);
#endif
        //g_object_set(G_OBJECT (enc->encoder), "EnableTwopassCBR", TRUE, NULL);
        //g_object_set(G_OBJECT (enc->encoder), "EnableStringentBitrate", TRUE, NULL);
    }


    g_object_set(G_OBJECT (enc->parser), "disable-passthrough", true, NULL);

    //    g_object_set(G_OBJECT (h264enc), "quant-i-frames", 1, NULL);
    //    g_object_set(G_OBJECT (h264enc), "quant-p-frames", 0, NULL);
    //    g_object_set(G_OBJECT (h264enc), "quant-b-frames", 0, NULL);
    //g_object_set(G_OBJECT (h264enc), "MeasureEncoderLatency", TRUE, NULL);
    //g_object_set(G_OBJECT (h264enc), "SliceIntraRefreshInterval", 0, NULL);
    //g_object_set(G_OBJECT (h264enc), "insert-sps-pps", TRUE, NULL);
    //g_object_set(G_OBJECT (h264enc), "qp-range", "-1, -1:10,10:-1,-1", NULL);

    if (enc->encoder_type == 0)
    {
        GstCaps *caps_for_filter = gst_caps_new_simple ("video/x-h264",
        "stream-format", G_TYPE_STRING, "byte-stream", NULL);
        g_object_set (G_OBJECT (enc->filter2), "caps", caps_for_filter, NULL);
    }else if (enc->encoder_type == 1)
    {
        GstCaps *caps_for_filter = gst_caps_new_simple ("video/x-h265",
        "stream-format", G_TYPE_STRING, "byte-stream", NULL);
        g_object_set (G_OBJECT (enc->filter2), "caps", caps_for_filter, NULL);
    }
    g_object_set(G_OBJECT (enc->appsink), "max-buffers", 1, NULL);
    g_object_set(G_OBJECT (enc->appsink), "async", TRUE, NULL);
    g_object_set(G_OBJECT (enc->appsink), "sync", FALSE, NULL);
    // Create gstreamer loop
    enc->loop = g_main_loop_new(NULL, FALSE);

    // add a message handler
    enc->bus = gst_pipeline_get_bus(GST_PIPELINE(enc->pipeline));
    bus_watch_id = gst_bus_add_watch(enc->bus, bus_enc_call, enc->loop);
    gst_object_unref(enc->bus);

    // add all elements into pipeline
    gst_bin_add_many(GST_BIN(enc->pipeline), enc->appsrc, enc->filter1,
                     enc->encoder, /*enc->parser,*/ enc->filter2, enc->appsink, NULL);

    // link elements into pipe: appsrc -> jpegenc -> appsink
    gst_element_link_many((GstElement*)enc->appsrc, enc->filter1, enc->encoder,
                           /*enc->parser,*/ enc->filter2, enc->appsink, NULL);

    fprintf(stderr, "Setting g_main_loop_run to GST_STATE_PLAYING\n");
    // Start pipeline so it could process incoming data
    gst_element_set_state(enc->pipeline, GST_STATE_PLAYING);

    // Indicate that thread was started
    bEncThreadStarted = true;

    // Loop will run until receiving EOS (end-of-stream), will block here
    g_main_loop_run(enc->loop);

    fprintf(stderr, "g_main_loop_run returned, stopping playback\n");

    // Stop pipeline to be released
    gst_element_set_state(enc->pipeline, GST_STATE_NULL);

    fprintf(stderr, "Deleting pipeline\n");
    // THis will also delete all pipeline elements
    gst_object_unref(GST_OBJECT(enc->pipeline));

    g_source_remove (bus_watch_id);
    g_main_loop_unref (enc->loop);

    // Indicate that thread was finished
    bEncThreadFinished = true;

    return NULL;
}

// Creates and sets up Gstreamer pipeline for encoding.
void* GstreamerDecThread(void* pThreadParam)
{
    gst_dec_t* dec = (gst_dec_t*)pThreadParam;
    guint bus_watch_id;

    dec->pipeline = gst_pipeline_new("mypipeline");
    dec->appsrc = gst_element_factory_make("appsrc", "mysource");
    
    if (dec->decoder_type == 0)
        dec->parser =  gst_element_factory_make ("h264parse", "myparse");
    else if (dec->decoder_type == 1)
        dec->parser =  gst_element_factory_make ("h265parse", "myparse");

#if defined( VIDEO_CODEC_JETSON_NANO )
    if (dec->decoder_type == 0)
        dec->decoder = gst_element_factory_make("omxh264dec", "myomx");
    else if (dec->decoder_type == 1)
        dec->decoder = gst_element_factory_make("omxh265dec", "myomx");
#elif defined( VIDEO_CODEC_JETSON_NX )
#elif defined( VIDEO_CODEC_RASPBERRY_PI )
#elif defined( VIDEO_CODEC_IMX_8M )
    if (dec->decoder_type == 0)
        dec->decoder = gst_element_factory_make("vpudec", "myvpudec");
    else if (dec->decoder_type == 1)
        dec->decoder = gst_element_factory_make("vpudec", "myvpudec");
#endif
 

    dec->appsink = gst_element_factory_make("appsink", "mysink");

    // Check if all elements were created
    if (!dec->pipeline || !dec->appsrc || !dec->parser ||
            !dec->decoder || !dec->appsink)
    {
        fprintf(stderr, "Could not gst_element_factory_make, terminating\n");
        bDecThreadStarted = bDecThreadFinished = true;
        return (void*)0xDEAD;
    }

    // blocksize is important for jpegenc to know how many data to expect from appsrc in a single frame, too
    //g_object_set(G_OBJECT (dec->appsrc), "num-buffers", 1, NULL);

    if (dec->decoder_type == 0)
        g_object_set(G_OBJECT (dec->parser), "disable-passthrough", true, NULL);
    else if (dec->decoder_type == 1)
        g_object_set(G_OBJECT (dec->parser), "disable-passthrough", true, NULL);


    if (dec->decoder_type == 0)
    {
        g_object_set(G_OBJECT (dec->decoder), "enable-low-outbuffer", TRUE, NULL);
        g_object_set(G_OBJECT (dec->decoder), "output-buffers", 1, NULL);
        g_object_set(G_OBJECT (dec->decoder), "disable-dpb", TRUE, NULL);
    }
    else if (dec->decoder_type == 1)
    {
        g_object_set(G_OBJECT (dec->decoder), "enable-low-outbuffer", TRUE, NULL);
        g_object_set(G_OBJECT (dec->decoder), "output-buffers", 1, NULL);
        g_object_set(G_OBJECT (dec->decoder), "disable-dpb", TRUE, NULL);
    }




    g_object_set(G_OBJECT (dec->appsink), "max-buffers", 1, NULL);
    g_object_set(G_OBJECT (dec->appsink), "async", FALSE, NULL);
    // Create gstreamer loop
    dec->loop = g_main_loop_new(NULL, FALSE);

    // add a message handler
    dec->bus = gst_pipeline_get_bus(GST_PIPELINE(dec->pipeline));
    bus_watch_id = gst_bus_add_watch(dec->bus, bus_dec_call, dec->loop);
    gst_object_unref(dec->bus);

    // add all elements into pipeline
    gst_bin_add_many(GST_BIN(dec->pipeline), dec->appsrc, dec->parser,
                     dec->decoder, dec->appsink, NULL);

    // link elements into pipe: appsrc -> jpegenc -> appsink
    gst_element_link_many((GstElement*)dec->appsrc, dec->parser, dec->decoder,
                          dec->appsink, NULL);

    fprintf(stderr, "Setting g_main_loop_run to GST_STATE_PLAYING\n");
    // Start pipeline so it could process incoming data
    gst_element_set_state(dec->pipeline, GST_STATE_PLAYING);

    // Indicate that thread was started
    bDecThreadStarted = true;

    // Loop will run until receiving EOS (end-of-stream), will block here
    g_main_loop_run(dec->loop);

    fprintf(stderr, "g_main_loop_run returned, stopping playback\n");

    // Stop pipeline to be released
    gst_element_set_state(dec->pipeline, GST_STATE_NULL);

    fprintf(stderr, "Deleting pipeline\n");
    // THis will also delete all pipeline elements
    gst_object_unref(GST_OBJECT(dec->pipeline));

    g_source_remove (bus_watch_id);
    g_main_loop_unref (dec->loop);

    // Indicate that thread was finished
    bDecThreadFinished = true;

    return NULL;
}

// Starts GstreamerThread that remains in memory and compresses frames as being fed by user app.
bool StartEncGstreamer(gst_enc_t *enc)
{
    pthread_t GtkThreadId;
    pthread_attr_t GtkAttr;

    // Start thread
    int result = pthread_attr_init(&GtkAttr);
    if (result != 0)
    {
        fprintf(stderr, "pthread_attr_init returned error %d\n", result);
        return false;
    }

    void* pParam = enc;
    result = pthread_create(&GtkThreadId, &GtkAttr,
            GstreamerEncThread, pParam);
    if (result != 0)
    {
        fprintf(stderr, "pthread_create returned error %d\n", result);
        return false;
    }

    return true;
}

// Starts GstreamerThread that remains in memory and compresses frames as being fed by user app.
bool StartDecGstreamer(gst_dec_t *dec)
{
    pthread_t GtkThreadId;
    pthread_attr_t GtkAttr;

    // Start thread
    int result = pthread_attr_init(&GtkAttr);
    if (result != 0)
    {
        fprintf(stderr, "pthread_attr_init returned error %d\n", result);
        return false;
    }

    void* pParam = dec;
    result = pthread_create(&GtkThreadId, &GtkAttr,
            GstreamerDecThread, pParam);
    if (result != 0)
    {
        fprintf(stderr, "pthread_create returned error %d\n", result);
        return false;
    }

    return true;
}

// Puts raw data for encoding into gstreamer. Must put exactly width*height bytes.
GstFlowReturn PushEncBuffer(gst_enc_t *enc, char* data, int size)
{
    GstFlowReturn ret;
    GstBuffer *pushbuffer;

    char* filebuffer = (char*)malloc(size);

    memcpy(filebuffer, data, size);

    pushbuffer = gst_buffer_new_wrapped(filebuffer, size);

    //GstMapInfo info;
    //gst_buffer_map(buffer, &info, GST_MAP_WRITE);
    //unsigned char* buf = info.data;
    //memmove(buf, data, size);
    //gst_buffer_unmap(buffer, &info);

    GST_BUFFER_PTS (pushbuffer) = enc->timestamp;
    GST_BUFFER_DTS (pushbuffer) = enc->timestamp;
    GST_BUFFER_DURATION (pushbuffer) = gst_util_uint64_scale_int (1, GST_SECOND, enc->fps);
    enc->timestamp += GST_BUFFER_DURATION (pushbuffer);

    ret = gst_app_src_push_buffer(GST_APP_SRC(enc->appsrc), pushbuffer);
    return ret;
}

// Reads compressed jpeg frame. Will block if there is nothing to read out.
char* PullEncFrame(gst_enc_t *enc, int* outlen)
{
    // Will block until sample is ready. In our case "sample" is encoded picture.
    //GstSample* sample = gst_app_sink_try_pull_sample(GST_APP_SINK(enc->appsink), 1000000);
    GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(enc->appsink));

    if(sample == NULL)
    {
        //fprintf(stderr, "gst_app_sink_pull_sample returned null\n");
        return NULL;
    }

    // Actual compressed image is stored inside GstSample.
    GstBuffer* buffer = gst_sample_get_buffer (sample);
    GstMapInfo map;
    gst_buffer_map (buffer, &map, GST_MAP_READ);

    // Allocate appropriate buffer to store compressed image
    char* pRet = new char[map.size];
    // Copy image
    memcpy(pRet, map.data, map.size);
//    memmove(pRet, map.data, map.size);

    gst_buffer_unmap (buffer, &map);
    gst_sample_unref (sample);

    // Inform caller of image size
    *outlen = map.size;

    return pRet;
}

// Puts raw data for encoding into gstreamer. Must put exactly width*height bytes.
GstFlowReturn PushDecBuffer(gst_dec_t *dec, char* data, int size)
{
    GstFlowReturn ret;
    GstBuffer* pushbuffer;

    char* filebuffer = (char*)malloc(size);

    memcpy(filebuffer, data, size);

    pushbuffer = gst_buffer_new_wrapped(filebuffer, size);

    ret = gst_app_src_push_buffer(GST_APP_SRC(dec->appsrc), pushbuffer);
    return ret;
}

// Reads compressed jpeg frame. Will block if there is nothing to read out.
char* PullDecFrame(gst_dec_t *dec, int* outlen)
{
    // Will block until sample is ready. In our case "sample" is encoded picture.
    //GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(dec->appsink));
    GstSample* sample = gst_app_sink_try_pull_sample(GST_APP_SINK(dec->appsink), 1000000);

    if(sample == NULL)
    {
        //fprintf(stderr, "gst_app_sink_pull_sample returned null\n");
        return NULL;
    }

    // Actual compressed image is stored inside GstSample.
    GstBuffer* buffer = gst_sample_get_buffer (sample);
    GstMapInfo map;
    gst_buffer_map (buffer, &map, GST_MAP_READ);

    // Allocate appropriate buffer to store compressed image
    char* pRet = new char[map.size];
    // Copy image
    memcpy(pRet, map.data, map.size);
//    memmove(pRet, map.data, map.size);
        // Inform caller of image size
    *outlen = map.size;

    gst_buffer_unmap (buffer, &map);
    gst_sample_unref (sample);



    return pRet;
}

std::string vcodec::VideoCodecGstreamer::getVersion()
{
   return VIDEO_CODEC_GSTREAMER_VERSION;
}

vcodec::VideoCodecGstreamer::VideoCodecGstreamer() :
    m_isEncodeChangeParams(true),
    m_isDecodeChangeParams(true),
    m_bitRate(3),
    m_FPS(30),
    m_GOPsize(30),
    decoder(new gst_dec_t()),
    encoder(new gst_enc_t())
{
    ((gst_enc_t*)encoder)->bitrate = m_bitRate;
    ((gst_enc_t*)encoder)->fps = m_FPS;
    ((gst_enc_t*)encoder)->gop_size = m_GOPsize;

    /* Initialization */
    gst_init(NULL, NULL); // Will abort if GStreamer init error found
}

vcodec::VideoCodecGstreamer::~VideoCodecGstreamer()
{
    delete encoder;
    delete decoder;
}

bool vcodec::VideoCodecGstreamer::setProperty(std::string property_name, double propertyValue)
{
    if (property_name == "BIT_RATE")
    {
        m_bitRate = (int)propertyValue;
        ((gst_enc_t*)encoder)->bitrate = m_bitRate;
        if (bEncThreadStarted)
#if defined( VIDEO_CODEC_JETSON_NANO )
            g_object_set(G_OBJECT(((gst_enc_t*)encoder)->encoder), "bitrate", ((gst_enc_t*)encoder)->bitrate*1000, NULL);
#elif defined( VIDEO_CODEC_JETSON_NX )
#elif defined( VIDEO_CODEC_RASPBERRY_PI )
#elif defined( VIDEO_CODEC_IMX_8M )
            g_object_set(G_OBJECT(((gst_enc_t*)encoder)->encoder), "bitrate", ((gst_enc_t*)encoder)->bitrate, NULL);
#endif
        return true;
    }
    if (property_name == "FPS")
    {
        m_FPS = (int)propertyValue;
        ((gst_enc_t*)encoder)->fps = m_FPS;
        return true;
    }
    if (property_name == "GOP_SIZE")
    {
        m_GOPsize = (int)propertyValue;
        ((gst_enc_t*)encoder)->gop_size = m_GOPsize;
        return true;
    }

    return false;
}

bool vcodec::VideoCodecGstreamer::getProperty(std::string property_name, double& property_value)
{
    if (property_name == "BIT_RATE")
    {
        property_value = m_bitRate;
        return true;
    }
    if (property_name == "FPS")
    {
        property_value = m_FPS;
        return true;
    }
    if (property_name == "GOP_SIZE")
    {
        property_value = m_GOPsize;
        return true;
    }

    return false;
}

bool first_enc_frame = false;
void start_encode_gstream(gst_enc_t* enc, vsource::Frame& inFrame, vsource::Frame& outFrame)
{
    if (first_enc_frame)
    {
        PushEncBuffer(enc, (char*)inFrame.data, inFrame.size);
        first_enc_frame = false;
        outFrame.size = 0;
        return;
    }
    PushEncBuffer(enc, (char*)inFrame.data, inFrame.size);
    char* buf = PullEncFrame(enc, (int*)&outFrame.size);
    if (buf != NULL)
    {
        memcpy(outFrame.data, buf, outFrame.size);
        delete[] buf;
    }
    else
        outFrame.size = 0;
}

bool first_dec_frame = false;
void start_decode_gstream(gst_dec_t* dec, vsource::Frame& inFrame, vsource::Frame& outFrame)
{
    if (first_dec_frame)
    {
        PushDecBuffer(dec, (char*)inFrame.data, inFrame.size);
        first_dec_frame = false;
        return;
    }
    PushDecBuffer(dec, (char*)inFrame.data, inFrame.size);
    char* buf = PullDecFrame(dec, (int*)&outFrame.size);
    if (buf != NULL)
    {
        memcpy(outFrame.data, buf, outFrame.size);
        printf("%d\n", outFrame.size);
        //delete[] buf;
    }
    else
        outFrame.size = 0;
}

bool vcodec::VideoCodecGstreamer::transcode(vsource::Frame& input,
                                            vsource::Frame& output)
{
    if (m_inputFrame.height != input.height ||
            m_inputFrame.width != input.width ||
            m_inputFrame.fourcc != input.fourcc ||
            m_isDecodeChangeParams || m_isEncodeChangeParams)
    {
        m_inputFrame = input;

        if (m_inputFrame.fourcc != (uint32_t)vsource::ValidFourccCodes::H264 &&
                m_inputFrame.fourcc != (uint32_t)vsource::ValidFourccCodes::H265 &&
                m_inputFrame.fourcc != (uint32_t)vsource::ValidFourccCodes::JPEG)
        {
            m_isEncodeChangeParams = true;

            // Init encoder
            ((gst_enc_t*)encoder)->width = m_inputFrame.width;
            ((gst_enc_t*)encoder)->height = m_inputFrame.height;

            switch ((vsource::ValidFourccCodes)output.fourcc) {
            case vsource::ValidFourccCodes::H264:
            {
                ((gst_enc_t*)encoder)->encoder_type = 0;
                m_outputFrame = vsource::Frame(m_inputFrame.width, m_inputFrame.height,
                              (uint32_t)vsource::ValidFourccCodes::H264);
                break;
            }
            case vsource::ValidFourccCodes::H265:
            {
                ((gst_enc_t*)encoder)->encoder_type = 1;
                m_outputFrame = vsource::Frame(m_inputFrame.width, m_inputFrame.height,
                              (uint32_t)vsource::ValidFourccCodes::H265);
                break;
            }
            case vsource::ValidFourccCodes::JPEG:
            {
                ((gst_enc_t*)encoder)->encoder_type = 2;
                m_outputFrame = vsource::Frame(m_inputFrame.width, m_inputFrame.height,
                              (uint32_t)vsource::ValidFourccCodes::JPEG);
                break;
            }
            default:
                return false;
            }

            ((gst_enc_t*)encoder)->bitrate = m_bitRate;
            ((gst_enc_t*)encoder)->fps = m_FPS;
            ((gst_enc_t*)encoder)->gop_size = m_GOPsize;
            // Start conversion thread
            StartEncGstreamer((gst_enc_t*)encoder);

            // Ensure thread is running (or ran and stopped)
            while(bEncThreadStarted == false)
                usleep(10000); // Yield to allow thread to start
            if(bEncThreadFinished == true)
            {
                fprintf(stderr,
                        "Gstreamer thread could not start, terminating\n");
                return -1;
            }

            m_isEncodeChangeParams = false;
            m_isDecodeChangeParams = false;
        }else
        {
            m_isDecodeChangeParams = true;
            // Init Decoder
            if (m_inputFrame.fourcc == (uint32_t)vsource::ValidFourccCodes::H264 ||
                    m_inputFrame.fourcc == (uint32_t)vsource::ValidFourccCodes::H265 ||
                    m_inputFrame.fourcc == (uint32_t)vsource::ValidFourccCodes::JPEG)
            {
                ((gst_dec_t*)decoder)->width = m_inputFrame.width;
                ((gst_dec_t*)decoder)->height = m_inputFrame.height;
                switch ((vsource::ValidFourccCodes)output.fourcc) {
                case vsource::ValidFourccCodes::H264:
                {
                    ((gst_dec_t*)decoder)->decoder_type = 0;
                    break;
                }
                case vsource::ValidFourccCodes::H265:
                {
                    ((gst_dec_t*)decoder)->decoder_type = 1;
                    break;
                }
                case vsource::ValidFourccCodes::JPEG:
                {
                    ((gst_dec_t*)decoder)->decoder_type = 2;
                    break;
                }
                default:
                    return false;
                }
                m_outputFrame = vsource::Frame(m_inputFrame.width, m_inputFrame.height,
                              (uint32_t)vsource::ValidFourccCodes::NV12);

                ((gst_dec_t*)decoder)->bitrate = m_bitRate;
                // Start conversion thread
                StartDecGstreamer((gst_dec_t*)decoder);

                // Ensure thread is running (or ran and stopped)
                while(bDecThreadStarted == false)
                    usleep(10000); // Yield to allow thread to start
                if(bDecThreadFinished == true)
                {
                    fprintf(stderr,
                            "Gstreamer thread could not start, terminating\n");
                    return -1;
                }

                m_isDecodeChangeParams = false;
                m_isEncodeChangeParams = false;
            }
        }
    }else
    {
        // Copy new data
        m_inputFrame = input;
    }

    if (m_outputFrame.fourcc == (uint32_t)vsource::ValidFourccCodes::NV12)
    {
        if (!m_isDecodeChangeParams)
        {
            start_decode_gstream((gst_dec_t*)decoder, m_inputFrame, m_outputFrame);
            output = m_outputFrame;
            if (output.size > 0)
                return true;
        }
        return false;
    }
    else if (m_outputFrame.fourcc == (uint32_t)vsource::ValidFourccCodes::H264 ||
             m_outputFrame.fourcc == (uint32_t)vsource::ValidFourccCodes::H265 ||
             m_outputFrame.fourcc == (uint32_t)vsource::ValidFourccCodes::JPEG)
    {
        if (!m_isEncodeChangeParams)
        {
            start_encode_gstream((gst_enc_t*)encoder, m_inputFrame, m_outputFrame);
            output = m_outputFrame;
            if (output.size > 0)
                return true;
        }
        return false;
    }

    return false;
}

