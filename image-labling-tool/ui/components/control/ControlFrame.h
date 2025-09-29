#ifndef CONTROLFRAME_H
#define CONTROLFRAME_H

#include "core/property/Property.h"
#include "core/property/PropertyUpdateHandler.h"
#include "ui/forms/forms.h"
#include <QFrame>
#include <QButtonGroup>
#include <functional>
#include <qwidget.h>
#include "ui/enum/InteractionMode.h"

class DataManager;

class ControlFrame : public QFrame
{
    Q_OBJECT

    friend class PropertyUpdateHandler<ControlFrame>;
public:
    explicit ControlFrame(QWidget *parent, DataManager *dataSrc);

    virtual ~ControlFrame();

    using WidgetConfigurator = std::function<void(QWidget*)>;

    void addCheckboxControl(QCheckBox* checkBox, WidgetConfigurator configure);
    void addInputControl(const QString& name, QWidget* widget, WidgetConfigurator configure);
    void addDiplayControl(QWidget* control, WidgetConfigurator configure);

    void addToolsButton(const QString& buttonName, std::function<void()> onClick);
    void addToolsControl(QWidget* control, WidgetConfigurator configure);

    struct BaseProperty {
        inline static QString interactionMode       = "interactionMode";
        inline static QString showCategoryLabel     = "showCategoryLabel";
        inline static QString scale                 = "scale";
        inline static QString fitInView             = "fitInView";
        inline static QString labelBorderWidth      = "labelBorderWidth";
        inline static QString categoryLabelTextSize = "categoryLabelTextSize";
    };

    void setAutoLabelingTotalImages(int count);
    void emitUpdateAutolabelingProgress(int increment);
    void emitCloseAutoLabelingDialog();

    bool isShowCategoryLabel();
    double labelBorderWidth();
    double categoryLabelTextSize();

public slots:
    void update(const QString& property, PropertyValue value);
    void update(const QVector<QPair<QString, PropertyValue>>& props);
    void showAutoLabelingDialog();

signals:
    void controlValueChanged(const QString& property, PropertyValue value);
    void autoLabeling(QString modelPath, double miniumScore);
    void updateAutolabelingProgress(int increment);
    void closeAutoLabelingDialog();

protected:
    virtual QHash<QString, PropertyUpdateHandler<ControlFrame>::UpdateHandler> initSetterMap();
    void setupConnections();

    Ui::ControlFrame *_ui;
    QButtonGroup *_interactionModeGroup;
    PropertyUpdateHandler<ControlFrame> _propertyUpdateHandler;

    int _autoLabelingTotalImages; // use for auto labeling dialog
private:
    DataManager *_dataSrc = nullptr;
};

#endif // CONTROLFRAME_H
