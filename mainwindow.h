#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileIconProvider>
#include <QLabel>
#include <QEvent>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class ThumbnailFileIconProvider : public QFileIconProvider
{
public:
    QIcon icon(const QFileInfo &info) const override;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void createDesktopFile();

private:
    Ui::MainWindow *ui;

    QString openExecutablePicker();
    QString openIconPicker();
    void updateImagePreview(const QString &path, QLabel *previewLabel);
};

#endif // MAINWINDOW_H
