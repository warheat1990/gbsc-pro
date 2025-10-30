"""
Macro manager for loading and managing serial command macros.
"""

import yaml
from pathlib import Path
from typing import Dict, List, Optional
import logging


class Macro:
    """Represents a macro - a sequence of serial commands."""

    def __init__(self, name: str, filepath: Path, commands: List[str]):
        """
        Initialize a macro.

        Args:
            name: Macro name (from filename)
            filepath: Path to the YAML file
            commands: List of serial commands (e.g., ["r 42 00", "w 42 00 01"])
        """
        self.name = name
        self.filepath = filepath
        self.commands = commands

    def __repr__(self) -> str:
        return f"Macro({self.name}, {len(self.commands)} commands)"


class MacroManager:
    """Manages loading and execution of macros from YAML files."""

    def __init__(self, macros_dir: Optional[Path] = None):
        """
        Initialize the macro manager.

        Args:
            macros_dir: Directory containing macro YAML files.
                       If None, uses 'macros/' relative to project root.
        """
        self.logger = logging.getLogger(__name__)

        if macros_dir is None:
            # Default to macros/ directory at project root
            self.macros_dir = Path(__file__).parent.parent / 'macros'
        else:
            self.macros_dir = macros_dir

        self.macros: Dict[str, Macro] = {}

    def load_macros(self) -> int:
        """
        Load all macros from the macros directory.

        Returns:
            Number of macros loaded
        """
        self.macros.clear()

        if not self.macros_dir.exists():
            self.logger.warning(f"Macros directory not found: {self.macros_dir}")
            return 0

        # Find all YAML files
        macro_files = sorted(list(self.macros_dir.glob("*.yaml")) + list(self.macros_dir.glob("*.yml")))

        for macro_file in macro_files:
            try:
                with open(macro_file, 'r', encoding='utf-8') as f:
                    data = yaml.safe_load(f)

                if not data:
                    self.logger.warning(f"Empty macro file: {macro_file.name}")
                    continue

                # Extract name and commands
                name = data.get('name', macro_file.stem)
                commands = data.get('commands', [])

                if not commands:
                    self.logger.warning(f"No commands in macro file: {macro_file.name}")
                    continue

                # Create macro
                macro = Macro(name, macro_file, commands)
                self.macros[name] = macro
                self.logger.info(f"Loaded macro: {name} ({len(commands)} commands)")

            except Exception as e:
                self.logger.error(f"Error loading macro {macro_file.name}: {e}")

        self.logger.info(f"Loaded {len(self.macros)} macro(s)")
        return len(self.macros)

    def get_macro(self, name: str) -> Optional[Macro]:
        """
        Get a macro by name.

        Args:
            name: Macro name

        Returns:
            Macro if found, None otherwise
        """
        return self.macros.get(name)

    def get_all_macros(self) -> Dict[str, Macro]:
        """
        Get all loaded macros.

        Returns:
            Dictionary of macro name -> Macro
        """
        return self.macros

    def get_macro_names(self) -> List[str]:
        """
        Get list of all macro names.

        Returns:
            List of macro names
        """
        return list(self.macros.keys())
