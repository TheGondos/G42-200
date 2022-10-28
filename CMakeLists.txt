add_library(G422 SHARED
    Sources/G422/Sources/G422_ANIMATIONS.cpp
    Sources/G422/Sources/G422_AUX.cpp
    Sources/G422/Sources/G422_DVC_DISPLAYS.cpp
    Sources/G422/Sources/G422_DVC_SETUP.cpp
    Sources/G422/Sources/G422_ENG_APUSYS.cpp
    Sources/G422/Sources/G422_ENG_OMS.cpp
    Sources/G422/Sources/G422_ENG_RAMCASTER.cpp
    Sources/G422/Sources/G422_ENG_RT66.cpp
    Sources/G422/Sources/G422_OAPI.cpp
    Sources/G422/Sources/G422_SCENARIO_RW.cpp
    Sources/G422/Sources/G422_SIM_CONTROLS.cpp
    Sources/G422/Sources/G422_SIM_DYNAMICS.cpp
    Sources/G422/Sources/G422_ENG_RCS.cpp
    Sources/G422/Sources/G422_VSL_SETUP.cpp
    Sources/G422/Sources/G422_VSL_UPDATE.cpp
)

add_dependencies(G422
	${OrbiterTgt}
	Orbitersdk
	UCSO_API
)

install(TARGETS G422
#	RUNTIME
	DESTINATION ${ORBITER_INSTALL_MODULE_DIR}
)

target_include_directories(G422 PUBLIC
	${ORBITER_SOURCE_SDK_INCLUDE_DIR}
    ${ORBITER_BINARY_SDK_DIR}/include
    ${ORBITER_BINARY_SDK_DIR}/XRSound
)

target_link_libraries(G422
	${ORBITER_SDK_LIB}
    UCSO_API
    XRSound_dll
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Orbiter/Config
    DESTINATION ${ORBITER_INSTALL_ROOT_DIR}/
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Orbiter/Meshes
    DESTINATION ${ORBITER_INSTALL_ROOT_DIR}/
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Orbiter/Scenarios
    DESTINATION ${ORBITER_INSTALL_ROOT_DIR}/
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Orbiter/Sound
    DESTINATION ${ORBITER_INSTALL_ROOT_DIR}/
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Orbiter/Textures
    DESTINATION ${ORBITER_INSTALL_ROOT_DIR}/
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Orbiter/Images
    DESTINATION ${ORBITER_INSTALL_ROOT_DIR}/
)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Sources/G422/Resources/VC_EICAS_SRC.bmp ${CMAKE_CURRENT_SOURCE_DIR}/Sources/G422/Resources/reheat_tape.bmp
    DESTINATION ${ORBITER_INSTALL_ROOT_DIR}/Bitmaps/G422
)
