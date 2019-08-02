"""This module contains an OpenVPN base handler and supporting enums."""
from abc import ABCMeta, abstractmethod
from enum import IntEnum
from typing import Dict, List


class Event(IntEnum):
	UP                    = 0
	DOWN                  = 1
	ROUTE_UP              = 2
	IPCHANGE              = 3
	TLS_VERIFY            = 4
	AUTH_USER_PASS_VERIFY = 5
	CLIENT_CONNECT        = 6
	CLIENT_DISCONNECT     = 7
	LEARN_ADDRESS         = 8
	CLIENT_CONNECT_V2     = 9
	TLS_FINAL             = 10
	ENABLE_PF             = 11
	ROUTE_PREDOWN         = 12


class Result(IntEnum):
    """OpenVPN plugin result values."""
    SUCCESS  = 0
    ERROR    = 1
    DEFERRED = 2


class BaseHandler(metaclass=ABCMeta):
    """Abstract base handler class."""
    def __init__(self, args: List[str], env: Dict[str, str]):
        """Instantiates a new Handler.

        :param args: a list of arguments
        :param env: a dict of environment variables
        """
        pass

    @abstractmethod
    def handle(self, event: int, args: List[str], env: Dict[str, str]) -> int:
        """Called for handling OpenVPN events.

        :param event: the OpenVPN event ID
        :param args: a list of arguments
        :param env: a dict of environment variables
        :returns: a result integer
        """
        pass

    def shutdown(self):
        """Called when shutting down/closing the plugin."""
        pass
