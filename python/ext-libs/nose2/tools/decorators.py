"""
This module provides decorators that assist the test author to write tests.
"""


def with_setup(setup):
    """
    A decorator that sets the setup method to be executed before the test.

    It currently works only for function test cases.

    :param setup: The method to be executed before the test.
    :type setup: function
    """

    def decorator(testcase):
        testcase.setup = setup

        return testcase

    return decorator


def with_teardown(teardown):
    """
    A decorator that sets the teardown method to be after before the test.

    It currently works only for function test cases.

    :param teardown: The method to be executed after the test.
    :type teardown: function
    """

    def decorator(testcase):
        testcase.tearDownFunc = teardown

        return testcase

    return decorator
