#ifndef AUGMENTDIALOG_H
#define AUGMENTDIALOG_H

#include "ui/forms/forms.h"
#include "infra/DataManager.h"
#include "core/file/DatasetFile.h"
#include "core/action/Action.h"
#include <QDialog>

class AugmentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AugmentDialog( QWidget *parent = nullptr,DataManager *dataSrc = nullptr);
    ~AugmentDialog();

private slots:
    void openFileDialog();
    void on_closePushButton_clicked();
    void on_generatePushButton_clicked();
    void on_deletePushButton_clicked();
    void updateSelectionCount();

private:
    Ui::AugmentDialog *ui;
    DataManager* _dataSrc;
    void loadImageList(const QString &folder);

};

#endif // AUGMENTDIALOG_H
