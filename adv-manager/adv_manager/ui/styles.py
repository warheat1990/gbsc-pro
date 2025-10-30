"""
UI styling and theme definitions.
"""

from PyQt6.QtGui import QColor, QPalette
from PyQt6.QtWidgets import QApplication


class AppTheme:
    """Application color theme."""

    # Primary colors
    PRIMARY = "#2196F3"
    PRIMARY_DARK = "#1976D2"
    PRIMARY_LIGHT = "#BBDEFB"

    # Secondary colors
    SECONDARY = "#FF9800"
    SECONDARY_DARK = "#F57C00"
    SECONDARY_LIGHT = "#FFE0B2"

    # Status colors
    SUCCESS = "#4CAF50"
    WARNING = "#FF9800"
    ERROR = "#F44336"
    INFO = "#2196F3"

    # Background colors
    BG_DARK = "#263238"
    BG_MEDIUM = "#37474F"
    BG_LIGHT = "#FFFFFF"
    BG_ALTERNATE = "#F5F5F5"

    # Text colors
    TEXT_PRIMARY = "#212121"
    TEXT_SECONDARY = "#757575"
    TEXT_DISABLED = "#BDBDBD"
    TEXT_ON_DARK = "#FFFFFF"

    # Border colors
    BORDER = "#E0E0E0"
    BORDER_FOCUS = "#2196F3"


def get_app_stylesheet() -> str:
    """
    Get the main application stylesheet.

    Returns:
        CSS stylesheet string
    """
    return f"""
    /* Main Window */
    QMainWindow {{
        background-color: {AppTheme.BG_LIGHT};
    }}

    /* Labels */
    QLabel {{
        color: {AppTheme.TEXT_PRIMARY};
        font-size: 10pt;
    }}

    /* Buttons */
    QPushButton {{
        background-color: {AppTheme.PRIMARY};
        color: white;
        border: none;
        padding: 6px 12px;
        border-radius: 4px;
        font-size: 10pt;
        font-weight: bold;
    }}

    QPushButton:hover {{
        background-color: {AppTheme.PRIMARY_DARK};
    }}

    QPushButton:pressed {{
        background-color: {AppTheme.PRIMARY_DARK};
        padding-top: 7px;
        padding-bottom: 5px;
    }}

    QPushButton:disabled {{
        background-color: {AppTheme.TEXT_DISABLED};
        color: {AppTheme.TEXT_SECONDARY};
    }}

    /* Input fields */
    QLineEdit {{
        border: 1px solid {AppTheme.BORDER};
        border-radius: 4px;
        padding: 5px;
        background-color: white;
        font-size: 10pt;
    }}

    QLineEdit:focus {{
        border: 2px solid {AppTheme.BORDER_FOCUS};
    }}

    /* SpinBox - minimal styling, let Qt handle arrows */
    QSpinBox {{
        border: 1px solid {AppTheme.BORDER};
        border-radius: 4px;
        padding: 5px;
        background-color: white;
        font-size: 10pt;
        min-width: 80px;
    }}

    QSpinBox:focus {{
        border: 2px solid {AppTheme.BORDER_FOCUS};
    }}

    /* ComboBox - use native styling */
    QComboBox {{
        font-size: 10pt;
        padding: 5px;
    }}

    /* CheckBox */
    QCheckBox {{
        spacing: 8px;
        font-size: 10pt;
    }}

    QCheckBox::indicator {{
        width: 18px;
        height: 18px;
        border: 2px solid {AppTheme.BORDER};
        border-radius: 3px;
        background-color: white;
    }}

    QCheckBox::indicator:checked {{
        background-color: {AppTheme.PRIMARY};
        border-color: {AppTheme.PRIMARY};
    }}

    /* Tabs */
    QTabWidget::pane {{
        border: 1px solid {AppTheme.BORDER};
        border-radius: 4px;
        top: -1px;
    }}

    QTabBar::tab {{
        background-color: {AppTheme.BG_ALTERNATE};
        border: 1px solid {AppTheme.BORDER};
        border-bottom: none;
        border-top-left-radius: 4px;
        border-top-right-radius: 4px;
        padding: 8px 16px;
        margin-right: 2px;
        font-size: 10pt;
    }}

    QTabBar::tab:selected {{
        background-color: white;
        border-bottom: 2px solid {AppTheme.PRIMARY};
        font-weight: bold;
    }}

    QTabBar::tab:hover {{
        background-color: {AppTheme.PRIMARY_LIGHT};
    }}

    /* ScrollArea */
    QScrollArea {{
        border: none;
    }}

    /* ScrollBar */
    QScrollBar:vertical {{
        border: none;
        background-color: {AppTheme.BG_ALTERNATE};
        width: 12px;
        margin: 0;
    }}

    QScrollBar::handle:vertical {{
        background-color: {AppTheme.TEXT_DISABLED};
        border-radius: 6px;
        min-height: 20px;
    }}

    QScrollBar::handle:vertical:hover {{
        background-color: {AppTheme.TEXT_SECONDARY};
    }}

    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {{
        height: 0px;
    }}

    /* Status Bar */
    QStatusBar {{
        background-color: {AppTheme.BG_ALTERNATE};
        border-top: 1px solid {AppTheme.BORDER};
        font-size: 9pt;
    }}

    /* Group Box */
    QGroupBox {{
        border: 1px solid {AppTheme.BORDER};
        border-radius: 4px;
        margin-top: 12px;
        font-weight: bold;
        font-size: 10pt;
    }}

    QGroupBox::title {{
        subcontrol-origin: margin;
        subcontrol-position: top left;
        padding: 0 5px;
        color: {AppTheme.PRIMARY};
    }}

    /* Tool Tip */
    QToolTip {{
        background-color: {AppTheme.BG_DARK};
        color: white;
        border: 1px solid {AppTheme.BORDER};
        padding: 5px;
        font-size: 9pt;
        border-radius: 3px;
    }}

    /* MessageBox */
    QMessageBox {{
        background-color: white;
        font-size: 10pt;
    }}
    """


