"""
Logging utility for the application.
"""

import logging
import sys
from pathlib import Path
from datetime import datetime


def get_logger(name: str = "adv_manager", level: int = logging.INFO) -> logging.Logger:
    """
    Get or create a logger instance.

    Args:
        name: Logger name
        level: Logging level

    Returns:
        Configured logger instance
    """
    logger = logging.getLogger(name)

    # Only configure if not already configured
    if not logger.handlers:
        logger.setLevel(level)

        # Console handler
        console_handler = logging.StreamHandler(sys.stdout)
        console_handler.setLevel(level)
        console_formatter = logging.Formatter(
            '%(asctime)s - %(name)s - %(levelname)s - %(message)s',
            datefmt='%H:%M:%S'
        )
        console_handler.setFormatter(console_formatter)
        logger.addHandler(console_handler)

        # File handler (optional - can be enabled later)
        # log_dir = Path.home() / '.gbsc_pro' / 'logs'
        # log_dir.mkdir(parents=True, exist_ok=True)
        # log_file = log_dir / f'register_editor_{datetime.now():%Y%m%d}.log'
        # file_handler = logging.FileHandler(log_file)
        # file_handler.setLevel(logging.DEBUG)
        # file_formatter = logging.Formatter(
        #     '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
        # )
        # file_handler.setFormatter(file_formatter)
        # logger.addHandler(file_handler)

    return logger


class OperationLogger:
    """Context manager for logging operations."""

    def __init__(self, logger: logging.Logger, operation: str):
        """
        Initialize operation logger.

        Args:
            logger: Logger instance
            operation: Operation description
        """
        self.logger = logger
        self.operation = operation
        self.start_time = None

    def __enter__(self):
        self.start_time = datetime.now()
        self.logger.info(f"Starting: {self.operation}")
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        elapsed = (datetime.now() - self.start_time).total_seconds()
        if exc_type is None:
            self.logger.info(f"Completed: {self.operation} ({elapsed:.3f}s)")
        else:
            self.logger.error(f"Failed: {self.operation} - {exc_val}")
        return False
