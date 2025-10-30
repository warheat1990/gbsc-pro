"""
Device model for ADV7280/ADV7391 video processors.
"""

from typing import Dict, List, Any, Optional
from .register import Register
from .preset import Preset


class AccessSequence:
    """Represents a sequence of register operations needed to access special registers."""

    def __init__(self, name: str, config: Dict[str, Any]):
        """
        Initialize an access sequence.

        Args:
            name: Sequence name
            config: Sequence configuration from YAML
        """
        self.name = name
        self.description = config.get('description', '')
        self.enable_steps = config.get('enable', [])
        self.disable_steps = config.get('disable', [])

    def __repr__(self) -> str:
        return f"AccessSequence({self.name})"


class RegisterMap:
    """Represents a register map (e.g., user_sub_map, user_sub_map_2)."""

    def __init__(self, name: str, config: Dict[str, Any]):
        """
        Initialize a register map.

        Args:
            name: Map name (e.g., 'user_sub_map')
            config: Map configuration from YAML
        """
        self.name = name
        self.description = config.get('description', '')

        # Map value for firmware command (0x00, 0x20, 0x40, etc.)
        # If not specified, defaults to 0x00
        map_value_config = config.get('map_value', None)
        if map_value_config is not None:
            self.map_value = int(map_value_config, 0) if isinstance(map_value_config, str) else map_value_config
        else:
            self.map_value = 0x00

        # I2C address for maps that use a separate I2C device (like VPP)
        # If specified, commands use this address instead of device.address
        i2c_addr_config = config.get('i2c_address', None)
        if i2c_addr_config is not None:
            self.i2c_address = int(i2c_addr_config, 0) if isinstance(i2c_addr_config, str) else i2c_addr_config
        else:
            self.i2c_address = None

        # Address source (for dynamic I2C addresses like VPP map)
        self.address_source = config.get('address_source', None)

        # Use map switching - determines if firmware should switch register 0x0E before/after commands
        # Default is True (use map switching). Set to False for maps that don't need it (like VPP, ADV7391)
        self.use_map_switching = config.get('use_map_switching', True)

        self.registers: Dict[int, Register] = {}

        # Parse registers in this map
        for reg_addr, reg_config in config.get('registers', {}).items():
            if isinstance(reg_addr, str):
                reg_addr_int = int(reg_addr, 0)
            else:
                reg_addr_int = reg_addr
            self.registers[reg_addr_int] = Register(reg_addr_int, reg_config)

    def __repr__(self) -> str:
        return f"RegisterMap({self.name}, {len(self.registers)} registers, map_value=0x{self.map_value:02X})"


