/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTREGEX2_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTREGEX2_H_

#include "fw_dunit.hpp"
#include <ace/OS.h>
#include <ace/High_Res_Timer.h>
#include <string>

#define ROOT_NAME "ThinClientRegex2"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;

CacheHelper* cacheHelper = nullptr;
bool isLocalServer = false;

static bool isLocator = false;
static int numberOfLocators = 0;
const std::string locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);
#include "LocatorHelper.hpp"
void initClient(const bool isthinClient) {
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(isthinClient);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}
void cleanProc() {
  if (cacheHelper != nullptr) {
    delete cacheHelper;
    cacheHelper = nullptr;
  }
}

CacheHelper* getHelper() {
  ASSERT(cacheHelper != nullptr, "No cacheHelper initialized.");
  return cacheHelper;
}

void _verifyEntry(const char* name, const char* key, const char* val,
                  bool noKey, bool isCreated = false) {
  // Verify key and value exist in this region, in this process.
  const char* value = val ? val : "";
  std::string msg;
  if (noKey) {
    msg =
        std::string("Verify key ") + key + " does not exist in region " + name;
  } else if (!val) {
    msg = std::string("Verify value for key ") + key +
          " does not exist in region " + name;
  } else {
    msg = std::string("Verify value for key ") + key + " is: " + value +
          " in region " + name;
  }
  LOG(msg);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  auto keyPtr = CacheableKey::create(key);

  // if the region is no ack, then we may need to wait...
  if (!isCreated) {
    if (noKey == false) {  // need to find the key!
      ASSERT(regPtr->containsKey(keyPtr), "Key not found in region.");
    }
    if (val != nullptr) {  // need to have a value!
      ASSERT(regPtr->containsValueForKey(keyPtr), "Value not found in region.");
    }
  }

  // loop up to MAX times, testing condition
  uint32_t MAX = 100;
  //  changed sleep from 10 ms
  uint32_t SLEEP = 1000;  // milliseconds
  uint32_t containsKeyCnt = 0;
  uint32_t containsValueCnt = 0;
  uint32_t testValueCnt = 0;

  for (int i = MAX; i >= 0; i--) {
    if (isCreated) {
      if (!regPtr->containsKey(keyPtr)) {
        containsKeyCnt++;
      } else {
        break;
      }
      ASSERT(containsKeyCnt < MAX, "Key has not been created in region.");
    } else {
      if (noKey) {
        if (regPtr->containsKey(keyPtr)) {
          containsKeyCnt++;
        } else {
          break;
        }
        ASSERT(containsKeyCnt < MAX, "Key found in region.");
      }
      if (val == nullptr) {
        if (regPtr->containsValueForKey(keyPtr)) {
          containsValueCnt++;
        } else {
          break;
        }
        ASSERT(containsValueCnt < MAX, "Value found in region.");
      }

      if (val != nullptr) {
        auto checkPtr =
            std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr));

        ASSERT(checkPtr != nullptr, "Value Ptr should not be null.");
        LOG("In verify loop, get returned " + checkPtr->value() + " for key " +
            key);
        if (strcmp(checkPtr->value().c_str(), value) != 0) {
          testValueCnt++;
        } else {
          break;
        }
        ASSERT(testValueCnt < MAX, "Incorrect value found.");
      }
    }
    dunit::sleep(SLEEP);
  }
}

#define verifyEntry(x, y, z) _verifyEntry(x, y, z, __LINE__)

void _verifyEntry(const char* name, const char* key, const char* val,
                  int line) {
  LOG(std::string("verifyEntry() called from ") + std::to_string(line) + "\n");
  _verifyEntry(name, key, val, false);
  LOG("Entry verified.");
}

