#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialog>
#include <QListView>
#include <QTreeView>
#include <QImageReader>
#include <QPixmap>
#include <QSizePolicy>
#include <QFrame>
#include <QUrl>
#include <QGridLayout>

QIcon ThumbnailFileIconProvider::icon(const QFileInfo &info) const
{
    if (info.isFile()) {
        const QString suffix = info.suffix().toLower();

        if (suffix == "png" || suffix == "jpg" || suffix == "jpeg" ||
            suffix == "bmp" || suffix == "gif" || suffix == "svg" ||
            suffix == "xpm" || suffix == "ico" || suffix == "webp") {

            QPixmap pix(info.absoluteFilePath());

            if (!pix.isNull()) {
                QPixmap thumb = pix.scaled(
                    64, 64,
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation
                    );
                return QIcon(thumb);
            }
        }
    }

    return QFileIconProvider::icon(info);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Executable picker
    connect(ui->execBrowseButton, &QPushButton::clicked, this, [this]() {
        const QString fileName = openExecutablePicker();
        if (!fileName.isEmpty()) {
            ui->execField->setText(fileName);
        }
    });

    // Icon picker with preview + thumbnails
    connect(ui->iconBrowseButton, &QPushButton::clicked, this, [this]() {
        const QString fileName = openIconPicker();
        if (!fileName.isEmpty()) {
            ui->iconField->setText(fileName);
        }
    });

    // Custom path picker
    connect(ui->customPathBrowseButton, &QPushButton::clicked, this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Select Custom Directory");
        if (!dir.isEmpty()) {
            ui->customPathField->setText(dir);
        }
    });

    connect(ui->createDesktopFileButton, &QPushButton::clicked,
            this, &MainWindow::createDesktopFile);

    QString styleSheet = R"(
    QCheckBox::indicator {
        border: 1px solid white;
        width: 12px;
        height: 12px;
        background-color: transparent;
    }

    QCheckBox::indicator:checked {
        border: 1px solid white;
        image: url(:/images/checkmark.png);
    }

    QRadioButton::indicator {
        border: 1px solid white;
        width: 10px;
        height: 10px;
        border-radius: 5px;
        background-color: transparent;
    }

    QRadioButton::indicator:checked {
        border: 1px solid white;
        background-color: white;
    }

    QLineEdit {
        color: white;
    }

    QLineEdit::placeholder {
        color: gray;
    }
    )";

    this->setStyleSheet(styleSheet);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::openExecutablePicker()
{
    QFileDialog dialog(this, "Select Executable");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setViewMode(QFileDialog::Detail);

    dialog.setNameFilters(QStringList()
                          << "All Files (*)"
                          << "Executable Files (*.AppImage *.run *.bin *.sh)"
                          << "Desktop Files (*.desktop)");

    // Try to make it feel more like Dolphin
    if (auto *listView = dialog.findChild<QListView*>("listView")) {
        listView->setViewMode(QListView::ListMode);
        listView->setIconSize(QSize(32, 32));
    }

    if (auto *treeView = dialog.findChild<QTreeView*>()) {
        treeView->setIconSize(QSize(24, 24));
        treeView->setSortingEnabled(true);
        treeView->sortByColumn(0, Qt::AscendingOrder);
    }

    if (dialog.exec() == QDialog::Accepted) {
        const QStringList files = dialog.selectedFiles();
        if (!files.isEmpty())
            return files.first();
    }

    return QString();
}

