#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QDir>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Connect browse buttons for executable and icon
    connect(ui->execBrowseButton, &QPushButton::clicked, this, [=]() {
        QString fileName = QFileDialog::getOpenFileName(this, "Select Executable");
        if (!fileName.isEmpty()) {
            ui->execField->setText(fileName);
        }
    });

    connect(ui->iconBrowseButton, &QPushButton::clicked, this, [=]() {
        QString fileName = QFileDialog::getOpenFileName(this, "Select Icon");
        if (!fileName.isEmpty()) {
            ui->iconField->setText(fileName);
        }
    });

    // Connect browse button for custom path
    connect(ui->customPathBrowseButton, &QPushButton::clicked, this, [=]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Select Custom Directory");
        if (!dir.isEmpty()) {
            ui->customPathField->setText(dir);
        }
    });

    // Connect the "Create Desktop File" button to the slot
    connect(ui->createDesktopFileButton, &QPushButton::clicked, this, &MainWindow::createDesktopFile);

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
        color: white; /* Regular text color */
    }

    QLineEdit::placeholder {
        color: gray; /* Placeholder text color */
}

)";
    this->setStyleSheet(styleSheet);

}

MainWindow::~MainWindow()
{
    delete ui;
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

    // Step 1: Write the desktop file to a temporary location
    QString tempFilePath = QDir::tempPath() + "/" + name.replace(" ", "_") + ".desktop";
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

    // Step 2: Determine the destination path
    QString destinationPath;
    if (ui->systemRadioButton->isChecked()) {
        // System-wide installation
        destinationPath = "/usr/share/applications/" + name.replace(" ", "_") + ".desktop";
    } else if (ui->userRadioButton->isChecked()) {
        // User-only installation
        QString localPath = QDir::homePath() + "/.local/share/applications/";
        if (!QDir(localPath).exists()) {
            QDir().mkpath(localPath); // Create the directory if it doesn't exist
        }
        destinationPath = localPath + name.replace(" ", "_") + ".desktop";
        QFile::remove(destinationPath); // Remove existing file if any
        if (!QFile::copy(tempFilePath, destinationPath)) {
            QMessageBox::critical(this, "Error", "Failed to copy the desktop file to the user-only location.");
            return;
        }
        QFile::remove(tempFilePath); // Clean up temporary file
        QMessageBox::information(this, "Success", "Desktop file created successfully in user-only location!");
        return;
    } else if (ui->customRadioButton->isChecked()) {
        // Custom installation
        QString customPath = ui->customPathField->text();
        if (customPath.isEmpty() || !QDir(customPath).exists()) {
            QMessageBox::warning(this, "Invalid Custom Path", "Please select a valid custom path.");
            return;
        }
        destinationPath = customPath + "/" + name.replace(" ", "_") + ".desktop";
        QFile::remove(destinationPath); // Remove existing file if any
        if (!QFile::copy(tempFilePath, destinationPath)) {
            QMessageBox::critical(this, "Error", "Failed to copy the desktop file to the custom location.");
            return;
        }
        QFile::remove(tempFilePath); // Clean up temporary file
        QMessageBox::information(this, "Success", "Desktop file created successfully in custom location!");
        return;
    } else {
        QMessageBox::warning(this, "No Location Selected", "Please select an installation location.");
        return;
    }

    // Step 3: Create the script for privilege escalation
    QString scriptFilePath = QDir::tempPath() + "/move_desktop_file.sh";
    if (QFile::exists(scriptFilePath)) {
        QFile::remove(scriptFilePath); // Ensure no leftover script exists
    }

    QFile scriptFile(scriptFilePath);
    if (!scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Failed to write privilege escalation script.");
        return;
    }

    QTextStream scriptOut(&scriptFile);
    scriptOut << "#!/bin/bash\n";
    scriptOut << "cp " << tempFilePath << " " << destinationPath << "\n";
    scriptOut << "rm " << tempFilePath << "\n";
    scriptFile.close();

    // Ensure the script is executable
    QFile::setPermissions(scriptFilePath, QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther | QFile::ReadOwner | QFile::ReadGroup | QFile::ReadOther);

    // Step 4: Execute the script with pkexec
    QProcess process;
    process.start("pkexec", QStringList() << "/bin/bash" << scriptFilePath);

    if (!process.waitForFinished()) {
        QMessageBox::critical(this, "Error", "Failed to execute privilege escalation command.");
        return;
    }

    // Check for errors
    if (process.exitCode() != 0) {
        QMessageBox::critical(this, "Error", "Failed to move the file. Check permissions.");
        return;
    }

    // Clean up the script file
    QFile::remove(scriptFilePath);

    QMessageBox::information(this, "Success", "Desktop file created successfully (system-wide).");
}
