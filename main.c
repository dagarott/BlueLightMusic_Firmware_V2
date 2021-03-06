/**
 * Copyright (c) 2014 - 2017, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup ble_sdk_uart_over_ble_main main.c
 * @{
 * @ingroup  ble_sdk_app_nus_eval
 * @brief    UART over BLE application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service.
 * This application uses the @ref srvlib_conn_params module.
 */

#include "app_timer.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_hci.h"
#include "ble_nus.h"
#include "bsp_btn_ble.h"
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_ble_gatt.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include <stdint.h>
#include <string.h>

#if defined(UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined(UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

// debug
#include "project.h"
// debug

#define APP_BLE_CONN_CFG_TAG 1 /**< A tag identifying the SoftDevice BLE configuration. */

#define APP_FEATURE_NOT_SUPPORTED \
    BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2 /**< Reply when unsupported features are requested. */

#define DEVICE_NAME "Nordic_UART" /**< Name of device. Will be included in the advertising data. */
#define NUS_SERVICE_UUID_TYPE \
    BLE_UUID_TYPE_VENDOR_BEGIN /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_BLE_OBSERVER_PRIO 3 /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define APP_ADV_INTERVAL 64            /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS 180 /**< The advertising timeout (in units of seconds). */

#define MIN_CONN_INTERVAL \
    MSEC_TO_UNITS(20,     \
                  UNIT_1_25_MS) /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL \
    MSEC_TO_UNITS(75,     \
                  UNIT_1_25_MS) /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY 0         /**< Slave latency. */
#define CONN_SUP_TIMEOUT \
    MSEC_TO_UNITS(       \
        4000, UNIT_10_MS) /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY                                                                                                               \
    APP_TIMER_TICKS(5000) /**< Time from initiating event (connect or start of notification) to first time \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \
                             sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY \
    APP_TIMER_TICKS(                  \
        30000)                         /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT 3 /**< Number of attempts before giving up the connection parameter negotiation. \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \
                                        */

#define DEAD_BEEF \
    0xDEADBEEF /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define UART_TX_BUF_SIZE 256 /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256 /**< UART RX buffer size. */

BLE_NUS_DEF(m_nus);                 /**< BLE NUS service instance. */
NRF_BLE_GATT_DEF(m_gatt);           /**< GATT module instance. */
BLE_ADVERTISING_DEF(m_advertising); /**< Advertising module instance. */

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID; /**< Handle of the current connection. */
static uint16_t m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT -
                                         3; /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
static ble_uuid_t m_adv_uuids[] =           /**< Universally unique service identifier. */
    {{BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}};

/* digital Leds parameters */
uint8_t ws2812bpattern = 0;
bool flag_off_leds = false;
/* Songs paramaters */
uint8_t songpattern = FADE;
uint16_t delaysong = 0;
bool flag_song_enable = false;
bool flag_song_running = false;
song_param_t record[NUM_SONGS];
/* Haptic motor paramters */
#define HATPIC_ON 'B'
bool flag_haptic_running = false;
bool flag_haptic_enable = false;
nrf_pwm_values_common_t m_duty_value = 500;
static nrf_drv_pwm_t m_pwm2 = NRF_DRV_PWM_INSTANCE(1);
void haptic_motor_start(void);
void haptic_motor_stop(void);
/* Shutdown system parameters*/
#define PIN_IN_POWER_DOWN 30
#define PIN_OUT_POWER 28
#define POWER_OFF 'C'
#define START 'D'
/* bool flag_shutdown_enable = false;
bool flag_poweron_enable = false;
bool flag_system_running = false;
uint16_t pressedButtonCount = 0; */
/*  BT data reception parameters*/
#define START_FRAME '['
#define END_FRAME ']'

/**
 * @brief      { function_description }
 *
 * @param[in]  OnOff  On off
 */
static void PowerSystem(uint8_t OnOff)
{

    if (OnOff == true)
    {
        nrf_gpio_pin_write(28, 1);
    }
    else if (OnOff == false)
    {

        nrf_gpio_pin_write(28, 0);
    }
}
void gpio_init(void)
{
    // uint32_t err_code;

    nrf_gpio_cfg_output(PIN_OUT_POWER);
    nrf_gpio_cfg_sense_input(PIN_IN_POWER_DOWN, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_HIGH);

    // nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
    // in_config.pull = NRF_GPIO_PIN_PULLUP;
    // err_code = nrf_drv_gpiote_in_init(PIN_IN_POWER_DOWN, &in_config, in_pin_handler);
    // APP_ERROR_CHECK(err_code);
    // if (err_code != NRF_SUCCESS)
    // {
    //     // Initialization failed. Take recovery action.
    // }
    // nrf_drv_gpiote_in_event_enable(PIN_IN_POWER_DOWN, true);
}