QString MainWindow::openIconPicker()
{
    QFileDialog dialog(this, "Select Icon");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.resize(1000, 500);
    dialog.setMinimumSize(1000, 400);

    dialog.setNameFilters(QStringList()
                          << "Icon Files (*.png *.svg *.ico *.xpm *.jpg *.jpeg *.bmp *.gif *.webp)"
                          << "PNG Files (*.png)"
                          << "SVG Files (*.svg)"
                          << "ICO Files (*.ico)"
                          << "All Files (*)");

    dialog.setDirectory(QDir::homePath());

    auto *provider = new ThumbnailFileIconProvider();
    dialog.setIconProvider(provider);

    if (auto *fileNameEdit = dialog.findChild<QLineEdit*>("fileNameEdit")) {
        fileNameEdit->setMinimumWidth(fileNameEdit->minimumWidth() + 150);
    }

    if (auto *buttonBox = dialog.findChild<QDialogButtonBox*>()) {
        buttonBox->setMinimumWidth(buttonBox->minimumWidth() + 150);
    }

    auto *previewPanel = new QFrame(&dialog);
    previewPanel->setMinimumWidth(200);
    //previewPanel->setMaximumWidth(420);
    previewPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    if (auto *sidebar = dialog.findChild<QListView*>("sidebar")) {
        sidebar->setMinimumWidth(125);
        sidebar->setMaximumWidth(260);
        sidebar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    }

    if (auto *listView = dialog.findChild<QListView*>("listView")) {
        listView->setViewMode(QListView::IconMode);
        listView->setIconSize(QSize(64, 64));
        listView->setGridSize(QSize(90, 90));
        listView->setResizeMode(QListView::Adjust);
        listView->setMinimumWidth(550);

    }

    if (auto *treeView = dialog.findChild<QTreeView*>()) {
        treeView->setIconSize(QSize(24, 24));
        treeView->setSortingEnabled(true);
        treeView->sortByColumn(0, Qt::AscendingOrder);
        treeView->setMinimumWidth(550);
    }

    auto *previewLayout = new QVBoxLayout(previewPanel);

    previewLayout->setContentsMargins(8, 8, 8, 8);
    previewLayout->setSpacing(8);

    auto *previewTitle = new QLabel("Preview", previewPanel);
    previewTitle->setAlignment(Qt::AlignCenter);
    previewTitle->setStyleSheet("font-weight: bold; font-size: 16px;");

    auto *previewLabel = new QLabel(previewPanel);
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setMinimumSize(220, 220);
    previewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    previewPanel->setFrameShape(QFrame::StyledPanel);
    previewLabel->setStyleSheet(
        "QLabel {"
        "  border: 1px solid gray;"
        "  background-color: rgba(255,255,255,20);"
        "}"
        );
    previewLabel->setText("No image selected");
    previewLabel->installEventFilter(this);

    auto *pathLabel = new QLabel(previewPanel);
    pathLabel->setWordWrap(true);
    pathLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    pathLabel->setStyleSheet("color: lightgray;");

    auto *scaleLabel = new QLabel(previewPanel);
    scaleLabel->setObjectName("scaleLabel");
    scaleLabel->setAlignment(Qt::AlignCenter);
    scaleLabel->setStyleSheet("color: lightgray;");
    scaleLabel->setText(" ");

    previewLayout->addWidget(previewTitle);
    previewLayout->addWidget(previewLabel, 1);
    previewLayout->addWidget(pathLabel);
    previewLayout->addWidget(scaleLabel);

    QGridLayout *grid = qobject_cast<QGridLayout*>(dialog.layout());
    if (grid) {
        grid->addWidget(previewPanel, 0, 3, grid->rowCount(), 1);

        // Enforce usable file area width
        grid->setColumnMinimumWidth(2, 550);

        // Preview stays on the right and cannot take over
        grid->setColumnMinimumWidth(3, 600);

        grid->setColumnStretch(0, 0);
        grid->setColumnStretch(1, 0);
        grid->setColumnStretch(2, 1);
        grid->setColumnStretch(3, 3);
    }

    QObject::connect(&dialog, &QFileDialog::currentChanged,
                     this, [this, previewLabel, pathLabel](const QString &path) {
                         pathLabel->setText(path);
                         updateImagePreview(path, previewLabel);
                     });

    if (dialog.exec() == QDialog::Accepted) {
        const QStringList files = dialog.selectedFiles();
        if (!files.isEmpty())
            return files.first();
    }

    return QString();
}

