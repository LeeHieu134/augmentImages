#ifndef AUGMENTDIALOG_H
#define AUGMENTDIALOG_H
#include "../../base/datasource.h"
#include <QDialog>

namespace Ui {
class AugmentDialog;
}

class AugmentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AugmentDialog( QWidget *parent = nullptr,DataSource *dataSrc = nullptr);
    ~AugmentDialog();

private slots:
    void openFileDialog();
    void on_closePushButton_clicked();
    void on_generatePushButton_clicked();
    void on_deletePushButton_clicked();
    void updateSelectionCount();
    void applyFilter();

private:
    Ui::AugmentDialog *ui;
    DataSource* _dataSrc;
    void loadImageList(const QString &folder);
    QFileInfoList _allFiles;
    void addRowFromFile(const QFileInfo &imgFile, int row);

};

#endif // AUGMENTDIALOG_H
