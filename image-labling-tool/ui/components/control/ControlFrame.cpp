#include "ControlFrame.h"
#include "common/log/Logger.h"
#include "ui/dialog/autolabeling/AutoLabelingDialog.h"
#include "ui/dialog/augment/augmentdialog.h"
#include <qdialog.h>
#include <qnamespace.h>
#include <qtdeprecationdefinitions.h>

ControlFrame::ControlFrame(QWidget *parent, DataManager *dataSrc)
    : QFrame(parent)
    , _ui(new Ui::ControlFrame)
    , _dataSrc(dataSrc)
    , _propertyUpdateHandler(this)
{
    _ui->setupUi(this);

    _ui->drawModeRadioButton->setChecked(true);
    _ui->editModeRadioButton->setChecked(false);

    _interactionModeGroup = new QButtonGroup(this);
    _interactionModeGroup->addButton(_ui->drawModeRadioButton, static_cast<int>(InteractionMode::DrawMode));
    _interactionModeGroup->addButton(_ui->editModeRadioButton, static_cast<int>(InteractionMode::EditMode));

    setupConnections();
}

ControlFrame::~ControlFrame() {
    delete _ui;
}

void ControlFrame::setupConnections() {
    connect(_interactionModeGroup, &QButtonGroup::buttonClicked, this, [=](QAbstractButton *button) {
        emit controlValueChanged(BaseProperty::interactionMode, PropertyValue::from(static_cast<InteractionMode>(_interactionModeGroup->id(button))));
    });

    connect(_ui->showCategoryLabelCheckBox, &QCheckBox::checkStateChanged, this, [=](Qt::CheckState state) {
        emit controlValueChanged(BaseProperty::showCategoryLabel, PropertyValue::from(state == Qt::Checked ? true : false));
    });

    connect(_ui->scaleSpinBox, &QDoubleSpinBox::valueChanged, this, [=](double value) {
        emit controlValueChanged(BaseProperty::scale, PropertyValue::from(value));
    });

    connect(_ui->fitInViewPushButton, &QCheckBox::clicked, this, [=]() {
        emit controlValueChanged(BaseProperty::fitInView, PropertyValue::Empty);
    });

    connect(_ui->borderWidthInput, &QDoubleSpinBox::valueChanged, this, [=](double value) {
        emit controlValueChanged(BaseProperty::labelBorderWidth, PropertyValue::from(value));
    });

    connect(_ui->categoryLabelTextSize, &QDoubleSpinBox::valueChanged, this, [=](double value){
        emit controlValueChanged(BaseProperty::categoryLabelTextSize, PropertyValue::from(value));
    });

    connect(_ui->autoLabelingPushButton, &QPushButton::clicked, this, &ControlFrame::showAutoLabelingDialog);

    connect(_ui->augmentPushButton, &QPushButton::clicked, this, [=]() {
        if(!_dataSrc) {
            qWarning() << "DataSource is null, cannot open AugmentDialog!";
            return;
        }
        AugmentDialog* augmentDialog = new AugmentDialog(this, _dataSrc);
        augmentDialog->exec();
    });

}

void ControlFrame::update(const QString& property, PropertyValue value) {
    try {
        _propertyUpdateHandler.handleUpdate(property, value);
    } catch(const std::exception& ex) {
        Logger::logError(Q_FUNC_INFO, ex.what());
    }
}

void ControlFrame::update(const QVector<QPair<QString, PropertyValue>>& props) {
    try {
        _propertyUpdateHandler.handleBatchUpdate(props);
    } catch(const std::exception& ex) {
        Logger::logError(Q_FUNC_INFO, ex.what());
    }    
}