void MainWindow::updateImagePreview(const QString &path, QLabel *previewLabel)
{
    QFileInfo info(path);

    QLabel *scaleLabel = previewLabel->parent()->findChild<QLabel*>("scaleLabel");

    if (!info.exists() || !info.isFile()) {
        previewLabel->clear();
        previewLabel->setText("No image selected");
        previewLabel->setProperty("imagePath", QString());
        previewLabel->setProperty("fullPixmap", QPixmap());
        if (scaleLabel) scaleLabel->setText(" ");
        return;
    }

    QPixmap pix(path);
    if (pix.isNull()) {
        previewLabel->clear();
        previewLabel->setText("Preview not available");
        previewLabel->setProperty("imagePath", QString());
        previewLabel->setProperty("fullPixmap", QPixmap());
        if (scaleLabel) scaleLabel->setText(" ");
        return;
    }

    previewLabel->setText(QString());
    previewLabel->setProperty("imagePath", path);
    previewLabel->setProperty("fullPixmap", pix);

    QSize available = previewLabel->size() - QSize(12, 12);
    if (available.width() < 1 || available.height() < 1) {
        available = QSize(1, 1);
    }

    const QSize originalSize = pix.size();

    const double sx = double(available.width()) / double(originalSize.width());
    const double sy = double(available.height()) / double(originalSize.height());
    const double scale = qMin(1.0, qMin(sx, sy));

    const QSize targetSize(qMax(1, int(originalSize.width() * scale)),
                           qMax(1, int(originalSize.height() * scale)));

    QPixmap scaled = pix.scaled(targetSize,
                                Qt::KeepAspectRatio,
                                Qt::SmoothTransformation);

    previewLabel->setPixmap(scaled);

    if (scaleLabel) {
        scaleLabel->setText(QString("%1% (%2 × %3)")
                                .arg(int(scale * 100.0 + 0.5))
                                .arg(originalSize.width())
                                .arg(originalSize.height()));
    }
}

