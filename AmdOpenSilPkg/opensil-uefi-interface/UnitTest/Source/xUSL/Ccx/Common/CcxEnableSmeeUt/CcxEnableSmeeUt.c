/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved. */
/**
 * @file CcxEnableSmeeUt.c
 * @brief Unit tests for CcxEnableSmee function
 *
 */


#include "CcxEnableSmeeUt.h"

AMD_UNIT_TEST_STATUS
EFIAPI
TestPrerequisite (
  IN AMD_UNIT_TEST_CONTEXT Context
  )
{
  return AMD_UNIT_TEST_PASSED;
}

uint64_t xUslRdMsr (
  uint32_t MsrAddress
  )
{
  return 0;
}

bool xUslIsComputeUnitPrimary (
  void
  )
{
  return TRUE;
}

void
EFIAPI
TestBody (
  IN AMD_UNIT_TEST_CONTEXT Context
  )
{
  AMD_UNIT_TEST_FRAMEWORK *Ut = (AMD_UNIT_TEST_FRAMEWORK*) UtGetActiveFrameworkHandle ();
  const char* TestName        = UtGetTestName (Ut);
  const char* IterationName   = UtGetTestIteration (Ut);

  bool SmeeEnable = 0;

  Ut->Log(AMD_UNIT_TEST_LOG_INFO, __FUNCTION__, __LINE__,
    "%s (Iteration: %s) Test started.", TestName, IterationName);

  if (strcmp(IterationName, "SmeeEnableTest") == 0) {
    SmeeEnable = 1;

    // Act
    Ut->Log(AMD_UNIT_TEST_LOG_DEBUG, __FUNCTION__, __LINE__,
      "Call CcxEnableSmee");
    CcxEnableSmee (SmeeEnable);

    // Assert
    //
    // There is no status returned from CcxEnableSmee.
    //
  } else if (strcmp(IterationName, "SmeeEnableFalse") == 0) {
    // Arrange
    SmeeEnable = 0;

    // Act
    Ut->Log(AMD_UNIT_TEST_LOG_DEBUG, __FUNCTION__, __LINE__,
      "Call CcxEnableSmee");
    CcxEnableSmee (SmeeEnable);

    // Assert
    //
    // There is no status returned from CcxEnableSmee.
    //
  } else {
    Ut->Log(AMD_UNIT_TEST_LOG_ERROR, __FUNCTION__, __LINE__, "Iteration '%s' is not implemented.", IterationName);
    UtSetTestStatus (Ut, AMD_UNIT_TEST_ABORTED);
  }

  UtSetTestStatus (Ut, AMD_UNIT_TEST_PASSED);
  Ut->Log(AMD_UNIT_TEST_LOG_INFO, __FUNCTION__, __LINE__, "%s (Iteration: %s) Test ended.", TestName, IterationName);
}

AMD_UNIT_TEST_STATUS
EFIAPI
TestCleanUp (
  IN AMD_UNIT_TEST_CONTEXT Context
  )
{
  return AMD_UNIT_TEST_PASSED;
}

/**

 * main
 * @brief      Stating point for Excecution
 *
 * @details    This routine:
 *              - Handles the command line arguments.
 *                Example: CcxEnableSmeeUt.exe -o "E:\test" -i "SmeeEnableTest"
 *              - Declares the unit test framwork.
 *              - Run the tests.
 *              - Deallocate the Unit test famework.
 *
 * @param    argc                     Argument count
 * @param    *argv[]                  Argument vector
 *
 * @retval   AMD_UNIT_TEST_PASSED     Function succeed
 * @retval   NON-ZERO                 Error occurs
 */
int
main (
  int   argc,
  char  *argv[]
  )
{
  AMD_UNIT_TEST_STATUS     Status;
  AMD_UNIT_TEST_FRAMEWORK  Ut;

  // Initializing the UnitTest framework
  Status = UtInitFromArgs (
    &Ut,
    argc,
    argv
  );
  if (Status != AMD_UNIT_TEST_PASSED) {
    return Status;
  }

  // Logging the start of the test.
  Ut.Log(AMD_UNIT_TEST_LOG_INFO, __FUNCTION__, __LINE__,
    "Test %s started. TestStatus is %s.", UtGetTestName (&Ut), UtGetTestStatusString (&Ut));

  // Running test.
  Ut.Log(AMD_UNIT_TEST_LOG_INFO, __FUNCTION__, __LINE__, "Running test.");
  UtRunTest (&Ut);

  // Freeing up all framework related allocated memories
  Ut.Log(AMD_UNIT_TEST_LOG_INFO, __FUNCTION__, __LINE__,
    "Test %s ended.", UtGetTestName (&Ut));
  UtDeinit (&Ut);

  return AMD_UNIT_TEST_PASSED;
}
