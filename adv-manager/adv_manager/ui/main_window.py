"""
Main application window.
"""

from pathlib import Path
from PyQt6.QtWidgets import (QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
                            QLabel, QComboBox, QPushButton, QTabWidget,
                            QScrollArea, QStatusBar, QFileDialog, QMessageBox,
                            QDialog, QLineEdit, QGroupBox, QSplitter)
from PyQt6.QtCore import QTimer, Qt, pyqtSignal

from ..serial_bridge import SerialBridge
from ..config_manager import ConfigManager
from ..macro_manager import MacroManager
from ..models import Device, Preset
from ..utils.logger import get_logger
from .connection_widget import ConnectionWidget
from .register_widget import RegisterWidget
from .styles import get_app_stylesheet


class MainWindow(QMainWindow):
    """Main application window."""

    # Signal for thread-safe callback execution
    dump_completed = pyqtSignal(int, list)  # map_index, values

    def __init__(self):
        """Initialize the main window."""
        super().__init__()
        self.logger = get_logger(__name__)

        # Models and managers
        self.serial = SerialBridge()
        self.config_manager = ConfigManager()
        self.macro_manager = MacroManager()
        self.current_device: Device = None
        self.register_widgets = []
        self.tab_to_map = {}  # Maps tab index to register map name
        self.current_map_name = None  # Currently active register map

        # UI components
        self.connection_widget = None
        self.tabs = None
        self.preset_combo = None
        self.macro_combo = None
        self.status_bar = None

        self.init_ui()
        self.setWindowTitle("ADV Manager")
        self.resize(1400, 900)

        # Apply stylesheet
        self.setStyleSheet(get_app_stylesheet())

    def init_ui(self):
        """Initialize the user interface."""
        central = QWidget()
        main_layout = QVBoxLayout()
        main_layout.setContentsMargins(10, 10, 10, 10)
        main_layout.setSpacing(8)

        # === Serial Communication Group ===
        comm_group = QGroupBox("Serial Communication")
        comm_layout = QVBoxLayout()
        comm_layout.setSpacing(5)
        comm_layout.setContentsMargins(8, 5, 8, 5)

        self.connection_widget = ConnectionWidget(self.serial)
        self.connection_widget.connection_changed.connect(self._on_connection_changed)
        # Connect reload button from connection widget
        self.connection_widget.reload_btn.clicked.connect(self._reload_all_registers)
        comm_layout.addWidget(self.connection_widget)

        comm_group.setLayout(comm_layout)
        comm_group.setMaximumHeight(80)
        main_layout.addWidget(comm_group)


        # === Presets & Macros Group ===
        presets_macros_group = QGroupBox("Presets & Macros")
        presets_macros_layout = QVBoxLayout()
        presets_macros_layout.setSpacing(5)
        presets_macros_layout.setContentsMargins(8, 5, 8, 5)

        # Preset row
        preset_layout = QHBoxLayout()
        preset_layout.setSpacing(8)

        preset_label = QLabel("Preset:")
        preset_label.setMinimumWidth(50)
        preset_layout.addWidget(preset_label)

        self.preset_combo = QComboBox()
        self.preset_combo.setMinimumWidth(220)
        self.preset_combo.setPlaceholderText("Select preset...")
        self.preset_combo.currentIndexChanged.connect(self._on_preset_selected)
        preset_layout.addWidget(self.preset_combo)

        apply_preset_btn = QPushButton("Apply")
        apply_preset_btn.setMinimumWidth(80)
        apply_preset_btn.clicked.connect(self.apply_preset)
        preset_layout.addWidget(apply_preset_btn)

        save_preset_btn = QPushButton("Save Current")
        save_preset_btn.setMinimumWidth(110)
        save_preset_btn.clicked.connect(self.save_current_as_preset)
        preset_layout.addWidget(save_preset_btn)

        delete_preset_btn = QPushButton("Delete")
        delete_preset_btn.setMinimumWidth(80)
        delete_preset_btn.clicked.connect(self.delete_preset)
        preset_layout.addWidget(delete_preset_btn)

        preset_layout.addStretch()
        presets_macros_layout.addLayout(preset_layout)

        # Macro row
        macro_layout = QHBoxLayout()
        macro_layout.setSpacing(8)

        macro_label = QLabel("Macro:")
        macro_label.setMinimumWidth(50)
        macro_layout.addWidget(macro_label)

        self.macro_combo = QComboBox()
        self.macro_combo.setMinimumWidth(220)
        self.macro_combo.setPlaceholderText("Select macro...")
        macro_layout.addWidget(self.macro_combo)

        run_macro_btn = QPushButton("Run")
        run_macro_btn.setMinimumWidth(80)
        run_macro_btn.clicked.connect(self.run_macro)
        macro_layout.addWidget(run_macro_btn)

        macro_layout.addStretch()
        presets_macros_layout.addLayout(macro_layout)

        presets_macros_group.setLayout(presets_macros_layout)
        presets_macros_group.setMaximumHeight(100)
        main_layout.addWidget(presets_macros_group)

        # Horizontal splitter for tabs and console
        h_splitter = QSplitter(Qt.Orientation.Horizontal)
        h_splitter.setChildrenCollapsible(False)

        # Tabs for devices (left side)
        self.tabs = QTabWidget()
        self.tabs.setMinimumWidth(400)
        # Make tab labels wider to prevent truncation
        self.tabs.setStyleSheet("""
            QTabBar::tab {
                min-width: 150px;
                padding: 8px 20px;
            }
        """)
        # Connect tab change event
        self.tabs.currentChanged.connect(self._on_tab_changed)
        h_splitter.addWidget(self.tabs)

        # Serial Console (right side)
        from .serial_console import SerialConsole
        console_group = QGroupBox("Serial Console")
        console_layout = QVBoxLayout()
        console_layout.setContentsMargins(5, 5, 5, 5)
        self.console = SerialConsole(self.serial)
        self.console.command_executed.connect(self._on_console_command)
        console_layout.addWidget(self.console)
        console_group.setLayout(console_layout)
        h_splitter.addWidget(console_group)

        # Set initial sizes (50% tabs, 50% console)
        h_splitter.setSizes([500, 500])

        main_layout.addWidget(h_splitter)

        central.setLayout(main_layout)
        self.setCentralWidget(central)

        # Status bar
        self.status_bar = QStatusBar()
        self.setStatusBar(self.status_bar)
        self.status_bar.showMessage("Ready - Load a configuration file to begin")

    def _on_connection_changed(self, connected: bool):
        """Handle connection state changes."""
        if connected:
            self.logger.info("Connected to serial port")
            self.status_bar.showMessage(f"Connected to {self.serial.port}")

            # Load config files when connecting (which will trigger auto-read)
            if not self.config_manager.has_devices():
                QTimer.singleShot(100, self._auto_load_configs)
            else:
                # Config already loaded, just auto-read
                QTimer.singleShot(500, self._auto_read_registers)
        else:
            self.logger.info("Disconnected from serial port")
            self.status_bar.showMessage("Disconnected")

            # Clear all loaded devices and maps
            self.tabs.clear()
            self.preset_combo.clear()
            self.macro_combo.clear()
            self.register_widgets.clear()
            self.tab_to_map.clear()
            self.current_device = None
            self.current_map_name = None

            # Clear configuration manager
            self.config_manager.clear()

            # Clear serial console
            self.console.clear_output()

            self.logger.info("Cleared all device tabs and register maps")

    def _on_tab_changed(self, tab_index: int):
        """Handle main device tab change."""
        if tab_index < 0:
            return

        # Get all devices
        devices = self.config_manager.get_all_devices()
        if not devices or tab_index >= len(devices):
            return

        # Update current device based on tab index
        self.current_device = devices[tab_index]
        self.logger.info(f"Switched to device: {self.current_device.name}")

        # Load macros from macros/ directory (macros are now global, not device-specific)
        self._load_macros()

        # Update current map name to the first map of this device
        if self.current_device.register_maps:
            first_map = list(self.current_device.register_maps.keys())[0]
            self.current_map_name = first_map
            self.logger.info(f"Active map: {first_map}")

    def _on_map_tab_changed(self, map_tab_index: int, device, device_tabs):
        """Handle register map sub-tab change within a device."""
        if device_tabs not in self.tab_to_map:
            return

        if map_tab_index < 0 or map_tab_index not in self.tab_to_map[device_tabs]:
            return

        _, new_map_name = self.tab_to_map[device_tabs][map_tab_index]

        # Update current map name and device (no switching - firmware handles it)
        self.current_map_name = new_map_name
        self.current_device = device
        self.logger.info(f"Viewing {new_map_name}")

    def load_config(self):
        """Load configuration from YAML file."""
        file_path, _ = QFileDialog.getOpenFileName(
            self,
            "Load Configuration",
            str(Path.home()),
            "YAML Files (*.yaml *.yml);;All Files (*)"
        )

        if not file_path:
            return

        if self.config_manager.load_config(file_path):
            self._rebuild_ui_from_config()
            self.status_bar.showMessage(f"Loaded: {Path(file_path).name}")

            # Auto-read registers after a short delay if connected
            if self.serial.is_connected():
                QTimer.singleShot(500, self._auto_read_registers)
        else:
            QMessageBox.critical(
                self,
                "Load Error",
                f"Failed to load configuration from:\n{file_path}"
            )

    def save_config(self):
        """Save current configuration to YAML file."""
        if not self.config_manager.has_devices():
            QMessageBox.warning(self, "Warning", "No configuration loaded!")
            return

        file_path, _ = QFileDialog.getSaveFileName(
            self,
            "Save Configuration",
            str(Path.home()),
            "YAML Files (*.yaml);;All Files (*)"
        )

        if not file_path:
            return

        if self.config_manager.save_config(file_path):
            self.status_bar.showMessage(f"Saved: {Path(file_path).name}")
            QMessageBox.information(self, "Success", "Configuration saved successfully!")
        else:
            QMessageBox.critical(self, "Save Error", "Failed to save configuration!")

    def _rebuild_ui_from_config(self):
        """Rebuild UI from loaded configuration."""
        # Clear existing widgets
        self.tabs.clear()
        self.preset_combo.clear()
        self.macro_combo.clear()
        self.register_widgets.clear()
        self.tab_to_map.clear()

        # Get all devices
        devices = self.config_manager.get_all_devices()
        if not devices:
            return

        # Create a tab for each device (ADV7280, ADV7391, etc.)
        for device in devices:
            # Create nested tabs for this device's register maps
            device_tabs = QTabWidget()
            device_tabs.setStyleSheet("""
                QTabBar::tab {
                    min-width: 120px;
                    padding: 6px 16px;
                }
            """)

            # Store reference to device tabs widget to handle map changes
            device_tabs.currentChanged.connect(
                lambda idx, dev=device, dtabs=device_tabs: self._on_map_tab_changed(idx, dev, dtabs)
            )

            # Create a tab for each register map
            map_tab_index = 0
            for map_name, reg_map in device.register_maps.items():
                self._create_register_map_tab_for_device(device, map_name, reg_map, device_tabs)
                # Track mapping: tab_widget -> (tab_index -> (device, map_name))
                if device_tabs not in self.tab_to_map:
                    self.tab_to_map[device_tabs] = {}
                self.tab_to_map[device_tabs][map_tab_index] = (device, map_name)
                map_tab_index += 1

            # Add device tab to main tabs
            self.tabs.addTab(device_tabs, device.name)

        # Use first device as current (for macro combo)
        if devices:
            self.current_device = devices[0]

            # Populate presets from preset files directory (not device-specific)
            self._load_presets_from_directory()

            # Populate macros from macros/ directory (now global, not device-specific)
            self._load_macros()

            # Set initial map
            if self.current_device.register_maps:
                first_map = list(self.current_device.register_maps.keys())[0]
                self.current_map_name = first_map

        self.logger.info(f"UI rebuilt with {len(self.register_widgets)} registers")

    def _load_presets_from_directory(self):
        """Load all preset files from the presets directory."""
        import yaml

        presets_dir = Path(__file__).parent.parent.parent / 'presets'

        if not presets_dir.exists():
            self.logger.info("No presets directory found")
            return

        # Find all YAML files in presets directory
        preset_files = sorted(list(presets_dir.glob("*.yaml")) + list(presets_dir.glob("*.yml")))

        if not preset_files:
            self.logger.info("No preset files found")
            return

        # Load each preset file
        for preset_file in preset_files:
            try:
                with open(preset_file, 'r', encoding='utf-8') as f:
                    preset_data = yaml.safe_load(f)

                preset_name = preset_data.get('name', preset_file.stem)
                self.preset_combo.addItem(preset_name, str(preset_file))
                self.logger.info(f"Loaded preset: {preset_name} from {preset_file.name}")

            except Exception as e:
                self.logger.error(f"Error loading preset {preset_file.name}: {e}")

        self.logger.info(f"Loaded {len(preset_files)} preset(s)")

    def _load_macros(self):
        """Load all macros from the macros directory."""
        self.macro_combo.clear()

        # Load macros using MacroManager
        num_macros = self.macro_manager.load_macros()

        if num_macros == 0:
            self.logger.info("No macros found")
            return

        # Populate combo box
        for macro_name in self.macro_manager.get_macro_names():
            self.macro_combo.addItem(macro_name, macro_name)

        self.logger.info(f"Loaded {num_macros} macro(s)")

    def _create_register_map_tab_for_device(self, device, map_name: str, reg_map, device_tabs):
        """Create a tab for a specific register map within a device's tab widget."""
        scroll = QScrollArea()
        scroll.setWidgetResizable(True)

        widget = QWidget()
        layout = QVBoxLayout()
        layout.setSpacing(0)
        layout.setContentsMargins(0, 0, 0, 0)

        # Alternating background colors
        colors = ["#ffffff", "#f5f5f5"]
        color_idx = 0

        # Sort registers by address
        sorted_registers = sorted(reg_map.registers.values(), key=lambda r: r.address)

        for register in sorted_registers:
            # Firmware handles map switching automatically
            reg_widget = RegisterWidget(
                register,
                device,
                device.address,
                self.serial,
                colors[color_idx % 2],
                self.console
            )
            layout.addWidget(reg_widget)
            self.register_widgets.append(reg_widget)
            color_idx += 1

        layout.addStretch()
        widget.setLayout(layout)
        scroll.setWidget(widget)

        # Create a nice tab name
        tab_name = self._get_tab_name_for_map(map_name, reg_map)
        device_tabs.addTab(scroll, tab_name)

    def _get_tab_name_for_map(self, map_name: str, reg_map) -> str:
        """Generate a nice tab name for a register map."""
        # Map names to friendly display names
        name_mapping = {
            'user_sub_map': 'User Sub Map',
            'user_sub_map_2': 'User Sub Map 2',
            'interrupt_vdp': 'Interrupt/VDP',
            'vpp_map': 'VPP'
        }

        friendly_name = name_mapping.get(map_name, map_name.replace('_', ' ').title())
        num_regs = len(reg_map.registers)

        return f"{friendly_name} ({num_regs})"

    def _reload_all_registers(self):
        """Reload all register values from device (triggered by Reload Registers button)."""
        if not self.serial.is_connected():
            QMessageBox.warning(self, "Warning", "Please connect to serial port first!")
            return

        if not self.register_widgets:
            QMessageBox.warning(self, "Warning", "No registers loaded!")
            return

        self.logger.info("User requested register reload")
        self.console.log_message(f"Reloading all registers...", "#0066cc")
        self._auto_read_registers()

    def _auto_read_registers(self):
        """Automatically read all register values from device (called on connect/load)."""
        if not self.serial.is_connected():
            self.logger.warning("Auto-read skipped: not connected to serial")
            return

        if not self.register_widgets:
            self.logger.warning("Auto-read skipped: no register widgets loaded")
            return

        self.logger.info(f"Starting auto-read for {len(self.register_widgets)} widgets")
        self.status_bar.showMessage("Reading registers...")
        self.console.log_message(f"Auto-reading registers...", "#0066cc")

        # Firmware handles map switching, just do the dump
        self._auto_read_registers_dump_all()

    def _auto_read_registers_dump_all(self):
        """Read configured registers using dump command for all loaded maps."""
        if not self.register_widgets:
            self.logger.warning("Dump all: no register widgets")
            return

        # Group widgets by (device_addr, map_value) to handle maps with different I2C addresses
        map_groups = {}
        for widget in self.register_widgets:
            key = (widget.device_addr, widget.map_value)
            if key not in map_groups:
                map_groups[key] = []
            map_groups[key].append(widget)

        # Convert to list of ((device_addr, map_value), widgets) tuples for sequential processing
        self._dump_map_list = list(map_groups.items())
        self._dump_current_index = 0

        self.logger.info(f"Dump all: Found {len(self._dump_map_list)} map groups with {len(self.register_widgets)} total widgets")
        for (dev_addr, map_val), widgets in self._dump_map_list:
            self.logger.info(f"  - Device 0x{dev_addr:02X}, Map 0x{map_val:02X}: {len(widgets)} widgets")

        # Connect signal to handler
        self.dump_completed.connect(self._on_dump_completed)

        # Start dumping from first map
        self._dump_next_map()

    def _dump_next_map(self):
        """Dump the next map in sequence."""
        if self._dump_current_index >= len(self._dump_map_list):
            # All maps done
            self.logger.info("Auto-read: All maps completed")
            self.status_bar.showMessage("All registers loaded")
            self.dump_completed.disconnect(self._on_dump_completed)
            return

        (device_addr, map_value), widgets = self._dump_map_list[self._dump_current_index]
        self.logger.info(f"Auto-read: Processing map {self._dump_current_index + 1}/{len(self._dump_map_list)}: device=0x{device_addr:02X}, map=0x{map_value:02X}")

        # Find min and max register addresses for this map
        reg_addresses = [w.register.address for w in widgets]
        min_addr = min(reg_addresses)
        max_addr = max(reg_addresses)
        count = max_addr - min_addr + 1

        # Get map name and use_map_switching for logging
        map_name = widgets[0].register.register_map if widgets else f"Map 0x{map_value:02X}"
        use_map_switching = widgets[0].register.use_map_switching if widgets else True

        # Log the dump command
        # Choose command format based on use_map_switching
        if not use_map_switching:
            cmd = f"d {device_addr:02X} {min_addr:02X} {count:02X}"
        else:
            cmd = f"dm {device_addr:02X} {map_value:02X} {min_addr:02X} {count:02X}"
        self.console.log_message(f"> {cmd}  [{map_name}]", "#666666")

        # Queue the dump command for this map
        self.logger.info(f"Auto-read: Queueing dump command: device=0x{device_addr:02X}, map=0x{map_value:02X}, start=0x{min_addr:02X}, count={count}")

        def on_dump_complete(values):
            """Called when dump completes (from serial worker thread)."""
            # Emit signal to process in main thread
            self.dump_completed.emit(self._dump_current_index, values if values else [])

        self.serial.read_registers_dump(
            device_addr,
            map_value,
            min_addr,
            count,
            on_dump_complete,
            use_map_switching
        )

    def _on_dump_completed(self, map_index, values):
        """Handle dump completion (runs in main thread via signal)."""
        (device_addr, map_value), widgets = self._dump_map_list[map_index]

        # Find min address for this map
        reg_addresses = [w.register.address for w in widgets]
        min_addr = min(reg_addresses)

        if values and len(values) > 0:
            # Update widgets with values
            for widget in widgets:
                reg_addr = widget.register.address
                index = reg_addr - min_addr
                if index >= 0 and index < len(values):
                    value = values[index]
                    widget.register.update_value(value)
                    widget._update_widgets_from_register()
                    widget._update_bit_display()

            # Log the received values
            values_str = ' '.join(f'{v:02X}' for v in values)
            self.console.log_message(f"  {values_str}", "#009900")
        else:
            self.console.log_message(f"  ERR", "#cc0000")

        # Continue to next map
        self._dump_current_index += 1
        self._dump_next_map()

    def _on_preset_selected(self):
        """Handle preset selection change."""
        import yaml

        preset_path = self.preset_combo.currentData()
        if preset_path:
            try:
                with open(preset_path, 'r', encoding='utf-8') as f:
                    preset_data = yaml.safe_load(f)
                description = preset_data.get('description', 'No description')
                self.status_bar.showMessage(f"Preset: {description}")
            except Exception as e:
                self.logger.error(f"Error reading preset: {e}")

    def apply_preset(self):
        """Apply selected preset to devices (diff-based: only write changed registers)."""
        import yaml

        if not self.serial.is_connected():
            QMessageBox.warning(self, "Warning", "Please connect to serial port first!")
            return

        preset_path = self.preset_combo.currentData()
        if not preset_path:
            QMessageBox.warning(self, "Warning", "Please select a preset!")
            return

        try:
            # Load preset file
            with open(preset_path, 'r', encoding='utf-8') as f:
                preset_data = yaml.safe_load(f)

            preset_name = preset_data.get('name', 'Unknown')
            self.status_bar.showMessage(f"Applying preset: {preset_name}...")

            # Build list of registers to write (only changed ones)
            write_queue = []  # List of (device_address, map_value, reg_addr, new_value, use_map_switching)

            # Debug: log how many widgets we have
            self.logger.info(f"Preset apply: Total widgets available: {len(self.register_widgets)}")
            if self.register_widgets:
                sample_widget = self.register_widgets[0]
                self.logger.info(f"Sample widget: dev_addr=0x{sample_widget.device_addr:02X}, map_val={sample_widget.map_value}, reg_addr=0x{sample_widget.register.address:02X}")

            for device_name, device_preset in preset_data.get('devices', {}).items():
                device = self.config_manager.get_device(device_name)
                if not device:
                    self.logger.warning(f"Device '{device_name}' not found, skipping")
                    continue

                for map_name, map_preset in device_preset.get('maps', {}).items():
                    # Parse map_value (hexadecimal string format: "0x00")
                    map_value = int(map_preset.get('map_value', '0x00'), 16)

                    # Get the register map
                    reg_map = None
                    if device.register_maps and map_name in device.register_maps:
                        reg_map = device.register_maps[map_name]
                    elif map_name == 'default':
                        reg_map = None  # Legacy flat structure

                    # Determine the expected device address for this map
                    # Use reg_map.i2c_address if present (for VPP), otherwise device.address
                    expected_device_addr = reg_map.i2c_address if (reg_map and reg_map.i2c_address is not None) else device.address

                    for reg_addr_str, new_value_str in map_preset.get('registers', {}).items():
                        # Parse addresses and values (hexadecimal string format: "0x42", "0x09")
                        reg_addr = int(reg_addr_str, 16)
                        new_value = int(new_value_str, 16)

                        # Get current value from widget (search all widgets for matching device+map+address)
                        current_value = None
                        use_map_switching = True  # Default

                        # Find the widget for this specific device, map, and register
                        for widget in self.register_widgets:
                            if (widget.device_addr == expected_device_addr and
                                widget.map_value == map_value and
                                widget.register.address == reg_addr):
                                current_value = widget.get_current_value()
                                use_map_switching = widget.register.use_map_switching
                                break

                        # Fallback: if widget not found, try to get from register object
                        if current_value is None:
                            if reg_map and reg_addr in reg_map.registers:
                                register = reg_map.registers[reg_addr]
                                current_value = register.current_value if hasattr(register, 'current_value') else None
                                use_map_switching = register.use_map_switching
                            elif device.registers and reg_addr in device.registers:
                                register = device.registers[reg_addr]
                                current_value = register.current_value if hasattr(register, 'current_value') else None
                                use_map_switching = getattr(register, 'use_map_switching', True)

                        # Only add to write queue if value is different (or unknown)
                        # If current_value is None (never read), we must write it because we don't know the actual device value
                        if current_value is None or current_value != new_value:
                            # Debug logging (first 5 differences only to avoid spam)
                            if len(write_queue) < 5:
                                if current_value is None:
                                    self.logger.info(f"Preset diff: {device_name} {map_name} 0x{reg_addr:02X}: current=None (widget not found), preset={new_value}, searching for dev_addr=0x{expected_device_addr:02X}, map_val={map_value}")
                                else:
                                    self.logger.info(f"Preset diff: {device_name} {map_name} 0x{reg_addr:02X}: current={current_value}, preset={new_value}, dev_addr=0x{expected_device_addr:02X}, map_val={map_value}")
                            write_queue.append((expected_device_addr, map_value, reg_addr, new_value, use_map_switching))

            if not write_queue:
                QMessageBox.information(self, "Preset Applied",
                    f"Preset '{preset_name}' is already applied (no changes needed).")
                self.status_bar.showMessage(f"No changes needed for preset: {preset_name}")
                return

            # Show confirmation with diff summary
            reply = QMessageBox.question(
                self,
                "Apply Preset",
                f"Apply preset '{preset_name}'?\n\n{len(write_queue)} register(s) will be modified.",
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
                QMessageBox.StandardButton.Yes
            )

            if reply != QMessageBox.StandardButton.Yes:
                self.status_bar.showMessage("Preset application cancelled")
                return

            # Write all changed registers sequentially, waiting for each to complete
            self._preset_write_queue = write_queue
            self._preset_write_index = 0
            self._preset_name = preset_name

            def write_next_register():
                """Write the next register in the queue."""
                if self._preset_write_index >= len(self._preset_write_queue):
                    # All done
                    self.logger.info(f"Applied preset '{self._preset_name}': wrote {len(self._preset_write_queue)} registers")
                    self.console.log_message(f"Applied preset '{self._preset_name}': wrote {len(self._preset_write_queue)} registers", "#009900")
                    self.status_bar.showMessage(f"Applied preset '{self._preset_name}': {len(self._preset_write_queue)} registers updated")
                    return

                # Get current register to write
                device_addr, map_val, reg_addr, value, use_map_switching = self._preset_write_queue[self._preset_write_index]

                # Log command to console
                if use_map_switching:
                    cmd = f"wm {device_addr:02X} {map_val:02X} {reg_addr:02X} {value:02X}"
                else:
                    cmd = f"w {device_addr:02X} {reg_addr:02X} {value:02X}"
                self.console.log_message(f"> {cmd}", "#666666")

                # Callback when write completes
                def on_write_complete(success):
                    if success:
                        self.console.log_message(f"  OK", "#009900")

                        # Update the corresponding widget immediately
                        for widget in self.register_widgets:
                            if (widget.device_addr == device_addr and
                                widget.map_value == map_val and
                                widget.register.address == reg_addr):
                                widget.read_value()
                                break
                    else:
                        self.console.log_message(f"  ERR", "#cc0000")

                    # Move to next register
                    self._preset_write_index += 1
                    write_next_register()

                # Write register with callback
                self.serial.write_register(device_addr, map_val, reg_addr, value,
                                         callback=on_write_complete, use_map_switching=use_map_switching)

            # Start writing from first register
            QTimer.singleShot(0, write_next_register)

        except Exception as e:
            self.logger.error(f"Error applying preset: {e}")
            QMessageBox.critical(self, "Error", f"Failed to apply preset: {e}")

    def run_macro(self):
        """Run selected macro."""
        if not self.serial.is_connected():
            QMessageBox.warning(self, "Warning", "Please connect to serial port first!")
            return

        macro_name = self.macro_combo.currentData()
        if not macro_name:
            QMessageBox.warning(self, "Warning", "Please select a macro!")
            return

        macro = self.macro_manager.get_macro(macro_name)
        if not macro:
            QMessageBox.warning(self, "Warning", f"Macro '{macro_name}' not found!")
            return

        self.logger.info(f"Running macro '{macro.name}' with {len(macro.commands)} commands")
        self.console.log_message(f"Running macro: {macro.name}", "#0066cc")
        self.status_bar.showMessage(f"Running macro: {macro.name}...")

        # Execute commands sequentially
        self._macro_command_queue = macro.commands.copy()
        self._macro_command_index = 0
        self._macro_name = macro.name

        def execute_next_command():
            """Execute the next command in the macro queue."""
            if self._macro_command_index >= len(self._macro_command_queue):
                # All commands executed
                self.logger.info(f"Completed macro '{self._macro_name}'")
                self.console.log_message(f"Completed macro: {self._macro_name}", "#009900")
                self.status_bar.showMessage(f"Completed macro: {self._macro_name}")
                return

            # Get current command
            cmd = self._macro_command_queue[self._macro_command_index].strip()

            # Log command to console
            self.console.log_message(f"> {cmd}", "#666666")

            # Parse and execute command
            parts = cmd.split()
            if not parts:
                self._macro_command_index += 1
                execute_next_command()
                return

            cmd_type = parts[0].lower()

            try:
                if cmd_type == 'rm' and len(parts) == 4:
                    # Read with map: rm AA MM RR
                    addr = int(parts[1], 16)
                    map_val = int(parts[2], 16)
                    reg = int(parts[3], 16)

                    def on_read(value):
                        if value is not None:
                            self.console.log_message(f"  {value:02X}", "#009900")
                        else:
                            self.console.log_message(f"  ERR", "#cc0000")
                        self._macro_command_index += 1
                        execute_next_command()

                    self.serial.read_register(addr, map_val, reg, on_read)

                elif cmd_type == 'r' and len(parts) == 3:
                    # Read without map: r AA RR
                    addr = int(parts[1], 16)
                    reg = int(parts[2], 16)

                    def on_read(value):
                        if value is not None:
                            self.console.log_message(f"  {value:02X}", "#009900")
                        else:
                            self.console.log_message(f"  ERR", "#cc0000")
                        self._macro_command_index += 1
                        execute_next_command()

                    self.serial.read_register(addr, 0x00, reg, on_read, use_map_switching=False)

                elif cmd_type == 'wm' and len(parts) == 5:
                    # Write with map: wm AA MM RR VV
                    addr = int(parts[1], 16)
                    map_val = int(parts[2], 16)
                    reg = int(parts[3], 16)
                    val = int(parts[4], 16)

                    def on_write(success):
                        if success:
                            self.console.log_message(f"  OK", "#009900")
                        else:
                            self.console.log_message(f"  ERR", "#cc0000")
                        self._macro_command_index += 1
                        execute_next_command()

                    self.serial.write_register(addr, map_val, reg, val, on_write)

                elif cmd_type == 'w' and len(parts) == 4:
                    # Write without map: w AA RR VV
                    addr = int(parts[1], 16)
                    reg = int(parts[2], 16)
                    val = int(parts[3], 16)

                    def on_write(success):
                        if success:
                            self.console.log_message(f"  OK", "#009900")
                        else:
                            self.console.log_message(f"  ERR", "#cc0000")
                        self._macro_command_index += 1
                        execute_next_command()

                    self.serial.write_register(addr, 0x00, reg, val, on_write, use_map_switching=False)

                elif cmd_type == 'dm' and len(parts) == 5:
                    # Dump with map: dm AA MM RR QQ
                    addr = int(parts[1], 16)
                    map_val = int(parts[2], 16)
                    start_reg = int(parts[3], 16)
                    count = int(parts[4], 16)

                    def on_dump(values):
                        if values and len(values) > 0:
                            output = " ".join([f"{v:02X}" for v in values])
                            self.console.log_message(f"  {output}", "#009900")
                        else:
                            self.console.log_message(f"  ERR", "#cc0000")
                        self._macro_command_index += 1
                        execute_next_command()

                    self.serial.read_registers_dump(addr, map_val, start_reg, count, on_dump)

                elif cmd_type == 'd' and len(parts) == 4:
                    # Dump without map: d AA RR QQ
                    addr = int(parts[1], 16)
                    start_reg = int(parts[2], 16)
                    count = int(parts[3], 16)

                    def on_dump(values):
                        if values and len(values) > 0:
                            output = " ".join([f"{v:02X}" for v in values])
                            self.console.log_message(f"  {output}", "#009900")
                        else:
                            self.console.log_message(f"  ERR", "#cc0000")
                        self._macro_command_index += 1
                        execute_next_command()

                    self.serial.read_registers_dump(addr, 0x00, start_reg, count, on_dump, use_map_switching=False)

                else:
                    # Invalid command
                    self.console.log_message(f"  Invalid command format", "#cc0000")
                    self._macro_command_index += 1
                    execute_next_command()

            except (ValueError, IndexError) as e:
                self.console.log_message(f"  Error: {e}", "#cc0000")
                self._macro_command_index += 1
                execute_next_command()

        # Start executing from first command
        QTimer.singleShot(0, execute_next_command)

    def save_current_as_preset(self):
        """Save current register values from ALL devices/maps as a new preset file."""
        import yaml
        from datetime import datetime

        if not self.serial.is_connected():
            QMessageBox.warning(self, "Warning", "Please connect to serial port first!")
            return

        # Create dialog
        dialog = QDialog(self)
        dialog.setWindowTitle("Save as Preset")
        dialog_layout = QVBoxLayout()

        # Name input
        name_layout = QHBoxLayout()
        name_layout.addWidget(QLabel("Name:"))
        name_input = QLineEdit()
        name_input.setPlaceholderText("My Preset")
        name_layout.addWidget(name_input)
        dialog_layout.addLayout(name_layout)

        # Description input
        desc_layout = QHBoxLayout()
        desc_layout.addWidget(QLabel("Description:"))
        desc_input = QLineEdit()
        desc_input.setPlaceholderText("Preset description")
        desc_layout.addWidget(desc_input)
        dialog_layout.addLayout(desc_layout)

        # Buttons
        btn_layout = QHBoxLayout()
        ok_btn = QPushButton("Save")
        cancel_btn = QPushButton("Cancel")
        btn_layout.addWidget(ok_btn)
        btn_layout.addWidget(cancel_btn)
        dialog_layout.addLayout(btn_layout)

        dialog.setLayout(dialog_layout)

        def on_save():
            preset_name = name_input.text().strip()
            preset_desc = desc_input.text().strip()

            if not preset_name:
                QMessageBox.warning(dialog, "Warning", "Please enter a preset name!")
                return

            try:
                # Build preset data structure with ALL devices and maps
                preset_data = {
                    'name': preset_name,
                    'description': preset_desc if preset_desc else f"Created on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
                    'devices': {}
                }

                total_registers = 0

                # Iterate through ALL loaded devices
                for device_name, device in self.config_manager.devices.items():
                    device_data = {
                        'maps': {}
                    }

                    # Iterate through all register maps for this device
                    if device.register_maps:
                        for map_name, reg_map in device.register_maps.items():
                            map_data = {
                                'map_value': f"0x{reg_map.map_value:02X}",
                                'registers': {}
                            }

                            # Get values from widgets for registers defined in this map
                            # Use reg_map.i2c_address if present (for VPP), otherwise device.address
                            expected_device_addr = reg_map.i2c_address if reg_map.i2c_address is not None else device.address

                            for addr, register in reg_map.registers.items():
                                # Find the corresponding widget
                                for widget in self.register_widgets:
                                    if (widget.device_addr == expected_device_addr and
                                        widget.map_value == reg_map.map_value and
                                        widget.register.address == addr):
                                        value = widget.get_current_value()
                                        map_data['registers'][f"0x{addr:02X}"] = f"0x{value:02X}"
                                        total_registers += 1
                                        break

                            # Only add map to preset if it has registers
                            if map_data['registers']:
                                device_data['maps'][map_name] = map_data
                    else:
                        # Legacy: flat registers (no maps) - get values from widgets
                        flat_data = {'registers': {}}
                        for widget in self.register_widgets:
                            # Only save widgets that belong to this device
                            if widget.device_addr == device.address:
                                addr = widget.register.address
                                value = widget.get_current_value()
                                flat_data['registers'][f"0x{addr:02X}"] = f"0x{value:02X}"
                                total_registers += 1

                        if flat_data['registers']:
                            device_data['maps'] = {'default': flat_data}

                    # Only add device to preset if it has maps with registers
                    if device_data['maps']:
                        preset_data['devices'][device_name] = device_data

                # Create presets directory if it doesn't exist
                presets_dir = Path(__file__).parent.parent.parent / 'presets'
                presets_dir.mkdir(exist_ok=True)

                # Save to YAML file
                preset_filename = preset_name.lower().replace(' ', '_').replace('/', '_') + '.yaml'
                preset_path = presets_dir / preset_filename

                with open(preset_path, 'w', encoding='utf-8') as f:
                    yaml.dump(preset_data, f, default_flow_style=False, sort_keys=False, allow_unicode=True)

                # Check if preset already exists in combo box and update it, otherwise add new
                existing_index = -1
                for i in range(self.preset_combo.count()):
                    if self.preset_combo.itemData(i) == str(preset_path):
                        existing_index = i
                        break

                if existing_index >= 0:
                    # Update existing preset name (path stays the same)
                    self.preset_combo.setItemText(existing_index, preset_name)
                    self.preset_combo.setCurrentIndex(existing_index)
                else:
                    # Add new preset to combo box
                    self.preset_combo.addItem(preset_name, str(preset_path))
                    self.preset_combo.setCurrentIndex(self.preset_combo.count() - 1)

                QMessageBox.information(dialog, "Success",
                    f"Preset '{preset_name}' saved with {total_registers} registers from {len(preset_data['devices'])} device(s)!\n\nFile: {preset_filename}")
                self.status_bar.showMessage(f"Saved preset: {preset_name}")
                self.logger.info(f"Saved preset to: {preset_path}")
                dialog.accept()

            except Exception as e:
                self.logger.error(f"Error saving preset: {e}")
                QMessageBox.critical(dialog, "Error", f"Failed to save preset: {e}")

        ok_btn.clicked.connect(on_save)
        cancel_btn.clicked.connect(dialog.reject)

        dialog.exec()

    def delete_preset(self):
        """Delete the currently selected preset file."""
        import yaml
        import os

        preset_path = self.preset_combo.currentData()
        if not preset_path:
            QMessageBox.warning(self, "Warning", "Please select a preset to delete!")
            return

        try:
            # Load preset to get name
            with open(preset_path, 'r', encoding='utf-8') as f:
                preset_data = yaml.safe_load(f)
            preset_name = preset_data.get('name', 'Unknown')

            # Confirm deletion
            reply = QMessageBox.question(
                self,
                "Delete Preset",
                f"Are you sure you want to delete preset '{preset_name}'?\n\nFile: {Path(preset_path).name}",
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
                QMessageBox.StandardButton.No
            )

            if reply == QMessageBox.StandardButton.Yes:
                # Delete the file
                os.remove(preset_path)

                # Remove from combo box
                current_index = self.preset_combo.currentIndex()
                self.preset_combo.removeItem(current_index)

                self.status_bar.showMessage(f"Deleted preset: {preset_name}")
                self.logger.info(f"Deleted preset file: {preset_path}")

        except FileNotFoundError:
            QMessageBox.warning(self, "Warning", "Preset file not found!")
            # Remove from combo box anyway
            current_index = self.preset_combo.currentIndex()
            self.preset_combo.removeItem(current_index)
        except Exception as e:
            self.logger.error(f"Error deleting preset: {e}")
            QMessageBox.critical(self, "Error", f"Failed to delete preset: {e}")

    def _on_console_command(self, command: str, response: str):
        """
        Handle command executed from console.

        Synchronize with register UI if applicable.

        Args:
            command: Command that was executed
            response: Response received
        """
        parts = command.split()
        if not parts or not self.current_device:
            return

        cmd_type = parts[0].lower()

        try:
            if cmd_type in ['r', 'w'] and len(parts) >= 3:
                addr = int(parts[1], 16)
                reg_addr = int(parts[2], 16)

                # Check if this is for our current device
                if addr != self.current_device.address:
                    return

                # Find the register widget
                for widget in self.register_widgets:
                    if widget.register.address == reg_addr:
                        # Trigger a refresh of this register
                        QTimer.singleShot(100, widget.read_value)
                        break

        except (ValueError, IndexError):
            pass

    def _auto_load_configs(self):
        """Automatically load and merge all config files from configs/ directory."""
        try:
            # Get the script directory
            import os
            script_dir = Path(__file__).parent.parent.parent
            configs_dir = script_dir / "configs"

            if not configs_dir.exists():
                self.logger.warning(f"Configs directory not found: {configs_dir}")
                return

            # Load all YAML files from configs directory and merge them
            self.logger.info(f"Auto-loading configs from: {configs_dir}")

            if self.config_manager.load_configs_from_directory(str(configs_dir)):
                self._rebuild_ui_from_config()

                # Get count of loaded devices
                device_count = len(self.config_manager.get_all_devices())
                device_names = ', '.join(self.config_manager.get_device_names())

                self.status_bar.showMessage(f"Loaded {device_count} device(s): {device_names}")
                self.logger.info(f"Successfully loaded {device_count} device(s)")

                # Auto-read registers if connected
                if self.serial.is_connected():
                    QTimer.singleShot(500, self._auto_read_registers)
            else:
                self.logger.error("Failed to load configs from directory")
                self.status_bar.showMessage("Error loading configuration files")

        except Exception as e:
            self.logger.error(f"Error in auto-load configs: {e}")

    def closeEvent(self, event):
        """Handle window close event."""
        # Disconnect serial port
        if self.serial.is_connected():
            self.serial.disconnect()

        self.logger.info("Application closed")
        event.accept()
