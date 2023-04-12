#include "UserInterface/Forms/mainwindow.h"
#include <QApplication>
#include <QtGui>
#include <QtCore>
#include <QFontDatabase>
#include <QSplashScreen>
#include <QPixmap>
#include <QRect>
#include <QFont>
#include <QThread>
#include <QProcess>
#include "ApplicationSettings.h"
#include "EnterProc.h"
#include "Common/CommonUtils.h"
#include "Common/CommonWidgets.h"
#include "omp.h"

QString logFilePath;
bool logToFile = true;

void customMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QHash<QtMsgType, QString> msgLevelHash({{QtDebugMsg, "[D]"}, {QtInfoMsg, "[I]"}, {QtWarningMsg, "[W]"}, {QtCriticalMsg, "[C]"}, {QtFatalMsg, "[F]"}});
    QByteArray localMsg = msg.toLocal8Bit();
    QTime time = QTime::currentTime();
    QString formattedTime = time.toString("hh:mm:ss.zzz");
    QByteArray formattedTimeMsg = formattedTime.toLocal8Bit();
    QString logLevelName = msgLevelHash[type];
    QByteArray logLevelMsg = logLevelName.toLocal8Bit();

    if (logToFile)
    {
        QString contextFile = context.file;
        QString txt = contextFile.isEmpty() ?
                    QString("%1 %2 %3").arg(formattedTime, logLevelName, msg) :
                    QString("%1 %2 %3 (%4)").arg(formattedTime, logLevelName, msg, contextFile);
        QFile outFile(logFilePath);
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << txt << endl;
        outFile.close();
    }
    else
    {
        fprintf(stderr, "%s %s: %s (%s:%u, %s)\n", formattedTimeMsg.constData(), logLevelMsg.constData(), localMsg.constData(), context.file, context.line, context.function);
        fflush(stderr);
    }

    if (type == QtFatalMsg)
        abort();
}

QSplashScreen *makeSplashScreen()
{
    QPixmap logoPixmap(":/AnimusLogo.jpg");
    QRect desctopGeometry = CommonWidgetUtils::getDesktopAvailableGeometry();
    logoPixmap = logoPixmap.scaled(desctopGeometry.width(), desctopGeometry.height());

    auto splashScreen = new QSplashScreen(logoPixmap);

    QString message = QString("Please pay Kirill Volkov for the work\n%1")
            .arg(getAnimusExpiringTerm());

    QFont splashFont = splashScreen->font();
    splashFont.setPixelSize(36);
    splashScreen->setFont(splashFont);
    splashScreen->showMessage(message, Qt::AlignHCenter | Qt::AlignTop, Qt::red);
    splashScreen->show();

    return splashScreen;
}

int main(int argc, char *argv[])
{
    //qputenv("OPENCV_FFMPEG_CAPTURE_OPTIONS", "timeout;3000|rtsp_transport;udp");
    //qputenv("OPENCV_FFMPEG_CAPTURE_OPTIONS", "timeout;3000");
    //qputenv("OPENCV_FFMPEG_CAPTURE_OPTIONS", "rtsp_transport;udp");

    QApplication app(argc, argv);

    auto logDir = app.applicationDirPath() + "\\Logs";
    makeDir(logDir);
    logFilePath = QString("%1\\Animus_%2_%3.log").arg(logDir)
            .arg( QDate::currentDate().toString("yyyy_MM_dd") )
            .arg( QTime::currentTime().toString("hh_mm_ss") );

    logToFile = qgetenv("QTDIR").isEmpty(); //  check if the app is ran in Qt Creator
    qInstallMessageHandler(customMessageOutput); // custom message handler for debugging

    app.setApplicationVersion(APP_VERSION);

    app.addLibraryPath("./sqldrivers/");
    app.addLibraryPath("./mediaservice/");
    app.addLibraryPath("./imageformats/");
    app.addLibraryPath("./platforms/");
    app.addLibraryPath("./audio/");

    qInfo() << "Qt Version:" << QT_VERSION_STR;
    qInfo() << "Application Version:" << app.applicationVersion();
    qInfo() << "Application File Path:" << app.applicationFilePath();
    qInfo() << "Application Dir Path:" << app.applicationDirPath();
    qInfo() << "Application params (" << argc << "):";
    for (int i = 0; i < argc; i++)
        qInfo() << i << ") " << argv[i];
    qInfo() << "Library Paths:" << app.libraryPaths();
    qInfo() << "Sql Database Drivers:" << QSqlDatabase::drivers();
    qInfo() << "OMP Num Threads:" << omp_get_num_threads();
    qInfo() << "OMP Max Threads:" << omp_get_max_threads();
    qInfo() << "OMP Num Procs:" << omp_get_num_procs();

    QSplashScreen *splashScreen = nullptr;
    if (getAnimusLicenseState() != AnimusLicenseState::Licended)
        splashScreen = makeSplashScreen();
    app.processEvents();

    ApplicationSettings& applicationSettings = ApplicationSettings::Instance();

    EnterProc::setEnableComputingStatistics(applicationSettings.EnableComputingStatistics);
    //http://www.prog.org.ru/topic_20810_0.html
    //translate application strings and widgets

    QString appTranslatorPath, qtTranslatorPath;
    switch (applicationSettings.InterfaceLanguage)
    {
    case ApplicationLanguages::English:
        appTranslatorPath = ":/en.qm";
        qtTranslatorPath = "";
        break;
    case ApplicationLanguages::Russian:
        appTranslatorPath = ":/ru.qm";
        qtTranslatorPath = ":/qtbase_ru.qm";
        break;
    case ApplicationLanguages::Belarusian:
        appTranslatorPath = ":/by.qm";
        qtTranslatorPath = "";
        break;
    case ApplicationLanguages::Arabic:
        appTranslatorPath = ":/ar.qm";
        qtTranslatorPath = ":/qt_ar.qm";
        break;
    }

    QTranslator appTranslator;
    QTranslator qtTranslator;

    if (!appTranslatorPath.isEmpty())
    {
        appTranslator.load(appTranslatorPath);
        app.installTranslator(&appTranslator);
    }

    //translate system dialogs
    if (!qtTranslatorPath.isEmpty())
    {

        qtTranslator.load(qtTranslatorPath);
        app.installTranslator(&qtTranslator);
    }

    QFontDatabase::addApplicationFont(":/LCDMono2 Ultra.ttf");

    QFile stylesheetFile(":/stylesheet.qss");
    if (stylesheetFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        app.setStyleSheet(stylesheetFile.readAll());
        stylesheetFile.close();
    }

    quint32 logFolderMaxSizeMb = applicationSettings.LogFolderCleanup ?
                applicationSettings.LogFolderMaxSizeMb : 0;

#ifndef QT_DEBUG
    if (splashScreen != nullptr)
        QThread::sleep(3);
#endif

    MainWindow w;

    app.processEvents();
    if (splashScreen != nullptr)
        splashScreen->finish(&w);

    int appResult = app.exec();

    applicationSettings.sync();
    if (splashScreen != nullptr)
        delete splashScreen;
    EnterProc::outStatisticsToDebug(StatisticsSortMode::SortByProcName);

#ifndef QT_DEBUG
    if (appResult == ApplicationRestartExitCode)
    {
        QThread::msleep(500);
        auto program = qApp->arguments()[0];
        auto arguments = qApp->arguments();
        QProcess::startDetached(program, arguments);
    }
#endif


    if (logFolderMaxSizeMb > 0)
        cleanupLogFolder(logDir, logFilePath, logFolderMaxSizeMb);

    return appResult;
}