/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    uint32_t err_code;
    ble_gap_conn_params_t gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)DEVICE_NAME, strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_nus    Nordic UART Service structure.
 * @param[in] p_data   Data to be send to UART module.
 * @param[in] length   Length of the data.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_evt_t *p_evt)
{

    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
        //uint32_t err_code;
        flag_off_leds = false;
        //NRF_LOG_DEBUG("Received data from BLE NUS. Writing data on UART.");
        //NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);

        //for (uint32_t i = 0; i < p_evt->params.rx_data.length; i++) {
        //do {
        //err_code = app_uart_put(p_evt->params.rx_data.p_data[i]);
        //if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY)) {
        //    NRF_LOG_ERROR("Failed receiving NUS message. Error 0x%x. ", err_code);
        //    APP_ERROR_CHECK(err_code);
        //}
        //} while (err_code == NRF_ERROR_BUSY);
        //}
        //if (p_evt->params.rx_data.p_data[p_evt->params.rx_data.length - 1] == '\r') {
        //    while (app_uart_put('\n') == NRF_ERROR_BUSY)
        //       ;
        //}
        if ((p_evt->params.rx_data.p_data[0] == START_FRAME) & (p_evt->params.rx_data.p_data[2] == END_FRAME) & (p_evt->params.rx_data.length == 3))
        {
            switch (p_evt->params.rx_data.p_data[1])
            {
            case FADE:
                ws2812bpattern = FADE;
                flag_off_leds = false;
                break;
            case CYCLON:
                ws2812bpattern = CYCLON;
                flag_off_leds = false;
                break;
            case FLASHLED:
                ws2812bpattern = FLASHLED;
                flag_off_leds = false;
                break;
            case FLASHFADE:
                ws2812bpattern = FLASHFADE;
                flag_off_leds = false;
                break;
            case WIPE:
                ws2812bpattern = WIPE;
                flag_off_leds = false;
                break;
            case RING:
                ws2812bpattern = RING;
                flag_off_leds = false;
                break;
            case OFFLEDS:
                flag_off_leds = true;
                break;
            //case ONLEDS:
            //    flag_off_leds = false;
            //    break;
            case SONG1:

                if (!flag_song_running)
                {
                    songpattern = 0;         //first song
                    flag_song_enable = true; // enable songs
                }
                break;
            case SONG2:

                if (!flag_song_running)
                {
                    songpattern = 1;         //second song
                    flag_song_enable = true; // enable songs
                }
                break;
            case SONG3:

                if (!flag_song_running)
                {
                    songpattern = 2;         //third song
                    flag_song_enable = true; // enable songs
                }
                break;

            case HATPIC_ON:
                if (flag_haptic_running == false)
                    flag_haptic_enable = true;
                break;
            case POWER_OFF:
                //PowerSystem(false);
                songpattern = POWER_OFF; //shutdown song
                drv_speaker_stop();
                flag_song_enable = true; // enable songs
                break;
            default:
                break;
            }
        }
    }
}
/**@snippet [Handling the data received over BLE] */

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t err_code;
    ble_nus_init_t nus_init;

    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t *p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail = false;
    cp_init.evt_handler = on_conn_params_evt;
    cp_init.error_handler = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
    case BLE_ADV_EVT_FAST:
        err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
        APP_ERROR_CHECK(err_code);
        break;
    case BLE_ADV_EVT_IDLE:
        sleep_mode_enter();
        break;
    default:
        break;
    }
}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_contclang: cache cleared
ext   Unused.
 */
static void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        NRF_LOG_INFO("Connected");
        err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
        APP_ERROR_CHECK(err_code);
        m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        NRF_LOG_INFO("Disconnected");
        // LED indication will be changed when advertising starts.
        m_conn_handle = BLE_CONN_HANDLE_INVALID;
        break;

#ifndef S140
    case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
    {
        NRF_LOG_DEBUG("PHY update request.");
        ble_gap_phys_t const phys = {
            .rx_phys = BLE_GAP_PHY_AUTO,
            .tx_phys = BLE_GAP_PHY_AUTO,
        };
        err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
        APP_ERROR_CHECK(err_code);
    }
    break;
#endif

    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        // Pairing not supported
        err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
        APP_ERROR_CHECK(err_code);
        break;
