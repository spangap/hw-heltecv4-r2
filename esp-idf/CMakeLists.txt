# hw-heltecv4-r2 board HAL — an ordinary spangap component.
#
# Compiles the Heltec WiFi LoRa 32 V4 (R2) board code: the Vext peripheral power
# rail and the LoRa CS park (heltecv4r2.cpp), and its detect_hw().
#
# app_main is NOT here — it's generated into the reticulous/reticulous buildable
# that adds this board with `--with`. This component just exports the board's
# bring-up service (Heltecv4R2Board), which the generated app_main walks.
include(${CMAKE_CURRENT_LIST_DIR}/spangap_requires.cmake)
idf_component_register(
    SRCS
        "src/heltecv4r2.cpp"
        # This board's one self-assertion (detect_hw), called by
        # spangapInit() before any bus is claimed.
        "src/detect.cpp"
        ${SPANGAP_CONDITIONAL_SRCS}
    INCLUDE_DIRS "include" ${SPANGAP_CONDITIONAL_INCLUDE_DIRS}
    REQUIRES
        ${SPANGAP_REQUIRES}
        driver
        esp_event
        esp_netif
        esp_wifi
        lwip
)