QHash<QString, PropertyUpdateHandler<ControlFrame>::UpdateHandler> ControlFrame::initSetterMap() {
    return {
        { BaseProperty::interactionMode, [](ControlFrame* self, PropertyValue value) {
            auto mode = value.to<InteractionMode>();
            QAbstractButton* btn = self->_interactionModeGroup->button(static_cast<int>(mode));
            if (btn && !btn->isChecked()) {
                btn->setChecked(true);
            }
        }},

        { BaseProperty::showCategoryLabel, [](ControlFrame* self, PropertyValue value) {
            bool checked = value.to<bool>();
            auto* cb = self->_ui->showCategoryLabelCheckBox;
            if (cb->isChecked() != checked) {
                cb->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
            }
        }},

        { BaseProperty::scale, [](ControlFrame* self, PropertyValue value) {
            double val = value.to<double>();
            auto* spin = self->_ui->scaleSpinBox;
            if (spin->value() != val) {
                spin->setValue(val);
            }
        }},

        { BaseProperty::labelBorderWidth, [](ControlFrame* self, PropertyValue value) {
            double val = value.to<double>();
            auto* input = self->_ui->borderWidthInput;
            if (input->value() != val) {
                input->setValue(val);
            }
        }},
        
        { BaseProperty::categoryLabelTextSize, [](ControlFrame* self, PropertyValue value) {
            double val = value.to<double>();
            auto* spin = self->_ui->categoryLabelTextSize;
            if (spin->value() != val) {
                spin->setValue(val);
            }
        }}
    };
}

void ControlFrame::addCheckboxControl(QCheckBox* checkBox, WidgetConfigurator configure) {
    _ui->checkBoxControlsContainer->layout()->addWidget(checkBox);
    configure(checkBox);
}

void ControlFrame::addInputControl(const QString& name, QWidget* widget, WidgetConfigurator configure) {
    auto layout = dynamic_cast<QFormLayout*>(_ui->inputControlsContainer->layout());
    layout->addRow(name, widget);
    configure(widget);
}

void ControlFrame::addDiplayControl(QWidget* control, WidgetConfigurator configure) {
    auto layout = _ui->displayControlTab->layout();

    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    
    layout->addWidget(line);
    layout->addWidget(control);

    configure(control);
}

void ControlFrame::addToolsButton(const QString& buttonName, std::function<void()> onClick) {
    auto layout = _ui->toolControlsContainer->layout();
    QPushButton *button = new QPushButton(buttonName);
    connect(button, &QPushButton::clicked, this, [=]() {onClick();});

    layout->addWidget(button);
}

void ControlFrame::addToolsControl(QWidget* control, WidgetConfigurator configure) {
    auto layout = _ui->toolsTab->layout();

    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    
    layout->addWidget(line);
    layout->addWidget(control);

    configure(control);
}

void ControlFrame::showAutoLabelingDialog() {
    AutoLabelingDialog *dialog = new AutoLabelingDialog(_autoLabelingTotalImages, this);

    connect(dialog, &QDialog::finished, this, [=](int result) {
        if(result == QDialog::Accepted) {
            Logger::logInfo(Q_FUNC_INFO, QString("auto labeling with model: %1 and minium score: %2").arg(dialog->modelPath()).arg(dialog->miniumScore()));
            connect(this, &ControlFrame::updateAutolabelingProgress, dialog, &AutoLabelingDialog::updateProgress, Qt::UniqueConnection);
            emit autoLabeling(dialog->modelPath(), dialog->miniumScore());
        }
    }, Qt::SingleShotConnection);

    connect(this, &ControlFrame::closeAutoLabelingDialog, dialog, [=]() {
        dialog->close();
    }, Qt::SingleShotConnection);

    dialog->exec();
}

void ControlFrame::setAutoLabelingTotalImages(int count) {
    _autoLabelingTotalImages = count;
}

void ControlFrame::emitUpdateAutolabelingProgress(int increment) {
    emit updateAutolabelingProgress(increment);
}

void ControlFrame::emitCloseAutoLabelingDialog() {
    emit closeAutoLabelingDialog();
}

bool ControlFrame::isShowCategoryLabel() {
    return _ui->showCategoryLabelCheckBox->isChecked();
}

double ControlFrame::labelBorderWidth() {
    return _ui->borderWidthInput->value();
}

double ControlFrame::categoryLabelTextSize() {
    return _ui->categoryLabelTextSize->value();
}