class Device:
    """Represents a video processor device (ADV7280 or ADV7391)."""

    def __init__(self, name: str, config: Dict[str, Any]):
        """
        Initialize a device from configuration.

        Args:
            name: Device name (e.g., 'ADV7280')
            config: Device configuration dictionary from YAML
        """
        self.name = name
        self.description = config.get('description', '')

        # Parse address - support both decimal (66) and hexadecimal ("0x42") formats
        address_config = config.get('address', 0x42)
        if isinstance(address_config, str):
            self.address = int(address_config, 0)  # int(x, 0) auto-detects base
        else:
            self.address = address_config
        self.registers: Dict[int, Register] = {}  # Flat view of all registers
        self.register_maps: Dict[str, RegisterMap] = {}  # Register maps
        self.presets: Dict[str, Preset] = {}
        # Macros are now loaded from macros/ directory, not from device config

        # Parse register maps
        if 'register_maps' in config:
            for map_name, map_config in config['register_maps'].items():
                reg_map = RegisterMap(map_name, map_config)
                self.register_maps[map_name] = reg_map

                # Add metadata to registers
                for addr, register in reg_map.registers.items():
                    register.register_map = map_name
                    register.map_value = reg_map.map_value
                    register.i2c_address = reg_map.i2c_address  # None for main device, or specific address like 0x84 for VPP
                    register.use_map_switching = reg_map.use_map_switching  # Whether to use rm/wm/dm (True) or r/w/d (False)

                # Also add to flat registers dict for backward compatibility
                # Only add if not already present (first map wins for overlapping addresses)
                for addr, register in reg_map.registers.items():
                    if addr not in self.registers:
                        self.registers[addr] = register
        else:
            # Legacy: Parse flat registers structure (old format)
            for reg_addr, reg_config in config.get('registers', {}).items():
                if isinstance(reg_addr, str):
                    reg_addr_int = int(reg_addr, 0)
                else:
                    reg_addr_int = reg_addr
                register = Register(reg_addr_int, reg_config)
                register.register_map = 'user_sub_map'  # Default map
                self.registers[reg_addr_int] = register

        # Presets are now loaded from the presets/ directory, not from config files

        # Macros are now loaded from macros/ directory, not from device config

    def get_register(self, address: int) -> Optional[Register]:
        """
        Get a register by its address.

        Args:
            address: Register address

        Returns:
            Register if found, None otherwise
        """
        return self.registers.get(address)

    def get_access_sequence(self, name: str) -> Optional[AccessSequence]:
        """
        Get an access sequence by name.

        Args:
            name: Sequence name

        Returns:
            AccessSequence if found, None otherwise
        """
        return self.access_sequences.get(name)

    def get_map_value_for_map(self, map_name: Optional[str]) -> int:
        """
        Get the map value (for firmware command) from a register map name.

        Args:
            map_name: Register map name (e.g., 'user_sub_map', 'user_sub_map_2', 'vpp_map')

        Returns:
            Map value: 0x00 for user_sub_map, 0x40 for user_sub_map_2, etc.
        """
        if not map_name:
            return 0x00  # Default: User Sub Map

        # Get the register map
        reg_map = self.register_maps.get(map_name)
        if reg_map:
            return reg_map.map_value

        return 0x00  # Default to User Sub Map if unknown

    def get_preset(self, name: str) -> Optional[Preset]:
        """
        Get a preset by name.

        Args:
            name: Preset name

        Returns:
            Preset if found, None otherwise
        """
        return self.presets.get(name)

    # Macro methods removed - macros are now managed globally from macros/ directory

    def get_register_map(self, name: str) -> Optional['RegisterMap']:
        """
        Get a register map by name.

        Args:
            name: Register map name (e.g., 'user_sub_map', 'user_sub_map_2')

        Returns:
            RegisterMap if found, None otherwise
        """
        return self.register_maps.get(name)

    def add_preset(self, preset: Preset):
        """
        Add a new preset to the device.

        Args:
            preset: Preset to add
        """
        self.presets[preset.key] = preset

    def update_register_value(self, address: int, value: int):
        """
        Update a register's current value.

        Args:
            address: Register address
            value: New value (0-255)
        """
        register = self.get_register(address)
        if register:
            register.update_value(value)

    def get_all_registers_sorted(self) -> List[Register]:
        """
        Get all registers sorted by address.

        Returns:
            List of registers sorted by address
        """
        return sorted(self.registers.values(), key=lambda r: r.address)

    def to_dict(self) -> Dict[str, Any]:
        """
        Convert device to dictionary representation.

        Returns:
            Dictionary with device information
        """
        result = {
            'name': self.name,
            'description': self.description,
            'address': self.address,
            'presets': list(self.presets.keys()),
            # Macros no longer stored in device config
        }

        # Include register maps if present
        if self.register_maps:
            result['register_maps'] = {
                map_name: {
                    'description': reg_map.description,
                    'map_value': f"0x{reg_map.map_value:02X}",
                    'num_registers': len(reg_map.registers),
                    'address_source': reg_map.address_source if reg_map.address_source else 'fixed'
                }
                for map_name, reg_map in self.register_maps.items()
            }
        else:
            # Legacy format
            result['registers'] = {
                f"0x{addr:02X}": reg.to_dict()
                for addr, reg in self.registers.items()
            }

        return result

    def __repr__(self) -> str:
        if self.register_maps:
            maps_info = ', '.join([f"{name}:{len(m.registers)}" for name, m in self.register_maps.items()])
            return f"Device({self.name}, address=0x{self.address:02X}, maps=[{maps_info}])"
        else:
            return f"Device({self.name}, address=0x{self.address:02X}, registers={len(self.registers)})"