void createRegion(const char* name, bool ackMode,
                  bool clientNotificationEnabled = false) {
  LOG("createRegion() entered.");
  std::cout << "Creating region --  " << name << " ackMode is " << ackMode
            << "\n"
            << std::flush;
  auto regPtr = getHelper()->createRegion(name, ackMode, true, nullptr,
                                          clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}
void createPooledRegion(const std::string& name, bool ackMode,
                        const std::string& locators,
                        const std::string& poolname,
                        bool clientNotificationEnabled = false,
                        bool cachingEnable = true) {
  LOG("createRegion_Pool() entered.");
  std::cout << "Creating region --  " << name << " ackMode is " << ackMode
            << "\n"
            << std::flush;
  auto regPtr =
      getHelper()->createPooledRegion(name, ackMode, locators, poolname,
                                      cachingEnable, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Pooled Region created.");
}
void createEntry(const char* name, const char* key,
                 const char* value = nullptr) {
  LOG("createEntry() entered.");
  std::cout << "Creating entry -- key: " << key << " value: " << value
            << " in region " << name << "\n"
            << std::flush;
  // Create entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  if (value == nullptr) {
    value = "";
  }
  auto valPtr = CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(!regPtr->containsKey(keyPtr),
         "Key should not have been found in region.");
  ASSERT(!regPtr->containsValueForKey(keyPtr),
         "Value should not have been found in region.");

  regPtr->create(keyPtr, valPtr);
  // regPtr->put( keyPtr, valPtr );
  LOG("Created entry.");

  verifyEntry(name, key, value);
  LOG("Entry created.");
}

void updateEntry(const char* name, const char* key, const char* value) {
  LOG("updateEntry() entered.");
  std::cout << "Updating entry -- key: " << key << " value: " << value
            << " in region " << name << "\n"
            << std::flush;
  // Update entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(regPtr->containsKey(keyPtr), "Key should have been found in region.");
  ASSERT(regPtr->containsValueForKey(keyPtr),
         "Value should have been found in region.");

  regPtr->put(keyPtr, valPtr);
  LOG("Put entry.");

  verifyEntry(name, key, value);
  LOG("Entry updated.");
}

void doNetsearch(const char* name, const char* key, const char* value) {
  LOG("doNetsearch() entered.");
  std::cout << "Netsearching for entry -- key: " << key
            << " expecting value: " << value << " in region " << name << "\n"
            << std::flush;
  // Get entry created in Process A, verify entry is correct
  auto keyPtr = CacheableKey::create(key);

  auto regPtr = getHelper()->getRegion(name);
  std::cout << "netsearch region " << regPtr->getName() << "\n" << std::flush;

  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(!regPtr->containsKey(keyPtr),
         "Key should not have been found in region.");
  ASSERT(!regPtr->containsValueForKey(keyPtr),
         "Value should not have been found in region.");

  auto checkPtr = std::dynamic_pointer_cast<CacheableString>(
      regPtr->get(keyPtr));  // force a netsearch

  if (checkPtr != nullptr) {
    LOG("checkPtr is not null");
    LOG(std::string("In net search, get returned ") + checkPtr->value() +
        " for key " + key);
  } else {
    LOG("checkPtr is nullptr");
  }
  verifyEntry(name, key, value);
  LOG("Netsearch complete.");
}

const char* regex23 = "Key-[2-3]";
const char* regexWildcard = "Key-.*";
const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char* regionNames[] = {"DistRegionAck", "DistRegionNoAck"};
const char* vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char* nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    LOG("Starting SERVER1...");
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_notify_subscription.xml");
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepOne_Pool_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_",
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TESTPOOL1_", true);
    LOG("StepOne_Pool complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwo_Pool_Locator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TESTPOOL1_",
                       true);
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TESTPOOL1_", true);
    LOG("StepTwo_Pool complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    regPtr0->registerRegex(regex23);
    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepFour)
  {
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);
    regPtr1->registerRegex(regexWildcard);
    LOG("StepFour complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFive)
  {
    createEntry(regionNames[1], keys[0], vals[0]);
    createEntry(regionNames[1], keys[1], vals[1]);
    createEntry(regionNames[1], keys[2], vals[2]);
    createEntry(regionNames[1], keys[3], vals[3]);
    LOG("StepFive complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT2, StepSix)
  {
    createEntry(regionNames[0], keys[0], vals[0]);
    createEntry(regionNames[0], keys[1], vals[1]);
    createEntry(regionNames[0], keys[2], vals[2]);
    createEntry(regionNames[0], keys[3], vals[3]);

    SLEEP(1000);  // sleep for sometime to let updates come

    verifyEntry(regionNames[1], keys[0], vals[0]);
    verifyEntry(regionNames[1], keys[1], vals[1]);
    verifyEntry(regionNames[1], keys[2], vals[2]);
    verifyEntry(regionNames[1], keys[3], vals[3]);

    LOG("StepSix complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT1, StepSeven)
  {
    SLEEP(1000);  // sleep for sometime to let updates come

    verifyEntry(regionNames[0], keys[1], vals[1]);
    verifyEntry(regionNames[0], keys[2], vals[2]);

    doNetsearch(regionNames[0], keys[0], vals[0]);
    doNetsearch(regionNames[0], keys[3], vals[3]);

    updateEntry(regionNames[1], keys[0], nvals[0]);
    updateEntry(regionNames[1], keys[1], nvals[1]);
    updateEntry(regionNames[1], keys[2], nvals[2]);
    updateEntry(regionNames[1], keys[3], nvals[3]);

    LOG("StepSeven complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepEight)
  {
    SLEEP(1000);  // sleep for sometime to let updates come

    verifyEntry(regionNames[1], keys[0], nvals[0]);
    verifyEntry(regionNames[1], keys[1], nvals[1]);
    verifyEntry(regionNames[1], keys[2], nvals[2]);
    verifyEntry(regionNames[1], keys[3], nvals[3]);

    updateEntry(regionNames[0], keys[0], nvals[0]);
    updateEntry(regionNames[0], keys[1], nvals[1]);
    updateEntry(regionNames[0], keys[2], nvals[2]);
    updateEntry(regionNames[0], keys[3], nvals[3]);

    LOG("StepEight complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepNine)
  {
    SLEEP(1000);  // sleep for sometime to let updates come

    verifyEntry(regionNames[0], keys[0], vals[0]);
    verifyEntry(regionNames[0], keys[1], nvals[1]);
    verifyEntry(regionNames[0], keys[2], nvals[2]);
    verifyEntry(regionNames[0], keys[3], vals[3]);

    LOG("StepNine complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  {
    LOG("cleanProc 1...");
    cleanProc();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseCache2)
  {
    LOG("cleanProc 2...");
    cleanProc();
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer1)
  {
    LOG("closing Server1...");
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK_DEFINITION

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTREGEX2_H_