void MainWindow::createDesktopFile()
{
    QString name = ui->nameField->text();
    QString exec = ui->execField->text();
    QString icon = ui->iconField->text();
    QString comment = ui->commentField->text();
    bool terminal = ui->terminalCheckBox->isChecked();

    if (name.isEmpty() || exec.isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Please fill in both the Name and Exec fields.");
        return;
    }

    QString safeName = name;
    safeName.replace(" ", "_");

    QString tempFilePath = QDir::tempPath() + "/" + safeName + ".desktop";
    QFile tempFile(tempFilePath);
    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Failed to write temporary file.");
        return;
    }

    QTextStream out(&tempFile);
    out << "[Desktop Entry]\n";
    out << "Version=1.0\n";
    out << "Name=" << name << "\n";
    out << "Exec=" << exec << "\n";
    out << "Icon=" << icon << "\n";
    out << "Type=Application\n";

    if (!comment.isEmpty()) {
        out << "Comment=" << comment << "\n";
    }

    out << "Terminal=" << (terminal ? "true" : "false") << "\n";

    QStringList selectedCategories;
    if (ui->categoryDevelopment->isChecked()) selectedCategories << "Development";
    if (ui->categoryGames->isChecked()) selectedCategories << "Games";
    if (ui->categoryGraphics->isChecked()) selectedCategories << "Graphics";
    if (ui->categoryInternet->isChecked()) selectedCategories << "Internet";
    if (ui->categoryOffice->isChecked()) selectedCategories << "Office";
    if (ui->categoryMultimedia->isChecked()) selectedCategories << "Multimedia";
    if (ui->categorySettings->isChecked()) selectedCategories << "Settings";
    if (ui->categorySystem->isChecked()) selectedCategories << "System";
    if (ui->categoryUtility->isChecked()) selectedCategories << "Utility";

    if (!selectedCategories.isEmpty()) {
        out << "Categories=" << selectedCategories.join(";") << ";\n";
    }

    tempFile.close();

    QString destinationPath;

    if (ui->systemRadioButton->isChecked()) {
        destinationPath = "/usr/share/applications/" + safeName + ".desktop";
    } else if (ui->userRadioButton->isChecked()) {
        QString localPath = QDir::homePath() + "/.local/share/applications/";
        if (!QDir(localPath).exists()) {
            QDir().mkpath(localPath);
        }

        destinationPath = localPath + safeName + ".desktop";
        QFile::remove(destinationPath);

        if (!QFile::copy(tempFilePath, destinationPath)) {
            QMessageBox::critical(this, "Error", "Failed to copy the desktop file to the user-only location.");
            return;
        }

        QFile::remove(tempFilePath);
        QMessageBox::information(this, "Success", "Desktop file created successfully in user-only location!");
        return;
    } else if (ui->customRadioButton->isChecked()) {
        QString customPath = ui->customPathField->text();

        if (customPath.isEmpty() || !QDir(customPath).exists()) {
            QMessageBox::warning(this, "Invalid Custom Path", "Please select a valid custom path.");
            return;
        }

        destinationPath = customPath + "/" + safeName + ".desktop";
        QFile::remove(destinationPath);

        if (!QFile::copy(tempFilePath, destinationPath)) {
            QMessageBox::critical(this, "Error", "Failed to copy the desktop file to the custom location.");
            return;
        }

        QFile::remove(tempFilePath);
        QMessageBox::information(this, "Success", "Desktop file created successfully in custom location!");
        return;
    } else {
        QMessageBox::warning(this, "No Location Selected", "Please select an installation location.");
        return;
    }

    QString scriptFilePath = QDir::tempPath() + "/move_desktop_file.sh";
    if (QFile::exists(scriptFilePath)) {
        QFile::remove(scriptFilePath);
    }

    QFile scriptFile(scriptFilePath);
    if (!scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Failed to write privilege escalation script.");
        return;
    }

    QTextStream scriptOut(&scriptFile);
    scriptOut << "#!/bin/bash\n";
    scriptOut << "cp \"" << tempFilePath << "\" \"" << destinationPath << "\"\n";
    scriptOut << "rm \"" << tempFilePath << "\"\n";
    scriptFile.close();

    QFile::setPermissions(
        scriptFilePath,
        QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther |
            QFile::ReadOwner | QFile::ReadGroup | QFile::ReadOther |
            QFile::WriteOwner
        );

    QProcess process;
    process.start("pkexec", QStringList() << "/bin/bash" << scriptFilePath);

    if (!process.waitForFinished()) {
        QMessageBox::critical(this, "Error", "Failed to execute privilege escalation command.");
        return;
    }

    if (process.exitCode() != 0) {
        QMessageBox::critical(this, "Error", "Failed to move the file. Check permissions.");
        return;
    }

    QFile::remove(scriptFilePath);

    QMessageBox::information(this, "Success", "Desktop file created successfully (system-wide).");
}
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        QLabel *label = qobject_cast<QLabel*>(obj);
        if (label) {
            QVariant pixVar = label->property("fullPixmap");
            if (pixVar.isValid()) {
                QPixmap pix = pixVar.value<QPixmap>();
                if (!pix.isNull()) {
                    QSize available = label->size() - QSize(12, 12);
                    if (available.width() < 1 || available.height() < 1) {
                        available = QSize(1, 1);
                    }

                    const QSize originalSize = pix.size();

                    const double sx = double(available.width()) / double(originalSize.width());
                    const double sy = double(available.height()) / double(originalSize.height());
                    const double scale = qMin(1.0, qMin(sx, sy));   // cap at 100%

                    const QSize targetSize(qMax(1, int(originalSize.width() * scale)),
                                           qMax(1, int(originalSize.height() * scale)));

                    QPixmap scaled = pix.scaled(targetSize,
                                                Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation);

                    label->setPixmap(scaled);
                }
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}
