#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QPixmap>

class DesktopFileCreator : public QWidget {
public:
    DesktopFileCreator(QWidget *parent = nullptr);

private slots:
    void createDesktopFile();

private:
    QLineEdit *nameField;
    QLineEdit *execField;
    QLineEdit *iconField;
    QLineEdit *commentField;
    QCheckBox *terminalCheckBox;
    QList<QCheckBox *> categoryCheckboxes;
};

DesktopFileCreator::DesktopFileCreator(QWidget *parent)
    : QWidget(parent) {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Name Field
    QLabel *nameLabel = new QLabel("Name:");
    nameField = new QLineEdit();
    nameField->setStyleSheet("QLineEdit { background-color: black; color: white; border: 1px solid gray; }");  // Set border always visible

    mainLayout->addWidget(nameLabel);
    mainLayout->addWidget(nameField);

    // Exec Field and Browse Button (Side by Side)
    QLabel *execLabel = new QLabel("Exec:");
    mainLayout->addWidget(execLabel);

    QHBoxLayout *execLayout = new QHBoxLayout();  // Horizontal layout for input + button
    execField = new QLineEdit();
    execField->setStyleSheet("QLineEdit { background-color: black; color: white; border: 1px solid gray; }");  // Set border always visible
    execLayout->addWidget(execField);

    QPushButton *execBrowseButton = new QPushButton("Browse");
    execBrowseButton->setStyleSheet("QPushButton { background-color: #ffffff; color: #000000; }"
                                    "QPushButton:hover { background-color: #c1c1c1; }");
    execBrowseButton->setFixedSize(100, 30);
    connect(execBrowseButton, &QPushButton::clicked, this, [=]() {
        QString fileName = QFileDialog::getOpenFileName(this, "Select Executable");
        execField->setText(fileName);
    });
    execLayout->addWidget(execBrowseButton);  // Add the button next to the field
    mainLayout->addLayout(execLayout);

    // Icon Field and Browse Button (Side by Side)
    QLabel *iconLabel = new QLabel("Icon:");
    mainLayout->addWidget(iconLabel);

    QHBoxLayout *iconLayout = new QHBoxLayout();  // Horizontal layout for input + button
    iconField = new QLineEdit();
    iconField->setStyleSheet("QLineEdit { background-color: black; color: white; border: 1px solid gray; }");  // Set border always visible
    iconLayout->addWidget(iconField);

    QPushButton *iconBrowseButton = new QPushButton("Browse");
    iconBrowseButton->setStyleSheet("QPushButton { background-color: #ffffff; color: #000000; }"
                                    "QPushButton:hover { background-color: #c1c1c1; }");
    iconBrowseButton->setFixedSize(100, 30);
    connect(iconBrowseButton, &QPushButton::clicked, this, [=]() {
        QString fileName = QFileDialog::getOpenFileName(this, "Select Icon");
        iconField->setText(fileName);
    });
    iconLayout->addWidget(iconBrowseButton);  // Add the button next to the field
    mainLayout->addLayout(iconLayout);

    // Comment Field
    QLabel *commentLabel = new QLabel("Comment:");
    mainLayout->addWidget(commentLabel);

    commentField = new QLineEdit();
    commentField->setStyleSheet("QLineEdit { background-color: black; color: white; border: 1px solid gray; }");  // Set border always visible
    mainLayout->addWidget(commentField);

    // Terminal Checkbox
    QLabel *terminalLabel = new QLabel("Terminal:");
    terminalCheckBox = new QCheckBox();
    mainLayout->addWidget(terminalLabel);
    mainLayout->addWidget(terminalCheckBox);

    // Categories label and checkboxes
    QLabel *categoriesLabel = new QLabel("Categories:");
    mainLayout->addWidget(categoriesLabel);

    // Horizontal layout for checkboxes and image
    QHBoxLayout *checkboxImageLayout = new QHBoxLayout();

    // Checkbox panel (on the left)
    QVBoxLayout *checkboxPanel = new QVBoxLayout();
    QStringList categories = {"Development", "Games", "Graphics", "Internet", "Office", "Multimedia", "Settings", "System", "Utility"};
    for (const QString &category : categories) {
        QCheckBox *checkBox = new QCheckBox(category);
        categoryCheckboxes.append(checkBox);
        checkboxPanel->addWidget(checkBox);
    }

    // Add the checkboxes to the horizontal layout
    checkboxImageLayout->addLayout(checkboxPanel);

    // Image label (on the right)
    QLabel *imageLabel = new QLabel();
    QPixmap pixmap("/home/john/pics/icons/black.jpg");  // Replace with your image path
    imageLabel->setPixmap(pixmap);
    imageLabel->setFixedSize(pixmap.size());  // Set size to match the image
    checkboxImageLayout->addWidget(imageLabel);

    // Add the horizontal layout (checkboxes + image) to the main layout
    mainLayout->addLayout(checkboxImageLayout);

    // Center the Submit Button
    QHBoxLayout *submitLayout = new QHBoxLayout();
    submitLayout->addStretch();  // Add left spacer

    QPushButton *submitButton = new QPushButton("Create .desktop File");
    submitButton->setStyleSheet("QPushButton { background-color: #ffffff; color: #000000; }"
                                "QPushButton:hover { background-color: #c1c1c1; }");
    submitButton->setFixedSize(200, 40);  // Set fixed size for the button
    submitLayout->addWidget(submitButton);  // Add button to layout

    submitLayout->addStretch();  // Add right spacer
    mainLayout->addLayout(submitLayout);  // Add the centered button layout to the main layout

    // Button action listener
    connect(submitButton, &QPushButton::clicked, this, &DesktopFileCreator::createDesktopFile);
}

void DesktopFileCreator::createDesktopFile() {
    QString name = nameField->text();
    QString exec = execField->text();
    QString icon = iconField->text();
    QString comment = commentField->text();
    bool terminal = terminalCheckBox->isChecked();

    // Collect selected categories
    QStringList selectedCategories;
    for (int i = 0; i < categoryCheckboxes.size(); ++i) {
        if (categoryCheckboxes[i]->isChecked()) {
            selectedCategories.append(categoryCheckboxes[i]->text());
        }
    }
    QString categoriesString = selectedCategories.join(";");

    QString home = QDir::homePath();
    QString filePath = home + "/.local/share/applications/" + name.replace(" ", "_") + ".desktop";

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
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
        if (!categoriesString.isEmpty()) {
            out << "Categories=" << categoriesString << ";\n";
        }
        out.flush();  // Ensure everything is written to the file
        file.close(); // Make sure to close the file
        QMessageBox::information(this, "Success", "Desktop file created successfully at " + filePath);
    } else {
        QMessageBox::critical(this, "Error", "Could not open file for writing.");
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    DesktopFileCreator window;
    window.setWindowTitle("Desktop File Creator with Styled UI");
    window.resize(500, 400);
    window.show();

    return app.exec();
}
