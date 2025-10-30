"""
Widget for displaying and editing a single register.
"""

from PyQt6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QFormLayout,
                            QLabel, QCheckBox, QComboBox, QSpinBox, QPushButton, QFrame)
from PyQt6.QtGui import QFont, QColor, QPalette, QPixmap, QPainter, QIcon
from PyQt6.QtCore import Qt, pyqtSignal, QSize

from ..models import Register, RegisterField
from ..serial_bridge import SerialBridge
from .styles import get_register_widget_stylesheet, get_bit_cell_stylesheet


def create_arrow_icon(direction='up', size=16):
    """Create a simple arrow icon for SpinBox buttons."""
    pixmap = QPixmap(size, size)
    pixmap.fill(Qt.GlobalColor.transparent)

    painter = QPainter(pixmap)
    painter.setRenderHint(QPainter.RenderHint.Antialiasing)
    painter.setPen(Qt.PenStyle.NoPen)
    painter.setBrush(QColor("#212121"))

    if direction == 'up':
        # Draw up arrow triangle
        points = [
            (size // 2, size // 4),      # top
            (size * 3 // 4, size * 3 // 4),  # bottom right
            (size // 4, size * 3 // 4)   # bottom left
        ]
    else:
        # Draw down arrow triangle
        points = [
            (size // 4, size // 4),      # top left
            (size * 3 // 4, size // 4),  # top right
            (size // 2, size * 3 // 4)   # bottom
        ]

    from PyQt6.QtCore import QPointF
    from PyQt6.QtGui import QPolygonF
    polygon = QPolygonF([QPointF(x, y) for x, y in points])
    painter.drawPolygon(polygon)
    painter.end()

    return QIcon(pixmap)


class ClickableBitCell(QFrame):
    """Clickable bit cell for toggling individual bits."""

    clicked = pyqtSignal(int)  # Emits bit position when clicked

    def __init__(self, bit_pos: int, is_reserved: bool = False, is_unknown: bool = False, is_readonly: bool = False):
        super().__init__()
        self.bit_pos = bit_pos
        self.is_reserved = is_reserved
        self.is_unknown = is_unknown
        self.is_readonly = is_readonly
        self.setStyleSheet(get_bit_cell_stylesheet(is_reserved=is_reserved, is_unknown=is_unknown, is_readonly=is_readonly))
        # Don't show pointing cursor if readonly
        if not is_readonly:
            self.setCursor(Qt.CursorShape.PointingHandCursor)

    def mousePressEvent(self, event):
        """Handle mouse click."""
        if event.button() == Qt.MouseButton.LeftButton:
            self.clicked.emit(self.bit_pos)
        super().mousePressEvent(event)


class RegisterWidget(QWidget):
    """Widget for viewing and editing a single register."""

    # Signals
    value_changed = pyqtSignal(int, int)  # register_address, new_value

    def __init__(self, register: Register, device: 'Device', device_addr: int,
                 serial_bridge: SerialBridge, bg_color: str = "#ffffff",
                 console=None):
        """
        Initialize the register widget.

        Args:
            register: Register model
            device: Device model (for map_value lookup)
            device_addr: I2C device address
            serial_bridge: Serial communication bridge
            bg_color: Background color
            console: Serial console widget for logging
        """
        super().__init__()
        self.register = register
        self.device = device
        # Use register's I2C address if specified (for VPP map), otherwise use device address
        self.device_addr = register.i2c_address if register.i2c_address is not None else device_addr
        self.serial = serial_bridge
        self.console = console
        # Get map_value directly from register
        self.map_value = register.map_value

        self.field_widgets = []
        self.bit_displays = []
        self.is_reading = False
        self.is_writing = False

        self.init_ui(bg_color)

    def init_ui(self, bg_color: str):
        """Initialize the user interface."""
        layout = QVBoxLayout()
        layout.setContentsMargins(12, 12, 12, 12)
        layout.setSpacing(8)

        # Header with register name
        header = QLabel(f"0x{self.register.address:02X}: {self.register.name}")
        header_font = QFont()
        header_font.setBold(True)
        header_font.setPointSize(11)
        header.setFont(header_font)
        layout.addWidget(header)

        # Description
        if self.register.description:
            desc = QLabel(self.register.description)
            desc.setStyleSheet("color: gray; font-size: 9px;")
            desc.setWordWrap(True)
            layout.addWidget(desc)

        # Map info (show which map this register belongs to)
        if self.register.register_map:
            map_info = QLabel(f"Map: {self.register.register_map} (0x{self.register.map_value:02X})")
            map_info.setStyleSheet("color: gray; font-size: 9px;")
            layout.addWidget(map_info)

        # Fields form
        form = QFormLayout()
        form.setSpacing(8)
        form.setContentsMargins(0, 0, 0, 0)

        for field in self.register.fields:
            # Only add bit range if not already in field name
            bit_range = field.get_bit_range_str()
            if bit_range and bit_range not in field.name:
                label_text = f"{field.name} {bit_range}"
            else:
                label_text = field.name

            field_label = QLabel(label_text)
            field_label.setStyleSheet("font-weight: bold; font-size: 10px; vertical-align: center;")
            field_label.setMinimumHeight(24)

            if field.type == 'reserved':
                value_label = QLabel("(reserved - do not modify)")
                value_label.setStyleSheet("color: red; font-size: 9px;")
                form.addRow(field_label, value_label)
                self.field_widgets.append(None)

            elif field.type == 'unknown':
                value_label = QLabel("(unknown - undocumented)")
                value_label.setStyleSheet("color: orange; font-size: 9px;")
                form.addRow(field_label, value_label)
                self.field_widgets.append(None)

            elif field.type == 'bool':
                checkbox = QCheckBox(field.description)
                checkbox.setToolTip(field.description)
                checkbox.stateChanged.connect(self._on_field_changed)
                # Disable if register is readonly
                if self.register.readonly:
                    checkbox.setEnabled(False)
                form.addRow(field_label, checkbox)
                self.field_widgets.append(checkbox)

            elif field.type == 'enum':
                combo = QComboBox()
                combo.setMinimumWidth(150)
                combo.setMaximumWidth(350)
                for val, name in field.values.items():
                    combo.addItem(name, val)
                combo.setToolTip(field.description)
                combo.currentIndexChanged.connect(self._on_field_changed)
                # Disable if register is readonly
                if self.register.readonly:
                    combo.setEnabled(False)
                form.addRow(field_label, combo)
                self.field_widgets.append(combo)

            elif field.type == 'int':
                widget_layout = QHBoxLayout()
                widget_layout.setContentsMargins(0, 0, 0, 0)
                widget_layout.setSpacing(2)

                spinbox = QSpinBox()
                spinbox.setMaximumWidth(100)
                spinbox.setMinimumHeight(24)
                spinbox.setRange(field.range[0], field.range[1])
                spinbox.setToolTip(field.description)
                spinbox.valueChanged.connect(self._on_field_changed)
                # Disable if register is readonly
                if self.register.readonly:
                    spinbox.setEnabled(False)

                # Remove native buttons and create custom ones
                spinbox.setButtonSymbols(QSpinBox.ButtonSymbols.NoButtons)
                widget_layout.addWidget(spinbox)

                # Custom arrow buttons with visible arrows
                arrow_layout = QVBoxLayout()
                arrow_layout.setContentsMargins(0, 0, 0, 0)
                arrow_layout.setSpacing(1)

                up_btn = QPushButton("▲")
                up_btn.setMaximumSize(24, 12)
                up_btn.setMinimumSize(24, 12)
                up_btn.setStyleSheet("""
                    QPushButton {
                        background-color: #F5F5F5;
                        border: 1px solid #E0E0E0;
                        border-radius: 3px;
                        font-size: 9px;
                        font-weight: bold;
                        padding: 0;
                        margin: 0;
                        color: #212121;
                    }
                    QPushButton:hover {
                        background-color: #BBDEFB;
                        border-color: #2196F3;
                    }
                    QPushButton:pressed {
                        background-color: #2196F3;
                        color: white;
                    }
                """)
                up_btn.clicked.connect(lambda checked=False, sb=spinbox: sb.stepUp())
                # Disable if register is readonly
                if self.register.readonly:
                    up_btn.setEnabled(False)
                arrow_layout.addWidget(up_btn)

                down_btn = QPushButton("▼")
                down_btn.setMaximumSize(24, 12)
                down_btn.setMinimumSize(24, 12)
                down_btn.setStyleSheet("""
                    QPushButton {
                        background-color: #F5F5F5;
                        border: 1px solid #E0E0E0;
                        border-radius: 3px;
                        font-size: 9px;
                        font-weight: bold;
                        padding: 0;
                        margin: 0;
                        color: #212121;
                    }
                    QPushButton:hover {
                        background-color: #BBDEFB;
                        border-color: #2196F3;
                    }
                    QPushButton:pressed {
                        background-color: #2196F3;
                        color: white;
                    }
                """)
                down_btn.clicked.connect(lambda checked=False, sb=spinbox: sb.stepDown())
                # Disable if register is readonly
                if self.register.readonly:
                    down_btn.setEnabled(False)
                arrow_layout.addWidget(down_btn)

                widget_layout.addLayout(arrow_layout)

                # Set button to apply/reset value
                if field.default is not None:
                    reset_btn = QPushButton("Set")
                    reset_btn.setMaximumWidth(40)
                    reset_btn.setToolTip(f"Reset to default ({field.default})")
                    field_idx = len(self.field_widgets)
                    reset_btn.clicked.connect(lambda checked, idx=field_idx: self._reset_field(idx))
                    # Disable if register is readonly
                    if self.register.readonly:
                        reset_btn.setEnabled(False)
                    widget_layout.addWidget(reset_btn)

                widget_layout.addStretch()
                form.addRow(field_label, widget_layout)
                self.field_widgets.append(spinbox)

        layout.addLayout(form)

        # Bit visualization
        layout.addSpacing(10)
        bits_label = QLabel("Register Bit View:")
        bits_label.setStyleSheet("font-weight: bold; font-size: 10px;")
        layout.addWidget(bits_label)

        bits_layout = QHBoxLayout()
        bits_layout.setSpacing(2)
        bits_layout.setContentsMargins(0, 0, 0, 0)

        for bit_pos in range(7, -1, -1):
            # Check if this bit belongs to a reserved or unknown field
            is_reserved = False
            is_unknown = False
            for field in self.register.fields:
                if bit_pos in field.bits:
                    if field.type == 'reserved':
                        is_reserved = True
                        break
                    elif field.type == 'unknown':
                        is_unknown = True
                        break

            bit_cell = ClickableBitCell(bit_pos, is_reserved, is_unknown, self.register.readonly)
            bit_cell.clicked.connect(self._on_bit_clicked)

            cell_layout = QVBoxLayout()
            cell_layout.setContentsMargins(3, 3, 3, 3)
            cell_layout.setSpacing(0)

            bit_num = QLabel(str(bit_pos))
            bit_num.setAlignment(Qt.AlignmentFlag.AlignCenter)
            bit_num.setStyleSheet("font-size: 8px; color: gray;")
            cell_layout.addWidget(bit_num)

            bit_value = QLabel("0")
            bit_value.setAlignment(Qt.AlignmentFlag.AlignCenter)
            bit_value.setStyleSheet("font-size: 14px; font-weight: bold;")
            cell_layout.addWidget(bit_value)

            bit_field = QLabel("")
            bit_field.setAlignment(Qt.AlignmentFlag.AlignCenter)
            bit_field.setStyleSheet("font-size: 7px; color: #666;")
            bit_field.setWordWrap(True)
            cell_layout.addWidget(bit_field)

            bit_cell.setLayout(cell_layout)
            bits_layout.addWidget(bit_cell)
            self.bit_displays.append((bit_pos, bit_value, bit_field))

        layout.addLayout(bits_layout)
        layout.addStretch()

        self.setLayout(layout)

        # Apply background color
        if bg_color:
            self.setAutoFillBackground(True)
            from PyQt6.QtGui import QPalette, QColor
            palette = self.palette()
            palette.setColor(QPalette.ColorRole.Window, QColor(bg_color))
            self.setPalette(palette)

        # Initialize bit display with current register value
        self._update_bit_display()

    def _on_field_changed(self):
        """Handle field value change."""
        if not self.is_reading:
            self.write_value()

    def _on_bit_clicked(self, bit_pos: int):
        """Handle bit cell click - toggle the bit value."""
        if self.is_reading or self.is_writing:
            return

        # Don't allow toggling if register is readonly
        if self.register.readonly:
            return

        # Toggle the bit
        current_value = self.register.current_value
        new_value = current_value ^ (1 << bit_pos)  # XOR to toggle

        # Write directly to device without rebuilding from widgets
        self.is_writing = True

        # Log to console
        if self.console:
            # Choose command format based on use_map_switching
            if not self.register.use_map_switching:
                cmd = f"w {self.device_addr:02X} {self.register.address:02X} {new_value:02X}"
            else:
                cmd = f"wm {self.device_addr:02X} {self.map_value:02X} {self.register.address:02X} {new_value:02X}"
            self.console.log_message(f"> {cmd}", "#666666")

        def on_write_complete(success):
            self.is_writing = False
            if success:
                if self.console:
                    self.console.log_message(f"  OK", "#009900")
                self.register.update_value(new_value)
                self._update_widgets_from_register()
                self._update_bit_display()
                self.value_changed.emit(self.register.address, new_value)
            else:
                if self.console:
                    self.console.log_message(f"  ERR", "#cc0000")

        # Write register with the toggled value (firmware handles map switching if enabled)
        self.serial.write_register(self.device_addr, self.map_value, self.register.address, new_value, on_write_complete, self.register.use_map_switching)

    def _reset_field(self, field_idx: int):
        """Reset a field to its default value."""
        if field_idx < len(self.register.fields):
            field = self.register.fields[field_idx]
            widget = self.field_widgets[field_idx]
            if field.type == 'int' and field.default is not None and widget:
                widget.setValue(field.default)

    def read_value(self):
        """Read register value from device."""
        if not self.serial.is_connected() or self.is_reading:
            return

        self.is_reading = True

        def on_read_complete(value):
            self.is_reading = False
            if value is not None:
                self.register.update_value(value)
                self._update_widgets_from_register()
                self._update_bit_display()

        # Read register (firmware handles map switching)
        self.serial.read_register(self.device_addr, self.map_value, self.register.address, on_read_complete)

    def write_value(self):
        """Write register value to device."""
        if not self.serial.is_connected() or self.is_writing:
            return

        self.is_writing = True

        # Build new register value from widgets
        new_value = self.register.current_value
        for i, field in enumerate(self.register.fields):
            widget = self.field_widgets[i]
            if widget is None or field.type == 'reserved' or field.type == 'unknown':
                continue

            if field.type == 'bool':
                field_value = 1 if widget.isChecked() else 0
            elif field.type == 'enum':
                field_value = widget.currentData()
            elif field.type == 'int':
                field_value = widget.value()
            else:
                continue

            new_value = field.insert(new_value, field_value)

        # Update register value and bit display immediately (optimistic update)
        self.register.update_value(new_value)
        self._update_bit_display()

        # Log to console
        if self.console:
            # Choose command format based on use_map_switching
            if not self.register.use_map_switching:
                cmd = f"w {self.device_addr:02X} {self.register.address:02X} {new_value:02X}"
            else:
                cmd = f"wm {self.device_addr:02X} {self.map_value:02X} {self.register.address:02X} {new_value:02X}"
            self.console.log_message(f"> {cmd}", "#666666")

        def on_write_complete(success):
            self.is_writing = False
            if success:
                if self.console:
                    self.console.log_message(f"  OK", "#009900")
                self.value_changed.emit(self.register.address, new_value)
            else:
                if self.console:
                    self.console.log_message(f"  ERR", "#cc0000")
                # If write failed, re-read to get actual device value
                self.read_value()

        # Write register (firmware handles map switching if enabled)
        self.serial.write_register(self.device_addr, self.map_value, self.register.address, new_value, on_write_complete, self.register.use_map_switching)

    def _update_widgets_from_register(self):
        """Update widget values from register."""
        for i, field in enumerate(self.register.fields):
            widget = self.field_widgets[i]
            if widget is None:
                continue

            field_value = field.extract(self.register.current_value)

            widget.blockSignals(True)
            if field.type == 'bool':
                widget.setChecked(bool(field_value))
            elif field.type == 'enum':
                idx = widget.findData(field_value)
                if idx >= 0:
                    widget.setCurrentIndex(idx)
            elif field.type == 'int':
                widget.setValue(field_value)
            widget.blockSignals(False)

    def _update_bit_display(self):
        """Update bit visualization."""
        for bit_pos, bit_value_label, bit_field_label in self.bit_displays:
            bit_val = (self.register.current_value >> bit_pos) & 1
            bit_value_label.setText(str(bit_val))

            # Find field name for this bit
            field_name = ""
            for field in self.register.fields:
                if bit_pos in field.bits:
                    # Clean field name: remove bit range notation like [4:0]
                    field_name = field.name
                    # Remove patterns like [4:0], [3:0], [7], etc.
                    import re
                    field_name = re.sub(r'\[\d+:\d+\]|\[\d+\]', '', field_name).strip()
                    # Abbreviate long names
                    if len(field_name) > 10:
                        field_name = field_name[:8] + "..."
                    break
            bit_field_label.setText(field_name)

    def get_current_value(self) -> int:
        """Get current register value."""
        return self.register.current_value

    def set_enabled(self, enabled: bool):
        """Enable or disable all input widgets."""
        for widget in self.field_widgets:
            if widget:
                widget.setEnabled(enabled)
