"""
Register and RegisterField models for handling device registers.
"""

from typing import Dict, List, Any, Optional


class RegisterField:
    """Represents a single field within a register (one or more bits)."""

    def __init__(self, config: Dict[str, Any]):
        """
        Initialize a register field from configuration.

        Args:
            config: Field configuration dictionary from YAML
        """
        self.name = config.get('name', 'Unknown')
        self.bits = sorted(config.get('bits', [0]), reverse=True)
        self.type = config.get('type', 'int')
        self.description = config.get('description', '')

        # Convert enum values keys from string/binary to int
        raw_values = config.get('values', {})
        self.values = {}
        for key, val in raw_values.items():
            if isinstance(key, str):
                # Handle binary (0b...), hex (0x...), or decimal strings
                try:
                    int_key = int(key, 0)  # Auto-detect base
                except ValueError:
                    int_key = int(key)  # Fallback to decimal
            else:
                int_key = key
            self.values[int_key] = val

        self.range = config.get('range', [0, 255])
        self.default = config.get('default', None)

    def extract(self, register_value: int) -> int:
        """
        Extract field value from register value.

        Args:
            register_value: Full 8-bit register value

        Returns:
            Extracted field value
        """
        mask = 0
        for bit in self.bits:
            mask |= (1 << bit)
        return (register_value & mask) >> min(self.bits)

    def insert(self, current_value: int, field_value: int) -> int:
        """
        Insert field value into register value.

        Args:
            current_value: Current register value
            field_value: New field value to insert

        Returns:
            Updated register value
        """
        mask = 0
        for bit in self.bits:
            mask |= (1 << bit)
        return (current_value & ~mask) | ((field_value << min(self.bits)) & mask)

    def get_bit_range_str(self) -> str:
        """
        Get string representation of bit range.

        Returns:
            String like "[7]" or "[7:4]"
        """
        if len(self.bits) == 1:
            return f"[{self.bits[0]}]"
        else:
            return f"[{max(self.bits)}:{min(self.bits)}]"

    def validate(self, value: int) -> bool:
        """
        Validate if a value is valid for this field.

        Args:
            value: Value to validate

        Returns:
            True if valid, False otherwise
        """
        if self.type == 'bool':
            return value in [0, 1]
        elif self.type == 'enum':
            return value in self.values.keys()
        elif self.type == 'int':
            return self.range[0] <= value <= self.range[1]
        return True

    def get_value_description(self, value: int) -> str:
        """
        Get human-readable description of a value.

        Args:
            value: Field value

        Returns:
            Description string
        """
        if self.type == 'bool':
            return "Enabled" if value else "Disabled"
        elif self.type == 'enum' and value in self.values:
            return self.values[value]
        elif self.type == 'int':
            return str(value)
        return "Unknown"


class Register:
    """Represents a complete register with all its fields."""

    def __init__(self, address: int, config: Dict[str, Any]):
        """
        Initialize a register from configuration.

        Args:
            address: Register I2C address
            config: Register configuration dictionary from YAML
        """
        self.address = address
        self.name = config.get('name', 'Unknown')
        self.description = config.get('description', '')
        self.readonly = config.get('readonly', False)  # Read-only registers (e.g., status registers)
        self.register_map = None  # Set by Device class to track which map this belongs to
        self.map_value = 0x00  # Set by Device class from RegisterMap.map_value
        self.i2c_address = None  # Set by Device class for maps with separate I2C address (like VPP at 0x84)
        self.use_map_switching = True  # Set by Device class - whether to use rm/wm/dm (True) or r/w/d (False)
        self.current_value = 0
        self.fields: List[RegisterField] = []

        # Parse fields
        for field_config in config.get('fields', []):
            self.fields.append(RegisterField(field_config))

    def update_value(self, value: int):
        """
        Update the current register value.

        Args:
            value: New register value (0-255)
        """
        self.current_value = value & 0xFF

    def get_field_by_name(self, name: str) -> Optional[RegisterField]:
        """
        Get a field by its name.

        Args:
            name: Field name

        Returns:
            RegisterField if found, None otherwise
        """
        for field in self.fields:
            if field.name == name:
                return field
        return None

    def get_field_value(self, field_name: str) -> Optional[int]:
        """
        Get the current value of a specific field.

        Args:
            field_name: Name of the field

        Returns:
            Field value if found, None otherwise
        """
        field = self.get_field_by_name(field_name)
        if field:
            return field.extract(self.current_value)
        return None

    def set_field_value(self, field_name: str, value: int) -> bool:
        """
        Set the value of a specific field.

        Args:
            field_name: Name of the field
            value: New field value

        Returns:
            True if successful, False otherwise
        """
        field = self.get_field_by_name(field_name)
        if field and field.validate(value):
            self.current_value = field.insert(self.current_value, value)
            return True
        return False

    def get_bits_info(self) -> List[tuple]:
        """
        Get information about which field each bit belongs to.

        Returns:
            List of tuples (bit_position, field_name, bit_value)
        """
        result = []
        for bit_pos in range(7, -1, -1):
            bit_value = (self.current_value >> bit_pos) & 1
            field_name = ""
            for field in self.fields:
                if bit_pos in field.bits:
                    field_name = field.name
                    break
            result.append((bit_pos, field_name, bit_value))
        return result

    def to_dict(self) -> Dict[str, Any]:
        """
        Convert register to dictionary representation.

        Returns:
            Dictionary with register information
        """
        return {
            'address': self.address,
            'name': self.name,
            'description': self.description,
            'value': self.current_value,
            'fields': {
                field.name: field.extract(self.current_value)
                for field in self.fields
            }
        }

    def __repr__(self) -> str:
        return f"Register(0x{self.address:02X}, {self.name}, value=0x{self.current_value:02X})"