def get_register_widget_stylesheet(bg_color: str = "#ffffff") -> str:
    """
    Get stylesheet for register widgets.

    Args:
        bg_color: Background color in hex format

    Returns:
        CSS stylesheet string
    """
    return f"""
    QWidget {{
        background-color: {bg_color};
        border-bottom: 1px solid {AppTheme.BORDER};
    }}
    """


def get_bit_cell_stylesheet(is_selected: bool = False, is_reserved: bool = False, is_unknown: bool = False, is_readonly: bool = False) -> str:
    """
    Get stylesheet for bit display cells.

    Args:
        is_selected: Whether the cell is selected
        is_reserved: Whether this bit is a reserved bit
        is_unknown: Whether this bit is unknown/undocumented
        is_readonly: Whether this bit is part of a readonly register

    Returns:
        CSS stylesheet string
    """
    if is_reserved:
        bg_color = "#ffcccc"  # Light red for reserved bits
        border_color = "#cc9999"
    elif is_unknown:
        bg_color = "#ffddaa"  # Light orange for unknown bits
        border_color = "#ccaa88"
    elif is_selected:
        bg_color = AppTheme.PRIMARY_LIGHT
        border_color = AppTheme.PRIMARY
    elif is_readonly:
        bg_color = "#E8E8E8"  # Gray background for readonly bits
        border_color = "#B0B0B0"
    else:
        bg_color = "#F9F9F9"
        border_color = "#CCCCCC"

    # Make text color lighter for readonly
    text_color = "#888888" if is_readonly else "#212121"

    return f"""
    QFrame {{
        border: 1px solid {border_color};
        background-color: {bg_color};
        padding: 2px;
    }}
    QLabel {{
        color: {text_color};
    }}
    """


def get_connection_status_style(connected: bool) -> str:
    """
    Get style for connection status indicator.

    Args:
        connected: Connection status

    Returns:
        CSS stylesheet string
    """
    color = AppTheme.SUCCESS if connected else AppTheme.ERROR
    return f"""
    QLabel {{
        color: {color};
        font-weight: bold;
        font-size: 10pt;
    }}
    """


class IconHelper:
    """Helper for getting icons and symbols."""

    # Unicode symbols
    CONNECTED = "●"
    DISCONNECTED = "○"
    REFRESH = "⟲"
    SETTINGS = "⚙"
    SAVE = "💾"
    LOAD = "📁"
    PLAY = "▶"
    WARNING = "⚠"
    SUCCESS = "✓"
    ERROR = "✗"
    INFO = "ℹ"

    @staticmethod
    def get_status_icon(connected: bool) -> str:
        """Get icon for connection status."""
        return IconHelper.CONNECTED if connected else IconHelper.DISCONNECTED