#if !defined(S112)
//         case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:
//        {
//            ble_gap_data_length_params_t dl_params;
//
//            // Clearing the struct will effectivly set members to @ref BLE_GAP_DATA_LENGTH_AUTO
//            memset(&dl_params, 0, sizeof(ble_gap_data_length_params_t));
//            err_code = sd_ble_gap_data_length_update(p_ble_evt->evt.gap_evt.conn_handle, &dl_params, NULL);
//            APP_ERROR_CHECK(err_code);
//        } break;
#endif //! defined (S112)
    case BLE_GATTS_EVT_SYS_ATTR_MISSING:
        // No system attributes have been stored.
        err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GATTC_EVT_TIMEOUT:
        // Disconnect on GATT Client timeout event.
        err_code =
            sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GATTS_EVT_TIMEOUT:
        // Disconnect on GATT Server timeout event.
        err_code =
            sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_EVT_USER_MEM_REQUEST:
        err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
    {
        ble_gatts_evt_rw_authorize_request_t req;
        ble_gatts_rw_authorize_reply_params_t auth_reply;

        req = p_ble_evt->evt.gatts_evt.params.authorize_request;

        if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
        {
            if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ) ||
                (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
            {
                if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                {
                    auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                }
                else
                {
                    auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                }
                auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle, &auth_reply);
                APP_ERROR_CHECK(err_code);
            }
        }
    }
    break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST

    default:
        // No implementation needed.
        break;
    }
}

/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t *p_gatt, nrf_ble_gatt_evt_t const *p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Data len is set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
    NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x", p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}

/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    // err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, 64);
    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, BLE_GATT_ATT_MTU_DEFAULT);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
    case BSP_EVENT_SLEEP:
        sleep_mode_enter();
        break;

    case BSP_EVENT_DISCONNECT:
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        if (err_code != NRF_ERROR_INVALID_STATE)
        {
            APP_ERROR_CHECK(err_code);
        }
        break;

    case BSP_EVENT_WHITELIST_OFF:
        if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
        {
            err_code = ble_advertising_restart_without_whitelist(&m_advertising);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
        }
        break;

    default:
        break;
    }
}

/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          a string. The string will be be sent over BLE when the last character received was a
 *          'new line' '\n' (hex 0x0A) or if the string has reached the maximum data length.
 */
/**@snippet [Handling the data received over UART] */
void uart_event_handle(app_uart_evt_t *p_event)
{
    static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
    static uint8_t index = 0;
    uint32_t err_code;

    switch (p_event->evt_type)
    {
    case APP_UART_DATA_READY:
        UNUSED_VARIABLE(app_uart_get(&data_array[index]));
        index++;

        if ((data_array[index - 1] == '\n') || (index >= (m_ble_nus_max_data_len)))
        {
            NRF_LOG_DEBUG("Ready to send data over BLE NUS");
            NRF_LOG_HEXDUMP_DEBUG(data_array, index);

            do
            {
                uint16_t length = (uint16_t)index;
                err_code = ble_nus_string_send(&m_nus, data_array, &length);
                if ((err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_BUSY))
                {
                    APP_ERROR_CHECK(err_code);
                }
            } while (err_code == NRF_ERROR_BUSY);

            index = 0;
        }
        break;

    case APP_UART_COMMUNICATION_ERROR:
        APP_ERROR_HANDLER(p_event->data.error_communication);
        break;

    case APP_UART_FIFO_ERROR:
        APP_ERROR_HANDLER(p_event->data.error_code);
        break;

    default:
        break;
    }
}
/**@snippet [Handling the data received over UART] */

/**@brief  Function for initializing the UART module.
 */
/**@snippet [UART Initialization] */
// static void uart_init(void)
//{
//    uint32_t                     err_code;
//    app_uart_comm_params_t const comm_params =
//    {
//        .rx_pin_no    = RX_PIN_NUMBER,
//        .tx_pin_no    = TX_PIN_NUMBER,
//        .rts_pin_no   = RTS_PIN_NUMBER,
//        .cts_pin_no   = CTS_PIN_NUMBER,
//        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
//        .use_parity   = false,
//        .baud_rate    = NRF_UART_BAUDRATE_115200
//    };
//
//    APP_UART_FIFO_INIT(&comm_params,
//                       UART_RX_BUF_SIZE,
//                       UART_TX_BUF_SIZE,
//                       uart_event_handle,
//                       APP_IRQ_PRIORITY_LOWEST,
//                       err_code);
//    APP_ERROR_CHECK(err_code);
//}
/**@snippet [UART Initialization] */

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids = m_adv_uuids;

    init.config.ble_adv_fast_enabled = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout = APP_ADV_TIMEOUT_IN_SECONDS;

    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
