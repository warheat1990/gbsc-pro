#!/usr/bin/env python3
"""
ADV Manager - Main Entry Point

An experimental GUI application for interfacing with ADV7280/ADV7391
video processors via serial connection.

Author: Brisma
"""

import sys
import os
from PyQt6.QtWidgets import QApplication
from PyQt6.QtGui import QPalette, QColor

from adv_manager.ui import MainWindow
from adv_manager.utils import get_logger


def main():
    """Main application entry point."""
    logger = get_logger("main")
    logger.info("Starting ADV Manager")

    # Force light theme on macOS to avoid color conflicts with dark mode
    if sys.platform == 'darwin':
        os.environ['QT_QPA_PLATFORM_THEME'] = ''

    # Create Qt application
    app = QApplication(sys.argv)
    app.setApplicationName("ADV Manager")
    app.setOrganizationName("GBSC-Pro")

    # Force light color scheme to prevent OS dark mode from affecting the app
    app.setStyle('Fusion')
    light_palette = QPalette()
    light_palette.setColor(QPalette.ColorRole.Window, QColor(255, 255, 255))
    light_palette.setColor(QPalette.ColorRole.WindowText, QColor(0, 0, 0))
    light_palette.setColor(QPalette.ColorRole.Base, QColor(255, 255, 255))
    light_palette.setColor(QPalette.ColorRole.AlternateBase, QColor(245, 245, 245))
    light_palette.setColor(QPalette.ColorRole.ToolTipBase, QColor(255, 255, 255))
    light_palette.setColor(QPalette.ColorRole.ToolTipText, QColor(0, 0, 0))
    light_palette.setColor(QPalette.ColorRole.Text, QColor(0, 0, 0))
    light_palette.setColor(QPalette.ColorRole.Button, QColor(240, 240, 240))
    light_palette.setColor(QPalette.ColorRole.ButtonText, QColor(0, 0, 0))
    light_palette.setColor(QPalette.ColorRole.BrightText, QColor(255, 0, 0))
    light_palette.setColor(QPalette.ColorRole.Link, QColor(33, 150, 243))
    light_palette.setColor(QPalette.ColorRole.Highlight, QColor(33, 150, 243))
    light_palette.setColor(QPalette.ColorRole.HighlightedText, QColor(255, 255, 255))
    light_palette.setColor(QPalette.ColorRole.PlaceholderText, QColor(128, 128, 128))
    app.setPalette(light_palette)

    # Create and show main window
    window = MainWindow()
    window.show()

    logger.info("Application window created")

    # Run application event loop
    exit_code = app.exec()

    logger.info(f"Application exiting with code {exit_code}")
    return exit_code


if __name__ == '__main__':
    sys.exit(main())
