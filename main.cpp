#include "mainwindow.h"

#include <QApplication>
#include <QIcon>
#include <QStyleFactory>
#include <QPalette>
#include <QColor>

static void applyDarkFusionPalette(QApplication &app)
{
    app.setStyle(QStyleFactory::create("Fusion"));

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(53, 53, 53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(35, 35, 35));
    palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(53, 53, 53));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, Qt::black);

    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));

    app.setPalette(palette);

    app.setStyleSheet(
        "QToolTip { "
        "color: #ffffff; "
        "background-color: #2a2a2a; "
        "border: 1px solid #666666; "
        "}"
        );
}

// --- SINGLE SOURCE OF TRUTH: Fusion + visible indicators for checkboxes/radios on dark UI ---
static void installFusionAndIndicatorStyles(QApplication &app)
{
    // Force Fusion so QSS behaves predictably across Linux desktops (esp. on Qt 6)
    app.setStyle(QStyleFactory::create("Fusion"));

    // App-wide stylesheet:
    // - Bigger, clearly-visible checkbox & radio indicators on dark backgrounds
    // - White/bright checks so they "pop"
    // - Hover/pressed/disabled states
    // - Keeps your app text light on dark
    const char *qss = R"qss(
/* Base text color for dark UI */
QWidget { color: #EAEAEA; }

/* =======================
 *   CHECKBOX INDICATORS
 *   ======================= */
QCheckBox::indicator {
    width: 12px; height: 12px;
    border: 1px solid #9AA0A6;      /* subtle frame */
    background: #2A2A2A;            /* dark fill to match dark UI */
    margin-right: 6px;
}

QCheckBox::indicator:hover { border-color: #C3C7CF;  width: 14px; height: 14px;}
QCheckBox::indicator:pressed { background: #232323; }

QCheckBox::indicator:unchecked { image: none; }
QCheckBox::indicator:checked {
    /* Bright, high-contrast check */
    background: #0000ff;
    image: none;
}

/* Disabled look */
QCheckBox::indicator:disabled {
    border-color: #555;
    background: #2A2A2A;
}
QCheckBox:disabled { color: #8A8A8A; }

/* Optional: give group boxes / menus a slightly clearer edge on dark */
QGroupBox, QMenu, QMenuBar, QToolBar {
    background-color: #1E1E1E;
    border: 1px solid #2E2E2E;
}
)qss";

    app.setStyleSheet(qss);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/appicon.png"));

    // 👉 IMPORTANT: Call this BEFORE creating/showing MainWindow
    installFusionAndIndicatorStyles(app);

    applyDarkFusionPalette(app);

    MainWindow w;
    w.show();
    return app.exec();
}