// static void buttons_leds_init(bool * p_erase_bonds)
//{
//    bsp_event_t startup_event;
//
//    uint32_t err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS, bsp_event_handler);
//    APP_ERROR_CHECK(err_code);
//
//    err_code = bsp_btn_ble_init(NULL, &startup_event);
//    APP_ERROR_CHECK(err_code);
//
//    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
//}

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/**@brief Function for placing the application in low power state while waiting for events.
 */
/*static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}*/

/*
**@brief function for handling the systick timer timeout.
 *
 * @details this function will be called each time the systick timer expires.
 *          this function will start update_leds(), update_buzzer(), update_motor(),
 *                  get_pin_status() and update_power_state() functions.
 *
 * @param[in] p_context   pointer used for passing some arbitrary information (context) from the
 *                        app_start_timer() call to the timeout handler.
 */
void SysTick_Handler(void)
{
    static uint16_t delayws2812b = 0;
    static uint16_t delayhaptic = 0;

    //static bool flag_shutdown_enable = false;
    //static bool flag_poweron_enable = false;
    static bool flag_system_running = false;
    static uint16_t pressedButtonCount = 0;

    if (!flag_off_leds)
    {
        if (delayws2812b == 0)
        {
            switch (ws2812bpattern)
            {

            case FADE:
            {
                delayws2812b = FadeInOut();
                break;
            }
            case CYCLON:
            {
                delayws2812b = Cyclon();
                break;
            }
            case FLASHLED:
            {
                delayws2812b = Flash();
                break;
            }
            case FLASHFADE:
            {
                delayws2812b = FlashFadeInOut();
                break;
            }
            case WIPE:
            {
                delayws2812b = Wipe();
                break;
            }
            case RING:
            {
                delayws2812b = Ring();
                break;
            }
            default:
                break;
            }
        }

        else
        {
            delayws2812b--;
        }
    }
    if (flag_off_leds)
    {
        OffLeds();
    }

    /*songs part*/
    if (flag_song_enable == true)
    {
        if (delaysong == 0)
        {

            if (songpattern == 0)
            {
                //delaysong = PlayMusic(&record[0]);
                delaysong = PlayMusic(song1, num_notes_song1, length_note_song1, tempo_song1);
                flag_song_running = true;
            }
            else if (songpattern == 1)
            {
                //delaysong = PlayMusic(&record[1]);
                delaysong = PlayMusic(song2, num_notes_song2, length_note_song2, tempo_song2);
                flag_song_running = true;
            }
            else if (songpattern == 2)
            {
                //delaysong = PlayMusic(&record[2]);
                delaysong = PlayMusic(song3, num_notes_song3, length_note_song3, tempo_song3);
                flag_song_running = true;
            }
            else if (songpattern == POWER_OFF)
            {
                delaysong = PlayMusic(exitsong, num_notes_exitsong, length_note_exitsong, tempo_exitsong);
                flag_song_running = true;
            }
            else if (songpattern == START)
            {
                delaysong = PlayMusic(startsong, num_notes_startsong, length_note_startsong, tempo_startsong);
                flag_song_running = true;
            }

            if (delaysong == END_SONG)
            {
                flag_song_enable = false;
                flag_song_running = false;
                delaysong = 0;
                if (songpattern == POWER_OFF)
                {
                    //flag_shutdown_enable = true;
                    PowerSystem(false);
                }

                else if (songpattern == START)
                {
                    //flag_poweron_enable = true;
                    PowerSystem(true);
                    flag_system_running = true;
                    gpio_init(); //start to sense power button input
                }

                songpattern = NUM_SONGS + 1;
            }
        }
        else
        {
            delaysong--;
        }
        // drv_speaker_sample_play(0);
    }
    /* 
    if (flag_shutdown_enable)
    {
        flag_shutdown_enable = false;
        flag_system_running = false;
        PowerSystem(false);
    } */

    //if (((nrf_gpio_pin_read(PIN_IN_POWER_DOWN)) == false) & (flag_shutdown_enable == false) & (flag_system_running == true))
    if (((nrf_gpio_pin_read(PIN_IN_POWER_DOWN)) == false) & (flag_system_running == true))
    {
        pressedButtonCount++;
        if (pressedButtonCount == 2000)
        {
            pressedButtonCount = 0;
            flag_system_running = false;
            songpattern = POWER_OFF;
            delaysong = 0;
            flag_song_enable = true; // enable songs
            drv_speaker_stop();
        }
    }

    /* if ((flag_poweron_enable == true) & (flag_system_running == false))
    {

        PowerSystem(true);
        flag_system_running = true;
    } */

    if (flag_haptic_enable == true)
    {
        if (delayhaptic == 0)
        {
            haptic_motor_start();
            flag_haptic_running = true;
            delayhaptic++;
        }
        else if (delayhaptic < 1000)
        {
            delayhaptic++;
        } //1 sec

        else if (delayhaptic == 1000)
        {
            delayhaptic = 0;
            haptic_motor_stop();
            flag_haptic_running = false;
            flag_haptic_enable = false;
        }
    }
}
static void drv_speaker_evt_handler(drv_speaker_evt_t evt)
{
    switch (evt)
    {
    case DRV_SPEAKER_EVT_FINISHED:
    {
        // debug_printf(0, "drv_speaker_evt_handler: drv_speaker_evt_finished\r\n");
        //(void)ble_tss_spkr_stat_set(&m_tss, ble_tss_spkr_stat_finished);
    }
    break;

    case DRV_SPEAKER_EVT_BUFFER_WARNING:
    {
        // debug_printf(0, "drv_speaker_evt_handler: drv_speaker_evt_buffer_warning\r\n");
        //(void)ble_tss_spkr_stat_set(&m_tss, ble_tss_spkr_stat_buffer_warning);
    }
    break;
    //
    case DRV_SPEAKER_EVT_BUFFER_READY:
    {
        // debug_printf(0, "drv_speaker_evt_handler: drv_speaker_evt_buffer_ready\r\n");
        //(void)ble_tss_spkr_stat_set(&m_tss, ble_tss_spkr_stat_buffer_ready);
    }
    break;
    //
    default:
        APP_ERROR_CHECK_BOOL(false);
        break;
    }
}
void pwm_ready_callback(uint32_t pwm_id) // PWM callback function
{
    //static volatile bool ready_flag;            // A flag indicating PWM status.
    //ready_flag = true;
}

