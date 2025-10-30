"""
Configuration file management for loading and saving YAML configs.
"""

import yaml
from pathlib import Path
from typing import Dict, Any, List, Optional

from .models import Device
from .utils.logger import get_logger


class ConfigManager:
    """Manages loading and saving device configurations."""

    def __init__(self):
        """Initialize the configuration manager."""
        self.logger = get_logger(__name__)
        self.devices: Dict[str, Device] = {}
        self.config_file: Optional[Path] = None

    def load_config(self, file_path: str) -> bool:
        """
        Load configuration from YAML file.

        Args:
            file_path: Path to YAML configuration file

        Returns:
            True if successful, False otherwise
        """
        try:
            path = Path(file_path)
            if not path.exists():
                self.logger.error(f"Config file not found: {file_path}")
                return False

            with open(path, 'r', encoding='utf-8') as f:
                config = yaml.safe_load(f)

            # Parse devices
            self.devices.clear()
            for device_name, device_config in config.get('devices', {}).items():
                device = Device(device_name, device_config)
                self.devices[device_name] = device
                self.logger.info(f"Loaded device: {device_name} ({len(device.registers)} registers)")

            self.config_file = path
            return True

        except yaml.YAMLError as e:
            self.logger.error(f"YAML parsing error: {e}")
            return False
        except Exception as e:
            self.logger.error(f"Error loading config: {e}")
            return False

    def load_configs_from_directory(self, directory_path: str) -> bool:
        """
        Load and merge all YAML configuration files from a directory.

        Args:
            directory_path: Path to directory containing YAML files

        Returns:
            True if at least one config was loaded successfully, False otherwise
        """
        try:
            dir_path = Path(directory_path)
            if not dir_path.exists():
                self.logger.error(f"Config directory not found: {directory_path}")
                return False

            # Find all YAML files in directory
            config_files = sorted(list(dir_path.glob("*.yaml")) + list(dir_path.glob("*.yml")))

            if not config_files:
                self.logger.warning("No YAML config files found in directory")
                return False

            # Clear existing devices
            self.devices.clear()
            loaded_count = 0

            # Load and merge each config file
            for config_file in config_files:
                try:
                    self.logger.info(f"Loading config file: {config_file.name}")

                    with open(config_file, 'r', encoding='utf-8') as f:
                        config = yaml.safe_load(f)

                    # Parse and add devices from this file
                    for device_name, device_config in config.get('devices', {}).items():
                        if device_name in self.devices:
                            self.logger.warning(f"Device '{device_name}' already loaded, skipping duplicate from {config_file.name}")
                            continue

                        device = Device(device_name, device_config)
                        self.devices[device_name] = device
                        self.logger.info(f"  Added device: {device_name} ({len(device.registers)} registers)")
                        loaded_count += 1

                except yaml.YAMLError as e:
                    self.logger.error(f"YAML parsing error in {config_file.name}: {e}")
                    continue
                except Exception as e:
                    self.logger.error(f"Error loading {config_file.name}: {e}")
                    continue

            if loaded_count > 0:
                self.logger.info(f"Successfully loaded {loaded_count} device(s) from {len(config_files)} config file(s)")
                self.config_file = dir_path  # Store directory path
                return True
            else:
                self.logger.error("No devices were loaded successfully")
                return False

        except Exception as e:
            self.logger.error(f"Error loading configs from directory: {e}")
            return False

    def save_config(self, file_path: Optional[str] = None) -> bool:
        """
        Save current configuration to YAML file.

        Args:
            file_path: Path to save to (if None, uses current config_file)

        Returns:
            True if successful, False otherwise
        """
        try:
            if file_path:
                path = Path(file_path)
            elif self.config_file:
                path = self.config_file
            else:
                self.logger.error("No file path specified")
                return False

            # Build config structure
            config = {'devices': {}}
            for device_name, device in self.devices.items():
                device_config = {
                    'address': device.address,
                    'description': device.description,
                }

                # Add register maps
                if device.register_maps:
                    device_config['register_maps'] = {}
                    for map_name, reg_map in device.register_maps.items():
                        map_config = {
                            'description': reg_map.description,
                            'map_value': reg_map.map_value,
                            'registers': {}
                        }
                        if reg_map.address_source:
                            map_config['address_source'] = reg_map.address_source

                        # Add registers for this map
                        for addr, register in reg_map.registers.items():
                            reg_config = {
                                'name': register.name,
                                'description': register.description,
                                'fields': []
                            }

                            # Add readonly flag if present
                            if hasattr(register, 'readonly') and register.readonly:
                                reg_config['readonly'] = True

                            for field in register.fields:
                                field_config = {
                                    'name': field.name,
                                    'bits': sorted(field.bits, reverse=True),
                                    'type': field.type,
                                    'description': field.description
                                }
                                if field.type == 'enum' and field.values:
                                    field_config['values'] = field.values
                                if field.type == 'int' and field.range:
                                    field_config['range'] = field.range
                                if field.default is not None:
                                    field_config['default'] = field.default
                                if hasattr(field, 'readonly') and field.readonly:
                                    field_config['readonly'] = True

                                reg_config['fields'].append(field_config)

                            map_config['registers'][f"0x{addr:02X}"] = reg_config

                        device_config['register_maps'][map_name] = map_config
                else:
                    # Legacy: flat registers structure
                    device_config['registers'] = {}
                    for addr, register in device.registers.items():
                        reg_config = {
                            'name': register.name,
                            'description': register.description,
                            'fields': []
                        }

                        for field in register.fields:
                            field_config = {
                                'name': field.name,
                                'bits': sorted(field.bits, reverse=True),
                                'type': field.type,
                                'description': field.description
                            }
                            if field.type == 'enum' and field.values:
                                field_config['values'] = field.values
                            if field.type == 'int':
                                field_config['range'] = field.range
                            if field.default is not None:
                                field_config['default'] = field.default

                            reg_config['fields'].append(field_config)

                        device_config['registers'][f"0x{addr:02X}"] = reg_config

                # Presets are now saved in the presets/ directory, not in config files
                # Macros are now saved in the macros/ directory, not in config files

                config['devices'][device_name] = device_config

            # Write to file
            with open(path, 'w', encoding='utf-8') as f:
                yaml.dump(config, f, default_flow_style=False, sort_keys=False, allow_unicode=True)

            self.logger.info(f"Saved config to: {path}")
            return True

        except Exception as e:
            self.logger.error(f"Error saving config: {e}")
            return False

    def get_device(self, name: str) -> Optional[Device]:
        """
        Get a device by name.

        Args:
            name: Device name

        Returns:
            Device if found, None otherwise
        """
        return self.devices.get(name)

    def get_all_devices(self) -> List[Device]:
        """
        Get all loaded devices.

        Returns:
            List of devices
        """
        return list(self.devices.values())

    def add_device(self, device: Device):
        """
        Add a new device.

        Args:
            device: Device to add
        """
        self.devices[device.name] = device

    def has_devices(self) -> bool:
        """Check if any devices are loaded."""
        return len(self.devices) > 0

    def clear(self):
        """Clear all loaded devices and configuration."""
        self.devices.clear()
        self.config_file = None
        self.logger.info("Configuration cleared")

    def get_device_names(self) -> List[str]:
        """Get list of all device names."""
        return list(self.devices.keys())

    def export_preset_from_device(self, device_name: str, preset_name: str,
                                  register_values: Dict[int, int]) -> bool:
        """
        Create a new preset from current register values.

        Args:
            device_name: Device name
            preset_name: Name for the new preset
            register_values: Dictionary of register address -> value

        Returns:
            True if successful, False otherwise
        """
        device = self.get_device(device_name)
        if not device:
            return False

        from .models import Preset

        preset_key = preset_name.lower().replace(' ', '_')
        preset_config = {
            'name': preset_name,
            'description': f"Exported preset: {preset_name}",
            'registers': {
                f"0x{addr:02X}": value
                for addr, value in register_values.items()
            }
        }

        preset = Preset(preset_key, preset_config)
        device.add_preset(preset)

        return True
