#include "MarkerTemplateEditor.h"
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QPixmap>
#include <QUuid>
#include <QDebug>
#include "EnterProc.h"

void MarkerTemplateEditor::fillControls()
{
    EnterProc("MarkerTemplateEditor::fillControls");

    if (_parentTemplate != nullptr)
    {
        _edtParentMarker->setText(_parentTemplate->description());
    }

    if (_markerTemplate != nullptr)
    {
        _edtDescription->setText(_markerTemplate->description());
        _lblImage->setPixmap(_markerTemplate->image());
        _txtComments->setPlainText(_markerTemplate->comments());
        _chkIsMilitary->setChecked(_markerTemplate->useParty());
    }
}

void MarkerTemplateEditor::fetchControls()
{
    _markerTemplate->setDescription(_edtDescription->text());
    _markerTemplate->setComments(_txtComments->toPlainText());
    _markerTemplate->setImage(*_lblImage->pixmap());
    _markerTemplate->setUseParty(_chkIsMilitary->isChecked());
}


qint32 pixmapHash(const QPixmap &pix)
{
    QImage image = pix.toImage();
    qint32 hash = 0;

    for(int y = 0; y < image.height(); y++)
    {
        for(int x = 0; x < image.width(); x++)
        {
            QRgb pixel = image.pixel(x,y);
            hash += pixel;
            hash += (hash << 10);
            hash ^= (hash >> 6);
        }
    }

    return hash;
}

bool MarkerTemplateEditor::isModified()
{
    bool isModified = (_markerTemplate == nullptr) ||
            (_markerTemplate->description() != _edtDescription->text()) ||
            (_markerTemplate->comments() != _txtComments->toPlainText()) ||
            (_markerTemplate->useParty() != _chkIsMilitary->isChecked()) ||
            (pixmapHash(_markerTemplate->image()) != pixmapHash(*_lblImage->pixmap()) );
    return isModified;
}

void MarkerTemplateEditor::initWidgets()
{
    EnterProc("MarkerTemplateEditor::initWidgets");

    //Dialog Form
    this->setWindowTitle(tr("Marker Template Editor"));
    //this->setWindowModality(Qt::WindowModal); //????
    this->setModal(true);
    CommonWidgetUtils::updateWidgetGeometry(this, 600, 300);
    this->setAttribute(Qt::WA_DeleteOnClose, true);


    auto lblParentMarker = new QLabel(tr("Group"), this);
    _edtParentMarker = new QLineEdit(this);
    _edtParentMarker->setReadOnly(true);

    auto lblDescription = new QLabel(tr("Description"), this);
    _edtDescription = new QLineEdit(this);


    _chkIsMilitary = new QCheckBox(tr("Military"), this);

    auto lblComments = new QLabel(tr("Commеnts"), this);
    _txtComments = new QPlainTextEdit(this);


    QPixmap pixmap(":/unknown_marker.png");
    pixmap = pixmap.scaled(DefaultMarkerSize, DefaultMarkerSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    _lblImage = new QLabelEx(this);
    //_lblImage->setObjectName("VideoImageTunerIcon"); //used for stylesheet
    _lblImage->setPixmap(pixmap);
    _lblImage->setCursor(Qt::PointingHandCursor);
    connect(_lblImage, &QLabelEx::clicked, this,  [=]()
    {
        auto fileName =  CommonWidgetUtils::showOpenFileDialog(tr("Select Marker Template Image"), "",
                                                               "PNG Images (*.png);;Bitmap Images (.bmp);;JPEG Images (.jpeg);;All images (*.*)");
        if (!fileName.isEmpty())
        {
            QPixmap pixmap(fileName);
            if (!pixmap.isNull())
            {
                pixmap = pixmap.scaled(DefaultMarkerSize, DefaultMarkerSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                _lblImage->setPixmap(pixmap);
            }
            else
                CommonWidgetUtils::showInfoDialogAutoclose(QMessageBox::Information,  tr("This image is not valid."), 3000);
        }
    });


    auto mainGrid = new QGridLayout();
    mainGrid->setColumnStretch(1, 1);
    mainGrid->setColumnStretch(2, 0);
    this->setLayout(mainGrid);
    int row = 0;

    mainGrid->addWidget(lblParentMarker,  row, 0, 1, 1);
    mainGrid->addWidget(_edtParentMarker,  row, 1, 1, 1);
    row++;


    mainGrid->addWidget(lblDescription,   row, 0, 1, 1);
    mainGrid->addWidget(_edtDescription,  row, 1, 1, 1);
    row++;

    mainGrid->addWidget(_lblImage,        row, 1, 1, 1);
    row++;

    mainGrid->addWidget(_chkIsMilitary,   row, 1, 1, 1, Qt::AlignLeft);
    row++;

    mainGrid->addWidget(lblComments,      row, 0, 1, 1, Qt::AlignTop);
    mainGrid->addWidget(_txtComments,     row, 1, 1, 1);
    row++;

    mainGrid->setRowStretch(row, 1);
    row++;

    auto buttonBox = CommonWidgetUtils::makeDialogButtonBox(this, QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    mainGrid->addWidget(buttonBox,   row, 0, 1, 2);
    mainGrid->setRowStretch(row, 0);
    row++;
}

MarkerTemplateEditor::MarkerTemplateEditor(QWidget *parent, const QString &markerGUID, const QString &parentGUID) : QDialog(parent)
{
    EnterProc("MarkerTemplateEditor::MarkerTemplateEditor");

    MarkerThesaurus& markerThesaurus = MarkerThesaurus::Instance();
    _markerTemplate = markerGUID != "" ? markerThesaurus.getMarkerTemplateByGUID(markerGUID) : nullptr ;
    _parentTemplate = markerGUID != "" ? markerThesaurus.getMarkerTemplateByGUID(parentGUID) : nullptr ;

    initWidgets();
    fillControls();
}

MarkerTemplateEditor::~MarkerTemplateEditor()
{
    qDebug() << "Close MarkerTemplateEditor";
}

void MarkerTemplateEditor::accept()
{
    if (isModified())
    {
        MarkerThesaurus& markerThesaurus = MarkerThesaurus::Instance();

        if (_markerTemplate == nullptr)
        {
            QString parentGUID =  _parentTemplate != nullptr ? _parentTemplate->GUID() : "";
            _markerTemplate = markerThesaurus.createNewMarkerTemplate(parentGUID);
        }

        fetchControls();

        markerThesaurus.saveMarkerTemplate(_markerTemplate);

        done(QDialog::Accepted);
    }
    else
        done(QDialog::Rejected);
}