nrf_pwm_sequence_t const m_haptic_seq = {
    .values.p_common = &m_duty_value,
    .length = NRF_PWM_VALUES_LENGTH(m_duty_value),
    .repeats = 1,
    .end_delay = 0,
};

void haptic_motor_start(void)
{
    uint32_t err_code;

    nrf_drv_pwm_config_t const config0 = {
        .output_pins =

            {
                NRF_DRV_PWM_PIN_NOT_USED,     // channel 0
                0 | NRF_DRV_PWM_PIN_INVERTED, // channel 1
                NRF_DRV_PWM_PIN_NOT_USED,     // channel 2
                NRF_DRV_PWM_PIN_NOT_USED,     // channel 3
            },
        .irq_priority = APP_IRQ_PRIORITY_LOW,
        .base_clock = NRF_PWM_CLK_16MHz,
        .count_mode = NRF_PWM_MODE_UP,
        .top_value = 1000, //1kHz
        .load_mode = NRF_PWM_LOAD_COMMON,
        .step_mode = NRF_PWM_STEP_AUTO};
    err_code = nrf_drv_pwm_init(&m_pwm2, &config0, NULL);
    if (err_code != NRF_SUCCESS)
    {
        // Initialization failed. Take recovery action.
    }
    nrf_drv_pwm_simple_playback(&m_pwm2, &m_haptic_seq, 1, 0);
}
void haptic_motor_stop(void)
{

    nrf_drv_pwm_stop(&m_pwm2, false);
    nrf_gpio_pin_write(0, 0);
}

/**
 * @brief      { main funciton, set GPIO, init BT stack, SystTick, for ever loop }
 *
 * @return     { none }
 * @par
 */
int main(void)
{
    uint32_t err_code;
    drv_speaker_init_t speaker_init;
    speaker_init.evt_handler = drv_speaker_evt_handler;
    // bool     erase_bonds;

    // Initialize.
    //gpio_init();
    //PowerSystem(true);
    songpattern = START;
    flag_song_enable = true; // enable songs
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
    log_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
    NRF_LOG_INFO("UART Start!");
    err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
    drv_speaker_init(&speaker_init);
    SysTick_Config(SystemCoreClock / 1000); //1ms

    // Enter main loop.
    for (;;)
    {
        UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
        //power_manage();
    }
}

/**
 * @}
 */
