win32 {
    DEFINES += ANIMUS_CONNECTION_TYPE=AutoConnection
    DEFINES += "_GLIBCXX_USE_CXX11_ABI=0"

    OPENCV_BIN = E:/Projects/Animus/src/OpenCV/bin

    INCLUDEPATH += E:/Projects/Animus/src/OpenCV/include 
    LIBS += $$OPENCV_BIN/libopencv_core345.dll \
            $$OPENCV_BIN/libopencv_highgui345.dll \
            $$OPENCV_BIN/libopencv_imgcodecs345.dll \
            $$OPENCV_BIN/libopencv_imgproc345.dll \
            $$OPENCV_BIN/libopencv_features2d345.dll \
            $$OPENCV_BIN/libopencv_calib3d345.dll \
            $$OPENCV_BIN/libopencv_video345.dll \
            $$OPENCV_BIN/libopencv_videoio345.dll \
            $$OPENCV_BIN/opencv_ffmpeg345.dll

    INCLUDEPATH += SDL2/include
    LIBS += e:/Projects/Animus/src/SDL2/bin/SDL2.dll


    INCLUDEPATH += Riftek
    LIBS += e:/Projects/Animus/src/ImageProcessor/RTVideoTracker/RfVideoTracker_3_1.dll
}
unix {
    DEFINES += ANIMUS_CONNECTION_TYPE=QueuedConnection

    LIBS =-ldl

    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib/
    LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_features2d -lopencv_video -lopencv_videoio -lopencv_imgcodecs -lopencv_calib3d
    LIBS += -lSDL2
}
