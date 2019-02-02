cmake_minimum_required(VERSION 3.2)
project(homegenie-mini)

include(CMakeListsPrivate.txt)

add_custom_target(
    PLATFORMIO_BUILD ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    PLATFORMIO_UPLOAD ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run --target upload
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    PLATFORMIO_CLEAN ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run --target clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    PLATFORMIO_TEST ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    PLATFORMIO_PROGRAM ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run --target program
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    PLATFORMIO_UPLOADFS ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run --target uploadfs
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    PLATFORMIO_UPDATE_ALL ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion update
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    PLATFORMIO_REBUILD_PROJECT_INDEX ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion init --ide clion
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(${PROJECT_NAME} ${SRC_LIST} src/io/IOManager.cpp src/io/IOManager.h src/net/HTTPServer.cpp src/net/HTTPServer.h src/net/NetManager.cpp src/net/NetManager.h src/io/Logger.cpp src/io/Logger.h src/scripting/ProgramEngine.cpp src/scripting/ProgramEngine.h src/io/env/LightSensor.cpp src/io/env/LightSensor.h src/io/env/DS18B20.cpp src/io/env/DS18B20.h src/service/api/APIRequest.cpp src/service/api/APIRequest.h src/TaskManager.cpp src/TaskManager.h src/Task.cpp src/Task.h src/net/MQTTServer.cpp src/net/MQTTServer.h src/net/mqtt/MQTTBrokerConfig.cpp src/net/mqtt/MQTTBrokerConfig.h src/io/rf/x10/X10Message.cpp src/io/rf/x10/X10Message.h src/io/sys/Diagnostics.cpp src/io/sys/Diagnostics.h src/io/IOEvent.h src/io/IOEventDomains.h src/io/IOEventPaths.h src/service/EventRouter.cpp src/service/EventRouter.h src/service/api/APIHandler.h src/service/api/X10Handler.cpp src/service/api/X10Handler.h src/Utility.cpp src/Utility.h src/service/api/HomeGenieHandler.cpp src/service/api/HomeGenieHandler.h src/service/defs.h)

include_directories(./.piolibdeps/ArduinoJson_ID64)
include_directories(./.piolibdeps/ArduinoLog_ID1532)
include_directories(./.piolibdeps/ESP8266UPnP_ID2048)
include_directories(./.piolibdeps/LinkedList_ID443)
include_directories(./.piolibdeps/WebSockets_ID549/src)
include_directories(./.piolibdeps/OneWire_ID1)
include_directories(lib/NTPClient-master)
