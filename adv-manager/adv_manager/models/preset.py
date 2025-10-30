"""
Preset and Macro models for storing and executing register configurations.
"""

from typing import Dict, Any, List


class Preset:
    """Represents a saved configuration of register values."""

    def __init__(self, key: str, config: Dict[str, Any]):
        """
        Initialize a preset from configuration.

        Args:
            key: Preset key/identifier
            config: Preset configuration from YAML
        """
        self.key = key
        self.name = config.get('name', key)
        self.description = config.get('description', '')
        self.registers: Dict[int, int] = {}

        # Parse register values
        for reg_addr, value in config.get('registers', {}).items():
            if isinstance(reg_addr, str):
                reg_addr_int = int(reg_addr, 0)
            else:
                reg_addr_int = reg_addr
            self.registers[reg_addr_int] = value

    def get_register_value(self, address: int) -> int:
        """
        Get the preset value for a specific register.

        Args:
            address: Register address

        Returns:
            Register value, or None if not in preset
        """
        return self.registers.get(address)

    def to_dict(self) -> Dict[str, Any]:
        """
        Convert preset to dictionary for saving to YAML.

        Returns:
            Dictionary representation
        """
        return {
            'name': self.name,
            'description': self.description,
            'registers': {
                f"0x{addr:02X}": value
                for addr, value in self.registers.items()
            }
        }

    def __repr__(self) -> str:
        return f"Preset({self.name}, registers={len(self.registers)})"
