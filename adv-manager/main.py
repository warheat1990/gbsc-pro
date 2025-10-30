#!/usr/bin/env python3
"""
ADV Manager - Main Entry Point

An experimental GUI application for interfacing with ADV7280/ADV7391
video processors via serial connection.

Author: Brisma
"""

import sys
from PyQt6.QtWidgets import QApplication

from adv_manager.ui import MainWindow
from adv_manager.utils import get_logger


def main():
    """Main application entry point."""
    logger = get_logger("main")
    logger.info("Starting ADV Manager")

    # Create Qt application
    app = QApplication(sys.argv)
    app.setApplicationName("ADV Manager")
    app.setOrganizationName("GBSC-Pro")

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
